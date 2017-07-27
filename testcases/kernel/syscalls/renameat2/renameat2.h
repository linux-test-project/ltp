/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
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

#ifndef RENAMEAT2_H
#define RENAMEAT2_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_RENAMEAT2)
int renameat2(int olddirfd, const char *oldpath, int newdirfd,
				const char *newpath, unsigned int flags)
{
	return ltp_syscall(__NR_renameat2, olddirfd, oldpath, newdirfd,
						newpath, flags);
}
#endif

#endif /* RENAMEAT2_H */
