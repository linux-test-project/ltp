/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the function:
 * int posix_mem_offset(const void *restrict, size_t, off_t *restrict,
 *                      size_t *restrict, int *restrict)
 * is declared.
 *
 * @pt:TYM
 */

#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_TYPED_MEMORY_OBJECTS) && _POSIX_TYPED_MEMORY_OBJECTS != -1

typedef int (*posix_mem_offset_test) (const void *restrict, size_t,
				      off_t * restrict, size_t * restrict,
				      int *restrict);

int dummyfcn(void)
{
	posix_mem_offset_test dummyvar;
	dummyvar = posix_mem_offset;
	return 0;
}

#endif
