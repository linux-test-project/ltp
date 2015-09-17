/*
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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

#include "test.h"
#include "ltp_priv.h"

#define OPTS_MAX 32

void tst_mkfs(void (cleanup_fn)(void), const char *dev,
	      const char *fs_type, const char *const fs_opts[])
{
	int i, pos = 3;
	const char *argv[OPTS_MAX] = {"mkfs", "-t", fs_type};
	char fs_opts_str[1024] = "";

	if (!fs_type)
		tst_brkm(TBROK, cleanup_fn, "No fs_type specified");

	/*
	 * mkfs.xfs and mkfs.btrfs aborts if it finds a filesystem
	 * superblock on the device, which is the case here as we
	 * reuse one device for all tests.
	 */
	if (!strcmp(fs_type, "xfs")) {
		tst_resm(TINFO, "Appending '-f' flag to mkfs.%s", fs_type);
		argv[pos++] = "-f";
	}

	if (!strcmp(fs_type, "btrfs")) {
		/*
		 * The -f option was added to btrfs-progs v3.12
		 */
		if (!tst_system("mkfs.btrfs 2>&1 | grep -q '\\-f[ |]'")) {
			tst_resm(TINFO, "Appending '-f' flag to mkfs.%s",
				fs_type);
			argv[pos++] = "-f";
		}
	}

	if (fs_opts) {
		for (i = 0; fs_opts[i]; i++) {
			argv[pos++] = fs_opts[i];

			if (pos + 2 > OPTS_MAX) {
				tst_brkm(TBROK, cleanup_fn,
				         "Too much mkfs options");
			}

			if (i)
				strcat(fs_opts_str, " ");
			strcat(fs_opts_str, fs_opts[i]);
		}
	}

	argv[pos++] = dev;
	argv[pos] = NULL;

	tst_resm(TINFO, "Formatting %s with %s extra opts='%s'",
		 dev, fs_type, fs_opts_str);
	tst_run_cmd(cleanup_fn, argv, "/dev/null", NULL, 0);
}

const char *tst_dev_fs_type(void)
{
	const char *fs_type;

	fs_type = getenv("LTP_DEV_FS_TYPE");

	if (fs_type)
		return fs_type;

	return DEFAULT_FS_TYPE;
}
