/*
 * Copyright (C) 2017 Petr Vorel pvorel@suse.cz
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

#ifndef TST_SAFE_POSIX_IPC_H__
#define TST_SAFE_POSIX_IPC_H__

#include <mqueue.h>
#include <stdarg.h>

#define SAFE_MQ_OPEN(pathname, oflags, ...) \
	safe_mq_open(__FILE__, __LINE__, (pathname), (oflags), ##__VA_ARGS__)

static inline int safe_mq_open(const char *file, const int lineno,
			       const char *pathname, int oflags, ...)
{
	va_list ap;
	int rval;
	mode_t mode;
	struct mq_attr *attr;

	va_start(ap, oflags);

	/* Android's NDK's mode_t is smaller than an int, which results in
	 * SIGILL here when passing the mode_t type.
	 */
#ifndef ANDROID
	mode = va_arg(ap, mode_t);
#else
	mode = va_arg(ap, int);
#endif

	attr = va_arg(ap, struct mq_attr *);

	va_end(ap);

	rval = mq_open(pathname, oflags, mode, attr);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: mq_open(%s,%d,0%o,%p) failed",
			 file, lineno, pathname, oflags, mode, attr);
	}

	return rval;
}

#endif /* TST_SAFE_POSIX_IPC_H__ */
