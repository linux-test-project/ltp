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
 * Test Name: lchown02
 *
 * Test Description:
 *   Verify that,
 *   1) lchown(2) returns -1 and sets errno to EPERM if the effective user id
 *	of process does not match the owner of the file and the process is
 *	not super user.
 *   2) lchown(2) returns -1 and sets errno to EACCES if search permission is
 *	denied on a component of the path prefix.
 *   3) lchown(2) returns -1 and sets errno to EFAULT if pathname points
 *	outside user's accessible address space.
 *   4) lchown(2) returns -1 and sets errno to ENAMETOOLONG if the pathname
 *	component is too long.
 *   5) lchown(2) returns -1 and sets errno to ENOTDIR if the directory
 *	component in pathname is not a directory.
 *   6) lchown(2) returns -1 and sets errno to ENOENT if the specified file
 *	does not exists.
 *
 * Expected Result:
 *  lchown() should fail with return value -1 and set expected errno.
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
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  lchown02 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *
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

#define MODE_RWX	S_IRWXU | S_IRWXG | S_IRWXO
#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DIR_TEMP	"testdir_1"
#define TEST_FILE1	"tfile_1"
#define SFILE1		"sfile_1"
#define TEST_FILE2	"testdir_1/tfile_2"
#define SFILE2		"testdir_1/sfile_2"
#define TEST_FILE3	"t_file/tfile_3"
#define SFILE3		"t_file/sfile_3"

int no_setup();
int setup1();			/* setup function to test lchown for EPERM */
int setup2();			/* setup function to test lchown for EACCES */
int setup3();			/* setup function to test lchown for ENOTDIR */
int longpath_setup();		/* setup function to test chown for ENAMETOOLONG */

char Longpathname[PATH_MAX + 2];
char High_address_node[64];
char EXEC_DIR[PATH_MAX];
char main_test_dir[PATH_MAX + 2];

struct test_case_t {		/* test case struct. to hold ref. test cond's */
	char *pathname;
	char *desc;
	int exp_errno;
	int (*setupfunc) ();
} Test_cases[] = {
	{
	SFILE1, "Process is not owner/root", EPERM, setup1}, {
	SFILE2, "No Search permissions to process", EACCES, setup2}, {
	High_address_node, "Address beyond address space", EFAULT, no_setup},
	{
	(char *)-1, "Negative address", EFAULT, no_setup}, {
	Longpathname, "Pathname too long", ENAMETOOLONG, longpath_setup}, {
	"", "Pathname is empty", ENOENT, no_setup}, {
	SFILE3, "Path contains regular file", ENOTDIR, setup3}, {
	NULL, NULL, 0, no_setup}
};

char *TCID = "lchown02";	/* Test program identifier.    */
int TST_TOTAL = 7;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EPERM, EACCES, EFAULT, ENAMETOOLONG, ENOENT, ENOTDIR, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char *bad_addr = 0;

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *file_name;	/* ptr. for file name whose mode is modified */
	char *test_desc;	/* test specific error message */
	int ind;		/* counter to test different test conditions */
	uid_t User_id;		/* Effective user id of a test process */
	gid_t Group_id;		/* Effective group id of a test process */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/*
	 * Invoke setup function to call individual test setup functions
	 * to simulate test conditions.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Set uid/gid values to that of test process */
	User_id = geteuid();
	Group_id = getegid();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
			file_name = Test_cases[ind].pathname;
			test_desc = Test_cases[ind].desc;

			if (file_name == High_address_node) {
				file_name = get_high_address();
			}

			/*
			 * Call lchown(2) to test different test conditions.
			 * verify that it fails with -1 return value and
			 * sets appropriate errno.
			 */
			TEST(lchown(file_name, User_id, Group_id));

			/* Check return code from lchown(2) */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == Test_cases[ind].exp_errno) {
					tst_resm(TPASS,
						 "lchown() fails, %s, errno:%d",
						 test_desc, TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "lchown() fails, %s, "
						 "errno:%d, expected errno:%d",
						 test_desc, TEST_ERRNO,
						 Test_cases[ind].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "lchown() returned %ld, "
					 "expected -1, errno:%d", TEST_RETURN,
					 Test_cases[ind].exp_errno);
			}
		}		/* End of TEST CASE LOOPING. */
	}			/* End for TEST_LOOPING */

	/*
	 * Invoke cleanup() to delete the test directory/file(s) created
	 * in the setup().
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup(void) - performs all ONE TIME setup for this test.
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 *	Invoke individual test setup functions according to the order
 *	set in struct. definition.
 */
