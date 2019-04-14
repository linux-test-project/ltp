// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017-2019 Petr Vorel pvorel@suse.cz
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
#ifndef __ANDROID__
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
