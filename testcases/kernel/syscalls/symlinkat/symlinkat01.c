/******************************************************************************
 *
 *   Copyright (c) International Business Machines  Corp., 2006
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
 *
 * NAME
 *      symlinkat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of symlinkat
 *	added by kernel 2.6.16 or up.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 *
 * History
 *      08/25/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

#define MYRETCODE -999
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

struct test_struct;
static void setup();
static void cleanup();
static void setup_every_copy();
static void mysymlinkat_test(struct test_struct *desc);

#define TEST_DIR1 "olddir"
#define TEST_DIR2 "newdir"
#define TEST_DIR3 "deldir"
#define TEST_FILE1 "oldfile"
#define TEST_FILE2 "newfile"
#define TEST_FIFO "fifo"

static char dpathname[256] = "%s/" TEST_DIR2 "/" TEST_FILE1;
static int olddirfd, newdirfd = -1, cwd_fd = AT_FDCWD, stdinfd = 0, crapfd =
    -1, deldirfd;

struct test_struct {
	const char *oldfn;
	int *newfd;
	const char *newfn;
	const char *referencefn1;
	const char *referencefn2;
	int expected_errno;
} test_desc[] = {
	/* relative paths */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &newdirfd, TEST_FILE1,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* abs path at dst */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &newdirfd, dpathname,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* relative paths to cwd */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &cwd_fd,
		    TEST_DIR2 "/" TEST_FILE1, TEST_DIR1 "/" TEST_FILE1,
		    TEST_DIR2 "/" TEST_FILE1, 0},
	    /* abs path */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &cwd_fd, dpathname,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* relative paths to invalid */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &stdinfd,
		    TEST_DIR2 "/" TEST_FILE1, 0, 0, ENOTDIR},
	    /* abs path at dst */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &stdinfd, dpathname,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* relative paths to crap */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &crapfd,
		    TEST_DIR2 "/" TEST_FILE1, 0, 0, EBADF},
	    /* abs path at dst */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &crapfd, dpathname,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* relative paths to deleted */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &deldirfd,
		    TEST_DIR2 "/" TEST_FILE1, 0, 0, ENOENT},
	    /* abs path at dst */
	{
	"../" TEST_DIR1 "/" TEST_FILE1, &deldirfd, dpathname,
		    TEST_DIR1 "/" TEST_FILE1, TEST_DIR2 "/" TEST_FILE1, 0},
	    /* fifo link */
	    /*      { TEST_FIFO, &newdirfd, TEST_FILE1, TEST_DIR1"/"TEST_FIFO, TEST_DIR2"/"TEST_FILE1, 0 }, */
};

char *TCID = "symlinkat01";
int TST_TOTAL = sizeof(test_desc) / sizeof(*test_desc);

static int mysymlinkat(const char *oldfilename,
		       int newdirfd, const char *newfilename)
{
	return tst_syscall(__NR_symlinkat, oldfilename, newdirfd, newfilename);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			setup_every_copy();
			mysymlinkat_test(&test_desc[i]);

		}

	}

	cleanup();
	tst_exit();
}

static void setup_every_copy(void)
{
	close(newdirfd);
	unlink(dpathname);
	rmdir(TEST_DIR2);

	SAFE_MKDIR(cleanup, TEST_DIR2, 0700);
	newdirfd = SAFE_OPEN(cleanup, TEST_DIR2, O_DIRECTORY);
}

static void mysymlinkat_test(struct test_struct *desc)
{
	int fd;

	TEST(mysymlinkat(desc->oldfn, *desc->newfd, desc->newfn));

	/* check return code */
	if (TEST_ERRNO == desc->expected_errno) {
		if (TEST_RETURN == 0 && desc->referencefn1 != NULL) {
			int tnum = rand(), vnum = ~tnum;

			fd = SAFE_OPEN(cleanup, desc->referencefn1, O_RDWR);
			SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, &tnum,
				sizeof(tnum));
			SAFE_CLOSE(cleanup, fd);

			fd = SAFE_OPEN(cleanup, desc->referencefn2, O_RDONLY);
			SAFE_READ(cleanup, 1, fd, &vnum, sizeof(vnum));
			SAFE_CLOSE(cleanup, fd);

			if (tnum == vnum)
				tst_resm(TPASS, "Test passed");
			else
				tst_resm(TFAIL,
					 "The link file's content isn't as same as the original file's "
					 "although symlinkat returned 0");
		} else {
			tst_resm(TPASS,
				 "symlinkat() returned the expected  errno %d: %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}
	} else {
		tst_resm(TFAIL,
			 TEST_RETURN ==
			 0 ? "symlinkat() surprisingly succeeded" :
			 "symlinkat() Failed, errno=%d : %s", TEST_ERRNO,
			 strerror(TEST_ERRNO));
	}
}

static void setup(void)
{
	char *tmp;
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_MKDIR(cleanup, TEST_DIR1, 0700);
	SAFE_MKDIR(cleanup, TEST_DIR3, 0700);
	olddirfd = SAFE_OPEN(cleanup, TEST_DIR1, O_DIRECTORY);
	deldirfd = SAFE_OPEN(cleanup, TEST_DIR3, O_DIRECTORY);
	SAFE_RMDIR(cleanup, TEST_DIR3);
	fd = SAFE_OPEN(cleanup, TEST_DIR1 "/" TEST_FILE1, O_CREAT | O_EXCL, 0600);
	SAFE_CLOSE(cleanup, fd);

	/* gratuitous memory leak here */
	tmp = strdup(dpathname);
	snprintf(dpathname, sizeof(dpathname), tmp, get_current_dir_name());

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
