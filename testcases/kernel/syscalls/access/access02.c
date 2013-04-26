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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that access() succeeds to check the read/write/execute permissions
 *  on a file if the mode argument passed was R_OK/W_OK/X_OK.
 *
 *  Also verify that, access() succeeds to test the accessibility of the file
 *  referred to by symbolic link if the pathname points to a symbolic link.
 *
 *	07/2001 Ported by Wayne Boyer
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEMP_FILE	"temp_file"
#define SYM_FILE	"sym_file"
#define TEST_FILE1	"test_file1"
#define TEST_FILE2	"test_file2"
#define TEST_FILE3	"test_file3"
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define EXE_MODE	0777

char *TCID = "access02";
int TST_TOTAL = 4;

static int fd1, fd2, fd4;
static const char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static int setup1(void);
static int setup2(void);
static int setup3(void);
static int setup4(void);

static struct test_case_t {
	char *pathname;
	mode_t a_mode;
	int (*setupfunc) (void);
} test_cases[] = {
	{TEST_FILE1, R_OK, setup1},
	{TEST_FILE2, W_OK, setup2},
	{TEST_FILE3, X_OK, setup3},
	{SYM_FILE, W_OK, setup4},
};

static void setup(void);
static void cleanup(void);
static int access_verify(int);

int main(int ac, char **av)
{
	int lc;
	int i;
	char *msg;
	mode_t access_mode;
	char *file_name;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			file_name = test_cases[i].pathname;
			access_mode = test_cases[i].a_mode;

			/*
			 * Call access(2) to check the test file
			 * for specified access mode permissions.
			 */
			TEST(access(file_name, access_mode));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO,
					 "access(%s, %#o) failed",
					 file_name, access_mode);
				continue;
			}

			if (STD_FUNCTIONAL_TEST)
				access_verify(i);
			else
				tst_resm(TPASS, "call succeeded");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;

	tst_require_root(NULL);
	tst_sig(FORK, DEF_HANDLER, cleanup);

	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TINFO | TERRNO, NULL, "setuid failed");

	TEST_PAUSE;
	tst_tmpdir();

	for (i = 0; i < TST_TOTAL; i++)
		test_cases[i].setupfunc();
}

/*
 * setup1() - Setup function to test the functionality of access() for
 *	      the access mode argument R_OK.
 *
 *   Creat/open a testfile and write some data into it.
 *   This function returns 0.
 */
static int setup1(void)
{
	char write_buf[] = "abc";

	fd1 = open(TEST_FILE1, O_RDWR | O_CREAT, FILE_MODE);
	if (fd1 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE1, FILE_MODE);
	}

	/* write some data into testfile */
	if (write(fd1, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "write(%s) failed in setup1", TEST_FILE1);
	}

	return 0;
}

/*
 * setup2() - Setup function to test the functionality of access() for
 *	      the access mode argument W_OK.
 *
 *   Creat/open a testfile for writing under temporary directory.
 *   This function returns 0.
 */
static int setup2(void)
{
	fd2 = open(TEST_FILE2, O_RDWR | O_CREAT, FILE_MODE);
	if (fd1 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE2, FILE_MODE);
	}

	return 0;
}

/*
 * setup3() - Setup function to test the functionality of access() for
 *	      the access mode argument X_OK.
 *
 *   Creat/open a testfile and provide execute permissions to it.
 *   This function returns 0.
 */
static int setup3(void)
{
	int fd3;
#ifdef UCLINUX
	char exechead[] = "#!/bin/sh\n";
#endif

	fd3 = open(TEST_FILE3, O_RDWR | O_CREAT, FILE_MODE);
	if (fd3 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEST_FILE3, FILE_MODE);
	}
#ifdef UCLINUX
	if (write(fd3, exechead, sizeof(exechead)) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "write(%s) failed",
			 TEST_FILE3);
	}
#endif

	/* Close the test file created above */
	if (close(fd3) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "close(%s) failed",
			 TEST_FILE3);
	}

	/* Set execute permission bits on the test file. */
	if (chmod(TEST_FILE3, EXE_MODE) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "chmod(%s, %#o) failed",
			 TEST_FILE3, EXE_MODE);
	}

	return 0;
}

/*
 * setup4() - Setup function to test the functionality of access() for
 *	      symbolic link file.
 *
 *   Creat/open a temporary file and close it.
 *   Creat a symbolic link of temporary file.
 *   This function returns 0.
 */
static int setup4(void)
{
	fd4 = open(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE);
	if (fd4 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, %#o) failed",
			 TEMP_FILE, FILE_MODE);
	}

	/* Creat a symbolic link for temporary file */
	if (symlink(TEMP_FILE, SYM_FILE) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "symlink(%s, %s) failed", TEMP_FILE, SYM_FILE);
	}

	return 0;
}

/*
 * access_verify(i) -
 *
 *	This function verify the accessibility of the
 *	the testfile with the one verified by access().
 */
static int access_verify(int i)
{
	char write_buf[] = "abc";
	char read_buf[BUFSIZ];
	int rval;

	rval = 0;

	switch (i) {
	case 0:
		/*
		 * The specified file has read access.
		 * Attempt to read some data from the testfile
		 * and if successful, access() behaviour is
		 * correct.
		 */
		rval = read(fd1, &read_buf, sizeof(read_buf));
		if (rval == -1)
			tst_resm(TFAIL | TERRNO, "read(%s) failed", TEST_FILE1);
		break;
	case 1:
		/*
		 * The specified file has write access.
		 * Attempt to write some data to the testfile
		 * and if successful, access() behaviour is correct.
		 */
		rval = write(fd2, write_buf, strlen(write_buf));
		if (rval == -1)
			tst_resm(TFAIL | TERRNO, "write(%s) failed",
				 TEST_FILE2);
		break;
	case 2:
		/*
		 * The specified file has execute access.
		 * Attempt to execute the specified executable
		 * file, if successful, access() behaviour is correct.
		 */
		rval = system("./" TEST_FILE3);
		if (rval != 0)
			tst_resm(TFAIL, "Fail to execute the %s", TEST_FILE3);
		break;
	case 3:
		/*
		 * The file pointed to by symbolic link has
		 * write access.
		 * Attempt to write some data to this temporary file
		 * pointed to by symlink. if successful, access() bahaviour
		 * is correct.
		 */
		rval = write(fd4, write_buf, strlen(write_buf));
		if (rval == -1)
			tst_resm(TFAIL | TERRNO, "write(%s) failed", TEMP_FILE);
		break;
	default:
		break;
	}

	return rval;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	/* Close the testfile(s) created in the setup()s */
	if (close(fd1) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "close(%s) failed", TEST_FILE1);
	if (close(fd2) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "close(%s) failed", TEST_FILE2);
	if (close(fd4) == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "close(%s) failed", TEMP_FILE);

	tst_rmdir();
}
