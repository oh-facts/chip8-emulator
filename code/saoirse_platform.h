/* date = July 18th 2024 5:41 am */

#ifndef SAOIRSE_PLATFORM_H
#define SAOIRSE_PLATFORM_H

#define S_VERSION_MAJOR (0)
#define S_VERSION_MINOR (1)
#define S_VERSION_PATCH (0)

#include "stdio.h"
#include "string.h"
#include "time.h"
#include "base/base_core.h"
#include "base/base_math.h"
#include "base/base_input.h"
#include "base/base_string.h"
#include "base/base_tcxt.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

#include "base/base_input.cpp"
#include "base/base_string.cpp"
#include "base/base_math.cpp"
#include "base/base_tcxt.cpp"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

struct Rect
{
	v2f tl;
	v2f br;
};

struct Bitmap
{
	void *data;
	i32 w;
	i32 h;
	i32 n;
};

struct Glyph
{
	u8 *bmp;
	i32 w;
	i32 h;
	v2i bearing;
	i32 x0, y0, x1, y1;
	i32 advance;
};

struct Atlas
{
	Glyph glyphs[256];
};

internal Glyph *glyph_from_codepoint(Atlas *atlas, char c)
{
	Glyph *out = atlas->glyphs + (u32)c;
	return out;
}

enum FILE_TYPE
{
  FILE_TYPE_TEXT,
  FILE_TYPE_BINARY,
  FILE_TYPE_COUNT
};

struct File_data
{
	u8 *bytes;
	u64 size;
};

// ty pine
#if defined(OS_WIN32)
#define _file_open(file, filepath, mode) fopen_s(file, filepath, mode)
#elif defined (OS_LINUX) || defined (OS_APPLE)
#define _file_open(file, filepath, mode) *file = fopen(filepath, mode)
#endif

internal File_data read_file(Arena *arena, const char *filepath, FILE_TYPE type)
{
	File_data out = {};
  FILE *file;
  
  local_persist char *file_type_table[FILE_TYPE_COUNT] = 
  {
    "r",
    "rb"
  };
#if 0
	if (access(filepath, F_OK) != 0)
	{
    file = fopen(filepath, "wb+");
    
    fclose(file);
  }
#endif
  _file_open(&file, filepath, file_type_table[type]);
  
  fseek(file, 0, SEEK_END);
  
  out.size = ftell(file);
  //print("%d", len);
  
  fseek(file, 0, SEEK_SET);
  
  out.bytes = push_array(arena, u8, out.size);
  fread(out.bytes, sizeof(u8), out.size, file);
  
  fclose(file);
  
  return out;
}

internal void write_file(const char *filepath, FILE_TYPE type, void *data, size_t size)
{
	FILE *file;
	
	local_persist char *file_type_table[FILE_TYPE_COUNT] = 
  {
    "w",
    "wb"
  };
	
	_file_open(&file, filepath, file_type_table[type]);
	
	fwrite(data, size, 1, file);
	
	fclose(file);
	
}

typedef void *(*os_reserve_fn)(u64 size);
typedef b32 (*os_commit_fn)(void *ptr, u64 size);
typedef void (*os_decommit_fn)(void *ptr, u64 size);
typedef void (*os_release_fn)(void *ptr, u64 size);
typedef u64 (*os_get_page_size_fn)();
typedef Str8 (*os_get_app_dir_fn)(Arena *arena);
typedef Str8 (*os_open_file_dialog_fn)(Arena *arena);

global Input *g_input;
global os_reserve_fn os_reserve;
global os_commit_fn os_commit;
global os_decommit_fn os_decommit;
global os_release_fn os_release;
global os_get_page_size_fn os_get_page_size;
global os_get_app_dir_fn os_get_app_dir;
global os_open_file_dialog_fn os_open_file_dialog;

#include "render.h"

global r_alloc_texture_fn r_alloc_texture;
global r_submit_fn r_submit;

struct S_Platform_api
{
	os_reserve_fn os_reserve;
	os_commit_fn os_commit;
	os_decommit_fn os_decommit;
	os_release_fn os_release;
	os_get_page_size_fn os_get_page_size;
	os_get_app_dir_fn os_get_app_dir;
	os_open_file_dialog_fn os_open_file_dialog;
};

struct S_Render_api
{
	r_alloc_texture_fn r_alloc_texture;
	r_submit_fn r_submit;
};

