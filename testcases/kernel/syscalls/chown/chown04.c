/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: chown04
 *
 * Test Description:
 *   Verify that,
 *   1) chown(2) returns -1 and sets errno to EPERM if the effective user id
 *		 of process does not match the owner of the file and the process
 *		 is not super user.
 *   2) chown(2) returns -1 and sets errno to EACCES if search permission is
 *		 denied on a component of the path prefix.
 *   3) chown(2) returns -1 and sets errno to EFAULT if pathname points
 *		 outside user's accessible address space.
 *   4) chown(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *		 component is too long.
 *   5) chown(2) returns -1 and sets errno to ENOTDIR if the directory
 *		 component in pathname is not a directory.
 *   6) chown(2) returns -1 and sets errno to ENOENT if the specified file
 *		 does not exists.
 *
 * Expected Result:
 *  chown() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *		Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  chown04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *		        -i n : Execute test n times.
 *		        -I x : Execute test for x seconds.
 *		        -P x : Pause for x seconds between iterations.
 *		        -t   : Turn on syscall timing.
 *
 * HISTORY
 *		 07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by 'non-super-user' only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

#define MODE_RWX		 (S_IRWXU|S_IRWXG|S_IRWXO)
#define FILE_MODE		 (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_TEMP		 "testdir_1"
#define TEST_FILE1		 "tfile_1"
#define TEST_FILE2		 (DIR_TEMP "/tfile_2")
#define TEST_FILE3		 "t_file/tfile_3"

void setup1();
void setup2();
void setup3();
void longpath_setup();

char Longpathname[PATH_MAX + 2];
char high_address_node[64];

struct test_case_t {
	char *pathname;
	int exp_errno;
	void (*setupfunc)(void);
} test_cases[] = {
	{ TEST_FILE1, EPERM, setup1 },
	{ TEST_FILE2, EACCES, setup2 },
	{ high_address_node, EFAULT, NULL },
	{ (char *)-1, EFAULT, NULL },
	{ Longpathname, ENAMETOOLONG, longpath_setup},
	{ "", ENOENT, NULL},
	{ TEST_FILE3, ENOTDIR, setup3},
};

char *TCID = "chown04";
int TST_TOTAL = sizeof(test_cases) / sizeof(*test_cases);
int exp_enos[] = { EPERM, EACCES, EFAULT, ENAMETOOLONG, ENOENT, ENOTDIR, 0 };

struct passwd *ltpuser;

char *bad_addr = 0;

void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;
	char *msg;
	char *file_name;
	int i;
	uid_t user_id;
	gid_t group_id;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	user_id = geteuid();
	group_id = getegid();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			file_name = test_cases[i].pathname;

			if (file_name == high_address_node)
				file_name = get_high_address();

			TEST(chown(file_name, user_id, group_id));

			if (TEST_RETURN == 0) {
				tst_resm(TFAIL, "chown succeeded unexpectedly");
				continue;
			} else if (TEST_ERRNO == test_cases[i].exp_errno)
				tst_resm(TPASS|TTERRNO, "chown failed");
			else {
				tst_resm(TFAIL|TTERRNO,
				    "chown failed; expected: %d - %s",
				    test_cases[i].exp_errno,
				    strerror(test_cases[i].exp_errno));
			}
		}
	}

	cleanup();
	tst_exit();

}

void setup()
{
	int i;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	ltpuser = getpwnam("nobody");
	if (ltpuser == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getpwnam(\"nobody\") failed");
	if (seteuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "seteuid(%d) failed",
		    ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX|MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");

	test_cases[3].pathname = bad_addr;

	for (i = 0; i < TST_TOTAL; i++)
		if (test_cases[i].setupfunc != NULL)
			test_cases[i].setupfunc();
}

void setup1()
{
	int fd;
	uid_t old_uid;

	old_uid = geteuid();

	if ((fd = open(TEST_FILE1, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "opening \"%s\" failed",
		    TEST_FILE1);

	if (seteuid(0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(0) failed");

	if (fchown(fd, 0, 0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "fchown failed");

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "closing \"%s\" failed",
		    TEST_FILE1);

	if (seteuid(old_uid) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(%d) failed", old_uid);

}

void setup2()
{
	int fd;
	uid_t old_uid;

	old_uid = geteuid();

	if (seteuid(0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(0) failed");

	if (mkdir(DIR_TEMP, S_IRWXU) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir failed");

	if ((fd = open(TEST_FILE2, O_RDWR|O_CREAT, 0666)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "fchown failed");

	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "closing \"%s\" failed",
		    TEST_FILE2);

	if (seteuid(old_uid) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid(%d) failed", old_uid);

}

void setup3()
{
	int fd;

	if ((fd = open("t_file", O_RDWR|O_CREAT, MODE_RWX)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "opening \"t_file\" failed");
	if (close(fd) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "closing \"t_file\" failed");
}

void longpath_setup()
{
	int i;

	for (i = 0; i <= (PATH_MAX + 1); i++)
		Longpathname[i] = 'a';
}

void cleanup()
{
	TEST_CLEANUP;

	if (seteuid(0) == -1)
		tst_resm(TWARN|TERRNO, "seteuid(0) failed");

	tst_rmdir();

}