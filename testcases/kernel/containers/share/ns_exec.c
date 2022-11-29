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
 * Enters the namespace(s) of a process specified by a PID and then executes
 * the indicated program inside that namespace(s).
 *
 */

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "test.h"
#include "lapi/syscalls.h"
#include "lapi/sched.h"
#include "ns_common.h"

char *TCID = "ns_exec";
int ns_fd[NS_TOTAL];
int ns_fds;


void print_help(void)
{
	int i;

	printf("usage: ns_exec <NS_PID> <%s", params[0].name);

	for (i = 1; params[i].name; i++)
		printf("|,%s", params[i].name);
	printf("> <PROGRAM> [ARGS]\nSecond argument indicates the types"
	       " of a namespaces maintained by NS_PID\nand is specified"
	       " as a comma separated list.\nExample: ns_exec 1234 net,ipc"
	       " ip a\n");
}

static int open_ns_fd(const char *pid, const char *ns)
{
	int fd;
	char file_buf[30];

	sprintf(file_buf, "%s/%s/ns/%s", PROC_PATH, pid, ns);

	fd = open(file_buf, O_RDONLY);
	if (fd > 0) {
		ns_fd[ns_fds] = fd;
		++ns_fds;
		return 0;
	} else if (fd == -1 && errno != ENOENT) {
		tst_resm(TINFO | TERRNO, "open");
		return -1;
	}

	return 0;
}

static void close_ns_fd(void)
{
	int i;

	for (i = 0; i < ns_fds; i++)
		close(ns_fd[i]);
}

static int child_fn(void *arg)
{
	char **args = (char **)arg;

	execvp(args[3], args+3);
	tst_resm(TINFO | TERRNO, "execvp");
	return 1;
}

/*
 * ./ns_exec <NS_PID> <ipc,mnt,net,pid,user,uts> <PROGRAM> [ARGS]
 */
int main(int argc, char *argv[])
{
	int i, rv, pid;
	char *token;

	rv = syscall(__NR_setns, -1, 0);
	if (rv == -1 && errno == ENOSYS) {
		tst_resm(TINFO, "setns is not supported in the kernel");
		return 1;
	}

	if (argc < 4) {
		print_help();
		return 1;
	}

	memset(ns_fd, 0, sizeof(ns_fd));
	while ((token = strsep(&argv[2], ","))) {
		struct param *p = get_param(token);

		if (!p) {
			tst_resm(TINFO, "Unknown namespace: %s", token);
			print_help();
			return 1;
		}

		if (open_ns_fd(argv[1], token) != 0)
			return 1;
	}

	if (ns_fds == 0) {
		tst_resm(TINFO, "no namespace entries in /proc/%s/ns/",
			 argv[1]);
		return 1;
	}

	for (i = 0; i < ns_fds; i++) {
		if (syscall(__NR_setns, ns_fd[i], 0) == -1) {
			tst_resm(TINFO | TERRNO, "setns");
			close_ns_fd();
			return 1;
		}
	}

	pid = ltp_clone_quick(SIGCHLD, (void *)child_fn, (void *)argv);
	if (pid == -1) {
		tst_resm(TINFO | TERRNO, "ltp_clone_quick");
		close_ns_fd();
		return 1;
	}

	if (waitpid(pid, &rv, 0) == -1) {
		tst_resm(TINFO | TERRNO, "waitpid");
		return 1;
	}

	close_ns_fd();

	if (WIFEXITED(rv))
		return WEXITSTATUS(rv);

	return 0;
}
