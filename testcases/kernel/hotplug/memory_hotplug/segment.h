/*
 * memtoy:  segment.h - memory segments interface
 */
/*
 *  Copyright (c) 2005 Hewlett-Packard, Inc
 *  All rights reserved.
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _MEMTOY_SEGMENT_H_
#define _MEMTOY_SEGMENT_H_

/*
 * a "memory segment" known to memtoy
 */
typedef enum {
	SEGT_NONE=0,	/* unused segment */
	SEGT_ANON=1,	/* anonymous -- MAP_ANON */
	SEGT_FILE=2,
	SEGT_SHM=3,
	SEGT_NTYPES
} seg_type_t;

struct segment;
typedef struct segment segment_t;

/*
 * range:  optional (offset, length) arguments
 */
typedef struct range {
	off_t  offset;
	size_t length;
} range_t;

struct global_context;

extern void segment_init(struct global_context *);
extern void segment_cleanup(struct global_context *);

extern int segment_register(seg_type_t, char*, range_t*,  int);
extern int segment_show(char*);
extern int segment_remove(char*);
extern int segment_map(char*, range_t*, int);
extern int segment_unmap(char*);
extern int segment_touch(char*, range_t*, int);
extern int segment_mbind(char*, range_t*, int, nodemask_t*, int);
extern int segment_location(char*, range_t*);

#endif
