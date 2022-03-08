/*  time-schedule.c

    Programme to test how long a context switch takes.

    Copyright (C) 1998  Richard Gooch

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

/*
    This programme will determine the context switch (scheduling) overhead on
    a system. It takes into account SMP machines. True context switches are
    measured.

    Written by      Richard Gooch   15-SEP-1998

    Last updated by Richard Gooch   25-SEP-1998

*/
#include <unistd.h>
#ifdef _POSIX_THREADS
#ifndef _REENTRANT
#define _REENTRANT
#endif
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#define _POSIX_THREAD_SAFE_FUNCTIONS
#endif
#include <pthread.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#ifndef __KARMA__
#define mt_num_processors() 1	/*  Set to the number of processors   */
#define ERRSTRING strerror(errno)
#define FALSE 0
#define TRUE  1
#else
#include <karma.h>
#include <karma_mt.h>
#endif

#define MAX_ITERATIONS      1000

static unsigned int hog_other_cpus();
static void run_yielder(int use_threads, int read_fd);
static void *yielder_main(void *arg);
static void s_term_handler();
static void run_low_priority(unsigned int num, int read_fd);
static unsigned long compute_median(unsigned long values[MAX_ITERATIONS],
				    unsigned long max_value);
static unsigned int get_run_queue_size();
static unsigned long get_num_switches();
static void use_fpu_value(double val);

static volatile unsigned int sched_count = 0;
/*  For yielder  */
static int pipe_read_fd = -1;
static int pipe_write_fd = -1;
static pid_t child = -1;

