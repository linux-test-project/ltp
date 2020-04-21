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

#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>

#include "test.h"
#include "safe_file_ops_fn.h"

/*
 * Count number of expected assigned conversions. Any conversion starts with '%'.
 * The '%%' matches % and no assignment is done. The %*x matches as x would do but
 * the assignment is suppressed.
 *
 * NOTE: This is not 100% correct for complex scanf strings, but will do for
 *       all of our intended usage.
 */
static int count_scanf_conversions(const char *fmt)
{
	unsigned int cnt = 0;
	int flag = 0;

	while (*fmt) {
		switch (*fmt) {
		case '%':
			if (flag) {
				cnt--;
				flag = 0;
			} else {
				flag = 1;
				cnt++;
			}
			break;
		case '*':
			if (flag) {
				cnt--;
				flag = 0;
			}
			break;
		default:
			flag = 0;
		}

		fmt++;
	}

	return cnt;
}

int file_scanf(const char *file, const int lineno,
		     const char *path, const char *fmt, ...)
{
	va_list va;
	FILE *f;
	int exp_convs, ret;

	f = fopen(path, "r");

	if (f == NULL) {
		tst_resm(TWARN,
			"Failed to open FILE '%s' at %s:%d",
			 path, file, lineno);
		return 1;
	}

	exp_convs = count_scanf_conversions(fmt);

	va_start(va, fmt);
	ret = vfscanf(f, fmt, va);
	va_end(va);

	if (ret == EOF) {
		tst_resm(TWARN,
			 "The FILE '%s' ended prematurely at %s:%d",
			 path, file, lineno);
		goto err;
	}

	if (ret != exp_convs) {
		tst_resm(TWARN,
			"Expected %i conversions got %i FILE '%s' at %s:%d",
			 exp_convs, ret, path, file, lineno);
		goto err;
	}

	if (fclose(f)) {
		tst_resm(TWARN,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
		return 1;
	}

	return 0;

err:
	if (fclose(f)) {
		tst_resm(TWARN,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
	}
	return 1;
}

void safe_file_scanf(const char *file, const int lineno,
		     void (*cleanup_fn) (void),
		     const char *path, const char *fmt, ...)
{
	va_list va;
	FILE *f;
	int exp_convs, ret;

	f = fopen(path, "r");

	if (f == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "Failed to open FILE '%s' for reading at %s:%d",
			 path, file, lineno);
		return;
	}

	exp_convs = count_scanf_conversions(fmt);

	va_start(va, fmt);
	ret = vfscanf(f, fmt, va);
	va_end(va);

	if (ret == EOF) {
		tst_brkm(TBROK, cleanup_fn,
			 "The FILE '%s' ended prematurely at %s:%d",
			 path, file, lineno);
		return;
	}

	if (ret != exp_convs) {
		tst_brkm(TBROK, cleanup_fn,
			 "Expected %i conversions got %i FILE '%s' at %s:%d",
			 exp_convs, ret, path, file, lineno);
		return;
	}

	if (fclose(f)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
		return;
	}
}


/*
 * Try to parse each line from file specified by 'path' according
 * to scanf format 'fmt'. If all fields could be parsed, stop and
 * return 0, otherwise continue or return 1 if EOF is reached.
 */
int file_lines_scanf(const char *file, const int lineno,
		     void (*cleanup_fn)(void), int strict,
		     const char *path, const char *fmt, ...)
{
	FILE *fp;
	int ret = 0;
	int arg_count = 0;
	char line[BUFSIZ];
	va_list ap;

	if (!fmt) {
		tst_brkm(TBROK, cleanup_fn, "pattern is NULL, %s:%d",
			file, lineno);
		return 1;
	}

	fp = fopen(path, "r");
	if (fp == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"Failed to open FILE '%s' for reading at %s:%d",
			path, file, lineno);
		return 1;
	}

	arg_count = count_scanf_conversions(fmt);

	while (fgets(line, BUFSIZ, fp) != NULL) {
		va_start(ap, fmt);
		ret = vsscanf(line, fmt, ap);
		va_end(ap);

		if (ret == arg_count)
			break;
	}
	fclose(fp);

	if (strict && ret != arg_count) {
		tst_brkm(TBROK, cleanup_fn, "Expected %i conversions got %i"
			" FILE '%s' at %s:%d", arg_count, ret, path, file, lineno);
		return 1;
	}

	return !(ret == arg_count);
}

