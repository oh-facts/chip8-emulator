#include "SDL3/SDL.h"
#include "base/base_context_cracking.h"

// OS backends
#if defined(OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#else
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/mman.h>
#endif

// core include
#include "saoirse_platform.h"

#if defined(OS_WIN32)
#include "os/os_win32.cpp"
#define USE_OPENGL 1
#else
#include "os/os_linux.cpp"
#define USE_OPENGL 0
#endif

#if USE_OPENGL
#include "GL/gl.h"
#include "opengl/opengl_khr_platform.h"
#include "opengl/opengl_win32_platform.h"
#include "render_opengl.h"
#include "render_opengl.cpp"
#else
#include "render_sdl.h"
#include "render_sdl.cpp"
#endif

#if defined(OS_WIN32)
#define GAME_DLL "yk.dll"
#elif defined(OS_LINUX)
#define GAME_DLL "libyk.so"
#elif defined(OS_APPLE)
#define GAME_DLL "libyk2.dylib"
#endif

struct S_Window
{
	SDL_Window *win;
	i32 w;
	i32 h;
	b32 closed;
	b32 fullscreen;
};

Arena *arena;
Input input;
S_Window *win;

function void SDLCALL dialog_file_cb(void *userdata, const char * const *filelist, int filter) 
{
	File_dialog_data *data = (File_dialog_data*)userdata;

	if(filelist)
	{
		if(filelist[0])
		{
			if(!data->path.c)
			{
				// TODO(mizu) : fs specific max length
				data->path.c = (u8*)calloc(1, 256);
			}
			memset(data->path.c, 0, data->path.len);
			data->path.len = strlen(filelist[0]);
			memcpy(data->path.c, filelist[0], data->path.len);
		}
	}
	data->completed = 1;

	printf("%s\n", SDL_GetError());
}

function void open_file_dialog(File_dialog_data *data)
{
	SDL_ShowOpenFileDialog(dialog_file_cb, data, win->win, 0, 0, 0, SDL_FALSE);
}

function S_Window *create_window(Arena *arena, const char *title, i32 w, i32 h, b32 init_opengl)
{
	S_Window *out = push_struct(arena, S_Window);
	out->w = w;
	out->h = h;
	out->closed = 0;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("%s\n\r", SDL_GetError());
		printf("[Quitting Fatal]\n\r");
		INVALID_CODE_PATH();
	}

	if(init_opengl)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		out->win = SDL_CreateWindow("mizu mizu hot chippy 8 [OpenGL 4.5]", w, h, SDL_WINDOW_OPENGL);

		if(!SDL_GL_CreateContext(out->win))
		{
			printf("%s\n\r", SDL_GetError());
			printf("[Quitting Fatal]\n\r");
			INVALID_CODE_PATH();
		}

		SDL_GL_SetSwapInterval(1);
	}
	else
	{
		out->win = SDL_CreateWindow("mizu mizu hot chippy 8 [SDL]", w, h, 0);
	}

	return out;
}

