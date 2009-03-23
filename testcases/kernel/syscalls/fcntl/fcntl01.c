/*
 * NAME
 *	fcntl01.c
 *
 * DESCRIPTION
 *	Test F_DUPFD, F_SETFL cmds of fcntl
 *
 * CALLS
 *	fcntl
 *
 * ALGORITHM
 *
 *	1. Testing F_DUPFD cmd with arg less than, equal to, and greater
 *	   than the next available file descriptor.
 *
 *	2. Checking F_SETFL cmd with each valid flag (O_NDELAY, O_APPEND).
 *
 *	3. Checking, setting and reading `close on exec' flag.
 *
 * USAGE
 *	fcntl01
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	09/2002 added fd2 array to remove statid fds
 *
 * RESTRICTIONS
 *	None
 *
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

char *TCID = "fcntl01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int ac, char **av)
{
	int flags;
	int fail;
	char fname[40];
	int fd[10], fd2[10];
	int mypid, i;
	int lc;
	char *msg;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

/* //block0: */
		tst_resm(TINFO, "Enter block 0");
		fail = 0;
		/*
		 * Fill up descriptor table without using F_DUPFD
		 */
		mypid = getpid();
		for (i = 0; i < 8; i++) {
			sprintf(fname, "./fcntl%d.%d", i, mypid);
			if ((fd2[i] = fd[i] = open(fname, O_WRONLY | O_CREAT,
						   0666)) < 0) {
				fprintf(stderr, "\tcannot open '%s', errno "
					"= %d\n", fname, errno);
				exit(1);
			}
		}

		/*
		 * Create holes in descriptor table
		 */
		close(fd[2]);
		close(fd[3]);
		close(fd[4]);
		close(fd[5]);

		/*
		 * Check that fcntl returns proper fd when arg less than
		 * next available fd
		 */
		if ((fd[2] = fcntl(fd[1], F_DUPFD, 1)) < 0) {
			tst_resm(TFAIL, "fcntl failed, errno = %d", errno);
			fail = 1;
		}

		if (fd[2] < fd2[2]) {
			tst_resm(TFAIL, "new fd has unexpected value: "
				 "got %d, expected greater than %d", fd[2], 5);
			fail = 1;
		}

		/*
		 * Check that fcntl returns proper fd when arg equal to next
		 * available descriptor
		 */
		if ((fd[4] = fcntl(fd[1], F_DUPFD, fd2[3])) < 0) {
			tst_resm(TFAIL, "fcntl failed, errno = %d", errno);
			fail = 1;
		}

		if (fd[4] < fd2[3]) {
			tst_resm(TFAIL, "new fd has unexpected value, got %d, "
				 "expect greater than %d", fd[4], fd2[3]);
			fail = 1;
		}

		/*
		 * Check fcntl returns proper fd when arg greater than next
		 * available fd but less than end of descriptor table
		 */
		if ((fd[8] = fcntl(fd[1], F_DUPFD, fd2[5])) < 0) {
			tst_resm(TFAIL, "fcntl failed, errno = %d", errno);
			fail = 1;
		}

		if (fd[8] != fd2[5]) {
			tst_resm(TFAIL, "new fd has unexpected value: "
				 "got %d, expected %d", fd[8], fd2[5]);
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "Test 1: F_DUPFD cmd FAILED");
		} else {
			tst_resm(TINFO, "Test 1: F_DUPFD cmd PASSED");
		}
		tst_resm(TINFO, "Exit block 0");

