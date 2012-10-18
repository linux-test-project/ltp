/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
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
 * Test Name: bind02
 *
 * Test Description:
 *  Make sure bind() gives EACCESS error for (non-root) users.
 *
 * Usage:  <for command-line>
 *  bind01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *      07/2004 Written by Dan Jones
 *      07/2004 Ported to LTP format by Robbie Williamson
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include "test.h"
#include "usctest.h"

char *TCID = "bind02";		/* Test program identifier.    */
int testno;
int TST_TOTAL = 1;

/* This port needs to be a Privledged port */
#define TCP_PRIVLEGED_COM_PORT 463

struct passwd *pw;
struct group *gr;

uid_t uid;
gid_t gid;
char nobody_uid[] = "nobody";

int rc;

void try_bind()
{
	struct sockaddr_in servaddr;
	int sockfd, r_value;

	// Set effective user/group
	if ((rc = setegid(gid)) == -1) {
		tst_brkm(TBROK|TERRNO, 0, "setegid(%u) failed", gid);
		tst_exit();
	}
	if ((rc = seteuid(uid)) == -1) {
		tst_brkm(TBROK|TERRNO, 0, "seteuid(%u) failed", uid);
		tst_exit();
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		tst_brkm(TBROK|TERRNO, 0, "socket() failed");
		tst_exit();
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(TCP_PRIVLEGED_COM_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	r_value = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (r_value) {
		if (errno == EACCES) {
			tst_resm(TPASS, "correct error");
		} else {
			tst_resm(TFAIL, "incorrect error, %d", r_value);
		}
	} else {
		tst_resm(TFAIL, "user was able to bind successfully");
	}

	close(sockfd);

	// Set effective user/group
	if ((rc = setegid(0)) == -1) {
		tst_brkm(TBROK|TERRNO, 0, "setegid(0) reset failed");
		tst_exit();
	}
	if ((rc = seteuid(uid)) == -1) {
		/* XXX: is this seteuid() correct !?  it isnt a reset if we
		 *      made the same exact call above ...
		 */
		tst_brkm(TBROK|TERRNO, 0, "seteuid(%u) reset failed", uid);
		tst_exit();
	}

}

int main(int argc, char *argv[])
{

	/* FreeBSD has set limits for user login name -- MAXLOGNAME, but
	 * Linux doesn't have that limitation apparently. */
	char *username = NULL;

	if (argc != 2) {
		tst_resm(TINFO, "Defaulting to user nobody");
		username = strdup(nobody_uid);
	} else {
		/* Get test user uid/gid. */
		username = argv[1];
	}

	if ((pw = getpwnam(username)) == NULL) {
		tst_brkm(TBROK, 0, "Username - %s - not found", username);
		tst_exit();
	}

	if ((gr = getgrgid(pw->pw_gid)) == NULL) {
		tst_brkm(TBROK|TERRNO, 0, "getgrgid(%u) failed", pw->pw_gid);
		tst_exit();
	}

	uid = pw->pw_uid;
	gid = gr->gr_gid;

	tst_resm(TINFO, "Socket will try to be bind by user: %s, group: %s",
		 pw->pw_name, gr->gr_name);

	try_bind();
	tst_exit();

	tst_exit();
}
