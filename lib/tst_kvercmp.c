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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
#include "test.h"

void get_kver(int *k1, int *k2, int *k3)
{
	struct utsname uval;
	char *kver;
	char *r1, *r2, *r3;

	uname(&uval);
	kver = uval.release;
	r1 = strsep(&kver, ".");
	r2 = strsep(&kver, ".");
	r3 = strsep(&kver, ".");

	*k1 = atoi(r1);
	*k2 = atoi(r2);
	*k3 = atoi(r3);
}

int tst_kvercmp(int r1, int r2, int r3)
{
	int a1, a2, a3;
	int testver, currver;

	get_kver(&a1, &a2, &a3);
	testver = (r1 << 16) + (r2 << 8) + r3;
	currver = (a1 << 16) + (a2 << 8) + a3;

	return currver - testver;
}

static int tst_kexvcmp(char *tst_exv, char *cur_ver)
{
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0;
	int t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 0;
	int ret;

	sscanf(cur_ver, "%d.%d.%d-%d.%d", &c1, &c2, &c3, &c4, &c5);
	sscanf(tst_exv, "%d.%d.%d-%d.%d", &t1, &t2, &t3, &t4, &t5);

	if ((ret = c1 - t1))
		return ret;
	if ((ret = c2 - t2))
		return ret;
	if ((ret = c3 - t3))
		return ret;
	if ((ret = c4 - t4))
		return ret;

	return c5 - t5;
}

int tst_kvercmp2(int r1, int r2, int r3, struct tst_kern_exv *vers)
{
	int i;
	struct utsname uval;
	char *kver;
	const char *cur_dist_name = NULL;

	uname(&uval);
	kver = uval.release;
	if (strstr(kver, ".el5")) {
		cur_dist_name = "RHEL5";
	} else if (strstr(kver, ".el6")) {
		cur_dist_name = "RHEL6";
	}

	if (cur_dist_name == NULL)
		return tst_kvercmp(r1, r2, r3);

	for (i = 0; vers[i].dist_name; i++) {
		if (!strcmp(vers[i].dist_name, cur_dist_name)) {
			tst_resm(TINFO, "Detected %s using kernel version %s",
				 cur_dist_name, kver);
			return tst_kexvcmp(vers[i].extra_ver, kver);
		}
	}

	return tst_kvercmp(r1, r2, r3);
}
