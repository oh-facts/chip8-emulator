/* date = August 6th 2024 1:49 am */

#ifndef RENDER_SDL_H
#define RENDER_SDL_H


struct R_SDL_state
{
    SDL_Renderer *ren;
};

global R_SDL_state r_sdl_state;

internal void r_sdl_innit(SDL_Window *win);
internal R_Handle r_sdl_alloc_texture(void *data, i32 w, i32 h, i32 n, R_Texture_params *p);
internal void r_sdl_submit(R_Pass_list *list, v2i win_size);

#endif //RENDER_SDL_H
