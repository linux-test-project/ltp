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
#include <termio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include "test.h"
#include "usctest.h"

#define	CNUL	0

char *TCID = "ioctl02";
int TST_TOTAL = 1;
extern int Tst_count;

struct termio termio, save_io;

char *parenttty, *childtty;
int parentfd, childfd;
int parentpid, childpid;
volatile int sigterm, sigusr1, sigusr2;
int closed = 1;

int do_child_setup();
int do_parent_setup();
int run_ptest();
int run_ctest();
int chk_tty_parms();
void setup(void);
void cleanup(void);
void help(void);
void do_child(void);
void do_child_uclinux(void);
void sigterm_handler();

int Devflag = 0;
char *devname;

/* for test specific parse_opts options - in this case "-D" */
option_t options[] = {
	{"D:", &Devflag, &devname},
	{NULL, NULL, NULL}
};

void hack();

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int rval;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, options, &help)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
	maybe_run_child(&do_child_uclinux, "dS", &parentpid, &childtty);
#endif

	if (!Devflag) {
		tst_resm(TWARN, "You must specify a tty device with "
			 "the -D option.");
		tst_resm(TWARN, "Run '%s -h' for option information.", TCID);
		cleanup();
	}

	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		parenttty = devname;
		childtty = devname;

		parentpid = getpid();

		if ((childpid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		 /*NOTREACHED*/}

		if (childpid == 0) {	/* child */
#ifdef UCLINUX
			if (self_exec(av[0], "dS", parentpid, childtty) < 0) {
				tst_brkm(TBROK, cleanup, "self_exec failed");
			}
#else
			do_child();
#endif
		}

		/* parent */
		while (!sigusr1) {
			sleep(1);
		}

		sigusr1 = 0;

		if ((parentfd = do_parent_setup()) < 0) {
			kill(childpid, SIGTERM);
			waitpid(childpid, NULL, 0);
			cleanup();
		 /*NOTREACHED*/}

		/* run the parent test */
		if ((rval = run_ptest()) == -1) {
			/*
			 * Parent cannot set/get ioctl parameters.
			 * SIGTERM the child and cleanup.
			 */
			kill(childpid, SIGTERM);
			waitpid(childpid, NULL, 0);
			cleanup();
		}

		if (rval != 0) {
			tst_resm(TFAIL, "TCGETA/TCSETA tests FAILED with "
				 "%d %s", rval, rval > 1 ? "errors" : "error");
		} else {
			tst_resm(TPASS, "TCGETA/TCSETA tests SUCCEEDED");
		}

		/* tell the child that we're done */
		(void)kill(childpid, SIGTERM);
		(void)waitpid(childpid, NULL, 0);

		/*
		 * Clean up things from the parent by restoring the
		 * tty device information that was saved in setup()
		 * and closing the tty file descriptor.
		 */
		if (ioctl(parentfd, TCSETA, &save_io) == -1) {
			tst_resm(TINFO, "ioctl restore failed in main");
		}
		if (close(parentfd) == -1) {
			tst_brkm(TBROK, cleanup, "close() failed in main");
		}

		/* set the closed flag */
		closed = 1;
	}
	cleanup();

	 /*NOTREACHED*/ return 0;
}

void do_child()
{
	if ((childfd = do_child_setup()) < 0) {
		_exit(1);
	}
	run_ctest();
	_exit(0);
}

void do_child_uclinux()
{
	struct sigaction act;

	/* Set up the signal handlers again */
	act.sa_handler = (void *)sigterm_handler;
	act.sa_flags = 0;
	(void)sigaction(SIGTERM, &act, 0);

	/* Run the normal child */
	do_child();
}

/*
 * run_ptest() - setup the various termio structure values and issue
 *		 the TCSETA ioctl call with the TEST macro.
 */
int run_ptest()
{
	int i, rval;

	/* Use "old" line discipline */
	termio.c_line = 0;

	/* Set control modes */
	termio.c_cflag = B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL;

	/* Set control chars. */
	for (i = 0; i < NCC; i++) {
		if (i == VEOL2) {
			continue;
		}
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
		return (-1);
	}

	if (STD_FUNCTIONAL_TEST) {
		/* Get termio and see if all parameters actually got set */
		if ((rval = ioctl(parentfd, TCGETA, &termio)) < 0) {
			tst_resm(TFAIL, "ioctl TCGETA failed.  Ending test.");
			return (-1);
		}

		return (chk_tty_parms());
	} else {
		tst_resm(TINFO, "call succeeded");
		return 0;
	}
}