int main(int argc, char **argv)
{
	int use_threads = FALSE;
	int use_pipe = FALSE;
	int no_test = FALSE;
	int frob_fpu = FALSE;
	int read_fd = -1;
	int write_fd = -1;
	int num_low_priority = -1;
	int i, j;
	int fds[2];
	unsigned int count, num_yields, run_queue_size1, run_queue_size2,
	    num_hogs;
	unsigned long median, switches1, num_switches, num_overhead_switches;
	signed long overhead, total_diffs;
	signed long min_diff = 1000000000;
	signed long max_diff = -1000000000;
	double dcount = 0.0;
	unsigned long diffs[MAX_ITERATIONS];
	struct timeval before, after;
	sigset_t set;
	static char *usage =
	    "time-schedule [-h] [-thread] [-notest] [-pipe] [-fpu] [num_running]";

	setpgrp();
	/*  First create pipe used to sychronise low priority processes  */
	if (pipe(fds) != 0) {
		fprintf(stderr, "Error creating pipe\t%s\n", ERRSTRING);
		exit(1);
	}
	read_fd = fds[0];
	pipe_write_fd = fds[1];
	for (count = 1; count < argc; ++count) {
		if (strcmp(argv[count], "-thread") == 0) {
#ifdef _POSIX_THREADS
			use_threads = TRUE;
#else
			fprintf(stderr, "POSIX threads not available\n");
#endif
		} else if (strcmp(argv[count], "-pipe") == 0)
			use_pipe = TRUE;
		else if (strcmp(argv[count], "-notest") == 0)
			no_test = TRUE;
		else if (strcmp(argv[count], "-fpu") == 0)
			frob_fpu = TRUE;
		else if (isdigit(argv[count][0]))
			num_low_priority = atoi(argv[count]);
		else {
			fprintf(stderr,
				"Programme to time context switches (schedules)\n");
			fprintf(stderr,
				"(C) 1998  Richard Gooch <rgooch@atnf.csiro.au>\n");
			fprintf(stderr, "Usage:\t%s\n", usage);
			fprintf(stderr,
				"\t-thread\t\tswitch threads not processes\n");
			fprintf(stderr,
				"\t-pipe\t\tuse pipes not sched_yield()\n");
			fprintf(stderr,
				"\t-fpu\t\tpollute the FPU after each switch in main\n");
			fprintf(stderr,
				"\tnum_running\tnumber of extra processes\n");
			exit(0);
		}
	}
	if (no_test) {
		if (num_low_priority > 0)
			run_low_priority(num_low_priority, read_fd);
		while (TRUE)
			pause();
	}
	if (geteuid() == 0) {
		struct sched_param sp;

		memset(&sp, 0, sizeof sp);
		sp.sched_priority = 10;
		if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) {
			fprintf(stderr, "Error changing to RT class\t%s\n",
				ERRSTRING);
			exit(1);
		}
		if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
			fprintf(stderr, "Error locking pages\t%s\n", ERRSTRING);
			exit(1);
		}
	} else
		fprintf(stderr, "Not running with RT priority!\n");
	/*  Give shell and login programme time to finish up and get off the run
	   queue  */
	usleep(200000);
	if (use_pipe) {
		if (pipe(fds) != 0) {
			fprintf(stderr, "Error creating pipe\t%s\n", ERRSTRING);
			exit(1);
		}
		pipe_read_fd = fds[0];
		write_fd = fds[1];
	}
	num_hogs = hog_other_cpus();
	/*  Determine overhead. Do it in a loop=2. The first iteration should warm
	   the cache, the second will compute the overhead  */
	for (j = 0; j < 2; ++j) {
		switches1 = get_num_switches();
		gettimeofday(&before, NULL);
		for (i = 0; i < 20; ++i) {
			if (use_pipe) {
				char ch = 0;

				write(pipe_write_fd, &ch, 1);
				read(read_fd, &ch, 1);
			} else
				sched_yield();
			if (frob_fpu)
				++dcount;
		}
		gettimeofday(&after, NULL);
		num_overhead_switches = get_num_switches() - switches1;
		overhead = 1000000 * (after.tv_sec - before.tv_sec);
		overhead += after.tv_usec - before.tv_usec;
	}
	use_fpu_value(dcount);
	if (num_low_priority > 0)
		run_low_priority(num_low_priority, read_fd);
	/*  Set up for the benchmark  */
	run_yielder(use_threads, read_fd);
	memset(diffs, 0, sizeof diffs);
	run_queue_size1 = get_run_queue_size();
	total_diffs = 0;
	switches1 = get_num_switches();
	/*  Benchmark!  */
	for (count = 0; count < MAX_ITERATIONS; ++count) {
		signed long diff;

		gettimeofday(&before, NULL);
		/*  Generate 20 context switches  */
		for (i = 0; i < 10; ++i) {
			if (use_pipe) {
				char ch = 0;

				write(write_fd, &ch, 1);
				read(read_fd, &ch, 1);
			} else
				sched_yield();
			if (frob_fpu)
				dcount += 1.0;
		}
		gettimeofday(&after, NULL);
		diff = 1000000 * (after.tv_sec - before.tv_sec);
		diff += after.tv_usec - before.tv_usec;
		diffs[count] = diff;
		total_diffs += diff;
		if (diff < min_diff)
			min_diff = diff;
		if (diff > max_diff)
			max_diff = diff;
	}
	num_yields = sched_count;
	run_queue_size2 = get_run_queue_size();
	num_switches = get_num_switches() - switches1;
	if (!use_threads)
		kill(child, SIGTERM);
	fprintf(stderr, "Started %u hog processes\n", num_hogs);
	fprintf(stderr, "Syscall%s overhead: %.1f us\n", frob_fpu ? "/FPU" : "",
		(double)overhead / 20.0);
	if (switches1 > 0)
		fprintf(stderr, "Num switches during overhead check: %lu\n",
			num_overhead_switches);
	fprintf(stderr, "Minimum scheduling latency: %.1f (%.1f) us\n",
		(double)min_diff / 20.0, (double)(min_diff - overhead) / 20.0);
	median = compute_median(diffs, max_diff);
	fprintf(stderr, "Median scheduling latency:  %.1f (%.1f) us\n",
		(double)median / 20.0, (double)(median - overhead) / 20.0);
	fprintf(stderr, "Average scheduling latency: %.1f (%.1f) us\n",
		(double)total_diffs / (double)MAX_ITERATIONS / 20.0,
		(double)(total_diffs - overhead * MAX_ITERATIONS) /
		(double)MAX_ITERATIONS / 20.0);
	fprintf(stderr, "Maximum scheduling latency: %.1f (%.1f) us\n",
		(double)max_diff / 20.0, (double)(max_diff - overhead) / 20.0);
	fprintf(stderr, "Run queue size: %u, %u\n",
		run_queue_size1, run_queue_size2);
	use_fpu_value(dcount);
	if (use_threads)
		fprintf(stderr, "Number of yields: %u\n", num_yields);
	if (num_switches > 0)
		fprintf(stderr, "Num switches: %lu\n", num_switches);

	/*  Terminate all child processes  */
	sigemptyset(&set);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	kill(0, SIGTERM);
	return (0);
}				/*  End Function main  */

