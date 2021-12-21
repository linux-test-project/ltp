// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (C) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Author: Manoj Iyer, IBM Austin TX <manjo@austin.ibm.com>, 2001
 *
 * Tests the LINUX memory manager. The program is aimed at stressing the memory
 * manager by repeaded map/write/unmap of file/memory of random size (maximum
 * 1GB) this is done by multiple threads.
 *
 * Create a file of random size upto 1000 times 4096, map it, change the
 * contents of the file and unmap it. This is repeated several times for the
 * specified number of hours by a certain number of threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "tst_safe_pthread.h"
#include "tst_test.h"

static char *str_loops;
static char *str_threads;
static char *map_private;
static char *str_exec_time;

static int loops = 1000;
static int threads = 40;
static float exec_time = 24;

static volatile int sig_caught;
static int threads_running;

static int mkfile(int *size)
{
	int fd;
	int index = 0;
	char buf[4096];
	char template[PATH_MAX];

	memset(buf, 'a', 4096);
	snprintf(template, PATH_MAX, "ashfileXXXXXX");
	if ((fd = mkstemp(template)) == -1)
		tst_brk(TBROK | TERRNO, "mkstemp()");
	unlink(template);

	*size = (1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) * 4096;

	while (index < *size) {
		index += sizeof(buf);
		SAFE_WRITE(1, fd, buf, sizeof(buf));
	}

	fsync(fd);

	return fd;
}

static void exit_thread(void) __attribute__ ((noreturn));

static void exit_thread(void)
{
	tst_atomic_dec(&threads_running);
	pthread_exit(NULL);
}

void *map_write_unmap(void *args)
{
	int fsize;
	int fd;
	int i;
	void *addr;
	long tid = (long)args;

	tst_atomic_inc(&threads_running);

	for (i = 0; i < loops; i++) {
		if (sig_caught)
			exit_thread();

		if ((fd = mkfile(&fsize)) == -1)
			exit_thread();

		addr = SAFE_MMAP(NULL, fsize, PROT_WRITE | PROT_READ,
				 map_private ? MAP_PRIVATE : MAP_SHARED, fd, 0);

		memset(addr, 'A', fsize);

		tst_res(TINFO, "Thread %4li, addr [%p], size %4ikB, iter %4d",
			tid, addr, fsize/1024, i);

		usleep(1);

		SAFE_MUNMAP(addr, fsize);
		SAFE_CLOSE(fd);
	}

	exit_thread();
}

static void sig_handler(int signal)
{
	sig_caught = signal;
}

static void test_mmap(void)
{
	long i;
	pthread_t thids[threads];

	alarm(exec_time * 3600);

	while (!sig_caught) {
		for (i = 0; i < threads; i++) {
			SAFE_PTHREAD_CREATE(&thids[i], NULL,
			                    map_write_unmap, (void*)i);
			sched_yield();
		}

		for (i = 0; i < threads; i++)
			SAFE_PTHREAD_JOIN(thids[i], NULL);
	}

	if (sig_caught == SIGALRM) {
		tst_res(TPASS, "Test passed");
	} else {
		tst_res(TFAIL, "Unexpected signal caught %s",
		        tst_strsig(sig_caught));
	}
}

static void setup(void)
{
	if (tst_parse_int(str_loops, &loops, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of loops '%s'", str_loops);

	if (tst_parse_int(str_threads, &threads, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of threads '%s'", str_threads);

	if (tst_parse_float(str_exec_time, &exec_time, 0.0005, 9000))
		tst_brk(TBROK, "Invalid execution time '%s'", str_exec_time);

	tst_set_timeout(exec_time * 3600 + 300);

	SAFE_SIGNAL(SIGALRM, sig_handler);
	SAFE_SIGNAL(SIGBUS, sig_handler);
	SAFE_SIGNAL(SIGSEGV, sig_handler);

	unsigned int seed = time(NULL) % 100;

	srand(seed);

	tst_res(TINFO, "Seed %u", seed);
	tst_res(TINFO, "Number of loops %i", loops);
	tst_res(TINFO, "Number of threads %i", threads);
	tst_res(TINFO, "MAP_PRIVATE = %i", map_private ? 1 : 0);
	tst_res(TINFO, "Execution time %fH", exec_time);
}

static void cleanup(void)
{
	static int flag;

	if (tst_atomic_inc(&flag) != 1)
		exit_thread();

	if (!threads_running)
		return;

	tst_res(TINFO, "Waiting for %i threads to terminate", threads_running);

	sig_caught = 1;

	while ((volatile int)threads_running > 1) {
		tst_res(TINFO, "Running threads %i",
		        (volatile int)threads_running);
		usleep(500000);
	}
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"l:", &str_loops, "Number of map-write-unmap loops"},
		{"n:", &str_threads, "Number of worker threads"},
		{"p", &map_private, "Turns on MAP_PRIVATE (default MAP_SHARED)"},
		{"x:", &str_exec_time, "float Execution time in hours (default 24H)"},
		{}
	},
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_mmap,
};