/* //block1: */
		tst_resm(TINFO, "Enter block 1");
		fail = 0;
		flags = fcntl(fd[2], F_GETFL, 0);
		if (!(flags && O_WRONLY)) {
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_WRONLY);
			fail = 1;
		}

		/* Check setting of no_delay flag */
		if (fcntl(fd[2], F_SETFL, O_NDELAY) < 0) {
			tst_resm(TBROK, "fcntl fails, O_NDELAY, errno = %d",
				 errno);
			fail = 1;
		}

		flags = fcntl(fd[2], F_GETFL, 0);
		if (!(flags && (O_NDELAY | O_WRONLY))) {
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_NDELAY | O_WRONLY);
			fail = 1;
		}

		/* Check of setting append flag */
		if (fcntl(fd[2], F_SETFL, O_APPEND) < 0) {
			tst_resm(TFAIL, "fcntl fails, O_APPEND, errno = %d",
				 errno);
			fail = 1;
		}

		flags = fcntl(fd[2], F_GETFL, 0);
		if (!(flags && (O_APPEND | O_WRONLY))) {
			tst_resm(TFAIL, "unexpected flag ox%x, expected 0x%x",
				 flags, O_APPEND | O_WRONLY);
			fail = 1;
		}

		/* Check setting flags together */
		if (fcntl(fd[2], F_SETFL, O_NDELAY | O_APPEND) < 0) {
			tst_resm(TFAIL, "fcntl fails, O_NDELAY | O_APPEND,"
				 "errno = %d", errno);
			fail = 1;
		}

		flags = fcntl(fd[2], F_GETFL, 0);
		if (!(flags && (O_NDELAY | O_APPEND | O_WRONLY))) {
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags,
				 O_NDELAY | O_APPEND | O_SYNC | O_WRONLY);
			fail = 1;
		}

		/* Check that flags are not cummulative */
		if (fcntl(fd[2], F_SETFL, 0) < 0) {
			tst_resm(TFAIL, "fcntl fails, 0, errno = %d", errno);
			fail = 1;
		}

		flags = fcntl(fd[2], F_GETFL, 0);
		if (!(flags && O_WRONLY)) {
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_WRONLY);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test 2: F_SETFL cmd FAILED");
		} else {
			tst_resm(TINFO, "Test 2: F_SETFL cmd PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

/* //block2: */
		tst_resm(TINFO, "Enter block 2");
		/*
		 * Check ability to set (F_SETFD) the close on exec flag
		 */
		fail = 0;
		if ((flags = fcntl(fd[2], F_GETFD, 0)) < 0) {
			tst_resm(TFAIL, "fcntl F_GETFD fails, errno = %d",
				 errno);
			fail = 1;
		}
		if (flags != 0) {
			tst_resm(TFAIL, "unexpected flags got 0x%x expected "
				 "0x%x", flags, 0);
			fail = 1;
		}
		if ((flags = fcntl(fd[2], F_SETFD, 1)) < 0) {
			tst_resm(TFAIL, "fcntl F_SETFD fails, errno = %d",
				 errno);
			fail = 1;
		}
		if ((flags = fcntl(fd[2], F_GETFD, 0)) < 0) {
			tst_resm(TFAIL, "fcntl F_GETFD fails, errno = %d",
				 errno);
			fail = 1;
		}
		if (flags != 1) {
			tst_resm(TFAIL, "unexpected flags, got 0x%x, "
				 "expected 0x%x", flags, 1);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "Test 3: test close-on-exec FAILED");
		} else {
			tst_resm(TINFO, "Test 3: test close-on-exec PASSED");
		}
		tst_resm(TINFO, "Exit block 2");

		/*
		 * cleanup the open files, and remove data files
		 */
		close(fd[0]);
		close(fd[1]);
		close(fd[2]);
		close(fd[3]);
		close(fd[4]);
		close(fd[5]);
		close(fd[6]);
		close(fd[7]);
		close(fd[8]);
		for (i = 0; i < 8; i++) {
			sprintf(fname, "./fcntl%d.%d", i, mypid);
			if ((unlink(fname)) < 0) {
				tst_resm(TFAIL, "cannot unlink '%s', errno "
					 "= %d", fname, errno);
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);	/* capture signals */
	umask(0);
	TEST_PAUSE;		/* Pause if that option is specified */
	tst_tmpdir();		/* make temp dir and cd to it */
}

/*
 * cleanup
 *	performs all the ONE TIME cleanup for this test at completion or
 *	premature exit
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
