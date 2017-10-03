/*************************************************************************/
/* Copyright (c) Crackerjack Project., 2007				 */
/*									 */
/* This program is free software;  you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 2 of the License, or	 */
/* (at your option) any later version.					 */
/*									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		 */
/* the GNU General Public License for more details.			 */
/*									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program;  if not, write to the Free Software	       	 */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA*/
/*									 */
/*************************************************************************/
/*************************************************************************/
/*									 */
/* File:	unshare01.c					   	 */
/*									 */
/* Description: This tests the unshare() syscall.		      	 */
/*	     unshare() allows a process to disassociate parts of its	 */
/*		execution context that are currently being shared with other 	*/
/*		processes. Part of the execution context, such as the namespace	*/
/*		,is shared implicitly when a new process is created using 	*/
/*		fork(2) or vfork(2), while other parts, such as virtual memory	*/
/*		, may be shared by explicit request when creating a process 	*/
/*		using clone(2).							*/
/*										*/
/*		The main use of unshare() is to allow a process to control its	*/
/*		shared execution context without creating a new process.	*/
/*		 								*/
/*										*/
/*		The flags argument is a bit mask that specifies which parts of	*/
/*		the execution context should be unshared. This argument is 	*/
/*		specified by ORing together zero or more of the following cons-	*/
/*		tants:								*/
/*										*/
/*		CLONE_FILES:							*/
/*		    	Reverse the effect of the clone(2) CLONE_FILES flag. 	*/
/*			Unshare	the file descriptor table, so that the calling 	*/
/*			process no longer shares its file descriptors with any 	*/
/*			other process.						*/
/*		CLONE_FS:							*/
/*			Reverse the effect of the clone(2) CLONE_FS flag.Unshare*/
/*			file system attributes, so that the calling process no 	*/
/*			longer shares its root directory, current directory, or	*/
/*			umask attributes with any other process.		*/
/*		CLONE_NEWNS:							*/
/*		       This flag has the same effect as the clone(2) CLONE_NEWNS*/
/*			flag. Unshare the namespace, so that the calling process*/
/*			has a private copy of its namespacei which is not shared*/
/*			with any other process. Specifying this flag automat-	*/
/*			ically implies CLONE_FS as well. 			*/
/*										*/
/*		If flags is specified as zero, then unshare() is a no-op; no 	*/
/*		changes are made to the calling process's execution context. 	*/
/*									       	*/
/* Usage:  <for command-line>						 	*/
/* unshare01 [-c n] [-e][-i n] [-I x] [-p x] [-t]		     		*/
/*      where,  -c n : Run n copies concurrently.			     	*/
/*	      -e   : Turn on errno logging.				 	*/
/*	      -i n : Execute test n times.				  	*/
/*	      -I x : Execute test for x seconds.			    	*/
/*	      -P x : Pause for x seconds between iterations.			*/
/*	      -t   : Turn on syscall timing.					*/
/*									    	*/
/* Total Tests: 1							     	*/
/*									    	*/
/* Test Name:   unshare01					     		*/
/* History:     Porting from Crackerjack to LTP is done by		    	*/
/*	      Manas Kumar Nayak maknayak@in.ibm.com>				*/
/********************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sched.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <sys/param.h>
#include <stdio.h>

#include "test.h"
#include "safe_macros.h"
#include "config.h"

char *TCID = "unshare01";
int testno;
int TST_TOTAL = 1;

#ifdef HAVE_UNSHARE

/* Extern Global Functions */
/******************************************************************************/
/*									    */
/* Function:    cleanup						       */
/*									    */
/* Description: Performs all one time clean up for this test on successful    */
/*	      completion,  premature exit or  failure. Closes all temporary */
/*	      files, removes all temporary directories exits the test with  */
/*	      appropriate return code by calling tst_exit() function.       */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*	      On success - Exits calling tst_exit(). With '0' return code.  */
/*									    */
/******************************************************************************/
void cleanup(void)
{

	tst_rmdir();
}

/* Local  Functions */
/******************************************************************************/
/*									    */
/* Function:    setup							 */
/*									    */
/* Description: Performs all one time setup for this test. This function is   */
/*	      typically used to capture signals, create temporary dirs      */
/*	      and temporary files that may be used in the course of this    */
/*	      test.							 */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits by calling cleanup().		      */
/*	      On success - returns 0.				       */
/*									    */
/******************************************************************************/
void setup(void)
{
	tst_require_root();

	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}

int main(int ac, char **av)
{
	pid_t pid1;
	int lc;
	int rval;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;
		for (testno = 0; testno < TST_TOTAL; ++testno) {

			pid1 = fork();	//call to fork()
			if (pid1 == -1) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "fork failed");
			} else if (pid1 == 0) {
				switch (unshare(CLONE_FILES)) {
				case 0:
					printf("unshare with CLONE_FILES call "
					       "succeeded\n");
					rval = 0;
					break;
				case -1:
					if (errno == ENOSYS)
						rval = 1;
					else {
						perror("unshare failed");
						rval = 2;
					}
				}
				exit(rval);
			} else {
				SAFE_WAIT(cleanup, &rval);
				if (rval != 0 && WIFEXITED(rval)) {
					switch (WEXITSTATUS(rval)) {
					case 1:
						tst_brkm(TCONF, cleanup,
							 "unshare not supported in "
							 "kernel");
						break;
					default:
						tst_brkm(TFAIL, cleanup,
							 "unshare failed");
					}
				}
			}

			pid1 = fork();
			if (pid1 == -1) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "fork failed");
			} else if (pid1 == 0) {
				switch (unshare(CLONE_FS)) {
				case 0:
					printf("unshare with CLONE_FS call "
					       "succeeded\n");
					rval = 0;
					break;
				case -1:
					if (errno == ENOSYS)
						rval = 1;
					else {
						perror("unshare failed");
						rval = 2;
					}
				}
				exit(rval);
			} else {
				SAFE_WAIT(cleanup, &rval);
				if (rval != 0 && WIFEXITED(rval)) {
					switch (WEXITSTATUS(rval)) {
					case 1:
						tst_brkm(TCONF, cleanup,
							 "unshare not supported in "
							 "kernel");
						break;
					default:
						tst_brkm(TFAIL, cleanup,
							 "unshare failed");
					}
				}
			}

			pid1 = fork();
			if (pid1 == -1) {
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "fork() failed.");
			} else if (pid1 == 0) {
				switch (unshare(CLONE_NEWNS)) {
				case 0:
					printf("unshare call with CLONE_NEWNS "
					       "succeeded\n");
					rval = 0;
					break;
				case -1:
					if (errno == ENOSYS)
						rval = 1;
					else {
						perror("unshare failed");
						rval = 2;
					}
				}
				exit(rval);
			} else {
				SAFE_WAIT(cleanup, &rval);
				if (rval != 0 && WIFEXITED(rval)) {
					switch (WEXITSTATUS(rval)) {
					case 1:
						tst_brkm(TCONF, cleanup,
							 "unshare not supported in "
							 "kernel");
						break;
					default:
						tst_brkm(TFAIL, cleanup,
							 "unshare failed");
					}

				}

			}

		}

	}
	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "unshare is undefined.");
}
#endif