internal void s_global_platform_api_init(S_Platform_api *api)
{
	os_reserve = api->os_reserve;
	os_commit = api->os_commit;
	os_decommit = api->os_decommit;
	os_release = api->os_release;
	os_get_page_size = api->os_get_page_size;
	os_get_app_dir =  api->os_get_app_dir;
	os_open_file_dialog = api->os_open_file_dialog;
}

internal void s_global_render_api_init(S_Render_api *api)
{
	r_alloc_texture = api->r_alloc_texture;
	r_submit = api->r_submit;
}

#include "base/base_core.cpp"
#include "meta.h"
#include "draw.h"
#include "ui.h"
#include "draw_ui.h"
#include "chip8.h"
#include "chip8.cpp"
#include "saoirse.h"

#include <stdlib.h>

struct S_Platform
{
	int argc;
	char **argv;
	Str8 app_dir;
	S_Platform_api p_api;
	S_Render_api r_api;
	b32 initialized;
	b32 reloaded;
	u64 res;
	u64 cmt;
	void *memory;
	v2i win_size;
};

// ty yeti
#if 0
internal Str8 os_linux_get_app_dir(Arena *arena)
{
	char buffer[256];
  ssize_t len = readlink("/proc/self/exe", buffer, 256);
	
	char *c = &buffer[len];
  while(*(--c) != '/')
  {
    *c = 0;
    --len;
  }
	
  u8 *str = push_array(arena, u8, len);
	mem_cpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}
#endif

internal Str8 file_name_from_path(Arena *arena, Str8 path)
{
	char *cur = (char*)&path.c[path.len - 1];
	u32 count = 0;
	
	//NOTE(mizu): pig
	while(*cur != '/' && *cur != '\\' && *cur != '\0')
	{
		cur--;
		count++;
	}
	
	Str8 file_name_cstr = {};
	file_name_cstr.c = push_array(arena, u8, count + 1);
	file_name_cstr.len = count + 1;
	mem_cpy(file_name_cstr.c, cur + 1, count);
	file_name_cstr.c[count] = '\0';
	
	return file_name_cstr;
}

typedef void (*update_and_render_fn)(S_Platform *, Input *, f32 delta);

internal Bitmap bitmap(Str8 path)
{
	Bitmap out = {};
	
	stbi_set_flip_vertically_on_load(true);
	
	out.data = stbi_load((char*)path.c, &out.w, &out.h, &out.n, 0);
	
	if(!out.data)
	{
		printf("\nError loading%s :%s\n", path.c, stbi_failure_reason());
		INVALID_CODE_PATH();
	}
	
	return out;
}

internal Glyph *make_bmp_font(u8* path, char *codepoints, u32 num_cp, Arena* arena)
{
	u8 *file_data = read_file(arena, (char*)path, FILE_TYPE_BINARY).bytes;
	
	stbtt_fontinfo font;
  stbtt_InitFont(&font, (u8*)file_data, stbtt_GetFontOffsetForIndex((u8*)file_data,0));
  
	Glyph *out = push_array(arena, Glyph, num_cp);
	
	for(u32 i = 0; i < num_cp; i++)
	{
		i32 w,h,xoff,yoff;
		f32 size = stbtt_ScaleForPixelHeight(&font, 64);
		
		u8* bmp = stbtt_GetCodepointBitmap(&font, 0, size, codepoints[i] ,&w,&h, &xoff, &yoff);
		
		stbtt_GetCodepointHMetrics(&font, codepoints[i], &out[i].advance, &out[i].bearing.x);
		out[i].w = w;
		out[i].h = h;
		
		i32 x0, y0, x1, y1;
		stbtt_GetCodepointBox(&font, codepoints[i], &x0, &y0, &x1, &y1);
		
		out[i].bearing.y = y0;
		
		out[i].x0 = x0;
		out[i].y0 = y0;
		out[i].x1 = x1;
		out[i].y1 = y1;
		
		out[i].bmp = push_array(arena,u8,w * h * 4);
		
		u8* src_row = bmp + w*(h-1);
		u8* dest_row = out[i].bmp;
		
		for(u32 y = 0; y < h; y ++)
		{
			u32 *dest = (u32*)dest_row;
			u8 *src = src_row;
			for(u32 x = 0; x < w; x ++)
			{
				u8 alpha = *src++;
				
				*dest++ = ((alpha <<24) |
									 (alpha <<16) |
									 (alpha << 8) |
									 (alpha ));
			}
			dest_row += 4 * w;
			src_row -= w;
		}
		
		stbtt_FreeBitmap(bmp, 0);
		
	}
	
	
  return out;
}

#endif //SAOIRSE_PLATFORM_H
