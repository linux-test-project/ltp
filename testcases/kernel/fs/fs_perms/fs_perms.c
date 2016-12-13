/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *   Copyright (c) 2010 Cyril Hrubis chrubis@suse.cz
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
 *  FILE(s)     : fs_perms.c simpletest.sh textx.o Makefile README
 *  DESCRIPTION : Regression test for Linux filesystem permissions.
 *  AUTHOR      : Jeff Martin (martinjn@us.ibm.com)
 *  HISTORY     :
 *     (04/12/01)v.99  First attempt at using C for fs-regression test.  Only tests read and write bits.
 *     (04/19/01)v1.0  Added test for execute bit.
 *     (05/23/01)v1.1  Added command line parameter to specify test file.
 *     (07/12/01)v1.2  Removed conf file and went to command line parameters.
 *     (10/19/04)      Rewritten to fit ltp test interface.
 *                     Also now we try to run two different files, one is executed by execl,
 *                     has shebang and should end up executed by kernel, other one is empty
 *                     is executed by execlp and should end up executed by libc.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>

#include "test.h"

#define TEST_FILE_NAME1 "./test.file1"
#define TEST_FILE_NAME2 "./test.file2"

char *TCID = "fs_perms";
int TST_TOTAL = 1;

static void cleanup(void)
{
	seteuid(0);
	setegid(0);

	tst_rmdir();

}

/*
 * Create file and set permissions, user id, group id.
 *
 * If flag is non zero, the file contains #!/PATH/sh shebang otherwise it's
 * empty.
 */
static void testsetup(const char *file_name, int flag, mode_t mode,
		      int user_id, int group_id)
{
	FILE *file;

	file = fopen(file_name, "w");

	if (file == NULL)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Could not create test file %s.", file_name);

	/* create file with shebang */
	if (flag) {
		char buf[PATH_MAX];

		if (tst_get_path("sh", buf, PATH_MAX))
			tst_brkm(TBROK, cleanup,
				 "Could not find path to sh in $PATH.");

		if (fprintf(file, "#!%s\n", buf) < 0)
			tst_brkm(TBROK, cleanup, "Calling fprintf failed.");
	}

	if (fclose(file))
		tst_brkm(TBROK | TERRNO, cleanup, "Calling fclose failed.");

	if (chmod(file_name, mode))
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Could not chmod test file %s.", file_name);

	if (chown(file_name, user_id, group_id))
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Could not chown test file %s.", file_name);
}

/*
 * Test permissions.
 */
static int testfperm(const char *file_name, int flag, int user_id,
		     int group_id, char *fperm)
{
	FILE *file;
	int status;

	switch (fork()) {
	case 0:
		if (setgid(group_id))
			tst_brkm(TBROK | TERRNO, cleanup,
				 "Could not setgid to %d.", group_id);

		if (setuid(user_id))
			tst_brkm(TBROK | TERRNO, cleanup,
				 "Could not setuid to %d.", user_id);

		switch (tolower(fperm[0])) {
		case 'x':

			/*
			 * execlp runs file with sh in case kernel has
			 * no binmft handler for it, execl does not.
			 */
			if (flag)
				execl(file_name, file_name, NULL);
			else
				execlp(file_name, "test", NULL);

			exit(1);
			break;
		default:
			if ((file = fopen(file_name, fperm)) != NULL) {
				fclose(file);
				exit(0);
			}
			exit(1);
			break;
		}
		break;
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork failed");
		break;
	default:
		break;
	}

	wait(&status);

	return WEXITSTATUS(status);
}

static void print_usage(const char *bname)
{
	char *usage = "<file mode> <file UID> <file GID> "
	    "<tester UID> <tester GID> <permission "
	    "to test r|w|x> <expected result 0|1>";

	printf("Usage: %s %s\n", bname, usage);
}

static long str_to_l(const char *str, const char *name, int base)
{
	char *end;
	long i = strtol(str, &end, base);

	if (*end != '\0')
		tst_brkm(TBROK, NULL, "Invalid parameter '%s' passed. (%s)",
			 name, str);

	return i;
}

int main(int argc, char *argv[])
{
	char *fperm;
	gid_t fgroup_id, group_id;
	uid_t fuser_id, user_id;
	mode_t fmode;
	int exp_res;
	int res1, res2 = 1;

	tst_require_root();

	if (argc != 8) {
		print_usage(argv[0]);
		tst_exit();
	}

	if (strlen(argv[6]) > 1) {
		print_usage(argv[0]);
		tst_exit();
	}

	fmode = str_to_l(argv[1], "file mode", 8);
	fuser_id = str_to_l(argv[2], "file uid", 10);
	fgroup_id = str_to_l(argv[3], "file gid", 10);
	user_id = str_to_l(argv[4], "tester uid", 10);
	group_id = str_to_l(argv[5], "tester gid", 10);
	fperm = argv[6];
	exp_res = str_to_l(argv[7], "expected result", 10);

	tst_tmpdir();
	testsetup(TEST_FILE_NAME1, 0, fmode, fuser_id, fgroup_id);

	/* more tests for 'x' flag */
	if (tolower(fperm[0]) == 'x') {
		testsetup(TEST_FILE_NAME2, 1, fmode, fuser_id, fgroup_id);
		res2 = testfperm(TEST_FILE_NAME2, 1, user_id, group_id, fperm);

		if (res2 == exp_res)
			res2 = 1;
		else
			res2 = 0;
	}

	res1 = testfperm(TEST_FILE_NAME1, 0, user_id, group_id, fperm);

	tst_resm((exp_res == res1) && res2 ? TPASS : TFAIL,
		 "%c a %03o file owned by (%d/%d) as user/group (%d/%d)",
		 fperm[0], fmode, fuser_id, fgroup_id, user_id, group_id);
	tst_rmdir();
	tst_exit();
}
