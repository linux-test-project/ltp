/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: threading.h,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: threading.h,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef THREADING_H
#define THREADING_H

#ifdef WIN32
#include <windows.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "defs.h"
#include "globals.h"
#include "main.h"
#include "threading.h"

#define MAX_THREADS 32		  /* max number of threads, reader/writer, per test */
 
typedef struct thread_struct {
#ifdef WIN32
	HANDLE hThread;				/* handle to thread */
#else
	pthread_t hThread;			/* thread */
	BOOL bCanBeJoined;
#endif
	struct thread_struct *next;	/* pointer to next thread */
} thread_struct_t;

void clean_up(void *);
void CreateChild(child_args_t *);

#endif /* THREADING_H */