int main(int argc, char **argv)
{
	S_Platform pf = {};
	pf.argc = argc;
	pf.argv = argv;
	#if defined(OS_WIN32)
	pf.p_api.os_reserve = os_win32_reserve;
	pf.p_api.os_commit = os_win32_commit;
	pf.p_api.os_decommit = os_win32_decommit;
	pf.p_api.os_release = os_win32_release;
	pf.p_api.os_get_page_size = os_win32_get_page_size;
	pf.p_api.os_get_app_dir = os_win32_get_app_dir;
	#else
	pf.p_api.os_reserve = os_linux_reserve;
	pf.p_api.os_commit = os_linux_commit;
	pf.p_api.os_decommit = os_linux_decommit;
	pf.p_api.os_release = os_linux_release;
	pf.p_api.os_get_page_size = os_linux_get_page_size;
	pf.p_api.os_get_app_dir = os_linux_get_app_dir;
	#endif

	#if USE_OPENGL
	pf.r_api.r_alloc_texture = r_opengl_alloc_texture;
	pf.r_api.r_submit = r_opengl_submit;
	#else
	pf.r_api.r_alloc_texture = r_sdl_alloc_texture;
	pf.r_api.r_submit = r_sdl_submit;
	#endif
	pf.p_api.os_open_file_dialog = open_file_dialog;
	s_global_platform_api_init(&pf.p_api);
	s_global_render_api_init(&pf.r_api);
	
	arena = arena_create();
	pf.memory = push_struct(arena, State);
	Str8 app_dir = os_get_app_dir(arena);
	
	pf.app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit(GAME_DLL);
	Str8 dll_path = str8_join(arena, app_dir, dll_rel_path);
	void *game_dll = SDL_LoadObject((char *)dll_path.c);
	
	if(!game_dll)
	{
		printf("dll not found\n\r");
	}
	
	update_and_render_fn update_and_render = (update_and_render_fn)SDL_LoadFunction(game_dll, "update_and_render");
	
	if(!update_and_render)
	{
		printf("fn not found\n\r");
	}
	
	#if USE_OPENGL
	win = create_window(arena, "dear mizu hot chip 8", 960, 540, 1);
	opengl_load_functions();
	r_opengl_init();
	glEnable(GL_FRAMEBUFFER_SRGB);
	#else
	win = create_window(arena, "dear mizu hot chip 8", 960, 540, 0);
	r_sdl_innit(win->win);
	#endif

	u64 start = SDL_GetPerformanceCounter();
	u64 freq = SDL_GetPerformanceFrequency();
	
	f64 time_elapsed = 0;
	f64 time_since_last = 0;
	f64 delta = 0;

	while(!win->closed)
	{
		time_since_last = time_elapsed;

		SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{
            switch (event.type) 
			{
				case SDL_EVENT_QUIT:
				{
	                win->closed = 1;
				}break;

				case SDL_EVENT_KEY_UP:
                case SDL_EVENT_KEY_DOWN:
                {
					b32 pressed = event.type == SDL_EVENT_KEY_DOWN ? 1 : 0;
			        SDL_Keycode key = event.key.key;
        			SDL_Keymod mod = SDL_GetModState();

					if(key < KEY_COUNT)
					{
						input.keys[key] = pressed;
					}

					if ((mod & SDL_KMOD_CTRL) && key == SDLK_F && pressed) 
					{
						if(win->fullscreen)
						{
							SDL_SetWindowFullscreen(win->win, SDL_FALSE);
							SDL_SetWindowSize(win->win, 960, 540);
						}
						else
						{
							SDL_SetWindowFullscreen(win->win, SDL_TRUE);
						}

						win->fullscreen = !win->fullscreen;
	    			} 
					else if ((mod & SDL_KMOD_CTRL) && key == SDLK_Q) 
					{
						win->closed = 1;
        			}

				}break;

				case SDL_EVENT_MOUSE_MOTION:
				{
					input.mpos.x = event.motion.x;
					input.mpos.y = event.motion.y;
				}break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					b32 pressed = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0;

					local_persist b32 initialized = 0;
					local_persist b32 button_table_shift[8] = {};
					
					if(!initialized)
					{
						initialized = 1;
						button_table_shift[1] = MOUSE_BUTTON_LEFT;
						button_table_shift[2] = MOUSE_BUTTON_MIDDLE;
						button_table_shift[3] = MOUSE_BUTTON_RIGHT;
					};

					input.mb[button_table_shift[event.button.button]] = pressed;
				}break;
       		}
		}

		SDL_GetWindowSize(win->win, &win->w, &win->h);
		pf.win_size.x = win->w;
		pf.win_size.y = win->h;
		pf.res = total_res;
		pf.cmt = total_cmt;
		update_and_render(&pf, &input, delta);
		input_update(&input);

		#if USE_OPENGL
		SDL_GL_SwapWindow(win->win);
		#endif

		u64 end = SDL_GetPerformanceCounter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
	}
	
	return 0;
}