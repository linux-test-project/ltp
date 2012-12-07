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
 * int munmap(void *, size_t)
 * is declared.
 *
 * @pt:MF
 * @pt:SHM
 * @pt:TYM
 */

#include <sys/mman.h>

typedef int (*munmap_test) (void *, size_t);

int dummyfcn(void)
{
	munmap_test dummyvar;
	dummyvar = munmap;
	return 0;
}
