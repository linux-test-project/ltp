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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2007-Mar-09:  Initial version by Darren Hart <dvhltc@us.ibm.com>
 *      2008-Feb-26:  Closely emulate jvm Dinakar Guniguntala <dino@in.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

#define MAX_CPUS	8192
#define PRIO		43
#define MATRIX_SIZE	100
#define DEF_OPS		8	/* the higher the number, the more CPU intensive */
					/* (and therefore SMP performance goes up) */
#define PASS_CRITERIA	0.75	/* Avg concurrent time * pass criteria < avg seq time - */
					/* for every addition of a cpu */
#define ITERATIONS	128
#define HIST_BUCKETS	100

#define THREAD_WAIT	1
#define THREAD_WORK	2
#define THREAD_DONE	3

#define THREAD_SLEEP	1 * NS_PER_US

static int ops = DEF_OPS;
static int numcpus;
static float criteria;
static int *tids;
static int online_cpu_id = -1;
static int iterations = ITERATIONS;
static int iterations_percpu;

stats_container_t sdat, cdat, *curdat;
stats_container_t shist, chist;
static pthread_barrier_t mult_start;
static pthread_mutex_t mutex_cpu;

void usage(void)
{
	rt_help();
	printf("matrix_mult specific options:\n");
	printf
	    ("  -l#	   #: number of multiplications per iteration (load)\n");
	printf("  -i#	   #: number of iterations\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'i':
		iterations = atoi(v);
		break;
	case 'l':
		ops = atoi(v);
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

void matrix_init(double A[MATRIX_SIZE][MATRIX_SIZE],
		 double B[MATRIX_SIZE][MATRIX_SIZE])
{
	int i, j;
	for (i = 0; i < MATRIX_SIZE; i++) {
		for (j = 0; j < MATRIX_SIZE; j++) {
			A[i][j] = (double)(i * j);
			B[i][j] = (double)((i * j) % 10);
		}
	}
}

void matrix_mult(int m_size)
{
	double A[m_size][m_size];
	double B[m_size][m_size];
	double C[m_size][m_size];
	int i, j, k;

	matrix_init(A, B);
	for (i = 0; i < m_size; i++) {
		int i_m = m_size - i;
		for (j = 0; j < m_size; j++) {
			double sum = A[i_m][j] * B[j][i];
			for (k = 0; k < m_size; k++)
				sum += A[i_m][k] * B[k][j];
			C[i][j] = sum;
		}
	}
}

void matrix_mult_record(int m_size, int index)
{
	nsec_t start, end, delta;
	int i;

	start = rt_gettime();
	for (i = 0; i < ops; i++)
		matrix_mult(MATRIX_SIZE);
	end = rt_gettime();
	delta = (long)((end - start) / NS_PER_US);
	curdat->records[index].x = index;
	curdat->records[index].y = delta;
}

int set_affinity(void)
{
	cpu_set_t mask;
	int cpuid;

	pthread_mutex_lock(&mutex_cpu);
	do {
		++online_cpu_id;
		CPU_ZERO(&mask);
		CPU_SET(online_cpu_id, &mask);

		if (!sched_setaffinity(0, sizeof(mask), &mask)) {
			cpuid = online_cpu_id;	/* Save this value before unlocking mutex */
			pthread_mutex_unlock(&mutex_cpu);
			return cpuid;
		}
	} while (online_cpu_id < MAX_CPUS);
	pthread_mutex_unlock(&mutex_cpu);
	return -1;
}

void *concurrent_thread(void *thread)
{
	struct thread *t = (struct thread *)thread;
	int thread_id = (intptr_t) t->id;
	int cpuid;
	int i;
	int index;

	cpuid = set_affinity();
	if (cpuid == -1) {
		fprintf(stderr, "Thread %d: Can't set affinity.\n", thread_id);
		exit(1);
	}

	index = iterations_percpu * thread_id;	/* To avoid stats overlapping */
	pthread_barrier_wait(&mult_start);
	for (i = 0; i < iterations_percpu; i++)
		matrix_mult_record(MATRIX_SIZE, index++);

	return NULL;
}

int main_thread(void)
{
	int ret, i, j;
	nsec_t start, end;
	long smin = 0, smax = 0, cmin = 0, cmax = 0, delta = 0;
	float savg, cavg;
	int cpuid;

	if (stats_container_init(&sdat, iterations) ||
	    stats_container_init(&shist, HIST_BUCKETS) ||
	    stats_container_init(&cdat, iterations) ||
	    stats_container_init(&chist, HIST_BUCKETS)
	    ) {
		fprintf(stderr, "Cannot init stats container\n");
		exit(1);
	}

	tids = malloc(sizeof(int) * numcpus);
	if (!tids) {
		perror("malloc");
		exit(1);
	}
	memset(tids, 0, numcpus);

	cpuid = set_affinity();
	if (cpuid == -1) {
		fprintf(stderr, "Main thread: Can't set affinity.\n");
		exit(1);
	}

	/* run matrix mult operation sequentially */
	curdat = &sdat;
	curdat->index = iterations - 1;
	printf("\nRunning sequential operations\n");
	start = rt_gettime();
	for (i = 0; i < iterations; i++)
		matrix_mult_record(MATRIX_SIZE, i);
	end = rt_gettime();
	delta = (long)((end - start) / NS_PER_US);

	savg = delta / iterations;	/* don't use the stats record, use the total time recorded */
	smin = stats_min(&sdat);
	smax = stats_max(&sdat);

	printf("Min: %ld us\n", smin);
	printf("Max: %ld us\n", smax);
	printf("Avg: %.4f us\n", savg);
	printf("StdDev: %.4f us\n", stats_stddev(&sdat));

	if (stats_hist(&shist, &sdat) ||
	    stats_container_save("sequential",
				 "Matrix Multiplication Sequential Execution Runtime Scatter Plot",
				 "Iteration", "Runtime (us)", &sdat, "points")
	    || stats_container_save("sequential_hist",
				    "Matrix Multiplicatoin Sequential Execution Runtime Histogram",
				    "Runtime (us)", "Samples", &shist, "steps")
	    ) {
		fprintf(stderr,
			"Warning: could not save sequential mults stats\n");
	}

	pthread_barrier_init(&mult_start, NULL, numcpus + 1);
	set_priority(PRIO);
	curdat = &cdat;
	curdat->index = iterations - 1;
	online_cpu_id = -1;	/* Redispatch cpus */
	/* Create numcpus-1 concurrent threads */
	for (j = 0; j < numcpus; j++) {
		tids[j] = create_fifo_thread(concurrent_thread, NULL, PRIO);
		if (tids[j] == -1) {
			printf
			    ("Thread creation failed (max threads exceeded?)\n");
			exit(1);
		}
	}

	/* run matrix mult operation concurrently */
	printf("\nRunning concurrent operations\n");
	pthread_barrier_wait(&mult_start);
	start = rt_gettime();
	join_threads();
	end = rt_gettime();

	delta = (long)((end - start) / NS_PER_US);

	cavg = delta / iterations;	/* don't use the stats record, use the total time recorded */
	cmin = stats_min(&cdat);
	cmax = stats_max(&cdat);

	printf("Min: %ld us\n", cmin);
	printf("Max: %ld us\n", cmax);
	printf("Avg: %.4f us\n", cavg);
	printf("StdDev: %.4f us\n", stats_stddev(&cdat));

	if (stats_hist(&chist, &cdat) ||
	    stats_container_save("concurrent",
				 "Matrix Multiplication Concurrent Execution Runtime Scatter Plot",
				 "Iteration", "Runtime (us)", &cdat, "points")
	    || stats_container_save("concurrent_hist",
				    "Matrix Multiplication Concurrent Execution Runtime Histogram",
				    "Iteration", "Runtime (us)", &chist,
				    "steps")
	    ) {
		fprintf(stderr,
			"Warning: could not save concurrent mults stats\n");
	}

	printf("\nConcurrent Multipliers:\n");
	printf("Min: %.4f\n", (float)smin / cmin);
	printf("Max: %.4f\n", (float)smax / cmax);
	printf("Avg: %.4f\n", (float)savg / cavg);

	ret = 1;
	if (savg > (cavg * criteria))
		ret = 0;
	printf
	    ("\nCriteria: %.2f * average concurrent time < average sequential time\n",
	     criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return ret;
}

int main(int argc, char *argv[])
{
	setup();
	pass_criteria = PASS_CRITERIA;
	rt_init("l:i:h", parse_args, argc, argv);
	numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	/* the minimum avg concurrent multiplier to pass */
	criteria = pass_criteria * numcpus;
	int new_iterations, ret;

	if (iterations <= 0) {
		fprintf(stderr, "iterations must be greater than zero\n");
		exit(1);
	}

	printf("\n---------------------------------------\n");
	printf("Matrix Multiplication (SMP Performance)\n");
	printf("---------------------------------------\n\n");

	/* Line below rounds up iterations to a multiple of numcpus.
	 * Without this, having iterations not a mutiple of numcpus causes
	 * stats to segfault (overflow stats array).
	 */
	new_iterations = (int)((iterations + numcpus - 1) / numcpus) * numcpus;
	if (new_iterations != iterations)
		printf
		    ("Rounding up iterations value to nearest multiple of total online CPUs\n");

	iterations = new_iterations;
	iterations_percpu = iterations / numcpus;

	printf("Running %d iterations\n", iterations);
	printf("Matrix Dimensions: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
	printf("Calculations per iteration: %d\n", ops);
	printf("Number of CPUs: %u\n", numcpus);

	set_priority(PRIO);
	ret = main_thread();

	return ret;
}
