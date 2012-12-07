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
 * int mprotect(void *, size_t, int)
 * is declared.
 *
 * @pt:MPR
 */

#include <sys/mman.h>

typedef int (*mprotect_test) (void *, size_t, int);

int dummyfcn(void)
{
	mprotect_test dummyvar;
	dummyvar = mprotect;
	return 0;
}
