/*
 * NAME
 *	rename07
 *
 * DESCRIPTION
 *	This test will verify that rename(2) failed in ENOTDIR
 *
 * CALLS
 *	stat,open,rename,mkdir,close
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *              create the "old" directory and the "new" file
 *              rename the "old" directory to the "new" file
 *
 *	Test:
 *		Loop if the proper options are given.
 *                  verify rename() failed and returned ENOTDIR
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.*
 * USAGE
 *	rename07 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern void do_file_setup(char *);

char *TCID = "rename07";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { ENOTDIR, 0 };	/* List must end with 0 */

int fd;
char mname[255], fdir[255];
struct stat buf1, buf2;
dev_t olddev, olddev1;
ino_t oldino, oldino1;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* rename a directory to a file */
		/* Call rename(2) */
		TEST(rename(fdir, mname));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "rename(%s, %s) succeeded unexpectedly",
				 fdir, mname);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO != ENOTDIR) {
			tst_resm(TFAIL, "Expected ENOTDIR got %d", TEST_ERRNO);
		} else {
			tst_resm(TPASS, "rename() returned ENOTDIR");
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	sprintf(fdir, "./rndir_%d", getpid());
	sprintf(mname, "./tfile_%d", getpid());

	/* create "old" directory */
	if (stat(fdir, &buf1) != -1) {
		tst_brkm(TBROK, cleanup, "tmp directory %s found!", fdir);
	 }

	if (mkdir(fdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", fdir);
	 }

	if (stat(fdir, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat directory %s "
			 "in rename()", fdir);

	}

	/* save "old"'s dev and ino */
	olddev = buf1.st_dev;
	oldino = buf1.st_ino;

	/*
	 * create "new" file
	 */
	do_file_setup(mname);

	if (stat(mname, &buf2) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat file %s in rename()",
			 mname);

	}

	/* save "new"'s dev and ino */
	olddev1 = buf2.st_dev;
	oldino1 = buf2.st_ino;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();
}