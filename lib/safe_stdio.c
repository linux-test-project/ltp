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

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "safe_stdio_fn.h"

FILE *safe_fopen(const char *file, const int lineno, void (cleanup_fn)(void),
                 const char *path, const char *mode)
{
	FILE *f = fopen(path, mode);

	if (f == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: fopen(%s,%s) failed",
			 file, lineno, path, mode);
	}

	return f;
}

int safe_fclose(const char *file, const int lineno, void (cleanup_fn)(void),
                FILE *f)
{
	int ret;

	ret = fclose(f);

	if (ret) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: fclose(%p) failed", file, lineno, f);
	}

	return ret;
}

int safe_asprintf(const char *file, const int lineno, void (cleanup_fn)(void),
                  char **strp, const char *fmt, ...)
{
	int ret;
	va_list va;

	va_start(va, fmt);
	ret = vasprintf(strp, fmt, va);
	va_end(va);

	if (ret < 0) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: asprintf(%s,...) failed", file, lineno, fmt);
	}

	return ret;
}

FILE *safe_popen(const char *file, const int lineno, void (cleanup_fn)(void),
		 const char *command, const char *type)
{
	FILE *stream;
	const int saved_errno = errno;

	errno = 0;
	stream = popen(command, type);

	if (stream == NULL) {
		if (errno != 0) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				 "%s:%d: popen(%s,%s) failed",
				 file, lineno, command, type);
		} else {
			tst_brkm(TBROK, cleanup_fn,
				 "%s:%d: popen(%s,%s) failed: Out of memory",
				 file, lineno, command, type);
		}
	}

	errno = saved_errno;

	return stream;
}
