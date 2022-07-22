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

#define SAFE_MQ_CLOSE(__mqdes) \
	safe_mq_close(__FILE__, __LINE__, (__mqdes))

#define SAFE_MQ_UNLINK(name) \
	safe_mq_unlink(__FILE__, __LINE__, (name))

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
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mq_open(%s,%d,%04o,%p) failed", pathname, oflags,
			mode, attr);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid mq_open(%s) return value %d", pathname, rval);
	}

	return rval;
}

static inline int safe_mq_close(const char *file, const int lineno,
				mqd_t __mqdes)
{
	int rval;

	rval = mq_close(__mqdes);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mq_close(%d) failed", __mqdes);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid mq_close(%d) return value %d", __mqdes, rval);
	}

	return rval;
}

static inline int safe_mq_unlink(const char *file, const int lineno,
				const char* name)
{
	int rval;

	rval = mq_unlink(name);

	if (rval == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"mq_unlink(%s) failed", name);
	} else if (rval < 0) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			"Invalid mq_unlink(%s) return value %d", name, rval);
	}

	return rval;
}

#endif /* TST_SAFE_POSIX_IPC_H__ */
