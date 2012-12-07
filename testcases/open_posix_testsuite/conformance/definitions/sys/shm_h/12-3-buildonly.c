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
 * key_t ftok(const char *, int)
 * from sys/ipc.h is declared.
 */

#include <sys/shm.h>

typedef key_t(*ftok_test) (const char *, int);

int dummyfcn(void)
{
	ftok_test dummyvar;
	dummyvar = ftok;
	return 0;
}
