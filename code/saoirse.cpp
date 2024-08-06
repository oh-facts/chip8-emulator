#include "saoirse_platform.h"

extern "C"
{
	YK_API void update_and_render(S_Platform *, Input *, f32 delta);
}

void update_and_render(S_Platform * pf, Input *input, f32 delta)
{
	BEGIN_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	if(!pf->initialized)
	{
		pf->initialized = 1;
		
		s_global_platform_api_init(&pf->p_api);
		s_global_render_api_init(&pf->r_api);
		Arena *arena = arena_create(Megabytes(1), Gigabytes(1));
		State *state = push_struct(arena, State);
		pf->memory = (void*)state;
		state->arena = arena;
		state->trans = arena_create();
		state->temp = -1000;
		tcxt_init();
		
		R_Texture_params font_params = {
			R_TEXTURE_FILTER_LINEAR,
			R_TEXTURE_FILTER_LINEAR,
			R_TEXTURE_WRAP_CLAMP_TO_BORDER
		};
		
		R_Texture_params tiled_params = {
			R_TEXTURE_FILTER_LINEAR,
			R_TEXTURE_FILTER_LINEAR,
			R_TEXTURE_WRAP_REPEAT
		};
		
		char codepoints[] =
		{
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y','z',
			
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
			'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			
			'&', '.', '?', ',', '-', ':', '!', '\\', '/',
			
			' ', '\n'
		};
		
		Arena_temp temp = arena_temp_begin(state->trans);
		Str8 font_path = str8_join(state->trans, pf->app_dir, str8_lit("../data/delius.ttf"));
		Glyph *temp_font = make_bmp_font(font_path.c, codepoints, ARRAY_LEN(codepoints), state->trans);
		
		for(u32 i = 0; i < ARRAY_LEN(codepoints); i ++)
		{
			u32 c = codepoints[i];
			
			if(c != '\n' && c != ' ')
			{
				state->atlas_tex[c] = r_alloc_texture(temp_font[i].bmp, temp_font[i].w, temp_font[i].h, 1, &font_params);
			}
			
			state->atlas.glyphs[c].bearing = temp_font[i].bearing;
			state->atlas.glyphs[c].advance = temp_font[i].advance;
			state->atlas.glyphs[c].x0 = temp_font[i].x0;
			state->atlas.glyphs[c].x1 = temp_font[i].x1;
			state->atlas.glyphs[c].y0 = temp_font[i].y0;
			state->atlas.glyphs[c].y1 = temp_font[i].y1;
			
		}
		
		u32 *white_square = push_struct(arena, u32);
		*white_square = 0xFFFFFFFF;
		state->white_square = r_alloc_texture(white_square, 1, 1, 4, &tiled_params);

		state->cxt = ui_alloc_cxt();
		
		arena_temp_end(&temp);
	}
	
	g_input = input;

	State *state = (State*)pf->memory;
	
	Arena *trans = state->trans;
	Arena_temp temp = arena_temp_begin(trans);
	
	state->draw = {};
	state->draw.arena = trans;
	state->draw.white_square = state->white_square;
	state->draw.default_text_params =
	(D_Text_params){
		(v4f){{1,1,1,1}},
		0.00007,
		&state->atlas,
		state->atlas_tex
	};

	state->cxt->atlas = &state->atlas;
	
	f32 aspect_ratio = pf->win_size.x * 1.f / pf->win_size.y;
	v2f screen_norm ;
	screen_norm.x = input->mpos.x * 1.f / pf->win_size.y * 2.f - aspect_ratio;
	screen_norm.y = 1 - input->mpos.y * 1.f / pf->win_size.y * 2.f;
	
	state->cxt->mpos = screen_norm;
	state->cxt->mheld = input_is_mouse_held(input, MOUSE_BUTTON_LEFT);
	state->cxt->mclick = input_is_click(input, MOUSE_BUTTON_LEFT);
	state->cxt->frames++;
	
	f32 zoom = 1;
	f32 aspect = (pf->win_size.x * 1.f)/ pf->win_size.y;
	d_push_proj_view(&state->draw, m4f_ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.001, 1000).fwd);
	
	f32 scale = 4;
	
	d_draw_rect(&state->draw, v2f{{-scale * 0.5f, scale * 0.5f}}, v2f{{scale, scale}}, CHIP8_COLOR_BG_BG);
	chip_run(&state->chip, state->cxt, &state->draw, delta);
	ui_layout(state->cxt->root);
	
	d_draw_ui(&state->draw, state->cxt->root);
	d_pop_proj_view(&state->draw);
	
	r_submit(&state->draw.list, pf->win_size);
	
	arena_temp_end(&temp);
	
	END_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	process_debug_counters();
}