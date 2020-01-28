/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
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
 * NAME
 *	ioctl02.c
 *
 * DESCRIPTION
 *	Testcase to test the TCGETA, and TCSETA ioctl implementations for
 *	the tty driver
 *
 * ALGORITHM
 *	In this test, the parent and child open the parentty and the childtty
 *	respectively.  After opening the childtty the child flushes the stream
 *	and sends a SIGUSR1 to the parent (thereby asking it to continue its
 *	testing). The parent, which was waiting for this signal to arrive, now
 *	starts the testing. It issues a TCGETA ioctl to get all the tty
 *	parameters. It then changes them to known values by issuing a TCSETA
 *	ioctl.  Then the parent issues a TCGETA ioctl again and compares the
 *	received values with what it had set earlier. The test fails if TCGETA
 *	or TCSETA fails, or if the received values don't match those that were
 *	set. The parent does all the testing, the requirement of the child
 *	process is to moniter the testing done by the parent, and hence the
 *	child just waits for the parent.
 *
 * USAGE:  <for command-line>
 *  ioctl02 -D /dev/tty[0-9] [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	test must be run with the -D option
 *	test may have to be run as root depending on the tty permissions
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/ioctl.h"

#define	CNUL	0

char *TCID = "ioctl02";
int TST_TOTAL = 1;

static struct termio termio, save_io;

static char *parenttty, *childtty;
static int parentfd, childfd;
static int parentpid, childpid;
static volatile int sigterm, sigusr1, sigusr2;
static int closed = 1;

static int do_child_setup(void);
static int do_parent_setup(void);
static int run_ptest(void);
static int run_ctest(void);
static int chk_tty_parms();
static void setup(void);
static void cleanup(void);
static void help(void);
static void do_child(void);
void do_child_uclinux(void);
static void sigterm_handler(void);

static int Devflag;
static char *devname;

static option_t options[] = {
	{"D:", &Devflag, &devname},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	int rval;

	tst_parse_opts(ac, av, options, &help);

#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "dS", &parentpid, &childtty);
#endif

	if (!Devflag)
		tst_brkm(TBROK, NULL, "You must specify a tty device with "
			 "the -D option.");

	tst_require_root();

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		parenttty = devname;
		childtty = devname;

		parentpid = getpid();

		childpid = FORK_OR_VFORK();
		if (childpid < 0)
			tst_brkm(TBROK, cleanup, "fork failed");

		if (childpid == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "dS", parentpid, childtty) < 0)
				tst_brkm(TBROK, cleanup, "self_exec failed");
#else
			do_child();
#endif
		}

		while (!sigusr1)
			sleep(1);

		sigusr1 = 0;

		parentfd = do_parent_setup();
		if (parentfd < 0) {
			kill(childpid, SIGTERM);
			waitpid(childpid, NULL, 0);
			cleanup();
		}

		/* run the parent test */
		rval = run_ptest();
		if (rval == -1) {
			/*
			 * Parent cannot set/get ioctl parameters.
			 * SIGTERM the child and cleanup.
			 */
			kill(childpid, SIGTERM);
			waitpid(childpid, NULL, 0);
			cleanup();
		}

		if (rval != 0)
			tst_resm(TFAIL, "TCGETA/TCSETA tests FAILED with "
				 "%d %s", rval, rval > 1 ? "errors" : "error");
		else
			tst_resm(TPASS, "TCGETA/TCSETA tests SUCCEEDED");

		/* FIXME: check return codes. */
		(void)kill(childpid, SIGTERM);
		(void)waitpid(childpid, NULL, 0);

		/*
		 * Clean up things from the parent by restoring the
		 * tty device information that was saved in setup()
		 * and closing the tty file descriptor.
		 */
		if (ioctl(parentfd, TCSETA, &save_io) == -1)
			tst_resm(TINFO, "ioctl restore failed in main");
		SAFE_CLOSE(cleanup, parentfd);

		closed = 1;
	}
	cleanup();

	tst_exit();
}

static void do_child(void)
{
	childfd = do_child_setup();
	if (childfd < 0)
		_exit(1);
	run_ctest();
	_exit(0);
}

void do_child_uclinux(void)
{
	struct sigaction act;

	/* Set up the signal handlers again */
	act.sa_handler = (void *)sigterm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	(void)sigaction(SIGTERM, &act, 0);

	/* Run the normal child */
	do_child();
}

/*
 * run_ptest() - setup the various termio structure values and issue
 *		 the TCSETA ioctl call with the TEST macro.
 */
static int run_ptest(void)
{
	int i, rval;

	/* Use "old" line discipline */
	termio.c_line = 0;

	/* Set control modes */
	termio.c_cflag = B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL;

	/* Set control chars. */
	for (i = 0; i < NCC; i++) {
		if (i == VEOL2)
			continue;
		termio.c_cc[i] = CSTART;
	}

	/* Set local modes. */
	termio.c_lflag =
	    ((unsigned short)(ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH));

	/* Set input modes. */
	termio.c_iflag =
	    BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY |
	    IXOFF;

	/* Set output modes. */
	termio.c_oflag = OPOST | OLCUC | ONLCR | ONOCR;

	TEST(ioctl(parentfd, TCSETA, &termio));

	if (TEST_RETURN < 0) {
		tst_resm(TFAIL, "ioctl TCSETA failed : "
			 "errno = %d", TEST_ERRNO);
		return -1;
	}

	/* Get termio and see if all parameters actually got set */
	rval = ioctl(parentfd, TCGETA, &termio);
	if (rval < 0) {
		tst_resm(TFAIL, "ioctl TCGETA failed.  Ending test.");
		return -1;
	}

	return chk_tty_parms();
}

