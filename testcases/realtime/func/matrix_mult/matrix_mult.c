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
 *      2007-Mar-09:  Initial version by Darren Hart <dvhltc@us.ibm.com>
 *      2008-Feb-26:  Closely emulate jvm Dinakar Guniguntala <dino@in.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libjvmsim.h>
#include <libstats.h>

#define PRIO		43
#define MATRIX_SIZE	100
#define DEF_OPS		8		/* the higher the number, the more CPU intensive */
					/* (and therefore SMP performance goes up) */
#define PASS_CRITERIA	0.75		/* Avg concurrent time * pass criteria < avg seq time - */
					/* for every addition of a cpu */
#define ITERATIONS	128		/* HAS to be a multiple of 'numcpus' */
#define HIST_BUCKETS	100

#define THREAD_WAIT	1
#define THREAD_WORK	2
#define THREAD_DONE	3

#define THREAD_SLEEP	1 * NS_PER_US

static int run_jvmsim = 0;
static int ops = DEF_OPS;
static int numcpus;
static float criteria;
static int *mult_index;
static int *tids;
static int *flags;

stats_container_t sdat, cdat, *curdat;
stats_container_t shist, chist;
static pthread_barrier_t mult_start;

int gettid(void)
{
	return syscall(__NR_gettid);
}

