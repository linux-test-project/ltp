/*
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
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

#include "tst_resource.h"

static int file_copy(const char *file, const int lineno,
                     void (*cleanup_fn)(void), const char *path,
		     const char *filename, const char *dest)
{
	size_t len = strlen(path) + strlen(filename) + 2;
	char buf[len];

	snprintf(buf, sizeof(buf), "%s/%s", path, filename);

	/* check if file exists */
	if (access(buf, R_OK))
		return 0;

	safe_cp(file, lineno, cleanup_fn, buf, dest);

	return 1;
}

/* declared in tst_tmpdir.c */
const char *tst_get_startwd(void);

void tst_resource_copy(const char *file, const int lineno,
                       void (*cleanup_fn)(void),
		       const char *filename, const char *dest)
{
	if (!tst_tmpdir_created()) {
		tst_brkm(TBROK, cleanup_fn,
		         "Temporary directory doesn't exist at %s:%d",
		         file, lineno);
	}

	if (dest == NULL)
		dest = ".";

	const char *ltproot = getenv("LTPROOT");

	if (ltproot != NULL) {
		/* the data are either in testcases/data or testcases/bin */
		char buf[strlen(ltproot) + 64];

		snprintf(buf, sizeof(buf), "%s/testcases/data", ltproot);

		if (file_copy(file, lineno, cleanup_fn, buf, filename, dest))
			return;
		
		snprintf(buf, sizeof(buf), "%s/testcases/bin", ltproot);
		
		if (file_copy(file, lineno, cleanup_fn, buf, filename, dest))
			return;
	}

	const char *startwd = tst_get_startwd();

	/* try directory test started int first */
	if (file_copy(file, lineno, cleanup_fn, startwd, filename, dest))
		return;

	tst_brkm(TBROK, cleanup_fn, "Failed to copy resource '%s' at %s:%d",
	         filename, file, lineno);
}
