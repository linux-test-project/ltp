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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 12/24/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>

/** LTP Port **/
#include "test.h"
#include "safe_macros.h"

char *TCID = "hangup01";	/* Test program identifier.    */
int TST_TOTAL = 5;		/* Total number of test cases. */
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

void cleanup(void);

pid_t childpid;

void cleanup(void)
{

	int status;

	if (0 < childpid) {

		/* If the PID is still alive... */
		if (kill(childpid, 0) == 0 || errno == ESRCH) {

			/* KILL IT! */
			(void)kill(childpid, 15);

			/* And take care of any leftover zombies. */
			if (waitpid(childpid, &status, WNOHANG) < 0) {
				tst_resm(TWARN | TERRNO,
					 "waitpid(%d, ...) failed", childpid);
			}

		}

	}

}

/*
 * parent process for hangup test
 */
void parent(int masterfd, int childpid)
{
	char buf[BUFSZ];
	struct pollfd pollfds[1];
	size_t len = strlen(MESSAGE1);
	int hangupcount = 0;
	int datacount = 0;
	int status;
	int i;

	pollfds[0].fd = masterfd;
	pollfds[0].events = POLLIN;

	sleep(1);

	while ((i = poll(pollfds, 1, -1)) == 1) {
		if (read(masterfd, buf, len) == -1) {
			++hangupcount;
#ifdef DEBUG
			tst_resm(TINFO, "hangup %d", hangupcount);
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
					tst_brkm(TFAIL, cleanup,
						 "unexpected message 1");
				}
				len = strlen(MESSAGE2);
				break;
			case 2:
				if (strncmp(buf, MESSAGE2,
					    strlen(MESSAGE2)) != 0) {
					tst_brkm(TFAIL, cleanup,
						 "unexpected message 2");
				}
				len = strlen(MESSAGE3);
				break;
			case 3:
				if (strncmp(buf, MESSAGE3,
					    strlen(MESSAGE3)) != 0) {
					tst_brkm(TFAIL, cleanup,
						 "unexpected message 3");
				}
				break;
			default:
				tst_brkm(TFAIL, cleanup,
					 "unexpected data message");

			}
		}
	}
	if (i != 1) {
		tst_brkm(TFAIL, cleanup, "poll");
	}
	while (waitpid(childpid, &status, WNOHANG) < 0 && errno != ESRCH) ;

	tst_resm((status == 0 ? TPASS : TFAIL),
		 "child process exited with status %d", status);
}

/*
 * Child process for hangup test.  Write three messages to the slave
 * pty, with a hangup after each.
 */
int child(int masterfd)
{
	int slavefd;
	char *slavename;

	if ((slavename = ptsname(masterfd)) == NULL) {
		printf("ptsname[child] failed: %s\n", strerror(errno));
		return 1;
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		printf("open[1] failed: %s\n", strerror(errno));
		return 1;
	}
	if (write(slavefd, MESSAGE1, strlen(MESSAGE1)) != strlen(MESSAGE1)) {
		printf("write failed: %s\n", strerror(errno));
		return 1;
	}
	if (close(slavefd) != 0) {
		printf("close[1] failed: %s\n", strerror(errno));
		return 1;
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		printf("open[2] failed: %s\n", strerror(errno));
		return 1;
	}
	if (write(slavefd, MESSAGE2, strlen(MESSAGE2)) != strlen(MESSAGE2)) {
		printf("write[2] failed: %s\n", strerror(errno));
		return 1;
	}
	if (close(slavefd) != 0) {
		printf("close[2] failed: %s\n", strerror(errno));
		return 1;
	}
	if ((slavefd = open(slavename, O_RDWR)) < 0) {
		printf("open[3] failed: %s\n", strerror(errno));
		return 1;
	}
	if (write(slavefd, MESSAGE3, strlen(MESSAGE3)) != strlen(MESSAGE3)) {
		printf("write[3] failed: %s\n", strerror(errno));
		return 1;
	}
	if (close(slavefd) != 0) {
		printf("close[3] failed: %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

/*
 * main test driver
 */
int main(int argc, char **argv)
{
	int masterfd;		/* master pty fd */
	char *slavename;
	pid_t childpid;

/*--------------------------------------------------------------------*/
	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "ptsname");

	if (grantpt(masterfd) != 0)
		tst_brkm(TBROK | TERRNO, NULL, "grantpt");

	if (unlockpt(masterfd) != 0)
		tst_brkm(TBROK | TERRNO, NULL, "unlockpt");

	childpid = fork();
	if (childpid == -1)
		tst_brkm(TBROK | TERRNO, NULL, "fork");
	else if (childpid == 0)
		exit(child(masterfd));
	else
		parent(masterfd, childpid);
/*--------------------------------------------------------------------*/
	cleanup();

	tst_exit();
}
