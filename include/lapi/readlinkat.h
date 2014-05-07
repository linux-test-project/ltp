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

#ifndef __READLINKAT_H__
#define __READLINKAT_H__

#include "config.h"
#include "linux_syscall_numbers.h"
#include "lapi/fcntl.h"

#ifndef HAVE_READLINKAT
int readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
	return ltp_syscall(__NR_readlinkat, dirfd, pathname, buf, bufsiz);
}
#endif

#endif /* __READLINKAT_H__ */
