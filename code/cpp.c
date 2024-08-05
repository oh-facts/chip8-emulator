#include "stdio.h"
#include "math.h"

// 1. c++ constexpr fn wont work if its calling const functions. Fair. It wont work even if the
// those functions don't have side effects like math. Impeccable language design.
// 2. I moved over the math to the shader since I am using opengl now.

enum ColorKind
{
	ColorKind_HEX,
	ColorKind_GammaCorrectRGB,
	ColorKind_COUNT
};
typedef enum ColorKind ColorKind;

struct Color_piece
{
	char name[256];
	union
	{
		unsigned char b[4];
		unsigned int hex;
	};
	ColorKind kind;
};

typedef struct Color_piece Color_piece;

#define NUM (4)

static Color_piece table[NUM] = 
{
	{"CHIP8_COLOR_BG", {.hex = 0x683a68}, ColorKind_HEX},
	{"CHIP8_COLOR_FG", {.hex = 0x412752}, ColorKind_HEX},
	{"CHIP8_COLOR_TEXT", {.hex = 0x9775a6}, ColorKind_HEX},
	{"CHIP8_COLOR_BG_BG", {.hex = 0x2d162c}, ColorKind_HEX},
};

#define HEADER_FILE_START "#ifndef META_H \n#define META_H\n"
#define HEADER_FILE_END "#endif"

int main()
{
	printf(HEADER_FILE_START);
	printf("//Hello :D \n\n");
	
	for(int i = 0; i < NUM; i++)
	{
		printf("global v4f %s = {{%.4f, %.4f, %.4f, 1}};\n", table[i].name, table[i].b[2] / 255.f, table[i].b[1] / 255.f, table[i].b[0] / 255.f);
	}
	printf(HEADER_FILE_END);
}