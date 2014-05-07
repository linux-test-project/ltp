/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef READDIR_H
#define READDIR_H

#include <limits.h>

struct old_linux_dirent {
	long  d_ino;			/* inode number */
	off_t d_off;			/* offset to this old_linux_dirent */
	unsigned short d_reclen;	/* length of this d_name */
	char  d_name[NAME_MAX+1];	/* filename (null-terminated) */
};

#endif /* READDIR_H */
