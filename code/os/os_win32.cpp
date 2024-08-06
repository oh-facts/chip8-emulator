global u64 total_cmt;
global u64 total_res;

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

internal Str8 os_win32_get_app_dir(Arena *arena)
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