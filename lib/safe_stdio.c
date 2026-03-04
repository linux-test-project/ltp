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
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"fopen(%s,%s) failed", path, mode);
	}

	return f;
}

int safe_fclose(const char *file, const int lineno, void (cleanup_fn)(void),
                FILE *f)
{
	int ret;

	ret = fclose(f);

	if (ret == EOF) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"fclose(%p) failed", f);
	} else if (ret) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid fclose(%p) return value %d", f, ret);
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

	if (ret == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"asprintf(%s,...) failed", fmt);
	} else if (ret < 0) {
		tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
			"Invalid asprintf(%s,...) return value %d", fmt, ret);
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
			tst_brkm_(file, lineno, TBROK | TERRNO, cleanup_fn,
				"popen(%s,%s) failed", command, type);
		} else {
			tst_brkm_(file, lineno, TBROK, cleanup_fn,
				"popen(%s,%s) failed: Out of memory",
				command, type);
		}
	}

	errno = saved_errno;

	return stream;
}

size_t safe_fread(const char *file, const int lineno,
	void *ptr, size_t size, size_t n, FILE *stream)
{
	size_t ret;

	ret = fread(ptr, size, n, stream);
	if (ret != n) {
		tst_brkm_(file, lineno, TBROK, NULL,
			"fread(%p, %lu, %lu, %p) read %lu bytes",
			ptr, size, n, stream, ret);
	}

	return ret;
}

size_t safe_fwrite(const char *file, const int lineno,
	const void *ptr, size_t size, size_t n, FILE *stream)
{
	size_t ret;

	ret = fwrite(ptr, size, n, stream);
	if (ret != n) {
		tst_brkm_(file, lineno, TBROK, NULL,
			"fwrite(%p, %lu, %lu, %p) written %lu bytes",
			ptr, size, n, stream, ret);
	}

	return ret;
}

FILE *safe_freopen(const char *file, const int lineno,
	       const char *path, const char *mode, FILE *stream)
{
	FILE *f = freopen(path, mode, stream);

	if (!f) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"freopen(%s,%s,%p) failed", path, mode, stream);
	}

	return f;
}

int safe_fseek(const char *file, const int lineno,
		   FILE *f, long offset, int whence)
{
	int ret;

	errno = 0;
	ret = fseek(f, offset, whence);

	if (ret == -1) {
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL,
			"fseek(%p, %ld, %d)", f, offset, whence);
	}

	return ret;
}

long safe_ftell(const char *file, const int lineno,
	       FILE *f)
{
	long ret;

	errno = 0;
	ret = ftell(f);

	if (ret == -1)
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL, "ftell(%p)", f);

	return ret;
}

int safe_fileno(const char *file, const int lineno,
		FILE *f)
{
	int ret;

	errno = 0;
	ret = fileno(f);

	if (ret == -1)
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL, "fileno(%p)", f);

	return ret;
}

int safe_fflush(const char *file, const int lineno,
		FILE *f)
{
	int ret;

	errno = 0;
	ret = fflush(f);

	if (ret == EOF)
		tst_brkm_(file, lineno, TBROK | TERRNO, NULL, "fflush(%p)", f);

	return ret;
}
