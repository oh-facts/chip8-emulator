// core include
#include "saoirse_platform.h"

// backends
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <commdlg.h>

#include <GL/gl.h>
#include "opengl/opengl_khr_platform.h"
#include "opengl/opengl_win32_platform.h"
#include "render_opengl.h"
#include "render_opengl.cpp"

global u64 total_cmt;
global u64 total_res;

internal Str8 os_win32_open_file_dialog(Arena *arena)
{
	OPENFILENAME ofn = {} ;
	
	char szFile[MAX_PATH] = {};
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	
	GetOpenFileName( &ofn );
	
	Str8 out = push_str8f(arena, "%s", ofn.lpstrFile);
	
	return out;
}

internal void *os_win32_reserve(u64 size)
{
	void *out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	if (out != NULL)
	{
		total_res += size;
	}
	return out;
}

internal b32 os_win32_commit(void *ptr, u64 size)
{
	if (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) == NULL)
	{
		printf("VirtualAlloc commit failed: %lu\r\n", GetLastError());
		return 0;
	}
	total_cmt += size;
	
	if(total_cmt > 22806528)
	{
		volatile int i = 0;
	}
	
	return 1;
}

internal void os_win32_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

internal void os_win32_release(void *ptr, u64 size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

internal u64 os_win32_get_page_size()
{
	SYSTEM_INFO sysinfo = {};
	GetSystemInfo(&sysinfo);
	return sysinfo.dwPageSize;
}

Str8 os_win32_get_app_dir(Arena *arena)
{
	char buffer[256];
	DWORD len = GetModuleFileName(0, buffer, 256);
	
	char *c = &buffer[len];
  while(*(--c) != '\\')
  {
    *c = 0;
    --len;
  }
  
	u8 *str = push_array(arena, u8, len);
	mem_cpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}

struct W32_Window
{
	HWND hwnd;
	i32 w;
	i32 h;
	HDC dc;
	b32 closed;
};

struct OS_Event
{
	v2i mpos;
};

struct OS_Event_node
{
	OS_Event_node *next;
	OS_Event event;
};

struct OS_Event_list
{
	OS_Event_node *first;
	OS_Event_node *last;
	
	u32 count;
};

Arena *arena;
OS_Event_list event_list;

internal OS_Event *os_push_event()
{
	OS_Event_node *node = push_struct(arena, OS_Event_node);
	
	OS_Event_list *list = &event_list;
	list->count ++;
	if(list->last)
	{
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	
	OS_Event *event = &node->event;
	return event;
}

Input input;
W32_Window *win;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    
    case WM_MOUSEWHEEL:
    {
      i32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
      input.scroll = zDelta;
    }break;
		case WM_CREATE:
		{
			
		}break;
    case WM_DESTROY:
    {
      PostQuitMessage(0);
			win->closed = 1;
		}break;
    case WM_RBUTTONDOWN:
    {
      input.mb[MOUSE_BUTTON_RIGHT] = 1;
    }break;
    case WM_LBUTTONDOWN:
    {
      input.mb[MOUSE_BUTTON_LEFT] = 1;
    }break;
    case WM_RBUTTONUP:
		{
      input.mb[MOUSE_BUTTON_RIGHT] = 0;
    }break;
    case WM_LBUTTONUP:
    {
      input.mb[MOUSE_BUTTON_LEFT] = 0; 
    }break;
    case WM_MOUSEMOVE:
    {
      input.mpos.x = LOWORD(lParam);
      input.mpos.y = HIWORD(lParam);
			
			//OS_Event *event = os_push_event();
			//event->mpos.x = LOWORD(lParam);
			//event->mpos.y = HIWORD(lParam);
			//printf("%d %d %d\n", i++, input.mpos.x, input.mpos.y);
			
		}break;
    case WM_KEYUP:
    case WM_KEYDOWN:
    {
			b32 pressed = msg == WM_KEYDOWN;
			
      local_persist i32 win32_sr_key_table[256];
			local_persist b32 initialized;
			
			if(!initialized)
			{
				win32_sr_key_table[(u32)'W'] = KEY_UP;
				win32_sr_key_table[(u32)'A'] = KEY_LEFT;
				win32_sr_key_table[(u32)'S'] = KEY_DOWN;
				win32_sr_key_table[(u32)'D'] = KEY_RIGHT;
				win32_sr_key_table[VK_TAB] = KEY_TAB;
        win32_sr_key_table[VK_CONTROL] = KEY_CTRL;
				initialized = true;
      }
			
      if(wParam >= 'A' && wParam <= 'Z')
      {
        input.keys[wParam] = pressed;
      }
      
      if(wParam >= '0' && wParam <= '9')
      {
        input.keys[wParam] = pressed;
      }
      
			input.keys[win32_sr_key_table[wParam]] = pressed;
			
			if(wParam == 'Q' && input.keys[KEY_CTRL])
			{
				win->closed = true;
			}
			
			if(wParam == 'F' && pressed && input.keys[KEY_CTRL])
			{
				local_persist b32 is_fullscreen = 0;
				if(!is_fullscreen)
				{
					int w = GetSystemMetrics(SM_CXSCREEN);
					int h = GetSystemMetrics(SM_CYSCREEN);
					
					SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
					SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
				}
				else
				{
					RECT borderRect = {};
					AdjustWindowRectEx(&borderRect, WS_OVERLAPPEDWINDOW, 0, 0);
					
					win->w = 960;
					win->h = 540;
					
					win->w += borderRect.right - borderRect.left;
					win->h += borderRect.bottom - borderRect.top;
					
					SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
					SetWindowPos(hwnd, HWND_TOP, 0, 0, win->w, win->h, SWP_FRAMECHANGED);
				}
				
				is_fullscreen = !is_fullscreen;
			}
			
    }break;
		case WM_SIZE:
		{
			win->w = LOWORD(lParam);
			win->h = HIWORD(lParam);
		}break;
		
  }
  
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

W32_Window *os_win32_create_opengl_window(Arena *arena, const char *title, i32 w, i32 h)
{
	W32_Window *out = push_struct(arena, W32_Window);
	
	HGLRC rc = 0;
	
	WNDCLASSA wc = {};
  
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(0);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszClassName = "main";
  
  RegisterClass(&wc);
  
  HWND fake_win = 0;
  HDC fake_dc = 0;
  HGLRC fake_rc = 0;
  
  {
    fake_win = CreateWindowA(wc.lpszClassName, "fake_win", WS_OVERLAPPEDWINDOW, 100,100, 1, 1, 0, 0, wc.hInstance, 0);
    
    PIXELFORMATDESCRIPTOR pfd = {
      .nSize = sizeof(PIXELFORMATDESCRIPTOR),
      .nVersion = 1,
      .dwFlags = PFD_DOUBLEBUFFER | 
        PFD_SUPPORT_OPENGL | 
        PFD_DRAW_TO_WINDOW,
      .iPixelType = PFD_TYPE_RGBA,
      .cColorBits = 32,
      .cAlphaBits = 8,
      .cDepthBits = 24,
    };
    
    fake_dc = GetDC(fake_win);
    
    int px_fmt = ChoosePixelFormat(fake_dc, &pfd);
    
    SetPixelFormat(fake_dc, px_fmt, &pfd);
    
    fake_rc = wglCreateContext(fake_dc);
    
    wglMakeCurrent(fake_dc,fake_rc);
    
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
  }
  
  {
		RECT borderRect = {};
		AdjustWindowRectEx(&borderRect, WS_OVERLAPPEDWINDOW, 0, 0);
		
		out->w = w + borderRect.right - borderRect.left;
		out->h = h + borderRect.bottom - borderRect.top;
		
    out->hwnd = CreateWindowA(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 100,100, out->w, out->h, 0, 0, wc.hInstance, 0);
    win = out;
		
		ShowWindow(out->hwnd, SW_SHOWNORMAL);
    UpdateWindow(out->hwnd);
    
		out->dc = GetDC(out->hwnd);
    const int pixelAttribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB, 32,
      WGL_ALPHA_BITS_ARB, 8,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8,
      WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      WGL_SAMPLES_ARB, 4,
      0
    };
    
    int pixelFormatID; UINT numFormats;
    wglChoosePixelFormatARB(out->dc, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);
    PIXELFORMATDESCRIPTOR PFD;
    DescribePixelFormat(out->dc, pixelFormatID, sizeof(PFD), &PFD);
    SetPixelFormat(out->dc, pixelFormatID, &PFD);
    
    wglMakeCurrent(0,0);
    wglDeleteContext(fake_rc);
    ReleaseDC(fake_win, fake_dc);
    DestroyWindow(fake_win);
    
    out->dc = GetDC(out->hwnd);
    
    const int major_min = 4, minor_min = 5;
    int  contextAttribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
      WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };
    
    rc = wglCreateContextAttribsARB(out->dc, 0, contextAttribs);
    wglMakeCurrent(out->dc,rc);
  }
	
	return out;
}

