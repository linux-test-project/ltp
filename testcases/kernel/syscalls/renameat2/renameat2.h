/*
 * Copyright (C) 2015 Cedric Hnyda chnyda@suse.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 */

#ifndef RENAMEAT2_H
#define RENAMEAT2_H

#include <sys/types.h>
#include "config.h"
#include "linux_syscall_numbers.h"

#if !defined(HAVE_RENAMEAT2)
int renameat2(int olddirfd, const char *oldpath, int newdirfd,
				const char *newpath, unsigned int flags)
{
	return ltp_syscall(__NR_renameat2, olddirfd, oldpath, newdirfd,
						newpath, flags);
}
#endif

#endif /* RENAMEAT2_H */
