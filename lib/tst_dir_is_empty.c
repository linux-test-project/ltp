/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "test.h"
#include "safe_macros.h"

int tst_dir_is_empty_(void (cleanup_fn)(void), const char *name, int verbose)
{
	struct dirent *entry;
	DIR *dir = SAFE_OPENDIR(cleanup_fn, name);
	int ret = 1;

	while ((entry = SAFE_READDIR(cleanup_fn, dir)) != NULL) {
		const char *file = entry->d_name;

		if (!strcmp(file, "..") || !strcmp(file, "."))
			continue;

		if (verbose)
			tst_resm(TINFO, "found a file: %s", file);
		ret = 0;
		break;
	}

	SAFE_CLOSEDIR(cleanup_fn, dir);

	return ret;
}
