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

	const char *const argv[] = {"mkfs", dev, "-t", fs_type, fs_opts, NULL};

	tst_run_cmd(cleanup_fn, argv, "/dev/null", NULL);
}
