/*
 *   Copyright (c) International Business Machines  Corp., 2000
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
 *  FILE(s)     : fs_perms.c simpletest.sh textx.o Makefile README
 *  DESCRIPTION : Regression test for Linux filesystem permissions.
 *  AUTHOR      : Jeff Martin (martinjn@us.ibm.com)
 *  HISTORY     :
 *     (04/12/01)v.99  First attempt at using C for fs-regression test.  Only tests read and write bits.
 *     (04/19/01)v1.0  Added test for execute bit.
 *     (05/23/01)v1.1  Added command line parameter to specify test file.
 *     (07/12/01)v1.2  Removed conf file and went to command line parameters.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#include "test.h"

char *TCID = "fs_perms";
int TST_TOTAL = 1;

static int testsetup(mode_t mode, int cuserId, int cgroupId)
{
	int ret;
	char cmd_str[256];

	sprintf(cmd_str, "cp %s/testx test.file", getcwd(NULL, 0));
	tst_tmpdir();

	ret = unlink("test.file");
	if (ret && errno != ENOENT)
		goto done;
	ret = system(cmd_str);
	if (ret)
		goto done;
	ret = chmod("test.file", mode);
	if (ret)
		goto done;
	ret = chown("test.file", cuserId, cgroupId);

 done:
	return ret;
}

void cleanup(void)
{
	tst_rmdir();
	tst_exit();
}

static int testfperm(int userId, int groupId, char *fperm)
{
	/* SET CURRENT USER/GROUP PERMISSIONS */
	if (setegid(groupId)) {
		tst_brkm(TBROK, cleanup, "could not setegid to %d: %s", groupId, strerror(errno));
		seteuid(0);
		setegid(0);
		return -1;
	}
	if (seteuid(userId)) {
		tst_brkm(TBROK, cleanup, "could not seteuid to %d: %s", userId, strerror(errno));
		seteuid(0);
		setegid(0);
		return -1;
	}

	switch (tolower(fperm[0])) {
	case 'x': {
		int status;
		if (fork() == 0) {
			execlp("./test.file", "test.file", NULL);
			exit(1);
		}
		wait(&status);
		seteuid(0);
		setegid(0);
		return WEXITSTATUS(status);
	}
	default: {
		FILE *testfile;
		if ((testfile = fopen("test.file", fperm))) {
			fclose(testfile);
			seteuid(0);
			setegid(0);
			return 0;
		} else {
			seteuid(0);
			setegid(0);
			return 1;
		}
	}
	}
}

int main(int argc, char *argv[])
{
	char *fperm;
	int result, exresult = 0, cuserId = 0, cgroupId = 0, userId = 0, groupId = 0;
	mode_t mode;

	tst_require_root(tst_exit);

	switch (argc) {
	case 8:
		mode = strtol(argv[1], (char **)NULL, 010);
		cuserId = atoi(argv[2]);
		cgroupId = atoi(argv[3]);
		userId = atoi(argv[4]);
		groupId = atoi(argv[5]);
		fperm = argv[6];
		exresult = atoi(argv[7]);
		break;
	default:
		printf("Usage: %s <mode of file> <UID of file> <GID of file> <UID of tester> <GID of tester> <permission to test r|w|x> <expected result as 0|1>\n", argv[0]);
		return 1;
	}

	result = testsetup(mode, cuserId, cgroupId);
	if (result) {
		tst_brkm(TBROK, cleanup, "testsetup() failed: %s", strerror(errno));
	}

	result = testfperm(userId, groupId, fperm);
	unlink("test.file");
	tst_resm(exresult == result ? TPASS : TFAIL, "%c a %03o file owned by (%d/%d) as user/group(%d/%d)",
		fperm[0], mode, cuserId, cgroupId, userId, groupId);
	cleanup();
	return 0;
}
