/******************************************************************************/
/* Copyright (c) Kerlabs 2008.                                                */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * NAME
 * 	setresuid04.c
 *
 * DESCRIPTION
 * 	Check if setresuid behaves correctly with file permissions.
 *      The test creates a file as ROOT with permissions 0644, does a setresuid
 *      and then tries to open the file with RDWR permissions.
 *      The same test is done in a thread to check if process UIDs correctly
 *      passed to new thread.
 *
 * USAGE:  <for command-line>
 *  setresuid04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Created by Renaud Lottiaux
 *
 * RESTRICTIONS
 * 	Must be run as root.
 */
#define _GNU_SOURCE 1
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"
#include <pwd.h>
#include "compat_16.h"
#include <pthread.h>

TCID_DEFINE(setresuid04);
int TST_TOTAL = 1;
char nobody_uid[] = "nobody";
char testfile[256] = "";
struct passwd *ltpuser;

int fd = -1;

void setup(void);
void cleanup(void);
void* do_master_thread(void*);
void* do_sub_thread(void*);

int main(int ac, char **av)
{
	pthread_t threadid;

	tst_parse_opts(ac, av, NULL, NULL);
	setup();

	//create a new thread to perform the test
	if(pthread_create(&threadid, NULL, do_master_thread, NULL))
	{
		tst_brkm(TBROK, cleanup, "Thread creation failed");
	}

	// wait for the thread to join
	pthread_join(threadid, NULL);

	// cleanup and exit
	cleanup();
	tst_exit();
}

void *do_sub_thread(void* arg)
{
	int tst_fd2;

	// Test to open the file in thread
	TEST(tst_fd2 = open(testfile, O_RDWR));

	if (TEST_RETURN != -1) {
		tst_resm(TPASS,
			"open call succeeded as expected in thread2");
		close(tst_fd2);
	} else {
		tst_brkm(TBROK, cleanup,
			"open failed unexpectedly in thread2\n");
	}
	pthread_exit(NULL);
}

/*
 * do_master_thread()
 */
void* do_master_thread(void* arg)
{
	int lc;
	pthread_t tid;
	int* result;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int tst_fd;

		/* Reset tst_count in case we are looping */
		tst_count = 0;
		
		/* change effective uid of the process */
		if (SETRESUID(NULL, 0, ltpuser->pw_uid, 0) == -1) {
			tst_brkm(TBROK, cleanup, "setresuid failed");
		}

		/* Test 1: Check the process with new uid cannot open the file
		 *         with RDWR permissions.
		 */
		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			close(tst_fd);
			tst_brkm(TBROK, cleanup, "open succeeded unexpectedly\n");
		}

		if (TEST_ERRNO == EACCES) {
			tst_resm(TPASS, "open failed with EACCES as expected\n");
		} else {
			tst_brkm(TBROK, cleanup, "open failed unexpectedly");
		}

		/* Test 2: Check new thread  can open the file
		 *         with RDWR permissions.
		 */
		if(pthread_create(&tid, NULL, do_sub_thread, NULL) != 0)
		{
			tst_brkm(TBROK, cleanup, "thread creation failed");
		}
		// wait for the thread to complete the test
		pthread_join(tid, NULL);

		/* Test 3: Fallback to initial uid and check we can again open
		 *         the file with RDWR permissions.
		 */
		tst_count++;
		if (SETRESUID(NULL, 0, 0, 0) == -1) {
			tst_brkm(TBROK, cleanup, "setresuid failed");
		}

		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN == -1) {
			tst_brkm(TBROK, cleanup, "open failed unexpectedly");
		} else {
			tst_resm(TPASS, "open call succeeded\n");
			close(tst_fd);
		}
	}
	pthread_exit(NULL);
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	tst_require_root();

	ltpuser = getpwnam(nobody_uid);

	UID16_CHECK(ltpuser->pw_uid, "setresuid", cleanup)

	tst_tmpdir();

	sprintf(testfile, "setresuid04file%d.tst", getpid());

	/* Create test file */
	fd = SAFE_OPEN(cleanup, testfile, O_CREAT | O_RDWR, 0644);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit
 */
void cleanup(void)
{
	close(fd);

	tst_rmdir();

}
