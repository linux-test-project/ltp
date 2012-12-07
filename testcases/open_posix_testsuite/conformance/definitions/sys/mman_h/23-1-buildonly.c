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
 * int shm_open(const char *, int, mode_t)
 * is declared.
 *
 * @pt:SHM
 */

#include <sys/mman.h>

typedef int (*shm_open_test) (const char *, int, mode_t);

int dummyfcn(void)
{
	shm_open_test dummyvar;
	dummyvar = shm_open;
	return 0;
}
