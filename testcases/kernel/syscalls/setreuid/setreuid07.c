/*
 * Copyright (c) Kerlabs 2008.
 * Copyright (c) International Business Machines  Corp., 2008
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Created by Renaud Lottiaux
 */

/*
 * Check if setreuid behaves correctly with file permissions.
 * The test creates a file as ROOT with permissions 0644, does a setreuid
 * and then tries to open the file with RDWR permissions.
 * The same test is done in a thread to check if process UIDs correctly
 * passed to new thread.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

TCID_DEFINE(setreuid07);
int TST_TOTAL = 1;

static char testfile[256] = "";
static struct passwd *ltpuser;

static int fd = -1;

static void setup(void);
static void cleanup(void);
static void* do_master_thread(void* arg);

int main(int ac, char **av)
{
	pthread_t threadid;
	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	if (pthread_create(&threadid, NULL, do_master_thread, NULL))
	{
		tst_brkm(TBROK, cleanup, "Thread creation failed");
	}
	//wait for the thread to join
	pthread_join(threadid, NULL);

	tst_resm(TPASS, "Test case passed");
	cleanup();
	tst_exit();
}

static void *do_sub_thread(void* arg)
{
	int tst_fd2;

        //Test to open the file in thread2 created by thread1
	TEST(tst_fd2 = open(testfile, O_RDWR));

	if (TEST_RETURN != -1) {
		tst_resm(TPASS,
			"open call succeeded as expected in thread2");
	} else {
		tst_brkm(TBROK, cleanup,
			"open failed unexpectedly in thread2");
	}
	if (tst_fd2 >= 0)
		close(tst_fd2);
	pthread_exit(NULL);
}

static void* do_master_thread(void* arg)
{
	int lc;
	pthread_t tid;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int tst_fd;

		tst_count = 0;

		if (SETREUID(NULL, 0, ltpuser->pw_uid) == -1) {
			tst_brkm(TBROK, cleanup, "setreuid failed");
		}

		/* Test 1: Check the process with new uid cannot open the file
		 *         with RDWR permissions.
		 */
		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			tst_brkm(TBROK, cleanup, "open succeeded unexpectedly");
		}

		if (TEST_ERRNO == EACCES) {
			tst_resm(TPASS, "open failed with EACCES as expected");
		} else {
			tst_brkm(TBROK, cleanup, "open failed unexpectedly");
		}
		if (tst_fd >=0 )
		{
			close(fd);
		}

		/* Test 2: Check a son process cannot open the file
		 *         with RDWR permissions.
		 */

		if (pthread_create(&tid, NULL, do_sub_thread, NULL) != 0)
		{
			tst_brkm(TBROK, cleanup, "Thread creation failed");
		}
	 	// wait for the thread to complete the test
		pthread_join(tid, NULL);

		/* Test 3: Fallback to initial uid and check we can again open
		 *         the file with RDWR permissions.
		 */
		tst_count++;
		if (SETREUID(NULL, 0, 0) == -1) {
			tst_brkm(TBROK, cleanup, "setreuid failed");
		}

		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN == -1) {
			tst_brkm(TBROK, cleanup, "open failed unexpectedly");
		} else {
			tst_resm(TPASS, "open call succeeded");
		}
		if (tst_fd >= 0)
			close(tst_fd);
	}
	pthread_exit(NULL);
}

static void setup(void)
{
	tst_require_root();

	ltpuser = getpwnam("nobody");
	if (ltpuser == NULL)
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");

	tst_tmpdir();

	sprintf(testfile, "setreuid07file%d.tst", getpid());

	/* Create test file */
	fd = SAFE_OPEN(cleanup, testfile, O_CREAT | O_RDWR, 0644);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (fd >= 0)
		close(fd);

	tst_rmdir();
}