static unsigned int hog_other_cpus()
/*  [SUMMARY] Hog other CPUs with a high-priority job.
    [RETURNS] The number of hogged CPUs.
*/
{
	unsigned int count;

	for (count = mt_num_processors(); count > 1; --count) {
		switch (fork()) {
		case 0:
			/*  Child  */
			while (TRUE) ;
			break;
		case -1:
			/*  Error  */
			fprintf(stderr, "Error forking\t%s\n", ERRSTRING);
			kill(0, SIGTERM);
			break;
		default:
			/*  Parent  */
			break;
		}
	}
	return mt_num_processors() - 1;
}				/*  End Function hog_other_cpus  */

static void run_yielder(int use_threads, int read_fd)
/*  [SUMMARY] Run other process which will continuously yield.
    <use_threads> If TRUE, the yielding process is just a thread.
    <read_fd> The pipe to read the synchronisation byte from.
    [RETURNS] Nothing.
*/
{
	char ch;
	struct sigaction new_action;
#ifdef _POSIX_THREADS
	pthread_t thread;

	if (use_threads) {
		if (pthread_create(&thread, NULL, yielder_main, NULL) != 0) {
			fprintf(stderr, "Error creating thread\t%s\n",
				ERRSTRING);
			kill(0, SIGTERM);
		}
		read(read_fd, &ch, 1);
		return;
	}
#endif
	switch (child = fork()) {
	case 0:
		/*  Child  */
		break;
	case -1:
		/*  Error  */
		fprintf(stderr, "Error forking\t%s\n", ERRSTRING);
		kill(0, SIGTERM);
		break;
	default:
		/*  Parent  */
		read(read_fd, &ch, 1);
		return;
		/*break; */
	}
	memset(&new_action, 0, sizeof new_action);
	sigemptyset(&new_action.sa_mask);
	new_action.sa_handler = s_term_handler;
	if (sigaction(SIGTERM, &new_action, NULL) != 0) {
		fprintf(stderr, "Error setting SIGTERM handler\t%s\n",
			ERRSTRING);
		exit(1);
	}
	yielder_main(NULL);
}				/*  End Function run_yielder  */

static void *yielder_main(void *arg)
/*  [SUMMARY] Yielder function.
    <arg> An arbirtrary pointer. Ignored.
    [RETURNS] NULL.
*/
{
	char ch = 0;

	sched_count = 0;
	write(pipe_write_fd, &ch, 1);
	while (TRUE) {
		if (pipe_read_fd >= 0) {
			read(pipe_read_fd, &ch, 1);
			write(pipe_write_fd, &ch, 1);
		} else
			sched_yield();
		++sched_count;
	}
	return (NULL);
}				/*  End Function yielder_main  */

static void s_term_handler()
{
	fprintf(stderr, "Number of yields: %u\n", sched_count);
	exit(0);
}				/*  End Function s_term_handler  */

