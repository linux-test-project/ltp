/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

uid_t tst_get_unused_uid(void)
{
	struct passwd pwd;
	struct passwd *result;
	char *buf;
	long bufsize;
	int s;
	uid_t uid;

	bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufsize == -1)
		bufsize = 16384;

	buf = malloc(bufsize);
	if (buf == NULL)
		return -1;

	for (uid = 0; uid <= UINT_MAX - 1; uid++) {
		s = getpwuid_r(uid, &pwd, buf, bufsize, &result);
		if (result == NULL) {
			free(buf);
			/*
			 * When the given name or gid was not found, getgrgid_r
			 * may return 0 or ENOENT or ESRCH or EBADF or EPERM
			 * or ...
			 */
			switch (s) {
			case 0:
			case ENOENT:
			case ESRCH:
			case EBADF:
			case EPERM:
				return uid;
			default:
				return -1;
			}
		}
	}

	free(buf);
	return -1;
}

gid_t tst_get_unused_gid(void)
{
	struct group grp;
	struct group *result;
	char *buf;
	long bufsize;
	int s;
	gid_t gid;

	bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
	if (bufsize == -1)
		bufsize = 16384;

	buf = malloc(bufsize);
	if (buf == NULL)
		return -1;

	for (gid = 0; gid <= UINT_MAX - 1; gid++) {
		s = getgrgid_r(gid, &grp, buf, bufsize, &result);
		if (result == NULL) {
			free(buf);
			/*
			 * When the given name or gid was not found, getgrgid_r
			 * may return 0 or ENOENT or ESRCH or EBADF or EPERM
			 * or ...
			 */
			switch (s) {
			case 0:
			case ENOENT:
			case ESRCH:
			case EBADF:
			case EPERM:
				return gid;
			default:
				return -1;
			}
		}
	}

	free(buf);
	return -1;
}
