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

void setup(void);
void cleanup(void);

char *TCID = "fcntl01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int flags;
	char fname[40];
	int fd[10], fd2[10];
	int mypid, i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		mypid = getpid();
		for (i = 0; i < 8; i++) {
			sprintf(fname, "./fcntl%d.%d", i, mypid);
			if ((fd[i] =
			     open(fname, O_WRONLY | O_CREAT, 0666)) == -1)
				tst_resm(TBROK | TERRNO, "open failed");
			fd2[i] = fd[i];
		}

		close(fd[2]);
		close(fd[3]);
		close(fd[4]);
		close(fd[5]);

		if ((fd[2] = fcntl(fd[1], F_DUPFD, 1)) == -1)
			tst_resm(TFAIL | TERRNO, "fcntl(.., 1) failed");

		if (fd[2] < fd2[2])
			tst_resm(TFAIL, "new fd has unexpected value: "
				 "got %d, expected greater than %d", fd[2], 5);

		if ((fd[4] = fcntl(fd[1], F_DUPFD, fd2[3])) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl(.., fd2[3]) failed");

		if (fd[4] < fd2[3])
			tst_resm(TFAIL, "new fd has unexpected value, got %d, "
				 "expect greater than %d", fd[4], fd2[3]);

		if ((fd[8] = fcntl(fd[1], F_DUPFD, fd2[5])) < 0)
			tst_resm(TFAIL | TERRNO, "fcntl(.., fd2[5]) failed");

		if (fd[8] != fd2[5])
			tst_resm(TFAIL, "new fd has unexpected value: "
				 "got %d, expected %d", fd[8], fd2[5]);
/* //block1: */
		flags = fcntl(fd[2], F_GETFL, 0);
		if ((flags & O_WRONLY) == 0)
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_WRONLY);

		/* Check setting of no_delay flag */
		if (fcntl(fd[2], F_SETFL, O_NDELAY) == -1)
			tst_resm(TBROK | TERRNO, "fcntl(.., O_NDELAY) failed");

		flags = fcntl(fd[2], F_GETFL, 0);
		if ((flags & (O_NDELAY | O_WRONLY)) == 0)
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_NDELAY | O_WRONLY);

		/* Check of setting append flag */
		if (fcntl(fd[2], F_SETFL, O_APPEND) == -1)
			tst_resm(TFAIL | TERRNO, "fcntl(.., O_APPEND) failed");

		flags = fcntl(fd[2], F_GETFL, 0);
		if ((flags & (O_APPEND | O_WRONLY)) == 0)
			tst_resm(TFAIL, "unexpected flag ox%x, expected 0x%x",
				 flags, O_APPEND | O_WRONLY);

		/* Check setting flags together */
		if (fcntl(fd[2], F_SETFL, O_NDELAY | O_APPEND) < 0)
			tst_resm(TFAIL, "fcntl(.., O_NDELAY|O_APPEND) failed");

		flags = fcntl(fd[2], F_GETFL, 0);
		if ((flags & (O_NDELAY | O_APPEND | O_WRONLY)) == 0)
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags,
				 O_NDELAY | O_APPEND | O_SYNC | O_WRONLY);

		/* Check that flags are not cummulative */
		if (fcntl(fd[2], F_SETFL, 0) == -1)
			tst_resm(TFAIL, "fcntl(.., 0) failed");

		flags = fcntl(fd[2], F_GETFL, 0);
		if ((flags & O_WRONLY) == 0)
			tst_resm(TFAIL, "unexpected flag 0x%x, expected 0x%x",
				 flags, O_WRONLY);

/* //block2: */
		/*
		 * Check ability to set (F_SETFD) the close on exec flag
		 */
		if ((flags = fcntl(fd[2], F_GETFD, 0)) < 0)
			tst_resm(TFAIL | TERRNO,
				 "fcntl(.., F_GETFD, ..) #1 failed");
		if (flags != 0)
			tst_resm(TFAIL, "unexpected flags got 0x%x expected "
				 "0x%x", flags, 0);
		if ((flags = fcntl(fd[2], F_SETFD, 1)) == -1)
			tst_resm(TFAIL, "fcntl(.., F_SETFD, ..) failed");
		if ((flags = fcntl(fd[2], F_GETFD, 0)) == -1)
			tst_resm(TFAIL | TERRNO,
				 "fcntl(.., F_GETFD, ..) #2 failed");
		if (flags != 1)
			tst_resm(TFAIL, "unexpected flags, got 0x%x, "
				 "expected 0x%x", flags, 1);

		for (i = 0; i < ARRAY_SIZE(fd); i++)
			close(fd[i]);
		for (i = 0; i < 8; i++) {
			sprintf(fname, "./fcntl%d.%d", i, mypid);
			if ((unlink(fname)) == -1)
				tst_resm(TFAIL | TERRNO,
					 "unlinking %s failed", fname);
		}
	}
	cleanup();
	tst_exit();
}

/*
 * setup
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	umask(0);
	TEST_PAUSE;
	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();

}
