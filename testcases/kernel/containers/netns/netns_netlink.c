/* Copyright (c) 2014 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 * File: netns_netlink.c
 *
 * Tests a netlink interface inside a new network namespace.
 * Description:
 * 1. Unshares a network namespace (so network related actions
 *    have no effect on a real system)
 * 2. Forks a child which creates a NETLINK_ROUTE netlink socket
 *    and listens to RTMGRP_LINK (network interface create/delete/up/down)
 *    multicast group.
 * 4. Child then waits for parent approval to receive data from socket
 * 3. Parent creates a new TAP interface (dummy0) and immediately
 *    removes it (which should generate some data in child's netlink socket).
 *    Then it allows child to continue.
 * 4. As the child was listening to RTMGRP_LINK multicast group, it should
 *    detect the new interface creation/deletion (by reading data from netlink
 *    socket), if so, the test passes, otherwise it fails.
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "netns_helper.h"
#include "test.h"
#include "safe_macros.h"

#define MAX_TRIES 1000
#define IP_TUNTAP_MIN_VER 100519

char *TCID	= "netns_netlink";
int TST_TOTAL	= 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	tst_require_root();
	check_iproute(IP_TUNTAP_MIN_VER);
	check_netns();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(tst_rmdir);
}

int child_func(void)
{
	int fd, len, event_found, tries;
	struct sockaddr_nl sa;
	char buffer[4096];
	struct nlmsghdr *nlh;

	/* child will listen to a network interface create/delete/up/down
	 * events */
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = RTMGRP_LINK;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd == -1) {
		perror("socket");
		return 1;
	}
	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		perror("bind");
		close(fd);
		return 1;
	}

	/* waits for parent to create an interface */
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	/* To get rid of "resource temporarily unavailable" errors
	 * when testing with -i option */
	tries = 0;
	event_found = 0;
	nlh = (struct nlmsghdr *) buffer;
	while (tries < MAX_TRIES) {
		len = recv(fd, nlh, sizeof(buffer), MSG_DONTWAIT);
		if (len > 0) {
			/* stop receiving only on interface create/delete
			 * event */
			if (nlh->nlmsg_type == RTM_NEWLINK ||
			    nlh->nlmsg_type == RTM_DELLINK) {
				event_found++;
				break;
			}
		}
		usleep(10000);
		tries++;
	}

	close(fd);

	if (!event_found) {
		perror("recv");
		return 1;
	}

	return 0;
}

static void test(void)
{
	pid_t pid;
	int status;

	/* unshares the network namespace */
	if (unshare(CLONE_NEWNET) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "unshare failed");

	pid = tst_fork();
	if (pid < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "fork failed");
	}
	if (pid == 0) {
		_exit(child_func());
	}

	/* creates TAP network interface dummy0 */
	if (WEXITSTATUS(system("ip tuntap add dev dummy0 mode tap")))
		tst_brkm(TBROK, cleanup, "system() failed");

	/* removes previously created dummy0 device */
	if (WEXITSTATUS(system("ip tuntap del mode tap dummy0")))
		tst_brkm(TBROK, cleanup, "system() failed");

	/* allow child to continue */
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);


	SAFE_WAITPID(cleanup, pid, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		tst_resm(TFAIL, "netlink interface fail");
		return;
	}
	if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "child was killed with signal %s",
			 tst_strsig(WTERMSIG(status)));
		return;
	}

	tst_resm(TPASS, "netlink interface pass");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test();

	cleanup();
	tst_exit();
}
