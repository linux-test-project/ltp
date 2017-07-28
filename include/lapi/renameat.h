/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Fujitsu Ltd.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef RENAMEAT_H
#define RENAMEAT_H

#include <sys/types.h>
#include "config.h"
#include "lapi/syscalls.h"

#if !defined(HAVE_RENAMEAT)
int renameat(int olddirfd, const char *oldpath, int newdirfd,
			const char *newpath)
{
	return ltp_syscall(__NR_renameat, olddirfd, oldpath, newdirfd,
					newpath);
}
#endif

#endif /* RENAMEAT_H */