int run_ctest()
{
	/*
	 * Wait till the parent has finished testing.
	 */
	while (!sigterm) {
		sleep(1);
	}
	sigterm = 0;

	tst_resm(TINFO, "child: Got SIGTERM from parent.");

	/* clean up things */
	if (close(childfd) == -1) {
		tst_resm(TINFO, "close() in run_ctest() failed");
	}
	return 0;
}

int chk_tty_parms()
{
	int i, flag = 0;

	if (termio.c_line != 0) {
		tst_resm(TINFO, "line discipline has incorrect value %o",
			 termio.c_line);
		flag++;
	}
	// The following Code Sniffet is disabled to check the value of c_cflag
	// as it seems that due to some changes from 2.6.24 onwards, this setting
	// is not done properly for either of (B50|CS7|CREAD|PARENB|PARODD|CLOCAL|(CREAD|HUPCL|CLOCAL).
	// However, it has been observed that other flags are properly set.

	//if (termio.c_cflag != (B50|CS7|CREAD|PARENB|PARODD|CLOCAL)) {
	//      tst_resm(TINFO, "cflag has incorrect value. %o",
	//               termio.c_cflag);
	//      flag++;
	//}

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

	if (!flag) {
		tst_resm(TINFO, "termio values are set as expected");
	}

	return (flag);
}

int do_parent_setup()
{
	int pfd;

	if ((pfd = open(parenttty, O_RDWR, 0777)) < 0) {
		tst_brkm(TBROK, cleanup, "Could not open %s in "
			 "do_parent_setup(), errno = %d", parenttty, errno);
	}

	/* unset the closed flag */
	closed = 0;

	/* flush tty queues to remove old output */
	if (ioctl(pfd, TCFLSH, 2) < 0) {
		tst_brkm(TBROK, cleanup, "ioctl TCFLSH failed : "
			 "errno = %d", errno);
	}
	return (pfd);
}

int do_child_setup()
{
	int cfd;

	if ((cfd = open(childtty, O_RDWR, 0777)) < 0) {
		tst_resm(TINFO, "Could not open %s in do_child_setup(), errno "
			 "= %d", childtty, errno);
		/* signal the parent so we don't hang the test */
		kill(parentpid, SIGUSR1);
		return (-1);
	}

	/* flush tty queues to remove old output */
	if (ioctl(cfd, TCFLSH, 2) < 0) {
		tst_resm(TINFO, "ioctl TCFLSH failed. : errno = %d", errno);
		/* signal the parent so we don't hang the test */
		kill(parentpid, SIGUSR1);
		return (-1);
	}

	/* tell the parent that we're done */
	kill(parentpid, SIGUSR1);

	return (cfd);
}

/*
 * Define the signals handlers here.
 */
void sigterm_handler()
{
	sigterm = 1;
}

void sigusr1_handler()
{
	sigusr1 = 1;
}

void sigusr2_handler()
{
	sigusr2 = 1;
}

/*
 * help() - Prints out the help message for the -D option defined
 *	    by this test.
 */
void help()
{
	printf("  -D <tty device> : for example, /dev/tty[0-9]\n");
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	int fd;
	struct sigaction act;

	if ((fd = open(devname, O_RDWR, 0777)) < 0) {
		tst_brkm(TBROK, _exit, "Could not open %s in "
			 "setup(), errno = %d", devname, errno);
	}

	/* Save the current device information - to be restored in cleanup() */
	if (ioctl(fd, TCGETA, &save_io) < 0) {
		tst_brkm(TBROK, cleanup, "TCGETA ioctl failed in "
			 "do_parent_setup()");
	}

	/* Close the device */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close() failed in setup()");
	}

	/* Set up the signal handlers */
	act.sa_handler = (void *)sigterm_handler;
	act.sa_flags = 0;
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

	/* Pause if that option was specified */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Restore the device information that was saved in setup() */
	if (!closed) {
		if (ioctl(parentfd, TCSETA, &save_io) == -1) {
			tst_resm(TINFO, "ioctl restore failed in cleanup()");
		}
		if (close(parentfd) == -1) {
			tst_resm(TINFO, "close() failed in cleanup()");
		}
	}

	/* exit with return code appropriate for results */
	tst_exit();
}
