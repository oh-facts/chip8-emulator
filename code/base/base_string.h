/* date = June 4th 2024 1:13 am */

#ifndef BASE_STRING_H
#define BASE_STRING_H

#define str8_varg(S) (int)((S).len), ((S).c)

struct Str8
{
  u8 *c;
  u64 len;
};

#define str8_lit(c) Str8{(u8*)c, sizeof(c) - 1}

internal u64 str8_len(char *c);
internal Str8 str8(u8 *c, u64 len);

internal void str8_cpy(Str8 *dst, Str8 *src);
internal Str8 push_str8f(Arena *arena, char *fmt, ...);
internal Str8 push_str8fv(Arena *arena, char *fmt, va_list args);

internal Str8 str8_join(Arena *arena, Str8 a, Str8 b);

#endif //BASE_STRING_H