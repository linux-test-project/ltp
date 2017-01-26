/*
 * Copyright (c) 2016 Richard Palethorpe <richiejp@f-m.fm>
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
 * Test flow: create child process, 
 *	      map memory, 
 *	      mark memory with MADV_HWPOISON inside child process,
 *	      access memory,
 *	      if SIGBUS is delivered to child the test passes else it fails
 */

#include "tst_test.h"
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define MAPTYPE(m) m == MAP_SHARED ? "MAP_SHARED" : "MAP_PRIVATE"

static int maptypes[] = {
	MAP_PRIVATE,
	MAP_SHARED
};

static void run_child(int maptype)
{
	const size_t msize = 4096;
	void *mem = NULL;

	mem = SAFE_MMAP(NULL,
			msize,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | maptype,
			-1,
			0);

	if (madvise(mem, msize, MADV_HWPOISON) == -1) {
		tst_res(TFAIL | TERRNO,
			"madvise(%p, %ld, MADV_HWPOISON) = -1",
			mem,
			msize);
		return;
	}

	*((char *)mem) = 'd';

	tst_res(TFAIL,
		"Did not receive SIGBUS after accessing %s memory marked "
		"with MADV_HWPOISON",
		MAPTYPE(maptype));
}

static void run(unsigned int n)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		run_child(maptypes[n]);
		return;
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGBUS)
		tst_res(TPASS,
			"madvise(..., MADV_HWPOISON) on %s memory",
			MAPTYPE(maptypes[n]));
}

static struct tst_test test = {
	.tid = "madvise07",
	.test = run,
	.tcnt = ARRAY_SIZE(maptypes),
	.min_kver = "2.6.31",
	.needs_root = 1,
	.forks_child = 1
};

