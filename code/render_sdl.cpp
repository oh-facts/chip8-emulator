void r_sdl_innit(SDL_Window *win)
{
    r_sdl_state.ren = SDL_CreateRenderer(win, 0);
}

R_Handle r_sdl_alloc_texture(void *data, i32 w, i32 h, i32 n, R_Texture_params *p)
{
    return {};
}

void r_sdl_submit(R_Pass_list *list, v2i win_size)
{
    SDL_SetRenderDrawColor(r_sdl_state.ren, 255, 0, 255, 255);
    SDL_RenderClear(r_sdl_state.ren);


    R_Pass_node *node = list->first;
	for(i32 i = 0; i < list->num; i ++)
	{
		R_Pass *pass = &node->pass;
		
		switch(pass->kind)
		{
			default:{}break;
			case R_PASS_KIND_UI:
			{
				R_Batch_list *batches = &pass->rect_pass.rects;
				R_Batch *batch = batches->first;
				
				for(i32 j = 0; j < batches->num; j++)
				{
                    R_Rect *rects = (R_Rect*)batch->base;
                    for(i32 k = 0; k < batch->count; k++)
                    {
                        R_Rect *rect = rects + k;
                       // printf("%f %f %f %f\n", rect->tl.x, rect->tl.y, rect->br.x, rect->br.y);
                        rect->tl.x = ((rect->tl.x + 2)*win_size.x)/2.f;
                        rect->tl.y = win_size.y*2.f - ((rect->tl.y + 2)*win_size.y)/2.f;

                        rect->br.x = ((rect->br.x + 2)*win_size.x)/2.f;
                        rect->br.y = win_size.y*2.f - ((rect->br.y + 2)*win_size.y)/2.f;


                        //printf("%f %f %f %f\n", rect->tl.x, rect->tl.y, rect->br.x, rect->br.y);
                        //printf("===\n");
                        SDL_SetRenderDrawColor(r_sdl_state.ren, rect->color.x, rect->color.y, rect->color.z, rect->color.w);
                        SDL_FRect frect = { rect->tl.x, rect->tl.y, rect->br.x - rect->tl.x, rect->br.y - rect->tl.y };
                        SDL_RenderFillRect(r_sdl_state.ren, &frect);
                    }

					batch = batch->next;
				}
			}break;
		}
		
		node = node->next;
	}

    //INVALID_CODE_PATH();

    SDL_RenderPresent(r_sdl_state.ren);
}