int file_printf(const char *file, const int lineno,
		      const char *path, const char *fmt, ...)
{
	va_list va;
	FILE *f;

	f = fopen(path, "w");

	if (f == NULL) {
		tst_resm(TWARN,
			 "Failed to open FILE '%s' at %s:%d",
			 path, file, lineno);
		return 1;
	}

	va_start(va, fmt);

	if (vfprintf(f, fmt, va) < 0) {
		tst_resm(TWARN,
			"Failed to print to FILE '%s' at %s:%d",
			 path, file, lineno);
		goto err;
	}

	va_end(va);

	if (fclose(f)) {
		tst_resm(TWARN,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
		return 1;
	}

	return 0;

err:
	if (fclose(f)) {
		tst_resm(TWARN,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
	}
	return 1;
}

void safe_file_printf(const char *file, const int lineno,
		      void (*cleanup_fn) (void),
		      const char *path, const char *fmt, ...)
{
	va_list va;
	FILE *f;

	f = fopen(path, "w");

	if (f == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "Failed to open FILE '%s' for writing at %s:%d",
			 path, file, lineno);
		return;
	}

	va_start(va, fmt);

	if (vfprintf(f, fmt, va) < 0) {
		tst_brkm(TBROK, cleanup_fn,
			 "Failed to print to FILE '%s' at %s:%d",
			 path, file, lineno);
		return;
	}

	va_end(va);

	if (fclose(f)) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "Failed to close FILE '%s' at %s:%d",
			 path, file, lineno);
		return;
	}
}

//TODO: C implementation? better error condition reporting?
void safe_cp(const char *file, const int lineno,
	     void (*cleanup_fn) (void), const char *src, const char *dst)
{
	size_t len = strlen(src) + strlen(dst) + 16;
	char buf[len];
	int ret;

	snprintf(buf, sizeof(buf), "cp \"%s\" \"%s\"", src, dst);

	ret = system(buf);

	if (ret) {
		tst_brkm(TBROK, cleanup_fn,
			 "Failed to copy '%s' to '%s' at %s:%d",
			 src, dst, file, lineno);
	}
}

#ifndef HAVE_UTIMENSAT

static void set_time(struct timeval *res, const struct timespec *src,
			long cur_tv_sec, long cur_tv_usec)
{
	switch (src->tv_nsec) {
	case UTIME_NOW:
	break;
	case UTIME_OMIT:
		res->tv_sec = cur_tv_sec;
		res->tv_usec = cur_tv_usec;
	break;
	default:
		res->tv_sec = src->tv_sec;
		res->tv_usec = src->tv_nsec / 1000;
	}
}

#endif

void safe_touch(const char *file, const int lineno,
		void (*cleanup_fn)(void),
		const char *pathname,
		mode_t mode, const struct timespec times[2])
{
	int ret;
	mode_t defmode;

	defmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	ret = open(pathname, O_CREAT | O_WRONLY, defmode);
	if (ret == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"Failed to open file '%s' at %s:%d",
			pathname, file, lineno);
		return;
	}

	ret = close(ret);
	if (ret == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"Failed to close file '%s' at %s:%d",
			pathname, file, lineno);
		return;
	}

	if (mode != 0) {
		ret = chmod(pathname, mode);
		if (ret == -1) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				"Failed to chmod file '%s' at %s:%d",
				pathname, file, lineno);
			return;
		}
	}


#ifdef HAVE_UTIMENSAT
	ret = utimensat(AT_FDCWD, pathname, times, 0);
#else
	if (times == NULL) {
		ret = utimes(pathname, NULL);
	} else {
		struct stat sb;
		struct timeval cotimes[2];

		ret = stat(pathname, &sb);
		if (ret == -1) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				"Failed to stat file '%s' at %s:%d",
				pathname, file, lineno);
			return;
		}

		ret = gettimeofday(cotimes, NULL);
		if (ret == -1) {
			tst_brkm(TBROK | TERRNO, cleanup_fn,
				"Failed to gettimeofday() at %s:%d",
				file, lineno);
			return;
		}

		cotimes[1] = cotimes[0];

		set_time(cotimes, times,
			sb.st_atime, sb.st_atim.tv_nsec / 1000);
		set_time(cotimes + 1, times + 1,
			sb.st_mtime, sb.st_mtim.tv_nsec / 1000);

		ret = utimes(pathname, cotimes);
	}
#endif
	if (ret == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			"Failed to update the access/modification time on file"
			" '%s' at %s:%d", pathname, file, lineno);
	}
}
