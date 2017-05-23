/*
 * Copyright (C) 2013 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * reproducer for:
 * BUG: unable to handle kernel NULL ptr deref in selinux_socket_unix_may_send
 * fixed in 3.9.0-0.rc5:
 *   commit ded34e0fe8fe8c2d595bfa30626654e4b87621e0
 *   Author: Paul Moore <pmoore@redhat.com>
 *   Date:   Mon Mar 25 03:18:33 2013 +0000
 *     unix: fix a race condition in unix_release()
 */

#define _GNU_SOURCE
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include "config.h"
#include "test.h"
#include "safe_macros.h"
#include "lapi/semun.h"

char *TCID = "sendmsg02";

static int sem_id;
static int tflag;
static char *t_opt;
static option_t options[] = {
	{"s:", &tflag, &t_opt},
	{NULL, NULL, NULL}
};

static void setup(void);
static void cleanup(void);

static void client(int id, int pipefd[])
{
	int fd, semval;
	char data[] = "123456789";
	struct iovec w;
	struct sockaddr_un sa;
	struct msghdr mh;
	struct cmsghdr cmh;

	close(pipefd[0]);

	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	snprintf(sa.sun_path, sizeof(sa.sun_path), "socket_test%d", id);

	w.iov_base = data;
	w.iov_len = 10;

	memset(&cmh, 0, sizeof(cmh));
	mh.msg_control = &cmh;
	mh.msg_controllen = sizeof(cmh);

	memset(&mh, 0, sizeof(mh));
	mh.msg_name = &sa;
	mh.msg_namelen = sizeof(struct sockaddr_un);
	mh.msg_iov = &w;
	mh.msg_iovlen = 1;

	do {
		fd = socket(AF_UNIX, SOCK_DGRAM, 0);
		write(pipefd[1], &fd, 1);
		sendmsg(fd, &mh, MSG_NOSIGNAL);
		close(fd);
		semval = semctl(sem_id, 0, GETVAL);
	} while (semval != 0);
	close(pipefd[1]);
}

static void server(int id, int pipefd[])
{
	int fd, semval;
	struct sockaddr_un sa;

	close(pipefd[1]);

	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	snprintf(sa.sun_path, sizeof(sa.sun_path), "socket_test%d", id);

	do {
		fd = socket(AF_UNIX, SOCK_DGRAM, 0);
		unlink(sa.sun_path);
		bind(fd, (struct sockaddr *) &sa, sizeof(struct sockaddr_un));
		read(pipefd[0], &fd, 1);
		close(fd);
		semval = semctl(sem_id, 0, GETVAL);
	} while (semval != 0);
	close(pipefd[0]);
}

static void reproduce(int seconds)
{
	int i, status, pipefd[2];
	int child_pairs = sysconf(_SC_NPROCESSORS_ONLN)*4;
	int child_count = 0;
	int *child_pids;
	int child_pid;
	union semun u;

	child_pids = SAFE_MALLOC(cleanup, sizeof(int) * child_pairs * 2);

	u.val = 1;
	if (semctl(sem_id, 0, SETVAL, u) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "couldn't set semval to 1");

	/* fork child for each client/server pair */
	for (i = 0; i < child_pairs*2; i++) {
		if (i%2 == 0) {
			if (pipe(pipefd) < 0) {
				tst_resm(TBROK | TERRNO, "pipe failed");
				break;
			}
		}

		child_pid = fork();
		switch (child_pid) {
		case -1:
			tst_resm(TBROK | TERRNO, "fork");
			break;
		case 0:
			if (i%2 == 0)
				server(i, pipefd);
			else
				client(i-1, pipefd);
			exit(0);
		default:
			child_pids[child_count++] = child_pid;
		};

		/* this process can close the pipe now */
		if (i%2 == 0) {
			close(pipefd[0]);
			close(pipefd[1]);
		}
	}

	/* let clients/servers run for a while, then clear semval to signal
	 * they should stop running now */
	if (child_count == child_pairs*2)
		sleep(seconds);

	u.val = 0;
	if (semctl(sem_id, 0, SETVAL, u) == -1) {
		/* kill children if setting semval failed */
		for (i = 0; i < child_count; i++)
			kill(child_pids[i], SIGKILL);
		tst_resm(TBROK | TERRNO, "couldn't set semval to 0");
	}

	for (i = 0; i < child_count; i++) {
		if (waitpid(child_pids[i], &status, 0) == -1)
			tst_resm(TBROK | TERRNO, "waitpid for %d failed",
				child_pids[i]);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child %d returns %d", i, status);
	}
	free(child_pids);
}

static void help(void)
{
	printf("  -s NUM  Number of seconds to run.\n");
}

int main(int argc, char *argv[])
{
	int lc;
	long seconds;

	tst_parse_opts(argc, argv, options, &help);
	setup();

	seconds = tflag ? SAFE_STRTOL(NULL, t_opt, 1, LONG_MAX) : 15;
	for (lc = 0; TEST_LOOPING(lc); lc++)
		reproduce(seconds);
	tst_resm(TPASS, "finished after %ld seconds", seconds);

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	tst_tmpdir();

	sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IRWXU);
	if (sem_id == -1)
		tst_brkm(TBROK | TERRNO, NULL, "Couldn't allocate semaphore");

	TEST_PAUSE;
}

static void cleanup(void)
{
	semctl(sem_id, 0, IPC_RMID);
	tst_rmdir();
}
