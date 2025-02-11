// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2004
 *      07/2004 Written by Dan Jones
 *      07/2004 Ported to LTP format by Robbie Williamson
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * Make sure bind() of privileged port gives EACCESS error for non-root users.
 */

#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"

/* This port needs to be a privileged port */
#define TCP_PRIVILEGED_PORT 463
#define TEST_USERNAME "nobody"

static void run(void)
{
	struct sockaddr_in servaddr;
	int sockfd;

	sockfd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(TCP_PRIVILEGED_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	TST_EXP_FAIL(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)),
				 EACCES, "bind()");
	SAFE_CLOSE(sockfd);
}

static void setup(void)
{
	struct passwd *pw;
	struct group *gr;

	pw = SAFE_GETPWNAM(TEST_USERNAME);
	gr = SAFE_GETGRGID(pw->pw_gid);

	tst_res(TINFO, "Switching credentials to user: %s, group: %s",
		pw->pw_name, gr->gr_name);
	SAFE_SETEGID(gr->gr_gid);
	SAFE_SETEUID(pw->pw_uid);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
};
