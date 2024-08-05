/* date = July 18th 2024 6:07 am */

#ifndef SAOIRSE_H
#define SAOIRSE_H

struct State
{
	Arena *arena;
	Arena *trans;
	i32 temp;
	
	D_Bucket draw;
	
	Atlas atlas;
	R_Handle atlas_tex[256];
	
	R_Handle white_square;
	R_Handle face;
	
	Chip8 chip;
	
	UI_Context *cxt;
};

#endif //SAOIRSE_H
