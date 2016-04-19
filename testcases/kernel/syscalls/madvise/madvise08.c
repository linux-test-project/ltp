/*
 * Copyright (c) 2016-2017 Richard Palethorpe <richiejp@f-m.fm>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Check that memory marked with MADV_DONTDUMP is not included in a core dump
 * and check that the same memory then marked with MADV_DODUMP is included in
 * a core dump.
 *
 * In order to reliably find the core dump this test temporarily changes the
 * system wide core_pattern setting. Meaning all core dumps will be sent to the
 * test's temporary dir untill the setting is restored during cleanup.
 *
 * Test flow: map memory,
 *	      write generated character sequence to memory,
 *	      start child process,
 *	      mark memory with MADV_DONTDUMP in child,
 *	      abort child,
 *	      scan child's core dump for character sequence,
 *	      if the sequence is not found it is a pass otherwise a fail,
 *	      do the opposite for MADV_DODUMP,
 */

#include "tst_test.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(MADV_DONTDUMP) && defined(MADV_DODUMP)
#define CORE_PATTERN "/proc/sys/kernel/core_pattern"

struct test_case {
	int fd;
	char *fname;
	char *fcontent;
	void *fmem;
	size_t fmemsize;
	int ycount;
	char cpattern[1048];
	ssize_t cpsize;
	int cpfd;
	struct stat stat;
	int dfd;
} tc;

/*
 *TODO: Check/set /proc/PID/coredump_filter,
 *		RLIMIT_CORE, RLIMIT_FSIZE,
 *		PR_SET_DUMPABLE,
 *		CONFIG_COREDUMP (kernel)
 * Currently it is assumed that if the core dump can not be found it is because
 * they are disabled rather than a bug. Also tests may pass because of the core
 * and file size limits.
 */

static void setup(void)
{
	const char *dumpf = "dump-%p";
	char cwd[1024];
	char tmpcpattern[1048];

	tc.fd = 0;
	tc.fname = "mmf0";
	tc.fmem = NULL;
	tc.cpsize = -1;
	tc.cpfd = 0;
	tc.dfd = 0;
	tc.ycount = 1280;
	tc.fmemsize = tc.ycount + 2;

	SAFE_GETCWD(cwd, 1024);
	snprintf(tmpcpattern, 1048, "%s/%s", cwd, dumpf);
	tst_res(TINFO, "Temporary core pattern is '%s'", tmpcpattern);

	tc.cpfd = SAFE_OPEN(CORE_PATTERN, O_RDONLY);
	tc.cpsize = SAFE_READ(0, tc.cpfd, &tc.cpattern, 1048);
	SAFE_CLOSE(tc.cpfd);
	tc.cpfd = 0;

	if (tc.cpsize == 1048)
		tst_brk(TCONF, "Current core pattern is too large to save");

	tc.cpattern[tc.cpsize + 1] = '\0';
	tst_res(TINFO, "System core pattern is '%s'", tc.cpattern);

	tc.cpfd = SAFE_OPEN(CORE_PATTERN, O_TRUNC | O_WRONLY);
	SAFE_WRITE(1, tc.cpfd, tmpcpattern, strlen(tmpcpattern));
	SAFE_CLOSE(tc.cpfd);
	tc.cpfd = 0;
}

static void restore_cpattern(void)
{
	int fd, rval;
	size_t written;

	if (tc.cpfd != 0 && close(tc.cpfd))
		tst_res(TINFO | TERRNO, "close(tc.cpdf)");

	if (tc.cpsize < 1)
		return;

	fd = open(CORE_PATTERN, O_TRUNC | O_WRONLY);
	if (fd == -1) {
		tst_res(TINFO | TERRNO, "open(CORE_PATTERN, O_TRUNC)");
		goto error;
	}

	written = write(fd, tc.cpattern, tc.cpsize);
	if (written < (size_t)tc.cpsize) {
		tst_brk(TINFO | TERRNO, "write(fd, tc.pattern, ...)");
		goto error;
	}

	rval = close(fd);
	if (rval == -1) {
		tst_brk(TINFO | TERRNO, "close(fd)");
		goto error;
	}

	tc.cpsize = 0;
	return;
error:
	tst_brk(TBROK,
		"COULD NOT RESTORE "CORE_PATTERN"! It used to be '%s'",
		tc.cpattern);
}