static void run_low_priority(unsigned int num, int read_fd)
/*  [SUMMARY] Run low priority processes.
    <num> Number of processes.
    <read_fd> The pipe to read the synchronisation byte from.
    [RETURNS] Nothing.
*/
{
	char ch = 0;

	for (; num > 0; --num) {
		switch (fork()) {
		case 0:
			/*  Child  */
			if (geteuid() == 0) {
				struct sched_param sp;

				memset(&sp, 0, sizeof sp);
				sp.sched_priority = 0;
				if (sched_setscheduler(0, SCHED_OTHER, &sp) !=
				    0) {
					fprintf(stderr,
						"Error changing to SCHED_OTHER class\t%s\n",
						ERRSTRING);
					exit(1);
				}
			}
			if (nice(20) != 0) {
				fprintf(stderr, "Error nicing\t%s\n",
					ERRSTRING);
				kill(0, SIGTERM);
			}
			write(pipe_write_fd, &ch, 1);
			while (TRUE)
				sched_yield();
			break;
		case -1:
			/*  Error  */
			fprintf(stderr, "Error forking\t%s\n", ERRSTRING);
			kill(0, SIGTERM);
			break;
		default:
			/*  Parent  */
			read(read_fd, &ch, 1);
			break;
		}
	}
}				/*  End Function run_low_priority  */

static unsigned long compute_median(unsigned long values[MAX_ITERATIONS],
				    unsigned long max_value)
/*  [SUMMARY] Compute the median from an array of values.
    <values> The array of values.
    <max_value> The maximum value in the array.
    [RETURNS] The median value.
*/
{
	unsigned long count;
	unsigned long median = 0;
	unsigned long peak = 0;
	unsigned long *table;

	/*  Crude but effective  */
	if ((table = calloc(max_value + 1, sizeof *table)) == NULL) {
		fprintf(stderr, "Error allocating median table\n");
		exit(1);
	}
	for (count = 0; count < MAX_ITERATIONS; ++count) {
		++table[values[count]];
	}
	/*  Now search for peak. Position of peak is median  */
	for (count = 0; count < max_value + 1; ++count) {
		if (table[count] < peak)
			continue;
		peak = table[count];
		median = count;
	}
	free(table);
	return (median);
}				/*  End Function compute_median  */

static unsigned int get_run_queue_size()
/*  [SUMMARY] Compute the current size of the run queue.
    [RETURNS] The length of the run queue.
*/
{
	int dummy_i;
	unsigned int length = 0;
	FILE *fp;
	DIR *dp;
	struct dirent *de;
	char txt[64], dummy_str[64];

	if ((dp = opendir("/proc")) == NULL)
		return (0);
	while ((de = readdir(dp)) != NULL) {
		if (!isdigit(de->d_name[0]))
			continue;
		sprintf(txt, "/proc/%s/stat", de->d_name);
		if ((fp = fopen(txt, "r")) == NULL)
			return (length);
		fscanf(fp, "%d %s %s", &dummy_i, dummy_str, txt);
		if (txt[0] == 'R')
			++length;
		fclose(fp);
	}
	closedir(dp);
	return (length);
}				/*  End Function get_run_queue_size  */

static unsigned long get_num_switches()
/*  [SUMMARY] Get the number of context switches.
    [RETURNS] The number of context switches on success, else 0.
*/
{
	unsigned long val;
	FILE *fp;
	char line[256], name[64];

	if ((fp = fopen("/proc/stat", "r")) == NULL)
		return (0);

	while (fgets(line, sizeof line, fp) != NULL) {
		if (sscanf(line, "%s %lu", name, &val) != 2)
			continue;

		if (strcasecmp(name, "ctxt") != 0)
			continue;

		fclose(fp);
		return (val);
	}
	fclose(fp);
	return (0);
}				/*  End Function get_num_switches  */

static void use_fpu_value(double val)
/*  [SUMMARY] Dummy function to consume FPU value. Fools compiler.
    <val> The value.
    [RETURNS] Nothing.
*/
{
}				/*  End Function use_fpu_value  */
