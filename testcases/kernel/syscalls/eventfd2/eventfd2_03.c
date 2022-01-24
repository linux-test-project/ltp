/*
 *  eventfd-sem by Davide Libenzi (Simple test for eventfd sempahore)
 *  Copyright (C) 2009  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *  Reference: http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commitdiff;h=bcd0b235bf3808dec5115c381cd55568f63b85f0
 *  Reference: http://www.xmailserver.org/eventfd-sem.c
 *  eventfd: testing improved support for semaphore-like behavior in linux-2.6.30
 *
 */

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "eventfd2_03";
int TST_TOTAL = 1;

#ifndef EFD_SEMLIKE
#define EFD_SEMLIKE (1 << 0)
#endif

/* Dummy function as syscall from linux_syscall_numbers.h uses cleanup(). */
void cleanup(void)
{
}

static int eventfd2(int count, int flags)
{
	return tst_syscall(__NR_eventfd2, count, flags);
}

static void xsem_wait(int fd)
{
	u_int64_t cntr;

	if (read(fd, &cntr, sizeof(cntr)) != sizeof(cntr)) {
		perror("reading eventfd");
		exit(1);
	}
	fprintf(stdout, "[%u] wait completed on %d: count=%" PRIu64 "\n",
		getpid(), fd, cntr);
}

static void xsem_post(int fd, int count)
{
	u_int64_t cntr = count;

	if (write(fd, &cntr, sizeof(cntr)) != sizeof(cntr)) {
		perror("writing eventfd");
		exit(1);
	}
}

static void sem_player(int fd1, int fd2)
{
	fprintf(stdout, "[%u] posting 1 on %d\n", getpid(), fd1);
	xsem_post(fd1, 1);

	fprintf(stdout, "[%u] waiting on %d\n", getpid(), fd2);
	xsem_wait(fd2);

	fprintf(stdout, "[%u] posting 1 on %d\n", getpid(), fd1);
	xsem_post(fd1, 1);

	fprintf(stdout, "[%u] waiting on %d\n", getpid(), fd2);
	xsem_wait(fd2);

	fprintf(stdout, "[%u] posting 5 on %d\n", getpid(), fd1);
	xsem_post(fd1, 5);

	fprintf(stdout, "[%u] waiting 5 times on %d\n", getpid(), fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
	xsem_wait(fd2);
}

static void usage(char const *prg)
{
	fprintf(stderr, "use: %s [-h]\n", prg);
}

int main(int argc, char **argv)
{
	int c, fd1, fd2, status;
	pid_t cpid_poster, cpid_waiter;

	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		default:
			usage(argv[0]);
			return 1;
		}
	}
	if ((tst_kvercmp(2, 6, 27)) < 0) {
		tst_brkm(TCONF,
			 NULL,
			 "This test can only run on kernels that are 2.6.27 and higher");
	}
	if ((fd1 = eventfd2(0, EFD_SEMLIKE)) == -1 ||
	    (fd2 = eventfd2(0, EFD_SEMLIKE)) == -1) {
		perror("eventfd2");
		return 1;
	}
	if ((cpid_poster = fork()) == 0) {
		sem_player(fd1, fd2);
		exit(0);
	}
	if ((cpid_waiter = fork()) == 0) {
		sem_player(fd2, fd1);
		exit(0);
	}
	waitpid(cpid_poster, &status, 0);
	waitpid(cpid_waiter, &status, 0);

	tst_exit();
}
