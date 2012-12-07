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
* $Id: threading.c,v 1.7 2009/02/26 12:14:53 subrata_modak Exp $
* $Log: threading.c,v $
* Revision 1.7  2009/02/26 12:14:53  subrata_modak
* Clean Trailing Tab: Signed-off-by: Michal Simek <monstr@monstr.eu>.
*
* Revision 1.6  2009/02/26 12:02:23  subrata_modak
* Clear Trailing Whitespace. Signed-off-by: Michal Simek <monstr@monstr.eu>.
*
* Revision 1.5  2008/02/14 08:22:24  subrata_modak
* Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
*
* Revision 1.11  2006/04/21 23:10:43  yardleyb
* Major updates for v1_3_3 of disktest.  View README for details.
*
* Revision 1.10  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.9  2004/11/19 21:45:12  yardleyb
* Fixed issue with code added for -F option.  Cased disktest
* to SEG FAULT when cleaning up threads.
*
* Revision 1.8  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
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
#include "sfunc.h"
#include "main.h"
#include "childmain.h"
#include "threading.h"

/*
 * This routine will sit waiting for all threads to exit.  In
 * unix, this is done through pthread_join.  In Windows we
 * use a sleeping loop.
 */
void cleanUpTestChildren(test_ll_t * test)
{
	thread_struct_t *pTmpThread = NULL, *pTmpThreadLast = NULL;

	while (test->env->pThreads) {
		pTmpThread = test->env->pThreads->next;
		pTmpThreadLast = test->env->pThreads;

		closeThread(pTmpThreadLast->hThread);

		test->env->pThreads = pTmpThread;
		FREE(pTmpThreadLast);
		test->env->kids--;
	}
}

/*
 * This function will create children for us based on the action specified
 * during the call.  if we cannot create a child, we fail and exit with
 * errno as the exit status.
 */
void CreateTestChild(void *function, test_ll_t * test)
{
	thread_struct_t *pNewThread;
	hThread_t hTmpThread;

	hTmpThread = spawnThread(function, test);

	if (ISTHREADVALID(hTmpThread)) {
		if ((pNewThread =
		     (thread_struct_t *) ALLOC(sizeof(thread_struct_t))) ==
		    NULL) {
			pMsg(ERR, test->args,
			     "%d : Could not allocate memory for child thread...\n",
			     GETLASTERROR());
			exit(GETLASTERROR());
		}
		test->env->kids++;
		memset(pNewThread, 0, sizeof(thread_struct_t));
		pNewThread->next = test->env->pThreads;
		test->env->pThreads = pNewThread;
		test->env->pThreads->hThread = hTmpThread;
	} else {
		pMsg(ERR, test->args,
		     "%d : Could not create all child threads.\n",
		     GETLASTERROR());
		pMsg(INFO, test->args,
		     "Total Number of Threads created was %u\n",
		     test->env->kids);
		exit(GETLASTERROR());
	}
}

void createChild(void *function, test_ll_t * test)
{
	hThread_t hTmpThread;

	hTmpThread = spawnThread(function, test);

	if (ISTHREADVALID(hTmpThread)) {
		test->hThread = hTmpThread;
	} else {
		pMsg(ERR, test->args, "%d : Could not create child thread...\n",
		     GETLASTERROR());
		exit(GETLASTERROR());
	}
}

void cleanUp(test_ll_t * test)
{
	test_ll_t *pTmpTest = test;
	test_ll_t *pLastTest;
	while (pTmpTest != NULL) {
		pLastTest = pTmpTest;
		pTmpTest = pTmpTest->next;
		closeThread(pLastTest->hThread);
		FREE(pLastTest->env->action_list);
		FREE(pLastTest->args);
		FREE(pLastTest->env);
		FREE(pLastTest);
	}
}

hThread_t spawnThread(void *function, void *param)
{
	hThread_t hTmpThread;

#ifdef WINDOWS
	hTmpThread = CreateThread(NULL, 0, function, param, 0, NULL);
#else
	if (pthread_create(&hTmpThread, NULL, function, param) != 0) {
		hTmpThread = 0;
	}
#endif

	return hTmpThread;
}

void closeThread(hThread_t hThread)
{
#ifdef WINDOWS
	DWORD dwExitCode = 0;

	do {
		GetExitCodeThread(hThread, &dwExitCode);
		/*
		 * Sleep(0) will force this thread to
		 * relinquish the remainder of its time slice
		 */
		if (dwExitCode == STILL_ACTIVE)
			Sleep(0);
	} while (dwExitCode == STILL_ACTIVE);
#else
	pthread_join(hThread, NULL);
#endif
}
