/*
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef __MKDIRAT_H__
#define __MKDIRAT_H__

#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"

#ifndef HAVE_MKDIRAT
int mkdirat(int dirfd, const char *dirname, int mode)
{
	return ltp_syscall(__NR_mkdirat, dirfd, dirname, mode);
}
#endif

#endif /* __MKDIRAT_H__ */