void usage(void)
{
	rt_help();
	printf("matrix_mult specific options:\n");
	printf("  -j            enable jvmsim\n");
	printf("  -l#           #: number of multiplications per iteration (load)\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'j':
		run_jvmsim = 1;
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

void matrix_init(double  A[MATRIX_SIZE][MATRIX_SIZE], double  B[MATRIX_SIZE][MATRIX_SIZE])
{
	int i, j;
	for (i = 0; i < MATRIX_SIZE; i++) {
		for (j = 0; j < MATRIX_SIZE; j++) {
			A[i][j] = (double) (i*j);
			B[i][j] = (double) ((i*j)%10);
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
				sum += A[i_m][k]*B[k][j];
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
	delta = (long)((end - start)/NS_PER_US);
	curdat->records[index].x = index;
	curdat->records[index].y = delta;
}

int set_affinity(int cpuid)
{
	int tid = gettid();
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);

	if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
		printf("Thread %d: Can't set affinity: %s\n", tid, strerror(errno));
		exit(1);
	}

	return 0;
}

void *concurrent_thread(void *thread)
{
	struct thread *t = (struct thread *)thread;
	int thread_id = (intptr_t)t->arg;

	set_affinity(thread_id);
	pthread_barrier_wait(&mult_start);
	while (flags[thread_id] != THREAD_DONE) {
		pthread_mutex_lock(&t->mutex);
		flags[thread_id] = THREAD_WAIT;
		do {
			if (pthread_cond_wait(&t->cond, &t->mutex) != 0) {
				printf("cond_wait error!");
				exit(1);
			}
		} while (flags[thread_id] == THREAD_WAIT);
		pthread_mutex_unlock(&t->mutex);
		if (flags[thread_id] == THREAD_WORK)
			matrix_mult_record(MATRIX_SIZE, mult_index[thread_id]++);
	}

	return NULL;
}

void concurrent_ops(void)
{
	static int thread_id = 0;

	if (thread_id < (numcpus-1)) {
		struct timespec nsleep;
		struct thread *t;

		while (flags[thread_id] != THREAD_WAIT) {
			nsleep.tv_sec = 0;
			nsleep.tv_nsec = THREAD_SLEEP;
			nanosleep(&nsleep, NULL);
		}
		t = get_thread(tids[thread_id]);
		pthread_mutex_lock(&t->mutex);
		flags[thread_id] = THREAD_WORK;
		pthread_cond_signal(&t->cond);
		pthread_mutex_unlock(&t->mutex);
	} else
		matrix_mult_record(MATRIX_SIZE, mult_index[thread_id]++);

	if (++thread_id == numcpus)
		thread_id = 0;
}

void main_thread(void)
{
	int ret, i, j;
	nsec_t start, end;
	long smin = 0, smax = 0, cmin = 0, cmax = 0, delta = 0;
	float savg, cavg;

	if (	stats_container_init(&sdat, ITERATIONS) ||
		stats_container_init(&shist, HIST_BUCKETS) ||
		stats_container_init(&cdat, ITERATIONS/numcpus) ||
		stats_container_init(&chist, HIST_BUCKETS)
	)
	{
		fprintf (stderr, "Cannot init stats container\n");
		exit(1);
	}

	pthread_barrier_init(&mult_start, NULL, numcpus);

	mult_index = malloc(sizeof(int) * numcpus);
	if (!mult_index) {
		perror("malloc");
		exit(1);
	}
	memset(mult_index, 0, numcpus);
	tids = malloc(sizeof(int) * numcpus);
	if (!tids) {
		perror("malloc");
		exit(1);
	}
	memset(tids, 0, numcpus);
	flags = malloc(sizeof(int) * numcpus);
	if (!flags) {
		perror("malloc");
		exit(1);
	}
	memset(flags, 0, numcpus);

	set_affinity(numcpus-1);

	/* run matrix mult operation sequentially */
	curdat = &sdat;
	printf("\nRunning sequential operations\n");
	start = rt_gettime();
	for (i = 0; i < ITERATIONS; i++)
		matrix_mult_record(MATRIX_SIZE, i);
	end = rt_gettime();
	delta = (long)((end - start)/NS_PER_US);

	savg = delta/ITERATIONS; /* don't use the stats record, use the total time recorded */
	smin = stats_min(&sdat);
	smax = stats_max(&sdat);

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

	/* Create numcpus-1 concurrent threads */
	for (j = 0; j < (numcpus-1); j++) {
		tids[j] = create_fifo_thread(concurrent_thread, (void *)(intptr_t)j, PRIO);
		if (tids[j] == -1) {
			printf("Thread creation failed (max threads exceeded?)\n");
			break;
		}
	}

	pthread_barrier_wait(&mult_start);

	/* run matrix mult operation concurrently */
	curdat = &cdat;
	printf("\nRunning concurrent operations (%dx)\n", ITERATIONS);
	start = rt_gettime();
	for (i = 0; i < ITERATIONS; i++)
		concurrent_ops();
	end = rt_gettime();
	delta = (long)((end - start)/NS_PER_US);

	for (j = 0; j < (numcpus-1); j++) {
		struct thread *t;

		t = get_thread(tids[j]);
		pthread_mutex_lock(&t->mutex);
		flags[j] = THREAD_DONE;
		pthread_cond_signal(&t->cond);
		pthread_mutex_unlock(&t->mutex);
	}

	cavg = delta/ITERATIONS; /* don't use the stats record, use the total time recorded */
	cmin = stats_min(&cdat);
	cmax = stats_max(&cdat);

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

	printf("\nConcurrent Multipliers:\n");
	printf("Min: %.4f\n", (float)smin/cmin);
	printf("Max: %.4f\n", (float)smax/cmax);
	printf("Avg: %.4f\n", (float)savg/cavg);

	ret = 1;
	if (savg > (cavg * criteria))
		ret = 0;
	printf("\nCriteria: %.2f * average concurrent time < average sequential time\n",
		criteria);
	printf("Result: %s\n", ret ? "FAIL" : "PASS");

	return;
}

int main(int argc, char *argv[])
{
	setup();
	rt_init("jl:h", parse_args, argc, argv);
	numcpus = sysconf(_SC_NPROCESSORS_ONLN);
	/* the minimum avg concurrent multiplier to pass, TODO: make configurable */
	criteria = PASS_CRITERIA * numcpus;

	printf("\n---------------------------------------\n");
	printf("Matrix Multiplication (SMP Performance)\n");
	printf("---------------------------------------\n\n");
	printf("Running %d iterations\n", ITERATIONS);
	printf("Matrix Dimensions: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
	printf("Calculations per iteration: %d\n", ops);
	printf("Number of CPUs: %u\n", numcpus);

	if (run_jvmsim) {
		printf("jvmsim enabled\n");
		jvmsim_init();	/* Start the JVM simulation */
	} else {
		printf("jvmsim disabled\n");
	}

	set_priority(PRIO);
	main_thread();

	join_threads();

	return 0;
}
