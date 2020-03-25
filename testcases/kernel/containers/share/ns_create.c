/* Copyright (c) 2015 Red Hat, Inc.
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
 *
 * Written by Matus Marhefka <mmarhefk@redhat.com>
 *
 ***********************************************************************
 * Creates a child process in the new specified namespace(s), child is then
 * daemonized and is running in the background. PID of the daemonized child
 * process is printed on the stdout. As the new namespace(s) is(are) maintained
 * by the daemonized child process it(they) can be removed by killing this
 * process.
 *
 */

#define _GNU_SOURCE
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "test.h"
#include "lapi/namespaces_constants.h"
#include "ns_common.h"

char *TCID = "ns_create";


void print_help(void)
{
	int i;

	printf("usage: ns_create <%s", params[0].name);

	for (i = 1; params[i].name; i++)
		printf("|,%s", params[i].name);
	printf(">\nThe only argument is a comma separated list "
	       "of namespaces to create.\nExample: ns_create net,ipc\n");
}

static int child_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int i;

	if (setsid() == -1) {
		tst_resm(TINFO | TERRNO, "setsid");
		exit(1);
	}

	if (chdir("/") == -1) {
		tst_resm(TINFO | TERRNO, "chdir");
		exit(1);
	}

	/* close all inherrited file descriptors */
	for (i = 0; i < sysconf(_SC_OPEN_MAX); i++)
		close(i);

	pause();
	return 0;
}

/*
 * ./ns_create <ipc,mnt,net,pid,user,uts>
 */
int main(int argc, char *argv[])
{
	int pid, flags;
	char *token;

	if (argc < 2) {
		print_help();
		return 1;
	}

	flags = 0;
	while ((token = strsep(&argv[1], ","))) {
		struct param *p = get_param(token);

		if (!p) {
			tst_resm(TINFO, "Unknown namespace: %s", token);
			print_help();
			return 1;
		}

		flags |= p->flag;
	}

	pid = ltp_clone_quick(flags | SIGCHLD, child_fn, NULL);
	if (pid == -1) {
		tst_resm(TINFO | TERRNO, "ltp_clone_quick");
		return 1;
	}

	printf("%d", pid);
	return 0;
}
