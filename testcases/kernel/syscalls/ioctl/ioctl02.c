// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) 2023 Marius Kittler <mkittler@suse.de>
 */

/*\
 * [Description]
 *
 * Testcase to test the TCGETA, and TCSETA ioctl implementations for
 * the tty driver
 *
 * In this test, the parent and child open the parentty and the childtty
 * respectively.  After opening the childtty the child flushes the stream
 * and wakes the parent (thereby asking it to continue its testing). The
 * parent, then starts the testing. It issues a TCGETA ioctl to get all
 * the tty parameters. It then changes them to known values by issuing a
 * TCSETA ioctl. Then the parent issues a TCGETA ioctl again and compares
 * the received values with what it had set earlier. The test fails if
 * TCGETA or TCSETA fails, or if the received values don't match those
 * that were set. The parent does all the testing, the requirement of the
 * child process is to moniter the testing done by the parent, and hence
 * the child just waits for the parent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>

#include "lapi/ioctl.h"

#include "tst_checkpoint.h"
#include "tst_test.h"
#include "tst_safe_macros.h"

static struct termio termio, save_io;

static char *parenttty, *childtty;
static int parentfd = -1;
static int parentpid, childpid;

static void do_child(void);
static void run_ptest(void);
static void chk_tty_parms(void);
static void setup(void);
static void cleanup(void);

static char *device;

static void verify_ioctl(void)
{
	parenttty = device;
	childtty = device;

	parentpid = getpid();
	childpid = SAFE_FORK();
	if (!childpid) {
		do_child();
		exit(EXIT_SUCCESS);
	}

	TST_CHECKPOINT_WAIT(0);

	parentfd = SAFE_OPEN(parenttty, O_RDWR, 0777);
	SAFE_IOCTL(parentfd, TCFLSH, TCIOFLUSH);

	run_ptest();

	TST_CHECKPOINT_WAKE(0);
}

/*
 * run_ptest() - setup the various termio structure values and issue
 *		 the TCSETA ioctl call with the TEST macro.
 */
static void run_ptest(void)
{
	/* Use "old" line discipline */
	termio.c_line = 0;

	/* Set control modes */
	termio.c_cflag = B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL;

	/* Set control chars. */
	for (int i = 0; i < NCC; i++) {
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

	SAFE_IOCTL(parentfd, TCSETA, &termio);

	/* Get termio and see if all parameters actually got set */
	SAFE_IOCTL(parentfd, TCGETA, &termio);
	chk_tty_parms();
}

static void chk_tty_parms(void)
{
	int i, flag = 0;

	if (termio.c_line != 0) {
		tst_res(TFAIL, "line discipline has incorrect value %o",
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
		tst_res(TFAIL, "cflag has incorrect value. %o",
			 termio.c_cflag);
		flag++;
	}
#endif

	for (i = 0; i < NCC; i++) {
		if (i == VEOL2) {
			if (!termio.c_cc[i]) {
				continue;
			} else {
				tst_res(TFAIL, "control char %d has "
					 "incorrect value %d", i, termio.c_cc[i]);
				flag++;
				continue;
			}
		}

		if (termio.c_cc[i] != CSTART) {
			tst_res(TFAIL, "control char %d has incorrect "
				 "value %d.", i, termio.c_cc[i]);
			flag++;
		}
	}

	if (!
	    (termio.c_lflag
	     && (ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH))) {
		tst_res(TFAIL, "lflag has incorrect value. %o",
			 termio.c_lflag);
		flag++;
	}

	if (!
	    (termio.c_iflag
	     && (BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY
		 | IXOFF))) {
		tst_res(TFAIL, "iflag has incorrect value. %o",
			 termio.c_iflag);
		flag++;
	}

	if (!(termio.c_oflag && (OPOST | OLCUC | ONLCR | ONOCR))) {
		tst_res(TFAIL, "oflag has incorrect value. %o",
			 termio.c_oflag);
		flag++;
	}

	if (!flag)
		tst_res(TPASS, "TCGETA/TCSETA tests");
}

static void do_child(void)
{
	int cfd = SAFE_OPEN(childtty, O_RDWR, 0777);

	SAFE_IOCTL(cfd, TCFLSH, TCIOFLUSH);

	/* tell the parent that we're done */
	TST_CHECKPOINT_WAKE(0);

	TST_CHECKPOINT_WAIT(0);
	tst_res(TINFO, "child: parent has finished testing");
	SAFE_CLOSE(cfd);
}

static void setup(void)
{
	if (!device)
		tst_brk(TBROK, "You must specify a tty device with -D option");

	int fd = SAFE_OPEN(device, O_RDWR, 0777);

	SAFE_IOCTL(fd, TCGETA, &save_io);
	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (parentfd >= 0) {
		SAFE_IOCTL(parentfd, TCSETA, &save_io);
		SAFE_CLOSE(parentfd);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
	.options = (struct tst_option[]) {
		{"D:", &device, "Tty device. For example, /dev/tty[0-9]"},
		{}
	}
};