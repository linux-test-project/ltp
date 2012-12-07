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
 * void *mmap(void *, size_t, int, int, int, off_t)
 * is declared.
 *
 * @pt:MF
 * @pt:SHM
 * @pt:TYM
 */

#include <sys/mman.h>

typedef void *(*mmap_test) (void *, size_t, int, int, int, off_t);

int dummyfcn(void)
{
	mmap_test dummyvar;
	dummyvar = mmap;
	return 0;
}
