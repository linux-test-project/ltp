// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) International Business Machines Corp., 2001
 *  Copyright (c) Linux Test Project., 2019
 *
 *  DESCRIPTION:
 *
 *  mtest01 mallocs memory <chunksize> at a time until malloc fails.
 *
 *  Parent process starts several child processes (each child process is
 *  tasked with allocating some amount of memory), it waits until all child
 *  processes send SIGRTMIN signal and resumes all children by sending the
 *  SIGCONT signal.
 *
 *  Child process allocates certain amount of memory and fills it with some
 *  data (the '-w' option) so the pages are actually allocated when the desired
 *  amount of memory is allocated then it sends SIGRTMIN signal to the parent
 *  process, it pauses itself by raise SIGSTOP until get parent SIGCONT signal
 *  to continue and exit.
 */

#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lapi/abisize.h"
#include "tst_test.h"

#define FIVE_HUNDRED_MB         (500ULL*1024*1024)

#if defined(__s390__) || defined(__s390x__)
#define ALLOC_THRESHOLD		FIVE_HUNDRED_MB
#elif defined(TST_ABI32)
#define ALLOC_THRESHOLD		(2*FIVE_HUNDRED_MB)
#elif defined(TST_ABI64)
#define ALLOC_THRESHOLD		(6*FIVE_HUNDRED_MB)
#endif

#define STOP_THRESHOLD 15	/* seconds remaining before reaching timeout */

static pid_t *pid_list;
static sig_atomic_t children_done;
static int max_pids;
static unsigned long long alloc_maxbytes;

static int chunksize = 1024*1024;
static int maxpercent = 20;
static long maxbytes = 0;
static char *dowrite;
static char *verbose;

static char *opt_chunksize, *opt_maxbytes, *opt_maxpercent;
static struct tst_option mtest_options[] = {
	{"c:", &opt_chunksize,	"-c  size of chunk in bytes to malloc on each pass"},
	{"b:", &opt_maxbytes,	"-b  maximum number of bytes to allocate before stopping"},
	{"p:", &opt_maxpercent, "-p  percent of total memory used at which the program stops"},
	{"w",  &dowrite,   	"-w  write to the memory after allocating"},
	{"v",  &verbose,     	"-v  verbose"},
	{NULL, NULL, 		NULL}
};

static void parse_mtest_options(char *str_chunksize, int *chunksize,
		char *str_maxbytes, long *maxbytes,
		char *str_maxpercent, int *maxpercent)
{
	if (str_chunksize)
		if (tst_parse_int(str_chunksize, chunksize, 1, INT_MAX))
			tst_brk(TBROK, "Invalid chunksize '%s'", str_chunksize);

	if (str_maxbytes) {
		if (tst_parse_long(str_maxbytes, maxbytes, 1, LONG_MAX)) {
			tst_brk(TBROK, "Invalid maxbytes '%s'", str_maxbytes);
		} else if (str_maxpercent) {
			tst_brk(TBROK, "ERROR: -b option cannot be used with -p "
					"option at the same time");
		}
		alloc_maxbytes = (unsigned long long)maxbytes;
	}

	if (str_maxpercent) {
		if (tst_parse_int(str_maxpercent, maxpercent, 1, 99)) {
			tst_brk(TBROK, "Invalid maxpercent '%s'", str_maxpercent);
		} else if (str_maxbytes) {
			tst_brk(TBROK, "ERROR: -p option cannot be used with -b "
					"option at the same time");
		}
	}
}

static void handler(int sig LTP_ATTRIBUTE_UNUSED)
{
        children_done++;
}

static void do_write_mem(char *mem, int chunksize)
{
	int i, pagesz = getpagesize();

	for (i = 0; i < chunksize; i += pagesz)
		*(mem + i) = 'a';
}

static void setup(void)
{
	struct sysinfo sstats;
	unsigned long long total_free;

	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGRTMIN, &act, 0);

	parse_mtest_options(opt_chunksize, &chunksize,
			opt_maxbytes, &maxbytes,
			opt_maxpercent, &maxpercent);
	sysinfo(&sstats);
	total_free = sstats.freeram;

	max_pids = total_free * sstats.mem_unit
		/ (unsigned long)ALLOC_THRESHOLD + 10;
	pid_list = SAFE_MALLOC(max_pids * sizeof(pid_t));

	if (!alloc_maxbytes) {
		/* set alloc_maxbytes to the extra amount we want to allocate */
		alloc_maxbytes = ((float)maxpercent / 100.00)
			* (sstats.mem_unit * total_free);
		tst_res(TINFO, "Filling up %d%% of free ram which is %llu kbytes",
			 maxpercent, alloc_maxbytes / 1024);
	}
}

static void cleanup(void)
{
	if(pid_list)
		free(pid_list);
}

static void child_loop_alloc(unsigned long long alloc_bytes)
{
	unsigned long bytecount = 0;
	char *mem;

	tst_res(TINFO, "... child %d starting", getpid());

	while (1) {
		mem = SAFE_MALLOC(chunksize);
		if (dowrite)
			do_write_mem(mem, chunksize);

		if (verbose)
			tst_res(TINFO,
				"child %d allocated %lu bytes chunksize is %d",
				getpid(), bytecount, chunksize);
		bytecount += chunksize;
		if (bytecount >= alloc_bytes)
			break;
	}
	if (dowrite)
		tst_res(TINFO, "... [t=%d] %lu bytes allocated and used in child %d",
				tst_timeout_remaining(), bytecount, getpid());
	else
		tst_res(TINFO, "... [t=%d] %lu bytes allocated only in child %d",
				tst_timeout_remaining(), bytecount, getpid());

	kill(getppid(), SIGRTMIN);
	raise(SIGSTOP);
	exit(0);
}

static void mem_test(void)
{
	pid_t pid;
	int i = 0, pid_cntr = 0;
	unsigned long long alloc_bytes = alloc_maxbytes;
	const char *write_msg = "";

	if (dowrite)
		write_msg = "(and written to) ";

	/* to make mtest01 support -i N */
	children_done = 0;

	do {
		pid = SAFE_FORK();
		if (pid == 0) {
			alloc_bytes = MIN(ALLOC_THRESHOLD, alloc_bytes);
			child_loop_alloc(alloc_bytes);
		}

		pid_list[pid_cntr++] = pid;

		if (alloc_bytes <= ALLOC_THRESHOLD)
			break;

		alloc_bytes -= ALLOC_THRESHOLD;
	} while (pid_cntr < max_pids);

	/* wait in the loop for all children finish allocating */
	while (children_done < pid_cntr) {
		if (tst_timeout_remaining() < STOP_THRESHOLD) {
			tst_res(TWARN,
				"the remaininig time is not enough for testing");

			break;
		}

		usleep(100000);
	}

	if (children_done < pid_cntr) {
		tst_res(TFAIL, "kbytes allocated %sless than expected %llu",
				write_msg, alloc_maxbytes / 1024);

		for (i = 0; i < pid_cntr; i++)
			kill(pid_list[i], SIGKILL);

		return;
	}

	tst_res(TPASS, "%llu kbytes allocated %s",
			alloc_maxbytes / 1024, write_msg);

	for (i = 0; i < pid_cntr; i++) {
		TST_PROCESS_STATE_WAIT(pid_list[i], 'T', 0);
		kill(pid_list[i], SIGCONT);
	}
}

static struct tst_test test = {
	.forks_child = 1,
	.options = mtest_options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = mem_test,
};
