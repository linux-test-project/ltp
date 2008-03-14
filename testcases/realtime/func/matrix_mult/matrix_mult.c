/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008
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
 *      matrix_mult.c
 *
 * DESCRIPTION
 *      Compare running sequential matrix multiplication routines
 *      to running them in parallel to judge mutliprocessor
 *      performance
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *      Use "-j" to enable jvm simulator.
 *
 *      Compilation : gcc -lrt -lpthread matrix_mult.c
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-Mar-9:  Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libjvmsim.h>
#include <libstats.h>

#define PRIO 89
#define MATRIX_SIZE 100
#define DEF_OPS_MULTIPLIER 500		/* the higher the number, the more CPU intensive
					(and therefore SMP performance goes up) */
#define ITERATIONS 100
#define HIST_BUCKETS 100

static int run_jvmsim = 0;
static int ops_multiplier = DEF_OPS_MULTIPLIER;
static int ops;

void usage(void)
{
	rt_help();
	printf("matrix_mult specific options:\n");
	printf("  -j            enable jvmsim\n");
	printf("  -l#           #: number of multiplications per iteration per cpu (load)\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'j':
		run_jvmsim = 1;
		break;
	case 'l':
		ops_multiplier = atoi(v);
		break;
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

void matrix_init(float A[MATRIX_SIZE][MATRIX_SIZE], float B[MATRIX_SIZE][MATRIX_SIZE])
{
	int i, j;
	for (i = 0; i < MATRIX_SIZE; i++) {
		for (j = 0; j < MATRIX_SIZE; j++) {
			A[i][j] = (float)i*j;
			B[i][j] = (float)((i*j)%10);
		}
	}
}

void matrix_mult(void)
{
	float A[MATRIX_SIZE][MATRIX_SIZE];
	float B[MATRIX_SIZE][MATRIX_SIZE];
	float C[MATRIX_SIZE][MATRIX_SIZE];
	int i, j, k;

	matrix_init(A, B);

	for (i = 0; i < MATRIX_SIZE; i++) {
		for (j = 0; j < MATRIX_SIZE; j++) {
			for (k = 0; k < MATRIX_SIZE; k++) {
				C[i][j] += A[i][k]*B[k][j];
			}
		}
	}
}

/* arg: the number of concurrent threads being run */
void *matrixmult_thread(void *thread)
{
	struct thread *t = (struct thread *)thread;
	int i;

	for (i = 0; i < ops/(intptr_t)t->arg; i++) {
		matrix_mult();
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i, j;
	nsec_t start, end;
	long smin=0, smax=0, cmin=0, cmax=0, delta=0;
	float savg, cavg;
	int ret;
	int numcpus;
	int criteria;

	setup();
	rt_init("jl:h", parse_args, argc, argv);
	numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	criteria = MAX(1, numcpus/2); // the minimum avg concurrent multiplier to pass
	ops = numcpus * ops_multiplier;

	int tids[numcpus];

	printf("\n---------------------------------------\n");
	printf("Matrix Multiplication (SMP Performance)\n");
	printf("---------------------------------------\n\n");
	printf("Running %d iterations\n", ITERATIONS);
	printf("Matrix Dimensions: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
	printf("Calculations per iteration: %d\n", ops);
	printf("Number of CPUs: %u\n", numcpus);

	if (run_jvmsim) {
		printf("jvmsim enabled\n");
		jvmsim_init();	// Start the JVM simulation
	} else {
		printf("jvmsim disabled\n");
	}

	stats_container_t sdat, cdat;
	stats_container_t shist, chist;
	if (	stats_container_init(&sdat, ITERATIONS) ||
		stats_container_init(&shist, HIST_BUCKETS) ||
		stats_container_init(&cdat, ITERATIONS) ||
		stats_container_init(&chist, HIST_BUCKETS)
	)
	{
		fprintf (stderr, "Cannot init stats container\n");
		exit(1);
	}

	// run matrix mult operation sequentially
	printf("\nSequential:\n");
	for (i = 0; i < ITERATIONS; i++) {
		start = rt_gettime();
		tids[0] = create_fifo_thread(matrixmult_thread, (void*)1, PRIO);
		if (tids[0] == -1) {
			printf("Thread creation failed (max threads exceeded?)\n");
			break;
		}
		join_thread(tids[0]);
		end = rt_gettime();
		delta = (long)((end - start)/NS_PER_US);
		sdat.records[i].x = i;
		sdat.records[i].y = delta;
		if (i == 0)
			smin = smax = delta;
		else {
			smin = MIN(smin, delta);
			smax = MAX(smax, delta);
		}
	}
	savg = stats_avg(&sdat);
	printf("Min: %ld us\n", smin);
	printf("Max: %ld us\n", smax);
	printf("Avg: %.4f us\n", savg);
	printf("StdDev: %.4f us\n", stats_stddev(&sdat));

	if (
		stats_hist(&shist, &sdat) ||

		stats_container_save("sequential", "Matrix Multiplication Sequential Execution Runtime Scatter Plot",
				"Iteration", "Runtime (us)", &sdat, "points") ||
		stats_container_save("sequential_hist", "Matrix Multiplicatoin Sequential Execution Runtime Histogram",
				"Runtime (us)", "Samples", &shist, "steps")
	) {
		fprintf(stderr, "Warning: could not save sequential mults stats\n");
	}

	// run matrix mult operation concurrently
	printf("\nConcurrent (%dx):\n", numcpus);
	for (i = 0; i < ITERATIONS; i++) {
		start = rt_gettime();
		for (j = 0; j < numcpus; j++) {
			tids[j] = create_fifo_thread(matrixmult_thread, (void*)(intptr_t)numcpus, PRIO);
			if (tids[j] == -1) {
				printf("Thread creation failed (max threads exceeded?)\n");
				break;
			}
		}
		if (tids[j] == -1) {
			printf("j=%d\n", j);
			for (j=0;j<numcpus;j++)
				printf("tids[%d]=%d\n", j, tids[j]);
			break;
		}
		for (j = 0; j < numcpus; j++) {
			join_thread(tids[j]);
		}
		end = rt_gettime();
		delta = (long)((end - start)/NS_PER_US);
		cdat.records[i].x = i;
		cdat.records[i].y = delta;
		if (i == 0)
			cmin = cmax = delta;
		else {
			cmin = MIN(cmin, delta);
			cmax = MAX(cmax, delta);
		}
	}
	cavg = stats_avg(&cdat);
	printf("Min: %ld us\n", cmin);
	printf("Max: %ld us\n", cmax);
	printf("Avg: %.4f us\n", cavg);
	printf("StdDev: %.4f us\n", stats_stddev(&cdat));

	if (
		stats_hist(&chist, &cdat) ||

		stats_container_save("concurrent", "Matrix Multiplication Concurrent Execution Runtime Scatter Plot",
					"Iteration", "Runtime (us)", &cdat, "points") ||
		stats_container_save("concurrent_hist", "Matrix Multiplication Concurrent Execution Runtime Histogram",
					"Iteration", "Runtime (us)", &chist, "steps")
	) {
		fprintf(stderr, "Warning: could not save concurrent mults stats\n");
	}

	printf("\nSeq/Conc Ratios:\n");
	printf("Min: %.4f\n", (float)smin/cmin);
	printf("Max: %.4f\n", (float)smax/cmax);
	printf("Avg: %.4f\n", (float)savg/cavg);

	ret = 1;
	if (savg > (cavg * criteria))
		ret = 0;
	printf("\nCriteria: %d * average concurrent time < average sequential time\n",
		criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return ret;
}
