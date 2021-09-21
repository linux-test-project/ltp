/*
 * Copyright (c) 2012-2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAFE_FILE_OPS_FN
#define SAFE_FILE_OPS_FN

#include <sys/stat.h>
#include <time.h>

#include "lapi/utime.h"

/*
 * Count number of expected assigned conversions. Any conversion starts with '%'.
 * The '%%' matches % and no assignment is done. The %*x matches as x would do but
 * the assignment is suppressed.
 *
 * NOTE: This is not 100% correct for complex scanf strings, but will do for
 *       all of our intended usage.
 */
int tst_count_scanf_conversions(const char *fmt);

/*
 * All-in-one function to scanf value(s) from a file.
 */
int file_scanf(const char *file, const int lineno,
		const char *path, const char *fmt, ...)
		__attribute__ ((format (scanf, 4, 5)));

void safe_file_scanf(const char *file, const int lineno,
                     void (*cleanup_fn)(void),
		     const char *path, const char *fmt, ...)
		     __attribute__ ((format (scanf, 5, 6)));

int file_lines_scanf(const char *file, const int lineno,
		     void (*cleanup_fn)(void), int strict,
		     const char *path, const char *fmt, ...)
		     __attribute__ ((format (scanf, 6, 7)));

/*
 * All-in-one function that lets you printf directly into a file.
 */
int file_printf(const char *file, const int lineno,
                      const char *path, const char *fmt, ...)
                      __attribute__ ((format (printf, 4, 5)));

void safe_file_printf(const char *file, const int lineno,
                      void (*cleanup_fn)(void),
                      const char *path, const char *fmt, ...)
                      __attribute__ ((format (printf, 5, 6)));

void safe_try_file_printf(const char *file, const int lineno,
	void (*cleanup_fn)(void), const char *path, const char *fmt, ...)
	__attribute__ ((format (printf, 5, 6)));

/*
 * Safe function to copy files, no more system("cp ...") please.
 */
int safe_cp(const char *file, const int lineno,
             void (*cleanup_fn)(void),
	     const char *src, const char *dst);

/*
 * Safe function to touch a file.
 *
 * If the file (pathname) does not exist It will be created with
 * the specified permission (mode) and the access/modification times (times).
 *
 * If mode is 0 then the file is created with (0666 & ~umask)
 * permission or (if the file exists) the permission is not changed.
 *
 * times is a timespec[2] (as for utimensat(2)). If times is NULL then
 * the access/modification times of the file is set to the current time.
 */
int safe_touch(const char *file, const int lineno,
		void (*cleanup_fn)(void),
		const char *pathname,
		mode_t mode, const struct timespec times[2]);

/* helper functions to setup overlayfs mountpoint */
void create_overlay_dirs(void);
int mount_overlay(const char *file, const int lineno, int skip);

#endif /* SAFE_FILE_OPS_FN */
