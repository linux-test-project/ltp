// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

struct map {
	void *addr;
	size_t size;
	size_t buf_shift;
	struct map *next;
};

static struct map *maps;

static void setup_canary(struct map *map)
{
	size_t i;
	char *buf = map->addr;

	for (i = 0; i < map->buf_shift/2; i++) {
		char c = random();
		buf[map->buf_shift - i - 1] = c;
		buf[i] = c;
	}
}

static void check_canary(struct map *map)
{
	size_t i;
	char *buf = map->addr;

	for (i = 0; i < map->buf_shift/2; i++) {
		if (buf[map->buf_shift - i - 1] != buf[i]) {
			tst_res(TWARN,
				"pid %i: buffer modified address %p[%zi]",
				getpid(), (char*)map->addr + map->buf_shift, -i-1);
		}
	}
}

void *tst_alloc(size_t size)
{
	size_t page_size = getpagesize();
	unsigned int pages = (size / page_size) + !!(size % page_size) + 1;
	void *ret;
	struct map *map = SAFE_MALLOC(sizeof(struct map));
	static int print_msg = 1;

	if (print_msg) {
		tst_res(TINFO, "Test is using guarded buffers");
		print_msg = 0;
	}

	ret = SAFE_MMAP(NULL, page_size * pages, PROT_READ | PROT_WRITE,
	                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	mprotect(ret + (pages-1) * page_size, page_size, PROT_NONE);

	map->addr = ret;
	map->size = pages * page_size;
	map->next = maps;
	maps = map;

	if (size % page_size)
		map->buf_shift = page_size - (size % page_size);
	else
		map->buf_shift = 0;

	setup_canary(map);

	return ret + map->buf_shift;
}

char *tst_aprintf(const char *fmt, ...)
{
	va_list va;
	int len;
	char *ret;

        va_start(va, fmt);
        len = vsnprintf(NULL, 0, fmt, va)+1;
        va_end(va);

	ret = tst_alloc(len);

	va_start(va, fmt);
        vsprintf(ret, fmt, va);
        va_end(va);

	return ret;
}

static int count_iovec(int *sizes)
{
	int ret = 0;

	while (sizes[ret++] != -1);

	return ret - 1;
}

struct iovec *tst_iovec_alloc(int sizes[])
{
	int i, cnt = count_iovec(sizes);
	struct iovec *iovec;

	if (cnt <= 0)
		return NULL;

	iovec = tst_alloc(sizeof(struct iovec) * cnt);

	for (i = 0; i < cnt; i++) {
		if (sizes[i]) {
			iovec[i].iov_base = tst_alloc(sizes[i]);
			iovec[i].iov_len = sizes[i];
		} else {
			iovec[i].iov_base = NULL;
			iovec[i].iov_base = 0;
		}
	}

	return iovec;
}

void tst_buffers_alloc(struct tst_buffers bufs[])
{
	unsigned int i;

	for (i = 0; bufs[i].ptr; i++) {
		if (bufs[i].size)
			*((void**)bufs[i].ptr) = tst_alloc(bufs[i].size);
		else if (bufs[i].iov_sizes)
			*((void**)bufs[i].ptr) = tst_iovec_alloc(bufs[i].iov_sizes);
		else
			*((void**)bufs[i].ptr) = tst_strdup(bufs[i].str);
	}
}

char *tst_strdup(const char *str)
{
	char *ret = tst_alloc(strlen(str) + 1);

	return strcpy(ret, str);
}

void tst_free_all(void)
{
	struct map *i = maps;

	while (i) {
		struct map *j = i;
		check_canary(i);
		SAFE_MUNMAP(i->addr, i->size);
		i = i->next;
		free(j);
	}

	maps = NULL;
}
