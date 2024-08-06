void r_sdl_innit(SDL_Window *win)
{
    r_sdl_state.ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(r_sdl_state.ren, 255, 0, 255, 255);
	SDL_RenderClear(r_sdl_state.ren);
}

R_Handle r_sdl_alloc_texture(void *data, i32 w, i32 h, i32 n, R_Texture_params *p)
{
    return {};
}

void r_sdl_submit(R_Pass_list *list, v2i win_size)
{
    SDL_RenderPresent(r_sdl_state.ren);
}