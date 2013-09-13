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
#include "usctest.h"
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
	char *msg;
	int status;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK, cleanup, "Fork failed");

	if (pid == 0)
		do_master_child();

	if (waitpid(pid, &status, 0) == -1)
		tst_resm(TBROK | TERRNO, "waitpid failed");
	if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
		tst_resm(TFAIL, "child process terminated abnormally");
	else
		tst_resm(TPASS, "Test passed");

	cleanup();
	tst_exit();
}

static void do_master_child(void)
{
	int pid;
	int status;

	if (SETFSUID(NULL, ltpuser->pw_uid) == -1) {
		perror("setfsuid failed");
		exit(1);
	}

	/* Test 1: Check the process with new uid cannot open the file
	 *         with RDWR permissions.
	 */
	TEST(open(testfile, O_RDWR));

	if (TEST_RETURN != -1) {
		close(TEST_RETURN);
		printf("open succeeded unexpectedly\n");
		exit(1);
	}

	if (TEST_ERRNO == EACCES) {
		printf("open failed with EACCESS as expected\n");
	} else {
		printf("open returned unexpected errno - %d\n", TEST_ERRNO);
		exit(1);
	}

	/* Test 2: Check a son process cannot open the file
	 *         with RDWR permissions.
	 */
	pid = FORK_OR_VFORK();
	if (pid < 0) {
		perror("Fork failed");
		exit(1);
	}

	if (pid == 0) {
		/* Test to open the file in son process */
		TEST(open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			close(TEST_RETURN);
			printf("open succeeded unexpectedly\n");
			exit(1);
		}

		if (TEST_ERRNO == EACCES) {
			printf("open failed with EACCESS as expected\n");
		} else {
			printf("open returned unexpected errno - %d\n",
			       TEST_ERRNO);
			exit(1);
		}
	} else {
		/* Wait for son completion */
		if (waitpid(pid, &status, 0) == -1) {
			perror("waitpid failed");
			exit(1);
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
		exit(1);
	}

	TEST(open(testfile, O_RDWR));

	if (TEST_RETURN == -1) {
		perror("open failed unexpectedly");
		exit(1);
	} else {
		printf("open call succeeded\n");
		close(TEST_RETURN);
	}
	exit(0);
}

static void setup(void)
{
	tst_require_root(NULL);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK, cleanup, "getpwnam failed for user id %s",
			nobody_uid);

	UID16_CHECK(ltpuser->pw_uid, setfsuid, cleanup);

	tst_tmpdir();

	/* Create test file */
	fd = open(testfile, O_CREAT | O_RDWR, 0644);
	if (fd < 0)
		tst_brkm(TBROK, cleanup, "cannot creat test file");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);
	tst_rmdir();

	TEST_CLEANUP;
}
