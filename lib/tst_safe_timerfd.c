// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#include "tst_safe_timerfd.h"
#include "lapi/timerfd.h"
#include "tst_clocks.h"
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#define TTYPE (errno == ENOTSUP ? TCONF : TBROK)

int safe_timerfd_create(const char *file, const int lineno,
				      int clockid, int flags)
{
	int fd;

	fd = timerfd_create(clockid, flags);
	if (fd < 0) {
		tst_brk(TTYPE | TERRNO, "%s:%d timerfd_create(%s) failed",
			file, lineno, tst_clock_name(clockid));
	}

	return fd;
}

int safe_timerfd_gettime(const char *file, const int lineno,
				int fd, struct itimerspec *curr_value)
{
	int rval;

	rval = timerfd_gettime(fd, curr_value);
	if (rval != 0) {
		tst_brk(TTYPE | TERRNO, "%s:%d timerfd_gettime() failed",
			file, lineno);
	}

	return rval;
}

int safe_timerfd_settime(const char *file, const int lineno,
				int fd, int flags,
				const struct itimerspec *new_value,
				struct itimerspec *old_value)
{
	int rval;

	rval = timerfd_settime(fd, flags, new_value, old_value);
	if (rval != 0) {
		tst_brk(TTYPE | TERRNO, "%s:%d timerfd_settime() failed",
			file, lineno);
	}

	return rval;
}
