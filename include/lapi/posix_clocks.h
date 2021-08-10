// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linux Test Project
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

#include <time.h>

#ifndef LAPI_POSIX_CLOCKS_H__
#define LAPI_POSIX_CLOCKS_H__

#define MAX_CLOCKS 16

#ifndef CLOCK_MONOTONIC_RAW
# define CLOCK_MONOTONIC_RAW 4
#endif

#ifndef CLOCK_REALTIME_COARSE
# define CLOCK_REALTIME_COARSE 5
#endif

#ifndef CLOCK_MONOTONIC_COARSE
# define CLOCK_MONOTONIC_COARSE 6
#endif

#ifndef CLOCK_BOOTTIME
# define CLOCK_BOOTTIME 7
#endif

#ifndef CLOCK_REALTIME_ALARM
# define CLOCK_REALTIME_ALARM 8
#endif

#ifndef CLOCK_BOOTTIME_ALARM
# define CLOCK_BOOTTIME_ALARM 9
#endif

#ifndef CLOCK_TAI
#define CLOCK_TAI 11
#endif

#endif /* LAPI_POSIX_CLOCKS_H__ */
