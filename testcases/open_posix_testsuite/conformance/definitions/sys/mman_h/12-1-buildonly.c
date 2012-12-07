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
 * int mlockall(int)
 * is declared.
 *
 * @pt:ML
 */

#include <sys/mman.h>

typedef int (*mlockall_test) (int);

int dummyfcn(void)
{
	mlockall_test dummyvar;
	dummyvar = mlockall;
	return 0;
}
