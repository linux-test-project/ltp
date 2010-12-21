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
 *
 * $Id: defs.h,v 1.5 2008/02/14 08:22:22 subrata_modak Exp $
 * $Log: defs.h,v $
 * Revision 1.5  2008/02/14 08:22:22  subrata_modak
 * Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
 *
 * Revision 1.5  2006/04/21 23:10:43  yardleyb
 * Major updates for v1_3_3 of disktest.  View README for details.
 *
 * Revision 1.4  2005/10/12 23:13:35  yardleyb
 * Updates to code to support new function in disktest version 1.3.x.
 * Actual changes are recorded in the README
 *
 * Revision 1.3  2002/03/30 01:32:14  yardleyb
 * Major Changes:
 *
 * Added Dumping routines for
 * data miscompares,
 *
 * Updated performance output
 * based on command line.  Gave
 * one decimal in MB/s output.
 *
 * Rewrote -pL IO routine to show
 * correct stats.  Now show pass count
 * when using -C.
 *
 * Minor Changes:
 *
 * Code cleanup to remove the plethera
 * if #ifdef for windows/unix functional
 * differences.
 *
 * Revision 1.2  2002/02/21 19:37:34  yardleyb
 * Added license and header info
 *
 * Revision 1.1  2001/12/04 18:52:33  yardleyb
 * Checkin of new source files and removal
 * of outdated source
 *
 */
#ifndef _DEFS_H
#define _DEFS_H 1

#include "sys/types.h"

#ifdef WINDOWS
#include <windows.h>
#define ALLOC(size) HeapAlloc(GetProcessHeap(), 0, size)
#define RESIZE(mem, size) HeapReAlloc(GetProcessHeap(), 0, mem, size)
#define FREE(mem) HeapFree(GetProcessHeap(), 0, mem)
#define GETPID()	_getpid()
#define GETLASTERROR() GetLastError()
#define INVALID_FD(fd) (fd == INVALID_HANDLE_VALUE)

typedef __int64 OFF_T;
typedef int pid_t;

#else
#include <stdlib.h>
#define ALLOC(size) malloc(size)
#define RESIZE(mem, size) realloc(mem, size)
#define FREE(mem) free(mem)

#define GETPID()	getpid()
#define GETLASTERROR() errno
#define INVALID_FD(fd) (fd == -1)

#define TRUE 1
#define FALSE 0

typedef int BOOL;
typedef void * HANDLE;

/* typedef off_t OFF_T; */
typedef long long int OFF_T;

#endif /* WINDOWS */

typedef enum op {
	WRITER,READER,NONE,RETRY
} op_t;

typedef struct action {
	op_t    oper;
	unsigned long trsiz;
	OFF_T   lba;
} action_t;

#endif /* _DEFS_H */


