/*
 *
 *   Copyright (c) International Business Machines  Corp., 2003
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *
 *    AUTHOR
 *    	Paul Larson <plars@linuxtestproject.org>
 *
 *    DESCRIPTION
 * 	Compare a given kernel version against the current kernel version.
 * 	If they are the same - return 0
 * 	If the argument is > current kernel version - return positive int
 * 	If the argument is < current kernel version - return negative int
 *
 */


#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>

void get_kver(int *k1, int *k2, int *k3)
{
	struct utsname uval;
	char *kver;
	char *r1, *r2, *r3;
#if !defined(linux)
	extern char *strsep();          /* shut up some compilers */
#endif

	uname(&uval);
	kver = uval.release;
	r1 = strsep(&kver, ".");
	r2 = strsep(&kver, ".");
	r3 = strsep(&kver, ".");

	*k1 = atoi(r1);
	*k2 = atoi(r2);
	*k3 = atoi(r3);
}

int tst_kvercmp(int r1, int r2, int r3) {
	int a1, a2, a3;
	int testver, currver;

	get_kver(&a1, &a2, &a3);
	testver = (r1 << 16) + (r2 << 8) + r3;
	currver = (a1 << 16) + (a2 << 8) + a3;

	return currver - testver;
}
