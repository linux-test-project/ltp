
/*
 * Cheesy program to create a "graph" of nodes, spawn threads and
 * walk the graph.
 */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#define _GNU_SOURCE
#include <sys/mman.h>
#include <malloc.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <strings.h>
#include <time.h>
//#define __USE_GNU
#include <sched.h>

static int seed_random(void);
static void populate_graph(void *region, unsigned long long node_count);
static void alarm_func(int signum);
static void print_help(const char *name);

static struct timeval last;
volatile unsigned long speed = 0;
static unsigned long report_interval = 30;

/*
 * A quick note: each graph "node" consists of some pointer off to another
 * part of the graph array.
 */

static void print_help(const char *name)
{
	printf
	    ("Usage: %s [-p num_threads] [-d ram_divisor | -n num_nodes] [-s report_intrvl] [-a add_intrvl] [-t]\n",
	     name);
	printf
	    ("-d ram_divisor:	Use (total_ram / ram_divisor) as a graph (16).\n");
	printf("-p num_threads:	Start up some number of threads (1).\n");
	printf("-n num_nodes:	Create a graph with some number of nodes.\n");
	printf("-s report_intvl	Seconds between speed reports (30).\n");
	printf("-a add_intrvl:	Seconds between adding children (never).\n");
#ifdef __cpu_set_t_defined
	printf
	    ("-t:		Assign each process to its own processor (no).\n");
#else
	printf("-t:		Not enabled because you need kernel 2.5.8+.\n");
#endif
}

static void populate_graph(void *graph, unsigned long long node_count)
{
	unsigned long i;
	void **ptr;
	unsigned long gunk;

	seed_random();

	/* Each cell of the array points to another place in the array. */
	for (i = 0, ptr = graph; i < node_count; i++, ptr++) {
		gunk = (node_count - 1) * (rand() / (RAND_MAX + 1.0));
		*ptr = (void *)(graph + (gunk * sizeof(void *)));
	}
}

static int seed_random(void)
{
	int fp;
	long seed;

	fp = open("/dev/urandom", O_RDONLY);
	if (fp < 0) {
		perror("/dev/urandom");
		return 0;
	}

	if (read(fp, &seed, sizeof(seed)) != sizeof(seed)) {
		perror("read random seed");
		return 0;
	}

	close(fp);
	srand(seed);

	return 1;
}

static void alarm_func(int signum)
{
	struct timeval now;
	float time;

	gettimeofday(&now, NULL);
	time = (now.tv_usec + (now.tv_sec * 1000000))
	    - (last.tv_usec + (last.tv_sec * 1000000));
	time /= 1000000;

	printf("%d: %.0f nodes/sec.\n", getpid(), speed / time);
	fflush(stdout);
	speed = 0;
	last = now;

	alarm(report_interval);
}

static void walk_graph(void *graph)
{
	void **curr = graph;

	while (1) {
		curr = *curr;
		speed++;
	}
}

