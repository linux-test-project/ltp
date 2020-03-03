// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef TST_SAFE_TIMERFD_H__
#define TST_SAFE_TIMERFD_H__

#include "lapi/timerfd.h"

int safe_timerfd_create(const char *file, const int lineno,
				      int clockid, int flags);

#define SAFE_TIMERFD_CREATE(clockid, flags)\
	safe_timerfd_create(__FILE__, __LINE__, (clockid), (flags))

int safe_timerfd_gettime(const char *file, const int lineno,
				int fd, struct itimerspec *curr_value);

#define SAFE_TIMERFD_GETTIME(fd, curr_value)\
	safe_timerfd_gettime(__FILE__, __LINE__, (fd), (curr_value))

int safe_timerfd_settime(const char *file, const int lineno,
				int fd, int flags,
				const struct itimerspec *new_value,
				struct itimerspec *old_value);

#define SAFE_TIMERFD_SETTIME(fd, flags, new_value, old_value)\
	safe_timerfd_settime(__FILE__, __LINE__, (fd), (flags), (new_value), \
						 (old_value))

#endif /* SAFE_TIMERFD_H__ */