int main(int argc, char **argv)
{
	S_Platform pf = {};
	pf.argc = argc;
	pf.argv = argv;
	
	pf.p_api.os_reserve = os_win32_reserve;
	pf.p_api.os_commit = os_win32_commit;
	pf.p_api.os_decommit = os_win32_decommit;
	pf.p_api.os_release = os_win32_release;
	pf.p_api.os_get_page_size = os_win32_get_page_size;
	pf.p_api.os_get_app_dir = os_win32_get_app_dir;
	pf.p_api.os_open_file_dialog = os_win32_open_file_dialog;
	pf.r_api.r_alloc_texture = r_opengl_alloc_texture;
	pf.r_api.r_submit = r_opengl_submit;
	
	s_global_platform_api_init(&pf.p_api);
	s_global_render_api_init(&pf.r_api);
	
	arena = arena_create();
	pf.memory = push_struct(arena, State);
	Str8 app_dir = os_get_app_dir(arena);
	
	pf.app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit("yk.dll");
	Str8 dll_path = str8_join(arena, app_dir, dll_rel_path);
	HMODULE game_dll = LoadLibrary((char *)dll_path.c);
	
	if(!game_dll)
	{
		printf("dll not found\n\r");
	}
	
	update_and_render_fn update_and_render = (update_and_render_fn)GetProcAddress(game_dll, "update_and_render");
	
	if(!update_and_render)
	{
		printf("fn not found\n\r");
	}
	
	os_win32_create_opengl_window(arena,"dear mizu hot chip 8",960,540);
	opengl_load_functions();
	r_opengl_init();
	win->closed = 0;
	
	LARGE_INTEGER start_counter = {};
	QueryPerformanceCounter(&start_counter);
	u64 start = start_counter.QuadPart;
	
	LARGE_INTEGER perf_freq = {};
	QueryPerformanceFrequency(&perf_freq);
	u64 freq = perf_freq.QuadPart;
	
	f64 time_elapsed = 0;
	f64 time_since_last = 0;
	f64 delta = 0;
	wglSwapIntervalEXT(1);
	
	glEnable(GL_FRAMEBUFFER_SRGB);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	while(!win->closed)
	{
		time_since_last = time_elapsed;
		
		MSG msg;
		event_list.first = 0;
		event_list.last = 0;
		event_list.count = 0;
		
		while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }
		//printf("%d) %d %d\n", i++, input.mpos.x, input.mpos.y);
		if(input.mpos_old.x != input.mpos.x && input.mpos_old.y != input.mpos.y)
		{
#if 1
			
#else
			
			for(OS_Event_node *event = event_list.first; event != 0; event = event->next)
			{
				//printf("%d) %d %d\n", i++, event->event.mpos.x, event->event.mpos.y);
			}
			
#endif
		}
		
		//event_list.first = 0;
		
		f32 color[3] = {1,0,1};
		
		glClearBufferfv(GL_COLOR, 0, color);
		pf.win_size.x = win->w;
		pf.win_size.y = win->h;
		pf.res = total_res;
		pf.cmt = total_cmt;
		update_and_render(&pf, &input, delta);
		input_update(&input);
		SwapBuffers(win->dc);
		
		LARGE_INTEGER end_counter = {};
		QueryPerformanceCounter(&end_counter);
		
		i64 counter_elapsed = end_counter.QuadPart - start;
		time_elapsed = (1.f * counter_elapsed) / freq;
		
		delta = time_elapsed - time_since_last;
	}
	
	return 0;
}