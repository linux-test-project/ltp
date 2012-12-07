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
 * int munlock(const void *, size_t)
 * is declared.
 *
 * @pt:MR
 */

#include <sys/mman.h>

typedef int (*munlock_test) (const void *, size_t);

int dummyfcn(void)
{
	munlock_test dummyvar;
	dummyvar = munlock;
	return 0;
}
