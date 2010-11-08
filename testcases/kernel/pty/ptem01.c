/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 12/23/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <termio.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/types.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

/** LTP Port **/
#include "test.h"
#include "usctest.h"


char *TCID="ptem01";            /* Test program identifier.    */
int TST_TOTAL=6;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
/**************/



/*
 * pty master clone device
 */
#define MASTERCLONE "/dev/ptmx"

#define BUFSZ 4096


/*
 * test termio/termios ioctls
 */
int
test1(void)
{
	int masterfd, slavefd;
	char *slavename;
	struct termio termio;
	struct termios termios;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_resm(TBROK|TERRNO, "ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK|TERRNO, "grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TFAIL,"Could not open %s",slavename);
		tst_exit();
	}

	if (ioctl(slavefd, TCGETS, &termios) != 0) {
		tst_resm(TFAIL,"TCGETS");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_resm(TFAIL,"TCSETS");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETSW, &termios) != 0) {
		tst_resm(TFAIL,"TCSETSW");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETSF, &termios) != 0) {
		tst_resm(TFAIL,"TCSETSF");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_resm(TFAIL,"TCSETS");
		tst_exit();
	}

	if (ioctl(slavefd, TCGETA, &termio) != 0) {
		tst_resm(TFAIL,"TCGETA");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETA, &termio) != 0) {
		tst_resm(TFAIL,"TCSETA");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETAW, &termio) != 0) {
		tst_resm(TFAIL,"TCSETAW");
		tst_exit();
	}

	if (ioctl(slavefd, TCSETAF, &termio) != 0) {
		tst_resm(TFAIL,"TCSETAF");
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close slave");
		tst_exit();
	}

	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close master");
		tst_exit();
	}
	tst_resm(TPASS,"test1");

	/** NOT REACHED **/
	return 0;
}

/*
 * test window size setting and getting
 */
int
test2(void)
{
	int masterfd, slavefd;
	char *slavename;
	struct winsize wsz;
	struct winsize wsz1 = {24, 80, 5, 10};
	struct winsize wsz2 = {60, 100, 11, 777};

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_resm(TBROK|TERRNO, "ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK|TERRNO, "grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}


	if (ioctl(masterfd, TIOCSWINSZ, &wsz1) != 0) {
		tst_resm(TFAIL,"TIOCSWINSZ");
		tst_exit();
	}

	if (ioctl(slavefd, TIOCGWINSZ, &wsz) != 0) {
		tst_resm(TFAIL,"TIOCGWINSZ");
		tst_exit();
	}

	if (wsz.ws_row != wsz1.ws_row || wsz.ws_col != wsz1.ws_col ||
	    wsz.ws_xpixel != wsz1.ws_xpixel ||
	    wsz.ws_ypixel != wsz1.ws_ypixel) {
		tst_resm(TFAIL, "unexpected window size returned");
		tst_exit();
	}

	if (ioctl(masterfd, TIOCGWINSZ, &wsz) != 0) {
		tst_resm(TFAIL,"TIOCGWINSZ");
		tst_exit();
	}

	if (wsz.ws_row != wsz1.ws_row || wsz.ws_col != wsz1.ws_col ||
	    wsz.ws_xpixel != wsz1.ws_xpixel ||
	    wsz.ws_ypixel != wsz1.ws_ypixel) {
		tst_resm(TFAIL, "unexpected window size returned");
		tst_exit();
	}

	if (ioctl(slavefd, TIOCSWINSZ, &wsz2) != 0) {
		tst_resm(TFAIL,"TIOCSWINSZ");
		tst_exit();
	}

	if (ioctl(slavefd, TIOCGWINSZ, &wsz) != 0) {
		tst_resm(TFAIL,"TIOCGWINSZ");
		tst_exit();
	}

	if (wsz.ws_row != wsz2.ws_row || wsz.ws_col != wsz2.ws_col ||
	    wsz.ws_xpixel != wsz2.ws_xpixel ||
	    wsz.ws_ypixel != wsz2.ws_ypixel) {
		tst_resm(TFAIL, "unexpected window size returned");
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}

	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	tst_resm(TPASS,"test2");

	/** NOT REACHED **/
	return 0;
}

/*
 * test sending a break
 */
