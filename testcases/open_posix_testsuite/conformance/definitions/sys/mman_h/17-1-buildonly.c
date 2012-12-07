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
 * int munlockall(void)
 * is declared.
 *
 * @pt:ML
 */

#include <sys/mman.h>

typedef int (*munlockall_test) (void);

int dummyfcn(void)
{
	munlockall_test dummyvar;
	dummyvar = munlockall;
	return 0;
}
