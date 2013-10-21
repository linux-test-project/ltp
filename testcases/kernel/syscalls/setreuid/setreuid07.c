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
 * The same test is done in a fork to check if new UIDs are correctly
 * passed to the son.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"
#include "compat_16.h"

TCID_DEFINE(setreuid07);
int TST_TOTAL = 1;

static char testfile[256] = "";
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
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

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

	cleanup();
	tst_exit();
}

static void do_master_child(void)
{
	int lc;
	int pid;
	int status;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int tst_fd;

		tst_count = 0;

		if (SETREUID(NULL, 0, ltpuser->pw_uid) == -1) {
			perror("setreuid failed");
			exit(1);
		}

		/* Test 1: Check the process with new uid cannot open the file
		 *         with RDWR permissions.
		 */
		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			printf("open succeeded unexpectedly\n");
			close(tst_fd);
			exit(1);
		}

		if (TEST_ERRNO == EACCES) {
			printf("open failed with EACCES as expected\n");
		} else {
			perror("open failed unexpectedly");
			exit(1);
		}

		/* Test 2: Check a son process cannot open the file
		 *         with RDWR permissions.
		 */
		pid = FORK_OR_VFORK();
		if (pid < 0)
			tst_brkm(TBROK, cleanup, "Fork failed");

		if (pid == 0) {
			int tst_fd2;

			/* Test to open the file in son process */
			TEST(tst_fd2 = open(testfile, O_RDWR));

			if (TEST_RETURN != -1) {
				printf("call succeeded unexpectedly\n");
				close(tst_fd2);
				exit(1);
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == EACCES) {
				printf("open failed with EACCES as expected\n");
				exit(0);
			} else {
				printf("open failed unexpectedly\n");
				exit(1);
			}
		} else {
			/* Wait for son completion */
			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid failed");
				exit(1);
			}
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
				exit(WEXITSTATUS(status));
		}

		/* Test 3: Fallback to initial uid and check we can again open
		 *         the file with RDWR permissions.
		 */
		tst_count++;
		if (SETREUID(NULL, 0, 0) == -1) {
			perror("setreuid failed");
			exit(1);
		}

		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN == -1) {
			perror("open failed unexpectedly");
			exit(1);
		} else {
			printf("open call succeeded\n");
			close(tst_fd);
		}
	}
	exit(0);
}

static void setup(void)
{
	tst_require_root(NULL);

	ltpuser = getpwnam("nobody");
	if (ltpuser == NULL)
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");

	tst_tmpdir();

	sprintf(testfile, "setreuid07file%d.tst", getpid());

	/* Create test file */
	fd = open(testfile, O_CREAT | O_RDWR, 0644);
	if (fd < 0)
		tst_brkm(TBROK | TERRNO,
			cleanup, "cannot create test file");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);

	TEST_CLEANUP;

	tst_rmdir();
}