void setup()
{
	int ind;		/* counter for setup functions */

	/* Capture unexpected signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Get the current directory of the test executable */
	if (getcwd(EXEC_DIR, sizeof(EXEC_DIR)) == NULL) {
		tst_brkm(TBROK, cleanup,
			 "getcwd(3) fails to get working directory of process");
	}

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setgid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setgid failed to "
			 "to set the effective gid to %d", ltpuser->pw_uid);
		perror("setgid");
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* remember current dir, because create_link has been copied here */
	if (getcwd(main_test_dir, sizeof(main_test_dir)) == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to get the current working directory.");
	}

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* fix permissions on the tmpdir */
	if (chmod(".", 0711) != 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "chmod() failed");
	}

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");
	}
	Test_cases[3].pathname = bad_addr;

	/* call individual setup functions */
	for (ind = 0; Test_cases[ind].desc != NULL; ind++) {
		Test_cases[ind].setupfunc();
	}
}

/*
 * no_setup() - Some test conditions for mknod(2) do not any setup.
 *              Hence, this function just returns 0.
 *  This function simply returns 0.
 */
int no_setup()
{
	return 0;
}

/*
 * setup1() - setup function for a test condition for which chown(2)
 *	      returns -1 and sets errno to EPERM.
 *
 *  Create a testfile under temporary directory and invoke setuid to root
 *  program to change the ownership of testfile to that of "ltpuser2" user,
 *  create a symlink file of it with ownership as ltpuser2.
 */
int setup1()
{
	int fd;			/* file handler for testfile */
	char Path_name[PATH_MAX];	/* Buffer to hold command string */
	char Path2_name[PATH_MAX];	/* Buffer for just the path name */
	char Cmd_buffer[BUFSIZ];	/* Buffer to hold command string */

	/* Creat a testfile and close it */
	if ((fd = open(TEST_FILE1, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed", TEST_FILE1);
	}
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close(%s) Failed", TEST_FILE1);
	}

	/* Get the current working directory of the process */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "failed to get the current working directory.");
	}

	/* Provide permissions for temporary directory */
	if (chmod(Path_name, 0777) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "chmod(%s, 0777) failed", Path_name);
	}

	strcpy(Path2_name, Path_name);

	/* Get the path of test file created under temporary directory */
	strcat(Path_name, "/" TEST_FILE1);

	/* Get the command name to be executed as setuid to root */
	strcat((char *)Cmd_buffer, main_test_dir);
	strcat((char *)Cmd_buffer, (const char *)"/create_link ");
	strcat((char *)Cmd_buffer, Path_name);

	if (system((const char *)Cmd_buffer) != 0) {
		tst_brkm(TBROK, cleanup,
			 "Fail to modify %s ownership(s)!", TEST_FILE1);
	}
	return 0;
}

/*
 * setup2() - setup function for a test condition for which chown(2)
 *	      returns -1 and sets errno to EACCES.
 *  Create a test directory under temporary directory and create a test file
 *  under this directory with mode "0666" permissions.
 *  Modify the mode permissions on test directory such that process will not
 *  have search permissions on test directory.
 *
 *  The function returns 0.
 */
int setup2()
{
	int fd;			/* file handle for testfile */

	/* Creat a test directory */
	if (mkdir(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, cleanup, "mkdir(2) of %s failed", DIR_TEMP);
	}

	/* Creat a file under above test directory */
	if ((fd = open(TEST_FILE2, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}
	/* Close the test file created above */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}

	/* Creat a symlink of testfile */
	if (symlink(TEST_FILE2, SFILE2) < 0) {
		tst_brkm(TBROK, cleanup,
			 "symlink() of %s Failed, errno=%d : %s",
			 TEST_FILE2, errno, strerror(errno));
	}

	/* Modify mode permissions on test directory */
	if (chmod(DIR_TEMP, FILE_MODE) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", DIR_TEMP);
	}
	return 0;
}

/*
 * setup3() - setup function for a test condition for which chown(2)
 *	     returns -1 and sets errno to ENOTDIR.
 *
 *  Create a test file under temporary directory so that test tries to
 *  change mode of a testfile "tfile_3" under "t_file" which happens to be
 *  another regular file.
 */
int setup3()
{
	int fd;

	/* Creat a testfile under temporary directory */
	if ((fd = open("t_file", O_RDWR | O_CREAT, MODE_RWX)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(2) on t_file failed, errno=%d : %s",
			 errno, strerror(errno));
	}
	/* close the test file */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(t_file) Failed, errno=%d : %s",
			 errno, strerror(errno));
	}
	return 0;
}

/*
 * longpath_setup() - setup to create a node with a name length exceeding
 *                    the MAX. length of PATH_MAX.
 *   This function retruns 0.
 */
int longpath_setup()
{
	int ind;		/* counter variable */

	for (ind = 0; ind <= (PATH_MAX + 1); ind++) {
		Longpathname[ind] = 'a';
	}
	return 0;
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
 *      Restore the mode permissions on test directory.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Restore mode permissions on test directory created in setup2() */
	if (chmod(DIR_TEMP, MODE_RWX) < 0) {
		tst_brkm(TBROK, NULL, "chmod(2) of %s failed", DIR_TEMP);
	}

	/* Remove files and temporary directory created */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
