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

#ifndef __POSIX_CLOCK_IDS_H__
#define __POSIX_CLOCK_IDS_H__

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

#endif /* __POSIX_CLOCK_IDS_H__ */
