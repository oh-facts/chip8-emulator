// core
#include "saoirse_platform.h"

//backends
#include <SDL2/SDL.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "render_sdl.h"
#include "render_sdl.cpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

struct Linux_Window
{
    SDL_Window *win;
	i32 w;
	i32 h;
	b32 closed;
};

Linux_Window *os_linux_create_window(Arena *arena)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("you are a failure %s", SDL_GetError());
        INVALID_CODE_PATH();
    }

    Linux_Window *out = push_struct(arena, Linux_Window);
    out->win = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!out->win) {
        printf("you are a failure %s", SDL_GetError());
        INVALID_CODE_PATH();
    }

    out->closed = 0;

    return out;
}

u64 os_linux_get_page_size()
{
    return getpagesize();
}

Str8 os_linux_open_file_dialog(Arena *arena)
{
    return {};
}

global Arena *arena;
global Linux_Window *win;
global Input input;

int main(int argc, char **argv)
{
	S_Platform pf = {};
	pf.argc = argc;
	pf.argv = argv;
	
	pf.p_api.os_reserve = os_linux_reserve;
	pf.p_api.os_commit = os_linux_commit;
	pf.p_api.os_decommit = os_linux_decommit;
	pf.p_api.os_release = os_linux_release;
	pf.p_api.os_get_page_size = os_linux_get_page_size;
	pf.p_api.os_get_app_dir = os_linux_get_app_dir;
	pf.p_api.os_open_file_dialog = os_linux_open_file_dialog;
	pf.r_api.r_alloc_texture = r_sdl_alloc_texture;
	pf.r_api.r_submit = r_sdl_submit;
	
	s_global_platform_api_init(&pf.p_api);
	s_global_render_api_init(&pf.r_api);
	
	arena = arena_create();
	pf.memory = push_struct(arena, State);
	Str8 app_dir = os_get_app_dir(arena);
	
	pf.app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit("libyk.so");
	Str8 dll_path = str8_join(arena, app_dir, dll_rel_path);

    void *game_dll = dlopen((char *)dll_path.c, RTLD_LAZY);
	
	if(!game_dll)
	{
		printf("dll not found\n\r");
        INVALID_CODE_PATH();
	}
	
	update_and_render_fn update_and_render = (update_and_render_fn)dlsym(game_dll, "update_and_render");
	
	if(!update_and_render)
	{
		printf("fn not found\n\r");
        INVALID_CODE_PATH();
	}
	
    win = os_linux_create_window(arena);	
	r_sdl_innit(win->win);

    u64 start = SDL_GetPerformanceCounter();

    u64 freq = SDL_GetPerformanceFrequency();
	
	f64 time_elapsed = 0;
	f64 time_since_last = 0;
	f64 delta = 0;

	while(!win->closed)
	{
		time_since_last = time_elapsed;

		pf.win_size.x = win->w;
		pf.win_size.y = win->h;
		pf.res = total_res;
		pf.cmt = total_cmt;
		update_and_render(&pf, &input, delta);
		input_update(&input);
		
        u64 end = SDL_GetPerformanceCounter();

		i64 counter_elapsed = (end - start) / freq;
		time_elapsed = (1.f * counter_elapsed) / freq;
		
		delta = time_elapsed - time_since_last;
	}
	
	return 0;
}
