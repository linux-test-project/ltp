/*****************************************************************************
 * Copyright (c) Kerlabs 2008.                                               *
 * Copyright (c) International Business Machines  Corp., 2008                *
 * Created by Renaud Lottiaux                                                *
 *                                                                           *
 * This program is free software;  you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                 *
 * the GNU General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program;  if not, write to the Free Software Foundation,  *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 *****************************************************************************/

/*
 * Check if setfsuid behaves correctly with file permissions.
 * The test creates a file as ROOT with permissions 0644, does a setfsuid
 * and then tries to open the file with RDWR permissions.
 * The same test is done in a fork to check if new UIDs are correctly
 * passed to the son.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

TCID_DEFINE(setfsuid04);
int TST_TOTAL = 1;

static char nobody_uid[] = "nobody";
static char testfile[] = "setfsuid04_testfile";
static struct passwd *ltpuser;

static int fd = -1;

static void setup(void);
static void cleanup(void);
static void do_master_child(void);

int main(int ac, char **av)
{
	pid_t pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK, cleanup, "Fork failed");

	if (pid == 0)
		do_master_child();

	tst_record_childstatus(cleanup, pid);

	cleanup();
	tst_exit();
}

static void do_master_child(void)
{
	int pid;
	int status;

	if (SETFSUID(NULL, ltpuser->pw_uid) == -1) {
		perror("setfsuid failed");
		exit(TFAIL);
	}

	/* Test 1: Check the process with new uid cannot open the file
	 *         with RDWR permissions.
	 */
	TEST(open(testfile, O_RDWR));

	if (TEST_RETURN != -1) {
		close(TEST_RETURN);
		printf("open succeeded unexpectedly\n");
		exit(TFAIL);
	}

	if (TEST_ERRNO == EACCES) {
		printf("open failed with EACCESS as expected\n");
	} else {
		printf("open returned unexpected errno - %d\n", TEST_ERRNO);
		exit(TFAIL);
	}

	/* Test 2: Check a son process cannot open the file
	 *         with RDWR permissions.
	 */
	pid = FORK_OR_VFORK();
	if (pid < 0) {
		perror("Fork failed");
		exit(TFAIL);
	}

	if (pid == 0) {
		/* Test to open the file in son process */
		TEST(open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			close(TEST_RETURN);
			printf("open succeeded unexpectedly\n");
			exit(TFAIL);
		}

		if (TEST_ERRNO == EACCES) {
			printf("open failed with EACCESS as expected\n");
		} else {
			printf("open returned unexpected errno - %d\n",
			       TEST_ERRNO);
			exit(TFAIL);
		}
	} else {
		/* Wait for son completion */
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid failed");
			exit(TFAIL);
		}

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			exit(WEXITSTATUS(status));
	}

	/* Test 3: Fallback to initial uid and check we can again open
	 *         the file with RDWR permissions.
	 */
	tst_count++;
	if (SETFSUID(NULL, 0) == -1) {
		perror("setfsuid failed");
		exit(TFAIL);
	}

	TEST(open(testfile, O_RDWR));

	if (TEST_RETURN == -1) {
		perror("open failed unexpectedly");
		exit(TFAIL);
	} else {
		printf("open call succeeded\n");
		close(TEST_RETURN);
	}
	exit(TPASS);
}

static void setup(void)
{
	tst_require_root();

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK, cleanup, "getpwnam failed for user id %s",
			nobody_uid);

	UID16_CHECK(ltpuser->pw_uid, setfsuid, cleanup);

	tst_tmpdir();

	/* Create test file */
	fd = SAFE_OPEN(cleanup, testfile, O_CREAT | O_RDWR, 0644);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);
	tst_rmdir();
}
