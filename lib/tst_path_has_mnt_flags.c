/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
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

#include <unistd.h>
#include <mntent.h>
#include <stdio.h>
#include <string.h>
#include "test.h"

/*
 * Check whether a path is on a filesystem that is mounted with
 * specified flags.
 */
int tst_path_has_mnt_flags_(void (cleanup_fn)(void),
		const char *path, const char *flags[])
{
	struct mntent *mnt;
	size_t prefix_max = 0, prefix_len;
	int flags_matched = 0;
	FILE *f;
	int i;
	char *tmpdir = NULL;

	/*
	 * Default parameter is test temporary directory
	 */
	if (path == NULL)
		path = tmpdir = tst_get_tmpdir();

	if (access(path, F_OK) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"tst_path_has_mnt_flags: path %s doesn't exist", path);
		return -1;
	}

	f = setmntent("/proc/mounts", "r");
	if (f == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"tst_path_has_mnt_flags: failed to open /proc/mounts");
		return -1;
	}

	while ((mnt = getmntent(f))) {
		/* ignore duplicit record for root fs */
		if (!strcmp(mnt->mnt_fsname, "rootfs"))
			continue;

		prefix_len = strlen(mnt->mnt_dir);

		if (strncmp(path, mnt->mnt_dir, prefix_len) == 0
				&& prefix_len > prefix_max) {
			prefix_max = prefix_len;
			flags_matched = 0;
			i = 0;

			while (flags[i] != NULL) {
				if (hasmntopt(mnt, flags[i]) != NULL)
					flags_matched++;
				i++;
			}
		}
	}

	endmntent(f);

	free(tmpdir);

	return flags_matched;
}