static int run_ctest(void)
{
	/*
	 * Wait till the parent has finished testing.
	 */
	while (!sigterm)
		sleep(1);

	sigterm = 0;

	tst_resm(TINFO, "child: Got SIGTERM from parent.");

	if (close(childfd) == -1)
		tst_resm(TINFO, "close() in run_ctest() failed");
	return 0;
}

static int chk_tty_parms(void)
{
	int i, flag = 0;

	if (termio.c_line != 0) {
		tst_resm(TINFO, "line discipline has incorrect value %o",
			 termio.c_line);
		flag++;
	}
	/*
	 * The following Code Sniffet is disabled to check the value of c_cflag
	 * as it seems that due to some changes from 2.6.24 onwards, this
	 * setting is not done properly for either of (B50|CS7|CREAD|PARENB|
	 * PARODD|CLOCAL|(CREAD|HUPCL|CLOCAL).
	 * However, it has been observed that other flags are properly set.
	 */
#if 0
	if (termio.c_cflag != (B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL)) {
		tst_resm(TINFO, "cflag has incorrect value. %o",
			 termio.c_cflag);
		flag++;
	}
#endif

	for (i = 0; i < NCC; i++) {
		if (i == VEOL2) {
			if (termio.c_cc[VEOL2] == CNUL) {
				continue;
			} else {
				tst_resm(TINFO, "control char %d has "
					 "incorrect value %d %d", i,
					 termio.c_cc[i], CNUL);
				flag++;
				continue;
			}
		}

		if (termio.c_cc[i] != CSTART) {
			tst_resm(TINFO, "control char %d has incorrect "
				 "value %d.", i, termio.c_cc[i]);
			flag++;
		}
	}

	if (!
	    (termio.c_lflag
	     && (ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH))) {
		tst_resm(TINFO, "lflag has incorrect value. %o",
			 termio.c_lflag);
		flag++;
	}

	if (!
	    (termio.c_iflag
	     && (BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY
		 | IXOFF))) {
		tst_resm(TINFO, "iflag has incorrect value. %o",
			 termio.c_iflag);
		flag++;
	}

	if (!(termio.c_oflag && (OPOST | OLCUC | ONLCR | ONOCR))) {
		tst_resm(TINFO, "oflag has incorrect value. %o",
			 termio.c_oflag);
		flag++;
	}

	if (!flag)
		tst_resm(TINFO, "termio values are set as expected");

	return flag;
}

static int do_parent_setup(void)
{
	int pfd;

	pfd = SAFE_OPEN(cleanup, parenttty, O_RDWR, 0777);

	/* unset the closed flag */
	closed = 0;

	/* flush tty queues to remove old output */
	SAFE_IOCTL(cleanup, pfd, TCFLSH, 2);
	return pfd;
}

static int do_child_setup(void)
{
	int cfd;

	cfd = open(childtty, O_RDWR, 0777);
	if (cfd < 0) {
		tst_resm(TINFO, "Could not open %s in do_child_setup(), errno "
			 "= %d", childtty, errno);
		/* signal the parent so we don't hang the test */
		kill(parentpid, SIGUSR1);
		return -1;
	}

	/* flush tty queues to remove old output */
	if (ioctl(cfd, TCFLSH, 2) < 0) {
		tst_resm(TINFO, "ioctl TCFLSH failed. : errno = %d", errno);
		/* signal the parent so we don't hang the test */
		kill(parentpid, SIGUSR1);
		return -1;
	}

	/* tell the parent that we're done */
	kill(parentpid, SIGUSR1);

	return cfd;
}

/*
 * Define the signals handlers here.
 */
static void sigterm_handler(void)
{
	sigterm = 1;
}

static void sigusr1_handler(void)
{
	sigusr1 = 1;
}

static void sigusr2_handler(void)
{
	sigusr2 = 1;
}

static void help(void)
{
	printf("  -D <tty device> : for example, /dev/tty[0-9]\n");
}

static void setup(void)
{
	int fd;
	struct sigaction act;

	/* XXX: TERRNO required all over the place */
	fd = SAFE_OPEN(NULL, devname, O_RDWR, 0777);

	/* Save the current device information - to be restored in cleanup() */
	SAFE_IOCTL(cleanup, fd, TCGETA, &save_io);

	/* Close the device */
	SAFE_CLOSE(cleanup, fd);

	/* Set up the signal handlers */
	act.sa_handler = (void *)sigterm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	(void)sigaction(SIGTERM, &act, 0);

	act.sa_handler = (void *)sigusr1_handler;
	act.sa_flags = 0;
	(void)sigaction(SIGUSR1, &act, 0);

	act.sa_handler = (void *)sigusr2_handler;
	act.sa_flags = 0;
	(void)sigaction(SIGUSR2, &act, 0);

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	(void)sigaction(SIGTTOU, &act, 0);

	sigterm = sigusr1 = sigusr2 = 0;

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (!closed) {
		if (ioctl(parentfd, TCSETA, &save_io) == -1)
			tst_resm(TINFO, "ioctl restore failed in cleanup()");
		if (close(parentfd) == -1)
			tst_resm(TINFO, "close() failed in cleanup()");
	}
}
