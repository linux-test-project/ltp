/*
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 12/23/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/types.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/ioctl.h"

char *TCID = "ptem01";		/* Test program identifier.    */
int TST_TOTAL = 6;		/* Total number of test cases. */
/**************/

/*
 * pty master clone device
 */
#define MASTERCLONE "/dev/ptmx"

#define BUFSZ 4096

/*
 * test termio/termios ioctls
 */
int test1(void)
{
	int masterfd, slavefd;
	char *slavename;
	struct termio termio;
	struct termios termios;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "unlockpt() call failed");
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TFAIL, NULL, "Could not open %s", slavename);
	}

	if (ioctl(slavefd, TCGETS, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCGETS");
	}

	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETS");
	}

	if (ioctl(slavefd, TCSETSW, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETSW");
	}

	if (ioctl(slavefd, TCSETSF, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETSF");
	}

	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETS");
	}

	if (ioctl(slavefd, TCGETA, &termio) != 0) {
		tst_brkm(TFAIL, NULL, "TCGETA");
	}

	if (ioctl(slavefd, TCSETA, &termio) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETA");
	}

	if (ioctl(slavefd, TCSETAW, &termio) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETAW");
	}

	if (ioctl(slavefd, TCSETAF, &termio) != 0) {
		tst_brkm(TFAIL, NULL, "TCSETAF");
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close slave");
	}

	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close master");
	}
	tst_resm(TPASS, "test1");

	/** NOT REACHED **/
	return 0;
}

/*
 * test window size setting and getting
 */
int test2(void)
{
	int masterfd, slavefd;
	char *slavename;
	struct winsize wsz;
	struct winsize wsz1 = { 24, 80, 5, 10 };
	struct winsize wsz2 = { 60, 100, 11, 777 };

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "unlockpt() call failed");
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TBROK, NULL, "Could not open %s", slavename);
	}

	if (ioctl(masterfd, TIOCSWINSZ, &wsz1) != 0) {
		tst_brkm(TFAIL, NULL, "TIOCSWINSZ");
	}

	if (ioctl(slavefd, TIOCGWINSZ, &wsz) != 0) {
		tst_brkm(TFAIL, NULL, "TIOCGWINSZ");
	}

	if (wsz.ws_row != wsz1.ws_row || wsz.ws_col != wsz1.ws_col ||
	    wsz.ws_xpixel != wsz1.ws_xpixel ||
	    wsz.ws_ypixel != wsz1.ws_ypixel) {
		tst_brkm(TFAIL, NULL, "unexpected window size returned");
	}

	if (ioctl(masterfd, TIOCGWINSZ, &wsz) != 0) {
		tst_brkm(TFAIL, NULL, "TIOCGWINSZ");
	}

	if (wsz.ws_row != wsz1.ws_row || wsz.ws_col != wsz1.ws_col ||
	    wsz.ws_xpixel != wsz1.ws_xpixel ||
	    wsz.ws_ypixel != wsz1.ws_ypixel) {
		tst_brkm(TFAIL, NULL, "unexpected window size returned");
	}

	if (ioctl(slavefd, TIOCSWINSZ, &wsz2) != 0) {
		tst_brkm(TFAIL, NULL, "TIOCSWINSZ");
	}

	if (ioctl(slavefd, TIOCGWINSZ, &wsz) != 0) {
		tst_brkm(TFAIL, NULL, "TIOCGWINSZ");
	}

	if (wsz.ws_row != wsz2.ws_row || wsz.ws_col != wsz2.ws_col ||
	    wsz.ws_xpixel != wsz2.ws_xpixel ||
	    wsz.ws_ypixel != wsz2.ws_ypixel) {
		tst_brkm(TFAIL, NULL, "unexpected window size returned");
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close");
	}

	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close");
	}
	tst_resm(TPASS, "test2");

	/** NOT REACHED **/
	return 0;
}

/*
 * test sending a break
 */
