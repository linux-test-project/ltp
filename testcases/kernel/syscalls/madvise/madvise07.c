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
 * Check that accessing memory marked with MADV_HWPOISON results in SIGBUS.
 *
 * Test flow: map memory, 
 *	      create child process, 
 *	      mark memory with MADV_HWPOISON inside child process,
 *	      access memory,
 *	      if SIGBUS is delivered to child the test passes else it fails
 */

#include "tst_test.h"
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#ifdef MADV_HWPOISON

struct test_case {
	int fd;
	char *fname;
	char *fcontent;
	void *fmem;
	struct stat stat;
};
static struct test_case cases[1];

static void setup(void)
{
	cases[0].fd = 0;
	cases[0].fcontent = "abcdefghijklmnopqrstuvwxyz12345\n";
	cases[0].fname = "mmf0";
	cases[0].fmem = NULL;
}

static void cleanup(void)
{
	unsigned int i;
	struct test_case c;

	for (i = 0; i < ARRAY_SIZE(cases); i++) {
		c = cases[i];
		if (c.fmem != NULL && munmap(c.fmem, c.stat.st_size))
			tst_brk(TBROK | TERRNO, "munmap(cases[%d].fmem)", i);

		c.fmem = NULL;
		if (c.fd > 0 && close(c.fd))
			tst_brk(TBROK | TERRNO, "close(cases[%d].fd)", i);
	}
}

static void on_sigbus(int s __attribute__((__unused__)))
{
	tst_res(TPASS, "Received SIGBUS from madvise with MADV_HWPOISON");
	_exit(0);
}

static void run(unsigned int n)
{
	int i, rval;
	struct test_case c = cases[n];
	struct sigaction sa;
	pid_t pid;

	c.fd = SAFE_OPEN(c.fname, O_CREAT | O_RDWR, 0667);
	for (i = 0; i < 1280; i++)
		SAFE_WRITE(1, c.fd, c.fcontent, strlen(c.fcontent));

	SAFE_FSTAT(c.fd, &c.stat);
	c.fmem = SAFE_MMAP(NULL,
			   c.stat.st_size,
			   PROT_READ | PROT_WRITE,
			   MAP_SHARED,
			   c.fd,
			   0);

	pid = SAFE_FORK();
	if (pid > 0) {
		SAFE_WAITPID(pid, NULL, 0);
		return;
	}

	rval = madvise(c.fmem, c.stat.st_size, MADV_HWPOISON);
	if (rval == -1) {
		tst_res(TFAIL | TERRNO,
			"madvise(%p, %ld, MADV_HWPOISON) = -1",
			c.fmem,
			c.stat.st_size);
		return;
	}

	sa.sa_handler = on_sigbus;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGBUS, &sa, NULL);

	*((char *)c.fmem) = 'd';

	tst_res(TFAIL,
		"Did not receive SIGBUS after accessing memory marked "
		"with MADV_HWPOISON");
}

static struct tst_test test = {
	.tid = "madvise07",
	.test = run,
	.tcnt = ARRAY_SIZE(cases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "2.6.31",
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1
};

#else

static void run(void)
{
	tst_res(TCONF,
		"MADV_HWPOISON is missing yet kernel is new enough to "
		" support it");
}

static struct tst_test test = {
	.tid = "madvise07",
	.test_all = run,
	.min_kver = "2.6.31"
};

#endif
