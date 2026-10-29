// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*
 * common definitions for the IPC system calls.
 */

#ifndef TSE_NEWIPC_H__
#define TSE_NEWIPC_H__	1

#include <time.h>
#include <sys/types.h>

#define MSG_RD	0400
#define MSG_WR	0200
#define MSG_RW	(MSG_RD | MSG_WR)
#define MSGSIZE	1024
#define MSGTYPE	1
#define NR_MSGQUEUES	16

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
#define GET_USED_ARRAYS() \
	get_used_sysvipc(__FILE__, __LINE__, "/proc/sysvipc/sem")

void *probe_free_addr(const char *file, const int lineno);
#define PROBE_FREE_ADDR() \
	probe_free_addr(__FILE__, __LINE__)

#endif /* tse_newipc.h */
