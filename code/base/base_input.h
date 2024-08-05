/* date = July 4th 2024 2:42 pm */
#ifndef BASE_INPUT_H
#define BASE_INPUT_H

enum KEY
{
  KEY_UP = 1,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_DOWN,
  KEY_TAB,
  KEY_CTRL,
	KEY_COUNT = 128,
};

enum MOUSE_BUTTON
{
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_COUNT,
};

enum MOUSE_CURSOR
{
  MOUSE_CURSOR_ARROW,
  MOUSE_CURSOR_HAND,
  MOUSE_CURSOR_COUNT
};

struct Input
{
  b32 keys[KEY_COUNT];
  b32 keys_old[KEY_COUNT];
	
	b32 mb[MOUSE_BUTTON_COUNT];
	b32 mb_old[MOUSE_BUTTON_COUNT];
	
	v2i mpos;
  v2i mpos_old;
  b32 mmov;
  
  i32 scroll;
};

internal b32 input_is_key_tapped(Input *state, u32 key);
internal b32 input_is_key_held(Input *state, u32 key);
internal b32 input_is_click(Input* state, MOUSE_BUTTON button);
internal b32 input_is_mouse_held(Input* state, MOUSE_BUTTON button);

internal v2i input_get_mouse_mv(Input *state);

internal void input_update(Input *state);

#endif //BASE_INPUT_H