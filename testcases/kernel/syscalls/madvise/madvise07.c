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
 *
 * When MAP_PRIVATE is set (without MAP_POPULATE) madvise() may error with
 * EBUSY on the first attempt and succeed on the second, but without poisoning
 * any memory. A private mapping is only populated with pages once it is
 * accessed and poisoning an unmapped VM range is essentially undefined
 * behaviour. However madvise() itself causes the address to be mapped to the
 * zero page. If/when the zero page can be poisoned then the test may pass
 * without any error. For now we just consider it a configuration failure.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "tst_test.h"
#include "lapi/mmap.h"

static int maptypes[] = {
	MAP_PRIVATE,
	MAP_PRIVATE | MAP_POPULATE,
	MAP_SHARED
};

static char *mapname(int maptype)
{
	switch (maptype) {
	case MAP_PRIVATE: return "MAP_SHARED";
	case MAP_PRIVATE | MAP_POPULATE: return "MAP_PRIVATE | MAP_POPULATE";
	case MAP_SHARED: return "MAP_SHARED";
	default:
		tst_res(TWARN, "Unknown map type: %d", maptype);
	}
	return "Unknown";
}

static void run_child(int maptype)
{
	const size_t msize = getpagesize();
	void *mem = NULL;
	int first_attempt = 1;

	tst_res(TINFO,
		"mmap(..., MAP_ANONYMOUS | %s, ...)", mapname(maptype));
	mem = SAFE_MMAP(NULL,
			msize,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | maptype,
			-1,
			0);

do_madvise:
	tst_res(TINFO, "madvise(%p, %zu, MADV_HWPOISON)", mem, msize);
	if (madvise(mem, msize, MADV_HWPOISON) == -1) {
		if (errno == EINVAL) {
			tst_res(TCONF | TERRNO,
				"CONFIG_MEMORY_FAILURE probably not set in kconfig");
		} else if (errno == EBUSY && maptype == MAP_PRIVATE) {
			tst_res(TCONF,
				"Madvise failed with EBUSY");
			if (first_attempt--)
				goto do_madvise;
		} else {
			tst_res(TFAIL | TERRNO, "Could not poison memory");
		}
		exit(0);
	}

	*((char *)mem) = 'd';

	if (maptype == MAP_PRIVATE) {
		tst_res(TCONF,
			"Zero page poisoning is probably not implemented");
	} else {
		tst_res(TFAIL,
			"Did not receive SIGBUS after accessing %s memory marked with MADV_HWPOISON",
			mapname(maptype));
	}
}

static void run(unsigned int n)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		run_child(maptypes[n]);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGBUS)
		tst_res(TPASS,
			"madvise(..., MADV_HWPOISON) on %s memory",
			mapname(maptypes[n]));
	else if (WIFEXITED(status) && WEXITSTATUS(status) == TBROK)
		tst_res(TBROK, "Child exited abnormally");
}

static struct tst_test test = {
	.tid = "madvise07",
	.test = run,
	.tcnt = ARRAY_SIZE(maptypes),
	.min_kver = "2.6.31",
	.needs_root = 1,
	.forks_child = 1
};

