// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) 2023 Marius Kittler <mkittler@suse.de>
 */

/*\
 * Test TCGETA/TCGETS and TCSETA/TCSETS ioctl implementations for tty driver.
 *
 * In this test, the parent and child open the parentty and the childtty
 * respectively.  After opening the childtty the child flushes the stream
 * and wakes the parent (thereby asking it to continue its testing). The
 * parent, then starts the testing. It issues a TCGETA/TCGETS ioctl to
 * get all the tty parameters. It then changes them to known values by
 * issuing a TCSETA/TCSETS ioctl. Then the parent issues a TCSETA/TCGETS
 * ioctl again and compares the received values with what it had set
 * earlier. The test fails if TCGETA/TCGETS or TCSETA/TCSETS fails, or if
 * the received values don't match those that were set. The parent does
 * all the testing, the requirement of the child process is to moniter
 * the testing done by the parent, and hence the child just waits for the
 * parent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <asm/termbits.h>

#include "lapi/ioctl.h"
#include "tst_test.h"

static struct termio termio, termio_exp;
static struct termios termios, termios_exp, termios_bak;

static char *parenttty, *childtty;
static int parentfd = -1;
static int parentpid, childpid;

static void do_child(void);
static void prepare_termio(void);
static void run_ptest(void);
static void chk_tty_parms_termio(void);
static void chk_tty_parms_termios(void);
static void setup(void);
static void cleanup(void);

static char *device;

static struct variant {
	const char *name;
	void *termio, *termio_exp, *termio_bak;
	size_t termio_size;
	void (*check)(void);
	int tcget, tcset;
} variants[] = {
	{
		.name = "termio",
		.termio = &termio,
		.termio_exp = &termio_exp,
		.termio_size = sizeof(termio),
		.check = &chk_tty_parms_termio,
		.tcget = TCGETA,
		.tcset = TCSETA,
	},
	{
		.name = "termios",
		.termio = &termios,
		.termio_exp = &termios_exp,
		.termio_size = sizeof(termios),
		.check = &chk_tty_parms_termios,
		.tcget = TCGETS,
		.tcset = TCSETS,
	},
};

static void verify_ioctl(void)
{
	tst_res(TINFO, "Testing %s variant", variants[tst_variant].name);

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
	SAFE_CLOSE(parentfd);
}

static void prepare_termio(void)
{
	/* Use "old" line discipline */
	termios_exp.c_line = termio_exp.c_line = 0;

	/* Set control modes */
	termios_exp.c_cflag = termio_exp.c_cflag = B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL;

	/* Set control chars. */
	for (int i = 0; i < NCC; i++)
		termio_exp.c_cc[i] = CSTART;
	for (int i = 0; i < VEOL2; i++)
		termios_exp.c_cc[i] = CSTART;

	/* Set local modes. */
	termios_exp.c_lflag = termio_exp.c_lflag =
	    ((unsigned short)(ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH));

	/* Set input modes. */
	termios_exp.c_iflag = termio_exp.c_iflag =
	    BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY |
	    IXOFF;

	/* Set output modes. */
	termios_exp.c_oflag = termio_exp.c_oflag = OPOST | OLCUC | ONLCR | ONOCR;
}

/*
 * run_ptest() - setup the various termio/termios structure values and issue
 * the TCSETA/TCSETS ioctl call with the TEST macro.
 */
static void run_ptest(void)
{
	struct variant *v = &variants[tst_variant];

	/* Init termio/termios structures used to check if all params got set */
	memset(v->termio, 0, v->termio_size);

	SAFE_IOCTL(parentfd, v->tcset, v->termio_exp);

	/* Get termio and see if all parameters actually got set */
	SAFE_IOCTL(parentfd, v->tcget, v->termio);
	v->check();
}

static int cmp_attr(unsigned long long exp, unsigned long long act, const char *attr)
{
	if (act == exp)
		return 0;
	tst_res(TFAIL, "%s has incorrect value %llu, expected %llu",
		attr, act, exp);
	return 1;
}

static int cmp_c_cc(unsigned char *exp_c_cc, unsigned char *act_c_cc, int ncc)
{
	int i, fails = 0;
	char what[32];

	for (i = 0; i < ncc; ++i) {
		sprintf(what, "control char %d", i);
		fails += cmp_attr(exp_c_cc[i], act_c_cc[i], what);
	}
	return fails;
}

#define CMP_ATTR(term_exp, term, attr, flag)				\
({															\
	flag += cmp_attr((term_exp).attr, (term).attr, #attr);	\
	flag;                                                   \
})

#define CMP_C_CC(term_exp, term, flag)								\
({																	\
	flag += cmp_c_cc(term_exp.c_cc, term.c_cc, sizeof(term.c_cc));	\
	flag;															\
})

static void chk_tty_parms_termio(void)
{
	int flag = 0;

	flag = CMP_ATTR(termio_exp, termio, c_line, flag);
	flag = CMP_C_CC(termio_exp, termio, flag);
	flag = CMP_ATTR(termio_exp, termio, c_lflag, flag);
	flag = CMP_ATTR(termio_exp, termio, c_iflag, flag);
	flag = CMP_ATTR(termio_exp, termio, c_oflag, flag);

	if (!flag)
		tst_res(TPASS, "TCGETA/TCSETA tests");
}

static void chk_tty_parms_termios(void)
{
	int flag = 0;

	flag = CMP_ATTR(termios_exp, termios, c_line, flag);
	flag = CMP_C_CC(termios_exp, termios, flag);
	flag = CMP_ATTR(termios_exp, termios, c_lflag, flag);
	flag = CMP_ATTR(termios_exp, termios, c_iflag, flag);
	flag = CMP_ATTR(termios_exp, termios, c_oflag, flag);

	if (!flag)
		tst_res(TPASS, "TCGETS/TCSETS tests");
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
		tst_brk(TBROK, "You must specify a tty device with -d option");

	int fd = SAFE_OPEN(device, O_RDWR, 0777);

	SAFE_IOCTL(fd, TCGETS, &termios_bak);
	SAFE_CLOSE(fd);

	prepare_termio();
}

static void cleanup(void)
{
	if (parentfd >= 0) {
		SAFE_IOCTL(parentfd, TCSETS, &termios_bak);
		SAFE_CLOSE(parentfd);
	}
}

static struct tst_test test = {
	.timeout = 9,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
	.test_variants = 2,
	.options = (struct tst_option[]) {
		{"d:", &device, "Tty device. For example, /dev/tty[0-9]"},
		{}
	}
};
