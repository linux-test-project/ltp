/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef VMSPLICE_H
#define VMSPLICE_H

#include "config.h"
#include "lapi/syscalls.h"

#include "lapi/iovec.h"

#if !defined(HAVE_VMSPLICE)
ssize_t vmsplice(int fd, const struct iovec *iov,
	         unsigned long nr_segs, unsigned int flags)
{
	return tst_syscall(__NR_vmsplice, fd, iov, nr_segs, flags);
}
#endif

#endif /* VMSPLICE_H */
