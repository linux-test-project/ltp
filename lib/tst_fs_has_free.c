/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * DESCRIPTION
 *	Check if the mounted file system has enough free space,
 *	if it is, tst_fs_has_free() returns 1, otherwise 0.
 */

#if defined(__linux__)
#include <sys/vfs.h>
#elif defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <stdint.h>
#include "test.h"

int tst_fs_has_free(void (*cleanup)(void), const char *path,
		    unsigned int size, unsigned int mult)
{
	struct statfs sf;

	if (statfs(path, &sf)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "tst_fs_has_free: failed to statfs(%s)", path);
	}

	if ((uint64_t)sf.f_bavail * sf.f_bsize >= (uint64_t)size * mult)
		return 1;

	return 0;
}
