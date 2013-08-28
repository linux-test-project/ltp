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

void tst_mkfs(void (cleanup_fn)(void), const char *dev,
              const char *fs_type, const char *fs_opts)
{
	tst_resm(TINFO, "Formatting %s with %s extra opts='%s'",
	        dev, fs_type, fs_opts ? fs_opts : "");

	if (!fs_type)
		tst_brkm(TBROK, cleanup_fn, "No fs_type specified");

	const char *argv[] = {"mkfs", "-t", fs_type, NULL, NULL, NULL, NULL};
	int pos = 3;

	/*
	 * The mkfs.xfs aborts if it finds a filesystem superblock
	 * on the device, which is the case here as we reuse one
	 * device for all tests.
	 */
	if (!strcmp(fs_type, "xfs")) {
		tst_resm(TINFO, "Appending '-f' force flag to mkfs.xfs");
		argv[pos++] = "-f";
	}

	if (fs_opts)
		argv[pos++] = fs_opts;

	argv[pos] = dev;

	tst_run_cmd(cleanup_fn, argv, "/dev/null", NULL);
}
