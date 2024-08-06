global u64 total_cmt;
global u64 total_res;

internal void *os_linux_reserve(u64 size)
{
	void *out = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	total_res += size;
	return out;
}

internal b32 os_linux_commit(void *ptr, u64 size)
{
	if(mprotect(ptr, size, PROT_READ | PROT_WRITE) == -1)
	{
		int err = errno;
		printf("mprotect failed: %s\r\n", strerror(err));
		return 0;
	}
	total_cmt += size;
	return 1;
}

internal void os_linux_decommit(void *ptr, u64 size)
{
    madvise(ptr, size, MADV_DONTNEED);
    mprotect(ptr, size, PROT_NONE);
}

internal void os_linux_release(void *ptr, u64 size)
{
    munmap(ptr, size);
}

internal Str8 os_linux_get_app_dir(Arena *arena)
{
    char buffer[256];
    ssize_t len = readlink("/proc/self/exe", buffer, 256);

    char *c = &buffer[len];
    while(*(--c) != '/')
    {
        *c = 0;
        --len;
    }

    u8 *str = push_array(arena, u8, len);
    mem_cpy(str, buffer, len);

    Str8 out = str8(str, len);

    return out;
}