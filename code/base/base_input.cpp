b32 input_is_key_tapped(Input *state, u32 key)
{
  return state->keys[key] && !state->keys_old[key];
}

b32 input_is_key_held(Input *state, u32 key)
{
  return state->keys[key];
}

b32 input_is_click(Input* state, MOUSE_BUTTON button)
{
  return state->mb[button] && !state->mb_old[button];
}

b32 input_is_mouse_held(Input* state, MOUSE_BUTTON button)
{
  return state->mb[button];
}

v2i input_get_mouse_mv(Input *state)
{
  v2i out = state->mpos - state->mpos_old;
  return out;
}

void input_update(Input *state)
{
  for(u32 i = 0; i < KEY_COUNT; i ++)
  {
    state->keys_old[i] = state->keys[i];
  }
	
	for (u32 i = 0; i < MOUSE_BUTTON_COUNT; i++)
	{
		state->mb_old[i] = state->mb[i];
	}
  state->mpos_old = state->mpos;
  state->mmov = false;
  state->scroll = 0;
}