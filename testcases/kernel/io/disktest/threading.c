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
* $Id: threading.c,v 1.2 2003/04/17 15:21:58 robbiew Exp $
* $Log: threading.c,v $
* Revision 1.2  2003/04/17 15:21:58  robbiew
* Updated to v1.1.10
*
* Revision 1.7  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.6  2002/03/30 01:32:14  yardleyb
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
* Revision 1.5  2002/03/07 03:30:11  yardleyb
* Return errno on thread
* create failure
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

#ifdef WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#endif

#include "defs.h"
#include "globals.h"
#include "sfunc.h"
#include "main.h"
#include "childmain.h"
#include "threading.h"

/*
 * This routine will sit waiting for
 * all threads to exit.  In unix, this
 * is done through pthread_join.  In
 * Windows we use a sleeping loop.
 */
void clean_up(void)
{
	extern thread_struct_t *pThreads;	/* Global List of child processes */
	extern unsigned short kids;			/* global number of current child processes */
	thread_struct_t *pTmpThread = NULL, *pTmpThreadLast = NULL;
 
#ifdef WINDOWS
	DWORD dwExitCode = 0;
#endif

	while(pThreads) {
		pTmpThread = pThreads->next;
		pTmpThreadLast = pThreads;
#ifdef WINDOWS
		do {
			GetExitCodeThread(pThreads->hThread, &dwExitCode);
			/*
			 * Sleep(0) will force this thread to
			 * relinquish the remainder of its time slice
			 */
			if(dwExitCode == STILL_ACTIVE) Sleep(0);
		} while(dwExitCode == STILL_ACTIVE); 
#else
		pthread_join(pThreads->hThread, NULL);
#endif
		pThreads = pTmpThread;
		FREE(pTmpThreadLast);
		kids--;
	}
}

/*
 * This function will create children for us based on the action specified
 * during the call.  if we cannot create a child, we fail and exit with
 * errno as the exit status.
 */
void CreateChild(void *function, child_args_t *args)
{
	extern thread_struct_t *pThreads;	/* Global List of child processes */
	extern unsigned short kids;			/* global number of current child processes */
	thread_struct_t *pNewThread;
#ifdef WINDOWS
	HANDLE hTmpThread;
#else
	pthread_t hTmpThread;
#endif

	kids++;
#ifdef WINDOWS
	if((hTmpThread = CreateThread(NULL, 0, function, args, 0, NULL)) == NULL) {
		kids--;
		pMsg(ERR, "Failed trying to create thread with error code %u\n", GetLastError());
		pMsg(INFO, "Total Number of Threads created was %u\n", kids);
		exit(GetLastError());
	}
#else
	if(pthread_create(&hTmpThread, NULL, function, args) != 0) {
		kids--;
		pMsg(ERR, "Could not create child thread...\n");
		pMsg(INFO, "Total Number of Threads created was %u\n", kids);
		exit(errno);
	}
#endif
	pNewThread = (thread_struct_t *) ALLOC(sizeof(thread_struct_t));
	memset(pNewThread, 0, sizeof(thread_struct_t));
	pNewThread->next = pThreads;
	pThreads = pNewThread;
	pThreads->hThread = hTmpThread;
}