int
test3(void)
{
	int masterfd, slavefd;
	char *slavename;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_resm(TBROK|TERRNO, "ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK|TERRNO, "grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	if (tcsendbreak(masterfd, 10) != 0) {
		tst_resm(TFAIL,"tcsendbreak");
		tst_exit();
	}

	if (tcsendbreak(slavefd, 10) != 0) {
		tst_resm(TFAIL,"tcsendbreak");
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close slave");
		tst_exit();
	}

	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close master");
		tst_exit();
	}
	tst_resm(TPASS,"test3");

	/** NOT REACHED **/
	return 0;
}


/*
 * test multiple opens of slave side
 */
int
test4(void)
{
	int masterfd, slavefd, slavefd2, slavefd3;
	char *slavename;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_resm(TBROK|TERRNO, "ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK|TERRNO, "grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	if ((slavefd2 = open(slavename, O_RDWR)) < 0) {
		tst_resm(TFAIL,"Could not open %s (again)",slavename);
		tst_exit();
	}

	if ((slavefd3 = open(slavename, O_RDWR)) < 0) {
		tst_resm(TFAIL,"Could not open %s (once more)",slavename);
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close slave");
		tst_exit();
	}
	if (close(slavefd2) != 0) {
		tst_resm(TBROK,"close slave again");
		tst_exit();
	}
	if (close(slavefd3) != 0) {
		tst_resm(TBROK,"close slave once more");
		tst_exit();
	}
	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close master");
		tst_exit();
	}
	tst_resm(TPASS,"test4");

	/** NOT REACHED **/
	return 0;
}

#define NUMOPENS 6

/*
 * test several simultaneous opens
 */
int
test5(void)
{
	static int masterfd[NUMOPENS];
	static int slavefd[NUMOPENS];
	char *slavename;
	int i;

	for (i = 0; i < NUMOPENS; ++i) {
		masterfd[i] = open(MASTERCLONE, O_RDWR);
		if (masterfd[i] < 0) {
			tst_resm(TBROK,"%s",MASTERCLONE);
			tst_resm(TBROK, "out of ptys");
			for (i = 0; i < NUMOPENS; ++i) {
				if (masterfd[i] != 0) {
					(void) close(masterfd[i]);
				}
				if (slavefd[i] != 0) {
					(void) close(slavefd[i]);
				}
			}
			tst_exit();
		}

		slavename = ptsname(masterfd[i]);
		if (slavename == NULL) {
			tst_resm(TBROK|TERRNO, "ptsname() call failed");
			tst_exit();
		}

		if (grantpt(masterfd[i]) != 0) {
			tst_resm(TBROK|TERRNO, "grantpt() call failed");
			tst_exit();
		}

		if (unlockpt(masterfd[i]) != 0) {
			tst_resm(TBROK,"unlockpt() call failed");
			tst_exit();
		}

		if ((slavefd[i] = open(slavename, O_RDWR)) < 0) {
			tst_resm(TFAIL,"Iteration %d: Could not open %s",i,slavename);
			tst_exit();
		}

	}

	for (i = 0; i < NUMOPENS; ++i) {
		if (close(slavefd[i]) != 0) {
			tst_resm(TBROK,"Iteration %d: close slave",i);
			tst_exit();
		}
		if (close(masterfd[i]) != 0) {
			tst_resm(TBROK,"close master");
			tst_exit();
		}
	}
	tst_resm(TPASS,"test5");

	/** NOT REACHED **/
	return 0;
}


/*
 * test hangup semantics
 */
int
test6(void)
{
	static int masterfd;
	static int slavefd;
	char *slavename;
	struct termios termios;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_resm(TBROK|TERRNO, "ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK|TERRNO, "grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	if (ioctl(slavefd, TCGETS, &termios) != 0) {
		tst_resm(TFAIL,"TCGETS");
		tst_exit();
	}

	termios.c_cflag &= ~CBAUD;
	termios.c_cflag |= B0&CBAUD;
	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_resm(TFAIL,"TCGETS");
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	tst_resm(TPASS,"test6");

	/** NOT REACHED **/
	return 0;
}

/*
 * main test driver
 */
int
main(int argc, char **argv)
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	/*
 	 * all done
	 */
	tst_exit();
	/*NOTREACHED*/
	return 0;
}