int main(int argc, char *argv[])
{
	unsigned long long num_nodes, ram_size;
	unsigned long num_forks = 1;
	struct sysinfo info;
	void *shm;
	int *cond;
	struct sigaction zig;
	int c, add_wait = -1, is_parent = 1;
#ifdef __cpu_set_t_defined
	int affinity = 0;
	cpu_set_t my_cpu_mask;
#endif

	/* By default we'll use 1/16th of total RAM, rounded
	 * down to the nearest page. */
	if (sysinfo(&info) != 0) {
		perror("sysinfo");
		return 1;
	}

	ram_size = info.totalram / 16;
	ram_size = ram_size & ~(getpagesize() - 1);
	num_nodes = ram_size / sizeof(void *);

	/* Parse command line args */
	while ((c = getopt(argc, argv, "a:p:n:d:s:t")) != -1) {
		switch (c) {
		case 'p':
			num_forks = atoi(optarg);
			break;
		case 'd':
			ram_size = info.totalram / atoi(optarg);
			ram_size = ram_size & ~(getpagesize() - 1);
			num_nodes = ram_size / sizeof(void *);
			break;
		case 'n':
			num_nodes = atoi(optarg);
			ram_size = num_nodes * sizeof(void *);
			break;
		case 's':
			report_interval = atoi(optarg);
			break;
		case 'a':
			add_wait = atoi(optarg);
			break;
#ifdef __cpu_set_t_defined
		case 't':
			affinity = 1;
			break;
#endif
		default:
			print_help(argv[0]);
			return 0;
		}
	}

	/* Will we exceed half the address space size?  Use 1/4 of it at most.  */
	if (ram_size > ((unsigned long long)1 << ((sizeof(void *) * 8) - 1))) {
		printf
		    ("Was going to use %lluKB (%llu nodes) but that's too big.\n",
		     ram_size / 1024, num_nodes);
		ram_size = ((unsigned long long)1 << (sizeof(void *) * 8));
		ram_size /= 4;
		num_nodes = ram_size / sizeof(void *);
		printf("Clamping to %lluKB (%llu nodes) instead.\n",
		       ram_size / 1024, num_nodes);
	}

	/* Talk about what we're going to do. */
	printf("Going to use %lluKB (%llu nodes).\n", ram_size / 1024,
	       num_nodes);

	/* Make a shared anonymous map of the RAM */
	shm = mmap(NULL, ram_size, PROT_READ | PROT_WRITE,
		   MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if (shm == MAP_FAILED) {
		perror("mmap");
		return 2;
	}
	printf("mmap region: %p (%llu nodes)\n", shm, num_nodes);

	/* Create an SHM condition variable.  Bogus, I know... */
	cond = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if (cond == MAP_FAILED) {
		perror("mmap");
		return 4;
	}
	*cond = 1;

	/* Create a "graph" by populating it with random pointers. */
	printf("Populating nodes...");
	fflush(stdout);
	populate_graph(shm, num_nodes);
	printf("done.\n");

	printf("Creating %lu processes with reports every %lu seconds \
and %d seconds between adding children.\n", num_forks, report_interval, add_wait);

	/* Fork off separate processes.  The shared region is shared
	 * across all children.  If we only wanted one thread, we shouldn't
	 * fork anything.  Note that the "cond" mmap is a really crappy
	 * condition variable kludge that works well enough for HERE ONLY. */
	for (c = (add_wait >= 0 ? 0 : 1); c < num_forks; c++) {
		/* Child should wait for the condition and then break. */
		if (!fork()) {
#ifdef __cpu_set_t_defined
			if (affinity) {
				CPU_ZERO(&my_cpu_mask);
				CPU_SET(c, &my_cpu_mask);
				if (0 !=
				    sched_setaffinity(0, sizeof(cpu_set_t),
						      &my_cpu_mask)) {
					perror("sched_setaffinity");
				}
			}
#endif

			is_parent = 0;
			while (*cond) {
				usleep(10000);
			}
			break;
		}
	}
	if (is_parent) {
#ifdef __cpu_set_t_defined
		if (affinity) {
			CPU_ZERO(&my_cpu_mask);
			CPU_SET(0, &my_cpu_mask);
			if (0 !=
			    sched_setaffinity(0, sizeof(cpu_set_t),
					      &my_cpu_mask)) {
				perror("sched_setaffinity");
			}
		}
#endif
		printf("All threads created.  Launching!\n");
		*cond = 0;
	}

	/* now start the work */
	if (!is_parent) {
start_thread:
		/* Set up the alarm handler to print speed info. */
		memset(&zig, 0x00, sizeof(zig));
		zig.sa_handler = alarm_func;
		sigaction(SIGALRM, &zig, NULL);
		gettimeofday(&last, NULL);
		alarm(report_interval);

		/* Walk the graph. */
		walk_graph(shm);

		/* This function never returns */
	} else {
		/* Start the ramp-up.  The children will never die,
		 * so we don't need to wait() for 'em.
		 */
		while (add_wait != -1) {
			sleep(add_wait);
			if (fork() == 0) {
				/* goto is cheesy, but works. */
				goto start_thread;
			} else {
				printf("Added thread.\n");
			}
		}
		goto start_thread;
	}

	return 0;
}
