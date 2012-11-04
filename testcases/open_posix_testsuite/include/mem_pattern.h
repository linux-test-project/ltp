/*
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Function to fill mem with reasonably complicated pattern and function
 * that checks that pattern is correct.
 */

static void fill_mem(void *dst, size_t size)
{
	unsigned char *ptr = dst;

	while (--size > 0) {
		*ptr = (size % 256) ^ 0x42;
		ptr++;
	}
}

static int check_mem(void *src, size_t size)
{
	unsigned char *ptr = src;

	while (--size > 0) {
		if (*ptr != ((size % 256) ^ 0x42))
			return 1;
		ptr++;
	}

	return 0;
}
