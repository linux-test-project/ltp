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

#ifndef __LAPI_FCNTL_H__
#define __LAPI_FCNTL_H__

#ifndef O_CLOEXEC
# define O_CLOEXEC 02000000
#endif

#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC 1030
#endif

#ifndef F_SETPIPE_SZ
# define F_SETPIPE_SZ 1031
#endif

#ifndef F_GETPIPE_SZ
# define F_GETPIPE_SZ 1032
#endif

#ifndef F_OWNER_PGRP
# define F_OWNER_PGRP 2
#endif

#ifndef AT_FDCWD
# define AT_FDCWD -100
#endif

#ifndef AT_SYMLINK_FOLLOW
# define AT_SYMLINK_FOLLOW 0x400
#endif

#ifndef AT_SYMLINK_NOFOLLOW
# define AT_SYMLINK_NOFOLLOW 0x100
#endif

#ifndef AT_REMOVEDIR
# define AT_REMOVEDIR 0x200
#endif

#ifndef O_NOATIME
# define O_NOATIME 01000000
#endif

#ifndef O_PATH
# ifdef __sparc__
#  define O_PATH 0x1000000
# else
#  define O_PATH 010000000
# endif
#endif

#ifndef FALLOC_FL_KEEP_SIZE
# define FALLOC_FL_KEEP_SIZE 1
#endif

#ifndef RENAME_NOREPLACE
# define RENAME_NOREPLACE	(1 << 0)
#endif

#ifndef RENAME_EXCHANGE
# define RENAME_EXCHANGE		(1 << 1)
#endif

#ifndef RENAME_WHITEOUT
# define RENAME_WHITEOUT		(1 << 2)
#endif

/* splice, vmsplice, tee */

#ifndef SPLICE_F_NONBLOCK
# define SPLICE_F_NONBLOCK 2
#endif

#include "lapi/splice.h"
#include "lapi/vmsplice.h"
#include "lapi/tee.h"

#endif /* __LAPI_FCNTL_H__ */
