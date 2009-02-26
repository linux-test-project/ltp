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

/* 12/24/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>

/** LTP Port **/
#include "test.h"
#include "usctest.h"


char *TCID="hangup01";            /* Test program identifier.    */
int TST_TOTAL=5;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
/**************/

/*
 * pty master clone device
 */
#define MASTERCLONE "/dev/ptmx"

#define MESSAGE1 "I love Linux!"
#define MESSAGE2 "Use the LTP for all your Linux testing needs."
#define MESSAGE3 "For the latest version of the LTP tests, visit http://ltp.sourceforge.net"

#define NUMMESSAGES 3

#define BUFSZ 4096

/*
 * parent process for hangup test
 */
int
parent(int masterfd, int childpid)
{
	char buf[BUFSZ];
	struct pollfd pollfds[1];
	int hangupcount = 0;
	int datacount = 0;
	int status;
	int i;
	int len = strlen(MESSAGE1);

	pollfds[0].fd = masterfd;
	pollfds[0].events = POLLIN;

        sleep(1);

	while ((i = poll(pollfds, 1, -1)) == 1) {
		if (read(masterfd, buf, len) == -1) {
			++hangupcount;
#ifdef DEBUG
			tst_resm(TINFO,"hangup %d", hangupcount);
#endif
			if (hangupcount == NUMMESSAGES) {
				break;
			}
		} else {
			++datacount;
			switch (datacount) {
			case 1:
				if (strncmp(buf, MESSAGE1,
				    strlen(MESSAGE1)) != 0) {
					tst_resm(TFAIL, "unexpected message 1");
					tst_exit();
				}
				len = strlen(MESSAGE2);
				break;
			case 2:
				if (strncmp(buf, MESSAGE2,
				    strlen(MESSAGE2)) != 0) {
					tst_resm(TFAIL, "unexpected message 2");
					tst_exit();
				}
				len = strlen(MESSAGE3);
				break;
			case 3:
				if (strncmp(buf, MESSAGE3,
				    strlen(MESSAGE3)) != 0) {
					tst_resm(TFAIL, "unexpected message 3");
					tst_exit();
				}
				break;
			default:
				tst_resm(TFAIL, "unexpected data message");
				tst_exit();
				/*NOTREACHED*/
			}
		}
	}
	if (i != 1) {
		tst_resm(TFAIL,"poll");
		tst_exit();
	}
	while (wait(&status) != childpid) {
		;
	}
	if (status != 0) {
		tst_resm(TFAIL, "child process exited with status %d", status);
		tst_exit();
	}
	tst_resm(TPASS,"Pass");
	tst_exit();

	/*NOTREACHED*/
	return 0;
}


/*
 * Child process for hangup test.  Write three messages to the slave
 * pty, with a hangup after each.
 */
void
child(int masterfd)
{
	int slavefd;
	char *slavename;


	if ((slavename = ptsname(masterfd)) == (char *)0) {
		tst_resm(TBROK,"ptsname");
		tst_exit();
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,slavename);
		tst_exit();
	}
	if (write(slavefd, MESSAGE1, strlen(MESSAGE1)) != strlen(MESSAGE1)) {
		tst_resm(TBROK,"write");
		tst_exit();
	}
	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"open %s",slavename);
		tst_exit();
	}
	if (write(slavefd, MESSAGE2, strlen(MESSAGE2)) != strlen(MESSAGE2)) {
		tst_resm(TBROK,"write");
		tst_exit();
	}
	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		tst_resm(TBROK,"open %s",slavename);
		tst_exit();
	}
	if (write(slavefd, MESSAGE3, strlen(MESSAGE3)) != strlen(MESSAGE3)) {
		tst_resm(TBROK,"write");
		tst_exit();
	}
	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
}

/*
 * main test driver
 */
int main(int argc, char **argv)
{
	int masterfd;		/* master pty fd */
	char *slavename;
	int childpid;

/*--------------------------------------------------------------------*/
	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"open %s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == (char *)0) {
		tst_resm(TBROK,"ptsname");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK,"grantpt");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt");
		tst_exit();
	}

	childpid = fork();
	if (childpid == -1) {
		tst_resm(TBROK,"fork");
		tst_exit();
	} else if (childpid == 0) {
		child(masterfd);
		tst_exit();
	} else {
		parent(masterfd, childpid);
	}
/*--------------------------------------------------------------------*/
	tst_exit();
	/*NOTREACHED*/
	return 0;
}
