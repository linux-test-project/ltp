/*
 * Copyright (c) International Business Machines  Corp., 2003
 *    AUTHOR: Paul Larson <plars@linuxtestproject.org>
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/utsname.h>
#include "test.h"

static char *parse_digit(const char *str, int *d)
{
	unsigned long v;
	char *end;

	v = strtoul(str, &end, 10);
	if (str == end)
		return NULL;

	if (v > INT_MAX)
		return NULL;

	*d = v;

	return end;
}

int tst_parse_kver(const char *str_kver, int *v1, int *v2, int *v3)
{
	const char *str = str_kver;

	*v1 = 0;
	*v2 = 0;
	*v3 = 0;

	if (!(str = parse_digit(str, v1)))
		return 1;

	if (*(str++) != '.')
		return 1;

	if (!(str = parse_digit(str, v2)))
		return 1;

	/*
	 * Check for a short version e.g '2.4'
	 */
	if (*str == ' ' || *str == '\0')
		return 0;

	if (*(str++) != '.')
		return 1;

	/*
	 * Ignore rest of the string in order not to break on versions as
	 * 4.8.1-52-default.
	 */
	if (!parse_digit(str, v3))
		return 1;

	return 0;
}

int tst_kvcmp(const char *cur_kver, int r1, int r2, int r3)
{
	int a1, a2, a3;
	int testver, currver;

	if (tst_parse_kver(cur_kver, &a1, &a2, &a3)) {
		tst_resm(TWARN,
			 "Invalid kernel version %s, expected %%d.%%d.%%d",
		         cur_kver);
	}

	testver = (r1 << 16) + (r2 << 8) + r3;
	currver = (a1 << 16) + (a2 << 8) + a3;

	return currver - testver;
}

int tst_kvercmp(int r1, int r2, int r3)
{
	struct utsname uval;

	uname(&uval);

	return tst_kvcmp(uval.release, r1, r2, r3);
}

int tst_kvexcmp(const char *tst_exv, const char *cur_ver)
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

const char *tst_kvcmp_distname(const char *kver)
{
	if (strstr(kver, ".el5uek"))
		return "OL5UEK";

	if (strstr(kver, ".el5"))
		return "RHEL5";

	if (strstr(kver, ".el6uek"))
		return "OL6UEK";

	if (strstr(kver, ".el6"))
		return "RHEL6";

	return NULL;
}

int tst_kvercmp2(int r1, int r2, int r3, struct tst_kern_exv *vers)
{
	int i;
	const char *kver;
	struct utsname uval;
	const char *cur_dist_name;

	uname(&uval);
	kver = uval.release;
	cur_dist_name = tst_kvcmp_distname(kver);

	if (cur_dist_name == NULL)
		return tst_kvercmp(r1, r2, r3);

	for (i = 0; vers[i].dist_name; i++) {
		if (!strcmp(vers[i].dist_name, cur_dist_name)) {
			tst_resm(TINFO, "Detected %s using kernel version %s",
				 cur_dist_name, kver);
			return tst_kvexcmp(vers[i].extra_ver, kver);
		}
	}

	return tst_kvcmp(kver, r1, r2, r3);
}
