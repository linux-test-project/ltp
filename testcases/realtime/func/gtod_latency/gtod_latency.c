/******************************************************************************
 *
 *   Copyright  International Business Machines  Corp., 2007
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *      gtod-latency.c
 *
 * DESCRIPTION
 *       Simple program to measure the time between several pairs of calls to
 *       gettimeofday().  If the average delta is greater than just a few
 *       microseconds on an unloaded system, then something is probably wrong.
 *
 *       It is quite similar to the programs in the directory, but provides the
 *       additional capability to produce graphical output as a histogram or a
 *       scatter graph.*
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      Use "-j" to enable jvm simulator.
 *
 *      Compilation : gcc -lrt -lpthread gtod_latency.c
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Aug-17: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *      2006-Aug-23: Minor changes by John Kacur <jekacur@ca.ibm.com>
 *      2006-Nov-20: Augmented to use libstats
 *      2007-Jul-12: Latency tracing added by Josh Triplett <josh@kernel.org>
 *      2007-Jul-13: Quantiles added by Josh Triplett <josh@kernel.org>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>
#include <limits.h>
#include <libstats.h>
#include <librttest.h>

#define ITERATIONS 1000000
#define HIST_BUCKETS 20
#define QUANTILE_NINES 6

#define SCATTER_FILENAME	0
#define HIST_FILENAME		1

#define SCATTER_TITLE		0
#define HIST_TITLE		1

#define SCATTER_LABELX		0
#define SCATTER_LABELY		1
#define HIST_LABELX		2
#define HIST_LABELY		3

char *titles[] = {"scatter plot",
		  "histogram"};

char *filenames[] = {"scatter",
		     "hist"};

char *labels[] = {"scatter plot x-axis",
		  "scatter plot y-axis",
		  "histogram x-axis",
		  "histogram y-axis"};

static unsigned long long latency_threshold = 0;

void stats_cmdline_help(void)
{
	printf("Usage: ./gtod_latency {-[so|scatter-output] -[ho|hist-output]"
		" -[st|scatter-title] -[ht|hist-title] -[sxl|scatter-xlabel]"
		" -[syl|scatter-ylabel] -[hxl|hist-xlabel] -[hyl|hist-ylabel]"
		" -[lt|latency-trace]}"
		" -[help] \n");
	printf("**command-line options are not supported yet for this testcase\n");
}

int stats_cmdline(int argc, char *argv[])
{
	int i;
	char *flag;

	if (argc == 1)
		return 0;

	for(i = 1; i < argc; i++) {
		if (*argv[i] != '-') {
			printf("missing flag indicator\n");
			return -1;
		}

		flag = ++argv[i];

		if (!strcmp(flag, "help") || !strcmp(flag, "h")) {
			stats_cmdline_help();
			exit(0);
		}

		if (!strcmp(flag, "so") || !strcmp(flag, "scatter-output")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			filenames[SCATTER_FILENAME] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "ho") || !strcmp(flag, "hist-output")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			filenames[HIST_FILENAME] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "st") || !strcmp (flag, "scatter-title")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			titles[SCATTER_TITLE] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "ht") || !strcmp (flag, "hist-title")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			titles[HIST_TITLE] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "sxl") || !strcmp(flag, "scatter-xlabel")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			labels[SCATTER_LABELX] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "syl") || !strcmp(flag, "scatter-ylabel")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			labels[SCATTER_LABELY] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "hxl") || !strcmp(flag, "hist-xlabel")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			labels[HIST_LABELX] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "hyl") || !strcmp(flag, "hist-ylabel")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			labels[HIST_LABELY] = argv[++i];
			continue;
		}

		if (!strcmp(flag, "lt") || !strcmp(flag, "latency-trace")) {
			if (i + 1 == argc) {
				printf("flag has missing argument\n");
				return -1;
			}
			latency_threshold = strtoull(argv[++i], NULL, 0);
			continue;
		}

		printf("unknown flag given\n");
		return -1;
	}

	return 0;
}


/* return difference in microseconds */
unsigned long long tv_minus(struct timeval *tv_start, struct timeval *tv_end)
{
	unsigned long long usecs;
	usecs = (tv_end->tv_sec - tv_start->tv_sec) * 1000000;
	usecs += tv_end->tv_usec - tv_start->tv_usec;
	return usecs;
}

int main(int argc, char *argv[])
{
	int i, err;
	struct timeval tv_a, tv_b;
	unsigned long long delta;
	unsigned long long max, min;
	struct sched_param param;
	stats_container_t dat;
	stats_container_t hist;
	stats_quantiles_t quantiles;

	stats_container_init(&dat, ITERATIONS);
	stats_container_init(&hist, HIST_BUCKETS);
	stats_quantiles_init(&quantiles, QUANTILE_NINES);
	setup();

	if (stats_cmdline(argc, argv) < 0) {
		printf("usage: %s help\n", argv[0]);
		exit(1);
	}

	/* switch to SCHED_FIFO 99 */
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	err = sched_setscheduler(0, SCHED_FIFO, &param);

	/* Check that the user has the appropriate privileges */
	if (err) {
		if (errno == EPERM) {
			fprintf(stderr, "This program runs with a scheduling policy of SCHED_FIFO at priority %d\n", param.sched_priority);
			fprintf(stderr, "You don't have the necessary privileges to create such a real-time process.\n");
		} else {
                	fprintf(stderr, "Failed to set scheduler, errno %d\n", errno);
		}
		exit(1);
	}

	printf("\n----------------------\n");
	printf("Gettimeofday() Latency\n");
	printf("----------------------\n");
	printf("Iterations: %d\n\n", ITERATIONS);

	/* collect ITERATIONS pairs of gtod calls */
	max = min = 0;
	if (latency_threshold) {
		latency_trace_enable();
		latency_trace_start();
	}
	for (i = 0; i < ITERATIONS; i++) {
		gettimeofday(&tv_a, NULL);
		gettimeofday(&tv_b, NULL);
		delta = tv_minus(&tv_a, &tv_b);
		dat.records[i].x = i;
		dat.records[i].y = delta;
		if (i == 0 || delta < min) min = delta;
		if (delta > max) max = delta;
		if (latency_threshold && delta > latency_threshold)
			break;
	}
	if (latency_threshold) {
		latency_trace_stop();
		if (i != ITERATIONS) {
			printf("Latency threshold (%lluus) exceeded at iteration %d\n",
			       latency_threshold, i);
			latency_trace_print();
			stats_container_resize(&dat, i + 1);
		}
	}

	stats_hist(&hist, &dat);
	stats_container_save(filenames[SCATTER_FILENAME], titles[SCATTER_TITLE],
			     labels[SCATTER_LABELX], labels[SCATTER_LABELY], &dat, "points");
	stats_container_save(filenames[HIST_FILENAME], titles[HIST_TITLE],
			     labels[HIST_LABELX], labels[HIST_LABELY], &hist, "steps");

	/* report on deltas */
	printf("Min: %llu us\n", min);
	printf("Max: %llu us\n", max);
	printf("Avg: %.4f us\n", stats_avg(&dat));
	printf("StdDev: %.4f us\n", stats_stddev(&dat));
	printf("Quantiles:\n");
	stats_quantiles_calc(&dat, &quantiles);
	stats_quantiles_print(&quantiles);


	return 0;
}
