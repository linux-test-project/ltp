/*
 * NAME
 *	open08.c
 *
 * DESCRIPTION
 *	Check for the following errors:
 *	1.	EEXIST
 *	2.	EISDIR
 *	3.	ENOTDIR
 *	4.	ENAMETOOLONG
 *	5.	EACCES
 *	6.	EFAULT
 *
 * ALGORITHM
 *	1. Open a file with O_CREAT and O_EXCL, when the file already
 *	   exists. Check the errno for EEXIST
 *
 *	2. Pass a directory as the pathname and request a write access,
 *	   check for errno for EISDIR
 *
 *	3. Specify O_DIRECTORY as a parameter to open and pass a file as the
 *	   pathname, check errno for ENOTDIR
 *
 *	4. Attempt to open() a filename which is more than VFS_MAXNAMLEN, and
 *	   check for errno to be ENAMETOOLONG.
 *
 *	5. Attempt to open a test executable in WRONLY mode,
 *	   open(2) should fail with EACCES.
 *
 *	6. Attempt to pass an invalid pathname with an address pointing outside
 *	   the address space of the process, as the argument to open(), and
 *	   expect to get EFAULT.
 *
 * USAGE:  <for command-line>
 *  open08 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#define _GNU_SOURCE		/* for O_DIRECTORY */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

char *TCID = "open08";
extern int Tst_count;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int exp_enos[] = { EEXIST, EISDIR, ENOTDIR, ENAMETOOLONG, EACCES, EFAULT, 0 };

char *bad_addr = 0;

char filename[40] = "";
char fname[] = "/bin/cat";	/* test executable to open */
char bad_file[] =
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";

struct test_case_t {
	char *fname;
	int flags;
	int error;
} TC[] = {
	{
	filename, O_CREAT | O_EXCL, EEXIST}, {
	"/tmp", O_RDWR, EISDIR}, {
	filename, O_DIRECTORY, ENOTDIR}, {
	bad_file, O_RDWR, ENAMETOOLONG}, {
	fname, O_WRONLY, EACCES},
#if !defined(UCLINUX)
	{
	(char *)-1, O_CREAT, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(TC[0]);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(open(TC[i].fname, TC[i].flags,
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	int fildes;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setgid(ltpuser->pw_gid) == -1) {
		tst_brkm(TBROK | TERRNO, tst_exit, "setgid(%d) failed",
			ltpuser->pw_gid);
	} else if (setuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK | TERRNO, tst_exit, "setuid(%d) failed",
			ltpuser->pw_uid);
	}

	/* make a temp directory and cd to it */
	tst_tmpdir();

	sprintf(filename, "open3.%d", getpid());

	if ((fildes = creat(filename, 0600)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't creat %s", filename);
	}
	close(fildes);

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[5].fname = bad_addr;
#endif
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
