/*
* Tapetest
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
* $Id: threading.h,v 1.7 2008/02/14 08:22:24 subrata_modak Exp $
* $Log: threading.h,v $
* Revision 1.7  2008/02/14 08:22:24  subrata_modak
* Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
*
* Revision 1.9  2008/02/07 17:37:26  yardleyb
* "yxu@suse.de" corrected the way by which pthread_exit() handles pointer argument
*
* Revision 1.8  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.7  2004/11/19 21:45:12  yardleyb
* Fixed issue with code added for -F option.  Cased disktest
* to SEG FAULT when cleaning up threads.
*
* Revision 1.6  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.5  2002/03/30 01:32:14  yardleyb
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
* Revision 1.4  2002/02/28 04:25:45  yardleyb
* reworked threading code
* made locking code a macro.
*
* Revision 1.3  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
*
* Revision 1.2  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef THREADING_H
#define THREADING_H 1

#ifdef WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "defs.h"
#include "main.h"

#define MAX_THREADS 65536		/* max number of threads, reader/writer, per test */

#ifdef WINDOWS
#define LOCK(Mutex) WaitForSingleObject((void *) Mutex, INFINITE)
#define UNLOCK(Mutex) ReleaseMutex((void *) Mutex)
#define TEXIT(errno) ExitThread(errno); return(errno)
#define ISTHREADVALID(thread) (thread != NULL)
#else
#define LOCK(Mutex) \
		pthread_cleanup_push((void *) pthread_mutex_unlock, (void *) &Mutex); \
		pthread_mutex_lock(&Mutex)
#define UNLOCK(Mutex) \
		pthread_mutex_unlock(&Mutex); \
		pthread_cleanup_pop(0)
#define TEXIT(errno) pthread_exit((void*)errno)
#define ISTHREADVALID(thread) (thread != 0)
#endif

void cleanUpTestChildren(test_ll_t *);
void CreateTestChild(void *, test_ll_t *);
hThread_t spawnThread(void *, void *);
void closeThread(hThread_t);
void createChild(void *, test_ll_t *);
void cleanUp(test_ll_t *);

#endif /* THREADING_H */
