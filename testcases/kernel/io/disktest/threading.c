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
* $Id: threading.c,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: threading.c,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
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

#ifdef WIN32
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
* Upon exit of a child, the parent will be called to
* clean up its child zombies.  In most cases this will
* be called using the SIGCHLD signal passed back to the
* parent, however, if more then one child dies at a time
* then we have to try and clean up all child zombies.
* This is done with the while loop.
*
* this cleans up the shared memory only.  Need to learn
* what to do about other resources like process allocated buffers
*/
void clean_up(void *arg)
{
	extern thread_struct_t *pThreads;	/* Global List of child processes */
	extern unsigned short kids;			/* global number of current child processes */
	BOOL bFound = TRUE;
	thread_struct_t *pTmpThread = NULL, *pTmpThreadLast = NULL;
 
#ifdef WIN32
	DWORD dwExitCode;
	bFound = TRUE;
 
	while(bFound) {
		if(pThreads == NULL) break;
		pTmpThread = pThreads->next;
		pTmpThreadLast = pThreads;
		GetExitCodeThread(pThreads->hThread, &dwExitCode);
		if(dwExitCode != STILL_ACTIVE) { /* remove Thread from list, first in list */
			pThreads = pTmpThread;
			free(pTmpThreadLast);
			kids--;
		} else {/* look somewhere else */
			bFound = FALSE;
			while((pTmpThread != NULL) && (!bFound)) {
				GetExitCodeThread(pTmpThread->hThread, &dwExitCode);
				if(dwExitCode != STILL_ACTIVE) { /* remove Thread from list, others */
					pTmpThreadLast->next = pTmpThread->next;
					free(pTmpThread);
					kids--;
					bFound = TRUE;
				} else {
					pTmpThreadLast = pTmpThread;
					pTmpThread = pTmpThread->next;
				}
			}
		}
	}
#else
	pthread_t calling_thread;
	bFound = FALSE;

	if(arg == NULL) { /* called as part of main */
		/* look through list for threads that can be joined */
		bFound = TRUE;
		while(bFound) {
			if(pThreads == NULL) break;
			pTmpThread = pThreads->next;
			pTmpThreadLast = pThreads;
			if(pThreads->bCanBeJoined) {
				pthread_join(pThreads->hThread, NULL);
				pThreads = pTmpThread;
				free(pTmpThreadLast);
				kids--;
			} else {/* look somewhere else */
				bFound = FALSE;
				while((pTmpThread != NULL) && (!bFound)) {
					if(pTmpThread->bCanBeJoined) {
						pthread_join(pTmpThread->hThread, NULL);
						pTmpThreadLast->next = pTmpThread->next;
						free(pTmpThread);
						kids--;
						bFound = TRUE;
					} else {
						pTmpThreadLast = pTmpThread;
						pTmpThread = pTmpThread->next;
					}
				}
			}
		}
	} else { /* this is a thread dieing */
		bFound = FALSE;
		calling_thread = pthread_self();
		pTmpThread = pThreads;
		while((pTmpThread != NULL) && (!bFound)) {
			if(pTmpThread->hThread == calling_thread) {
				pTmpThread->bCanBeJoined = TRUE;
				bFound = TRUE;
			} else {
				pTmpThread = pTmpThread->next;
			}
		}
	}
#endif
}

/*
* This function will create children for us based on the action specified
* during the call.  if we cannot create a child, we fail and exit with
* errno as the exit status. What do we do about shared resources on failure???
*/
void CreateChild(child_args_t *args)
{
	extern thread_struct_t *pThreads;	/* Global List of child processes */
	extern unsigned short kids;			/* global number of current child processes */
	thread_struct_t *pNewThread, *pTmpThread;

#ifdef WIN32
	HANDLE hTmpThread;
 
	if((hTmpThread = CreateThread(NULL, 0, ChildMain, args, 0, NULL)) == NULL) {
		pMsg(ERR, "Failed trying to create thread with error code %u\n", GetLastError());
		pMsg(INFO, "Total Number of Threads created was %u\n", kids);
		exit(GetLastError());
	}
#else
	pthread_t hTmpThread;
 
	if(pthread_create(&hTmpThread, NULL, ChildMain, args) != 0) {
		pMsg(ERR, "Could not create child thread...\n");
		pMsg(INFO, "Total Number of Threads created was %u\n", kids);
		exit(1);
	}
#endif
	pTmpThread = pThreads;
	pNewThread = (thread_struct_t *) malloc(sizeof(thread_struct_t));
	memset(pNewThread, 0, sizeof(thread_struct_t));
	if(pThreads == NULL) { /* first thread */
		pThreads = pNewThread;
		pThreads->next = NULL;
		pThreads->hThread = hTmpThread;
	} else { /* not the first so add to end, no sorting */
		pTmpThread = pThreads;
		while(pTmpThread->next != NULL) pTmpThread = pTmpThread->next;
		pTmpThread->next = pNewThread;
		pTmpThread->next->hThread = hTmpThread;
		pTmpThread->next->next = NULL;
	}
	kids++;
	pTmpThread = pThreads;
}
