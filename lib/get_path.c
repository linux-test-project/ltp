/*
 * Copyright (C) 2010 Cyril Hrubis chrubis@suse.cz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

 /*
  * Looks for binary prog_name in $PATH.
  *
  * If such file exists and if you are able at least to read it, zero is
  * returned and absolute path to the file is filled into buf. In case buf is
  * too short to hold the absolute path + prog_name for the file we are looking
  * for -1 is returned as well as when there is no such file in all paths in
  * $PATH.
  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"

static int file_exist(const char *path)
{
	struct stat st;

	if (!access(path, R_OK) && !stat(path, &st) && S_ISREG(st.st_mode))
		return 1;

	return 0;
}

int tst_get_path(const char *prog_name, char *buf, size_t buf_len)
{
	const char *path = (const char *)getenv("PATH");
	const char *start = path;
	const char *end;
	size_t size, ret;

	if (path == NULL)
		return -1;

	do {
		end = strchr(start, ':');

		if (end != NULL)
			snprintf(buf, MIN(buf_len, (size_t) (end - start + 1)),
				 "%s", start);
		else
			snprintf(buf, buf_len, "%s", start);

		size = strlen(buf);

		/*
		 * "::" inside $PATH, $PATH ending with ':' or $PATH starting
		 * with ':' should be expanded into current working directory.
		 */
		if (size == 0) {
			snprintf(buf, buf_len, ".");
			size = strlen(buf);
		}

		/*
		 * If there is no '/' ad the end of path from $PATH add it.
		 */
		if (buf[size - 1] != '/')
			ret =
			    snprintf(buf + size, buf_len - size, "/%s",
				     prog_name);
		else
			ret =
			    snprintf(buf + size, buf_len - size, "%s",
				     prog_name);

		if (buf_len - size > ret && file_exist(buf))
			return 0;

		start = end + 1;

	} while (end != NULL);

	return -1;
}