int test3(void)
{
	int masterfd, slavefd;
	char *slavename;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "unlockpt() call failed");
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TBROK, NULL, "Could not open %s", slavename);
	}

	if (tcsendbreak(masterfd, 10) != 0) {
		tst_brkm(TFAIL, NULL, "tcsendbreak");
	}

	if (tcsendbreak(slavefd, 10) != 0) {
		tst_brkm(TFAIL, NULL, "tcsendbreak");
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close slave");
	}

	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close master");
	}
	tst_resm(TPASS, "test3");

	/** NOT REACHED **/
	return 0;
}

/*
 * test multiple opens of slave side
 */
int test4(void)
{
	int masterfd, slavefd, slavefd2, slavefd3;
	char *slavename;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "unlockpt() call failed");
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TBROK, NULL, "Could not open %s", slavename);
	}

	if ((slavefd2 = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TFAIL, NULL, "Could not open %s (again)", slavename);
	}

	if ((slavefd3 = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TFAIL, NULL, "Could not open %s (once more)",
			 slavename);
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close slave");
	}
	if (close(slavefd2) != 0) {
		tst_brkm(TBROK, NULL, "close slave again");
	}
	if (close(slavefd3) != 0) {
		tst_brkm(TBROK, NULL, "close slave once more");
	}
	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close master");
	}
	tst_resm(TPASS, "test4");

	/** NOT REACHED **/
	return 0;
}

#define NUMOPENS 6

/*
 * test several simultaneous opens
 */
int test5(void)
{
	static int masterfd[NUMOPENS];
	static int slavefd[NUMOPENS];
	char *slavename;
	int i;

	for (i = 0; i < NUMOPENS; ++i) {
		masterfd[i] = open(MASTERCLONE, O_RDWR);
		if (masterfd[i] < 0) {
			tst_resm(TBROK, "%s", MASTERCLONE);
			tst_resm(TBROK, "out of ptys");
			for (i = 0; i < NUMOPENS; ++i) {
				if (masterfd[i] != 0) {
					(void)close(masterfd[i]);
				}
				if (slavefd[i] != 0) {
					(void)close(slavefd[i]);
				}
			}
			tst_exit();
		}

		slavename = ptsname(masterfd[i]);
		if (slavename == NULL) {
			tst_brkm(TBROK | TERRNO, NULL,
				 "ptsname() call failed");
		}

		if (grantpt(masterfd[i]) != 0) {
			tst_brkm(TBROK | TERRNO, NULL,
				 "grantpt() call failed");
		}

		if (unlockpt(masterfd[i]) != 0) {
			tst_brkm(TBROK, NULL, "unlockpt() call failed");
		}

		if ((slavefd[i] = open(slavename, O_RDWR)) < 0) {
			tst_brkm(TFAIL, NULL,
				 "Iteration %d: Could not open %s", i,
				 slavename);
		}

	}

	for (i = 0; i < NUMOPENS; ++i) {
		if (close(slavefd[i]) != 0) {
			tst_brkm(TBROK, NULL, "Iteration %d: close slave", i);
		}
		if (close(masterfd[i]) != 0) {
			tst_brkm(TBROK, NULL, "close master");
		}
	}
	tst_resm(TPASS, "test5");

	/** NOT REACHED **/
	return 0;
}

/*
 * test hangup semantics
 */
int test6(void)
{
	static int masterfd;
	static int slavefd;
	char *slavename;
	struct termios termios;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "unlockpt() call failed");
	}

	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_brkm(TBROK, NULL, "Could not open %s", slavename);
	}

	if (ioctl(slavefd, TCGETS, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCGETS");
	}

	termios.c_cflag &= ~CBAUD;
	termios.c_cflag |= B0 & CBAUD;
	if (ioctl(slavefd, TCSETS, &termios) != 0) {
		tst_brkm(TFAIL, NULL, "TCGETS");
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close");
	}
	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close");
	}
	tst_resm(TPASS, "test6");

	/** NOT REACHED **/
	return 0;
}

/*
 * main test driver
 */
int main(int argc, char **argv)
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
}