static void cleanup(void)
{
	restore_cpattern();

	if (tc.fmem != NULL && munmap(tc.fmem, tc.fmemsize))
		tst_brk(TBROK | TERRNO, "munmap(tc.fmem)");
	tc.fmem = NULL;

	if (tc.fd > 0 && close(tc.fd))
		tst_brk(TBROK | TERRNO, "close(tc.fd)");

	if (tc.dfd > 0 && close(tc.dfd))
		tst_brk(TBROK | TERRNO, "close(tc.dfd)");
}

static int find_sequence(int pid)
{
	char c, expectc = 'x';
	int ycount = 0;
	char dumpname[256];

	snprintf(dumpname, 256, "dump-%d", pid);
	tst_res(TINFO, "Dump file should be %s", dumpname);
	if (access(dumpname, F_OK))
		tst_brk(TCONF | TERRNO,
			"Dump file was not found; assuming this is because "
			"core dumps are disabled and not a bug");
	tc.dfd = SAFE_OPEN(dumpname, O_RDONLY);

	while (SAFE_READ(0, tc.dfd, &c, 1)) {
		switch (c) {
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

			if (ycount == tc.ycount)
				expectc = 'z';
			break;
		case 'z':
			if (expectc == 'z')
				return 1;
		default:
			expectc = 'x';
		}
	}
	return 0;
}

static void run_child(int advice)
{
	char *advstr =
		advice == MADV_DONTDUMP ? "MADV_DONTDUMP" : "MADV_DODUMP";

	if (madvise(tc.fmem, tc.fmemsize, advice) == -1) {
		tst_res(TFAIL | TERRNO,
			"madvise(%p, %lu, %s) = -1",
			tc.fmem,
			tc.fmemsize,
			advstr);
		return;
	}
	abort();
}

static void run(void)
{
	int i, status;
	pid_t pid;
	char *fmemc;

	tc.fmem = SAFE_MMAP(NULL,
			    tc.fmemsize,
			    PROT_READ | PROT_WRITE,
			    MAP_ANONYMOUS | MAP_PRIVATE,
			    -1,
			    0);
	/*Write a generated character sequence to the mapped memory,
	  which we later look for in the core dump.*/
	fmemc = (char *)tc.fmem;
	*fmemc = 'x';
	for (i = 0; i < tc.ycount; i++)
		fmemc[i + 1] = 'y';
	fmemc[++i] = 'z';

	pid = SAFE_FORK();
	if (pid == 0) {
		run_child(MADV_DONTDUMP);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (!(WIFSIGNALED(status) && WCOREDUMP(status)))
		tst_brk(TCONF, "No core dump produced after abort()");

	if (find_sequence(pid)) {
		tst_res(TFAIL, "Found sequence in dump despite MADV_DONTDUMP");
		tst_res(TFAIL, "MADV_DODUMP is an automatic fail");
		return;
	} else {
		tst_res(TPASS, "madvise(..., MADV_DONTDUMP");
	}

	pid = SAFE_FORK();
	if (pid == 0) {
		run_child(MADV_DODUMP);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (!(WIFSIGNALED(status) && WCOREDUMP(status)))
		tst_brk(TCONF, "No core dump produced after abort()");

	restore_cpattern();

	if (find_sequence(pid))
		tst_res(TPASS, "madvise(..., MADV_DODUMP)");
	else
		tst_res(TFAIL, "No sequence in dump after MADV_DODUMP.");
}

static struct tst_test test = {
	.tid = "madvise07",
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "3.4.0",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1
};

#else

static void run(void)
{
	tst_res(TCONF,
		"MADV_DONTDUMP and/or MADV_DODUMP missing, yet kernel "
		"is new enough to support it");
}

static struct tst_test test = {
	.tid = "madvise07",
	.test_all = run,
	.min_kver = "3.4.0"
};

#endif
