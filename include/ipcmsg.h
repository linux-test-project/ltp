/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * ipcmsg.h - common definitions for the IPC message tests.
 */

#ifndef __IPCMSG_H
#define __IPCMSG_H	1

#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "test.h"

void cleanup(void);
void setup(void);

#define MSG_RD  0400            /* read permission for the queue */
#define MSG_WR  0200            /* write permission for the queue */
#define MSG_RW	MSG_RD | MSG_WR

#define MSGSIZE	1024		/* a resonable size for a message */
#define MSGTYPE 1		/* a type ID for a message */

#define NR_MSGQUEUES	16	/* MSGMNI as defined in linux/msg.h */

typedef struct mbuf {		/* a generic message structure */
	long mtype;
	char mtext[MSGSIZE + 1];  /* add 1 here so the message can be 1024   */
} MSGBUF;			  /* characters long with a '\0' termination */

#ifdef LIBIPC
key_t msgkey;                   /* the ftok() generated message key */
#else
extern key_t msgkey;                   /* the ftok() generated message key */
#endif

void init_buf(MSGBUF *, int, int);
void rm_queue(int);

key_t getipckey();
int getuserid(char *);

int get_max_msgqueues(void);
int get_used_msgqueues(void);

#endif /* ipcmsg.h */
