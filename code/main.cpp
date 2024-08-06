#include "SDL3/SDL.h"

// core include
#include "saoirse_platform.h"

// backends
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "GL/gl.h"
#include "opengl/opengl_khr_platform.h"
#include "opengl/opengl_win32_platform.h"
#include "render_opengl.h"
#include "render_opengl.cpp"
#include "os/os_win32.cpp"

struct S_Window
{
	SDL_Window *win;
	i32 w;
	i32 h;
	b32 closed;
};

Arena *arena;
Input input;
S_Window *win;


void SDLCALL dialog_file_cb(void *userdata, const char * const *filelist, int filter) 
{
	File_dialog_data *data = (File_dialog_data*)userdata;

	if(filelist)
	{
		if(filelist[0])
		{
			data->path.c = (u8*)malloc(100);//push_struct(dcb->arena, u8);
			data->path.len = strlen(filelist[0]);
			memcpy(data->path.c, filelist[0], data->path.len);
		}
	}
	data->completed = 1;
}

void open_file_dialog(File_dialog_data *data)
{
	SDL_ShowOpenFileDialog(dialog_file_cb, data, win->win, 0, 0, 0, SDL_FALSE);
}

S_Window *create_opengl_window(Arena *arena, const char *title, i32 w, i32 h)
{
	S_Window *out = push_struct(arena, S_Window);
	out->w = w;
	out->h = h;
	out->closed = 0;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	out->win = SDL_CreateWindow("SDL OpenGL", w, h, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(out->win);
	SDL_GL_SetSwapInterval(1);
	return out;
}

int main(int argc, char **argv)
{
	S_Platform pf = {};
	pf.argc = argc;
	pf.argv = argv;
	
	pf.p_api.os_reserve = os_win32_reserve;
	pf.p_api.os_commit = os_win32_commit;
	pf.p_api.os_decommit = os_win32_decommit;
	pf.p_api.os_release = os_win32_release;
	pf.p_api.os_get_page_size = os_win32_get_page_size;
	pf.p_api.os_get_app_dir = os_win32_get_app_dir;
	pf.p_api.os_open_file_dialog = open_file_dialog;
	pf.r_api.r_alloc_texture = r_opengl_alloc_texture;
	pf.r_api.r_submit = r_opengl_submit;
	
	s_global_platform_api_init(&pf.p_api);
	s_global_render_api_init(&pf.r_api);
	
	arena = arena_create();
	pf.memory = push_struct(arena, State);
	Str8 app_dir = os_get_app_dir(arena);
	
	pf.app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit("yk.dll");
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
	
	win = create_opengl_window(arena,"dear mizu hot chip 8",960,540);
	opengl_load_functions();
	r_opengl_init();
	u64 start = SDL_GetPerformanceCounter();
	u64 freq = SDL_GetPerformanceFrequency();
	
	f64 time_elapsed = 0;
	f64 time_since_last = 0;
	f64 delta = 0;
	
	glEnable(GL_FRAMEBUFFER_SRGB);

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
					if(event.key.key < KEY_COUNT)
					{
						input.keys[event.key.key] = pressed;
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
		//printf("%d %d\n", input.mpos.x, input.mpos.y);
		f32 color[3] = {1,0,1};

		glClearBufferfv(GL_COLOR, 0, color);
		pf.win_size.x = win->w;
		pf.win_size.y = win->h;
		pf.res = total_res;
		pf.cmt = total_cmt;
		update_and_render(&pf, &input, delta);
		input_update(&input);
		SDL_GL_SwapWindow(win->win);
		
		u64 end = SDL_GetPerformanceCounter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
	}
	
	return 0;
}