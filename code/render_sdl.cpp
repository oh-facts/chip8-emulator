void r_sdl_innit(SDL_Window *win)
{
    r_sdl_state.ren = SDL_CreateRenderer(win, 0);
    SDL_SetRenderVSync(r_sdl_state.ren, 1);
}

R_Handle r_sdl_alloc_texture(void *data, i32 w, i32 h, i32 n, R_Texture_params *p)
{
    SDL_Texture *texture = SDL_CreateTexture(r_sdl_state.ren, SDL_PIXELFORMAT_RGBA8888, 
                                        SDL_TEXTUREACCESS_STATIC, w, h);

    SDL_UpdateTexture(texture, 0, data, w * sizeof(u32));
    R_Handle out = {};
    out.u64_m[0] = (u64)texture;
    return out;
}

void r_sdl_submit(R_Pass_list *list, v2i win_size)
{
    SDL_SetRenderDrawColor(r_sdl_state.ren, 255, 0, 255, 255);
    SDL_RenderClear(r_sdl_state.ren);

    f32 aspect = win_size.x * 1.f / win_size.y;

    R_Pass_node *node = list->first;
	for(u32 i = 0; i < list->num; i ++)
	{
		R_Pass *pass = &node->pass;
		
		switch(pass->kind)
		{
			default:{}break;
			case R_PASS_KIND_UI:
			{
				R_Batch_list *batches = &pass->rect_pass.rects;
				R_Batch *batch = batches->first;
				
				for(u32 j = 0; j < batches->num; j++)
				{
                    R_Rect *rects = (R_Rect*)batch->base;
                    for(u32 k = 0; k < batch->count; k++)
                    {
                        R_Rect rect = rects[k];
                        // -aspect, aspect ; 1, 1 ->  0,0 ; w, h
                        rect.tl.x = (rect.tl.x + aspect) * win_size.x / (2.f * aspect);
                        rect.tl.y = win_size.y - (rect.tl.y + 1.f) * win_size.y / (2.f);

                        rect.br.x = (rect.br.x + aspect) * win_size.x / (2 * aspect);
                        rect.br.y = win_size.y - (rect.br.y + 1.f) * win_size.y / (2.f);

                        SDL_FRect dstRect = { rect.tl.x, rect.tl.y, rect.br.x - rect.tl.x, rect.br.y - rect.tl.y };
                        SDL_SetTextureColorMod((SDL_Texture*)rect.tex.u64_m[0], 
                                                rect.color.x * 255, rect.color.y * 255, 
                                                rect.color.z * 255);
                        
                        SDL_RenderTextureRotated(r_sdl_state.ren, (SDL_Texture*)rect.tex.u64_m[0], 0, &dstRect, 
                        0, 0, SDL_FLIP_VERTICAL);
                    }

					batch = batch->next;
				}
			}break;
		}
		
		node = node->next;
	}

    SDL_RenderPresent(r_sdl_state.ren);
}