// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Richard Palethorpe <richiejp@f-m.fm>
 * Copyright (c) 2017 SUSE LLC
 */
/*
 * Check that memory marked with MADV_DONTDUMP is not included in a core dump
 * and check that the same memory then marked with MADV_DODUMP is included in
 * a core dump.
 *
 * In order to reliably find the core dump this test temporarily changes the
 * system wide core_pattern setting. Meaning all core dumps will be sent to the
 * test's temporary dir until the setting is restored during cleanup.
 *
 * Test flow: map memory,
 *	      write generated character sequence to memory,
 *	      start child process,
 *	      mark memory with MADV_DONTDUMP in child,
 *	      abort child,
 *	      scan child's core dump for character sequence,
 *	      if the sequence is not found it is a pass otherwise a fail,
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "tst_test.h"
#include "lapi/mmap.h"

#define CORE_PATTERN "/proc/sys/kernel/core_pattern"
#define CORE_FILTER "/proc/self/coredump_filter"
#define YCOUNT 0x500L
#define FMEMSIZE (YCOUNT + 0x2L)
#define CORENAME_MAX_SIZE 512

static int dfd;
static void *fmem;

static const char * const save_restore[] = {
	CORE_PATTERN,
	NULL,
};

static void setup(void)
{
	char cwd[1024];
	char tmpcpattern[1048];
	char *fmemc;
	int i;
	unsigned int filter;
	struct rlimit limit;

	limit.rlim_max = RLIM_INFINITY;
	limit.rlim_cur = limit.rlim_max;
	SAFE_SETRLIMIT(RLIMIT_CORE, &limit);

	switch (prctl(PR_GET_DUMPABLE)) {
	case 0:
		tst_brk(TCONF, "Process is not dumpable.");
	case 1:
		break;
	default:
		tst_brk(TBROK | TERRNO, "prctl(PR_GET_DUMPABLE)");
	}

	SAFE_FILE_SCANF(CORE_FILTER, "%x", &filter);
	if (!(0x1 & filter))
		tst_brk(TCONF, "Anonymous private memory is not dumpable.");

	SAFE_GETCWD(cwd, sizeof(cwd));
	snprintf(tmpcpattern, sizeof(tmpcpattern), "%s/dump-%%p", cwd);
	tst_res(TINFO, "Temporary core pattern is '%s'", tmpcpattern);
	SAFE_FILE_PRINTF(CORE_PATTERN, "%s", tmpcpattern);

	fmem = SAFE_MMAP(NULL,
			 FMEMSIZE,
			 PROT_READ | PROT_WRITE,
			 MAP_ANONYMOUS | MAP_PRIVATE,
			 -1,
			 0);

	/*
	 * Write a generated character sequence to the mapped memory,
	 * which we later look for in the core dump.
	 */
	fmemc = (char *)fmem;
	*fmemc = 'x';
	for (i = 0; i < YCOUNT; i++)
		fmemc[i + 1] = 'y';
	fmemc[++i] = 'z';
}

static void cleanup(void)
{
	if (fmem)
		SAFE_MUNMAP(fmem, FMEMSIZE);

	if (dfd > 0)
		SAFE_CLOSE(dfd);
}

static int find_sequence(int pid)
{
	char expectc = 'x';
	ssize_t read, pos = 0;
	char rbuf[1024];
	int ycount = 0;
	char dumpname[256];

	snprintf(dumpname, 256, "dump-%d", pid);
	tst_res(TINFO, "Dump file should be %s", dumpname);
	if (access(dumpname, F_OK))
		tst_brk(TBROK | TERRNO, "Dump file was not found.");

	dfd = SAFE_OPEN(dumpname, O_RDONLY);

	read = SAFE_READ(0, dfd, &rbuf, sizeof(rbuf));
	while (read) {
		switch (rbuf[pos]) {
		case 'x':
			ycount = 0;
			expectc = 'y';
			break;
		case 'y':
			if (expectc == 'y') {
				ycount++;
			} else {
				expectc = 'x';
				break;
			}

			if (ycount == YCOUNT)
				expectc = 'z';
			break;
		case 'z':
			if (expectc == 'z') {
				SAFE_CLOSE(dfd);
				return 1;
			}
		default:
			expectc = 'x';
		}
		if (++pos >= read) {
			read = SAFE_READ(0, dfd, &rbuf, sizeof(rbuf));
			pos = 0;
		}
	}

	SAFE_CLOSE(dfd);
	return 0;
}

static pid_t run_child(int advice)
{
	int status;
	pid_t pid;
	char *advstr =
		advice == MADV_DONTDUMP ? "MADV_DONTDUMP" : "MADV_DODUMP";

	pid = SAFE_FORK();
	if (pid == 0) {
		if (madvise(fmem, FMEMSIZE, advice) == -1) {
			tst_res(TFAIL | TERRNO,
				"madvise(%p, %lu, %s) = -1",
				fmem,
				FMEMSIZE,
				advstr);
			exit(1);
		}
		abort();
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && WCOREDUMP(status))
		return pid;
	if (WIFEXITED(status))
		return 0;

	tst_res(TCONF, "No coredump produced after signal (%d)",
		WTERMSIG(status));

	return 0;
}

static void run(unsigned int test_nr)
{
	pid_t pid;

	if (!test_nr) {
		pid = run_child(MADV_DONTDUMP);
		if (pid && find_sequence(pid))
			tst_res(TFAIL,
				"Found sequence in dump when MADV_DONTDUMP set");
		else if (pid)
			tst_res(TPASS, "madvise(..., MADV_DONTDUMP)");
	} else {
		pid = run_child(MADV_DODUMP);
		if (pid && find_sequence(pid))
			tst_res(TPASS, "madvise(..., MADV_DODUMP)");
		else if (pid)
			tst_res(TFAIL,
				"No sequence in dump after MADV_DODUMP.");
	}
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "3.4.0",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.save_restore = save_restore
};
