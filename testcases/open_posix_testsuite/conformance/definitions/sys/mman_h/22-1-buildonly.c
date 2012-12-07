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
 * int posix_typed_mem_open(const char *, int, int)
 * is declared.
 *
 * @pt:TYM
 */

#include <sys/mman.h>
#include <unistd.h>

#if defined(_POSIX_TYPED_MEMORY_OBJECTS) && _POSIX_TYPED_MEMORY_OBJECTS != -1

typedef int (*posix_typed_mem_open_test) (const char *, int, int);

int dummyfcn(void)
{
	posix_typed_mem_open_test dummyvar;
	dummyvar = posix_typed_mem_open;
	return 0;
}

#endif
