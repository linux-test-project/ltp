/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

/*
 * common definitions for the IPC system calls.
 */

#ifndef __LIBNEWIPC_H
#define __LIBNEWIPC_H	1

#include <time.h>
#include <sys/types.h>

#define MSG_RD	0400
#define MSG_WR	0200
#define MSG_RW	(MSG_RD | MSG_WR)
#define MSGSIZE	1024
#define MSGTYPE	1
#define NR_MSGQUEUES	16
#define min(a, b)	(((a) < (b)) ? (a) : (b))

#define SEM_RD	0400
#define SEM_ALT	0200
#define SEM_RA	(SEM_RD | SEM_ALT)
#define PSEMS	10

#define SHM_RD	0400
#define SHM_WR	0200
#define SHM_RW	(SHM_RD | SHM_WR)
#define SHM_SIZE	2048
#define INT_SIZE	4
#define MODE_MASK	0x01FF

key_t getipckey(const char *file, const int lineno);
#define GETIPCKEY() \
	getipckey(__FILE__, __LINE__)

int get_used_sysvipc(const char *file, const int lineno, const char *sysvipc_file);
#define GET_USED_QUEUES() \
	get_used_sysvipc(__FILE__, __LINE__, "/proc/sysvipc/msg")
#define GET_USED_SEGMENTS() \
	get_used_sysvipc(__FILE__, __LINE__, "/proc/sysvipc/shm")

void *probe_free_addr(const char *file, const int lineno);
#define PROBE_FREE_ADDR() \
	probe_free_addr(__FILE__, __LINE__)

time_t get_ipc_timestamp(void);

#endif /* newlibipc.h */
