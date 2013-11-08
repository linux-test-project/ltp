/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
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

#ifndef __LIBMSGCTL_H__
#define __LIBMSGCTL_H__

#define FAIL	1
#define PASS	0

struct mbuffer {
	long type;
	struct {
		char len;
		char pbytes[99];
	} data;
};

int doreader(long key, int tid, long type, int child, int nreps);
int dowriter(long key, int tid, long type, int child, int nreps);
int fill_buffer(char *buf, char val, int size);
int verify(char *buf, char val, int size, int child);

#endif /*__LIBMSGCTL_H__ */
