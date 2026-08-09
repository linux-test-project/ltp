// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) 2026 Pavithra <pavrampu@linux.ibm.com>
 */

/*\
 * Test :manpage:`ptrace(2)` write to hugepage memory.
 *
 * A child process maps a hugepage via hugetlbfs, zeroes it, and sends
 * the mapped address to the parent. The parent attaches with
 * :manpage:`ptrace(2)`, then uses PTRACE_POKEDATA and PTRACE_PEEKDATA
 * to write and read back a known value at two different offsets within
 * the hugepage, verifying that ptrace operates correctly on
 * hugepage-backed memory regions.
 *
 * Requires root to mount hugetlbfs and for :manpage:`ptrace(2)`
 * PTRACE_ATTACH to work across processes when ptrace_scope is
 * restrictive.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "hugetlb.h"

#define CONST	0xdeadbeefL
#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static int fd = -1;

static void child(int hugefd, int pipefd[2])
{
	void *p;

	SAFE_CLOSE(pipefd[0]);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		      hugefd, 0);

	memset(p, 0, hpage_size);

	tst_res(TINFO, "Child mapped data at %p", p);

	SAFE_WRITE(SAFE_WRITE_ALL, pipefd[1], &p, sizeof(p));
	SAFE_CLOSE(pipefd[1]);

	pause();
	/* Child is killed by parent via SIGKILL, so cleanup is not reached */
}

static void do_poke(pid_t pid, void *p)
{
	tst_res(TINFO, "Poking at %p...", p);
	TEST(ptrace(PTRACE_POKEDATA, pid, p, (void *)CONST));
	if (TST_RET == -1)
		tst_brk(TFAIL | TTERRNO, "ptrace(POKEDATA) failed");

	tst_res(TINFO, "Peeking at %p...", p);
	TEST(ptrace(PTRACE_PEEKDATA, pid, p, NULL));
	if (TST_ERR)
		tst_brk(TFAIL | TTERRNO, "ptrace(PEEKDATA) failed");

	TST_EXP_EQ_LU((unsigned long)TST_RET, CONST);
}

static void run_test(void)
{
	int pipefd[2];
	pid_t cpid;
	void *p;
	int status;

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	SAFE_PIPE(pipefd);

	cpid = SAFE_FORK();

	if (cpid == 0) {
		child(fd, pipefd);
		exit(0);
	}

	SAFE_CLOSE(pipefd[1]);
	SAFE_READ(1, pipefd[0], &p, sizeof(p));
	SAFE_CLOSE(pipefd[0]);

	tst_res(TINFO, "Parent received address %p", p);

	SAFE_PTRACE(PTRACE_ATTACH, cpid, NULL, NULL);

	SAFE_WAITPID(cpid, &status, 0);
	if (!WIFSTOPPED(status))
		tst_brk(TBROK, "Child %d was not stopped", cpid);

	do_poke(cpid, p);
	do_poke(cpid, p + getpagesize());

	SAFE_KILL(cpid, SIGKILL);
	SAFE_WAITPID(cpid, &status, 0);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.hugepages = {1, TST_NEEDS},
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
};
