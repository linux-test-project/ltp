/*
 *  lat-sched  by Davide Libenzi ( linux kernel scheduler latency tester )
 *  Version 0.21 - Copyright (C) 2001  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 *
 *  The purpose of this tool is to measure the scheduler latency over a given
 *  runqueue length. The major difference with lat_ctx is that during the test
 *  time the runqueue is exactly NRTASK, while with lat_ctx ( that uses pipes )
 *  processes are typically sleeping on the pipe read.
 *  Build:
 *
 *  gcc -O0 -o lat-sched lat-sched.c
 *
 *  Use:
 *
 *  lat-sched [--ntasks n] [--ttime s] [--size b] [--stksize b] [--stkalign b]
 *            [--max-eck k] [--verbose] [--help]
 *
 *  --ntask    = Set the number of tasks ( runqueue length )
 *  --ttime    = Set thetest measure time in seconds
 *  --size     = Set the cache drain size in Kb
 *  --stksize  = Set the stack size for tasks
 *  --stkalign = Set the shift each task stack will have over stksize
 *  --max-eck  = Set the maximum correction factor above which the measure is invalid
 *  --verbose  = Activate verbose mode
 *  --help     = Print help screen
 *
 *
 *  Output: NRTASK MSRUN CSSEC LATSCH AVGWRK THRZ CHISQR
 *
 *  NRTASK   = Number of test tasks
 *  MSRUN    = Number of milliseconds the test is ran
 *  CSSEC    = Number of context switches / sec
 *  LATSCH   = Number of seconds for a context switch
 *  AVGWRK   = Number of context switches / NRTASK
 *  THRZ     = Number of task with zero context switch
 *  CHISQR   = Context switches chi-square over tasks
 *
 *  In case the measure will result invalid ( according to eck ) the output line
 *  will begin with a '*' character.
 *  Messages are printed on <stderr> while results are printed on <stdout>
 *
 *  IMPORTANT: To make this test work with a number of tasks that is less or equal
 *  to the number of CPUs the sys_sched_yield() optimization must be removed from
 *  the kernel sources in <kernel/sched.c>.
 *  This is a working sys_sched_yield() for kernel 2.4.10 :
 *
 *             asmlinkage long sys_sched_yield(void) {
 *                     if (current->policy == SCHED_OTHER)
 *                             current->policy |= SCHED_YIELD;
 *                     current->need_resched = 1;
 *                     return 0;
 *             }
 *
 */


#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <asm/atomic.h>



#define CACHELINE_SIZE	32


#define SSUMTIME		1
#define MAXTASKS		2048
#define MEASURE_TIME	4
#define STK_SIZE		512



char *stacks;
int *data;
int datasize = 0, stksize = STK_SIZE, stkalign = 0;
pid_t thr[MAXTASKS];
int nbtasks = 2;
int measure_time = MEASURE_TIME;
double eck = 0.9;
volatile atomic_t actthreads = ATOMIC_INIT(0);
long long int totalwork[MAXTASKS];
volatile int stop = 0, start = 0, count = 0;
int verbose = 0;


/*
 * cacheload() code comes from Larry McVoy LMBench
 * Bring howmuch data into the cache, assuming that the smallest cache
 * line is 16/32 bytes.
 */
int cacheload(int howmuch) {
	int done, sum = 0;
	register int *d = data;

#if CACHELINE_SIZE == 16

#define ASUM  sum+=d[0]+d[4]+d[8]+d[12]+d[16]+d[20]+d[24]+d[28]+\
		d[32]+d[36]+d[40]+d[44]+d[48]+d[52]+d[56]+d[60]+\
		d[64]+d[68]+d[72]+d[76]+d[80]+d[84]+d[88]+d[92]+\
		d[96]+d[100]+d[104]+d[108]+d[112]+d[116]+d[120]+d[124];\
		d+=128;	/* ints; bytes == 512 */

#elif CACHELINE_SIZE == 32

#define ASUM  sum+=d[0]+d[8]+d[16]+d[24]+d[32]+d[40]+d[48]+d[56]+\
		d[64]+d[72]+d[80]+d[88]+d[96]+d[104]+d[112]+d[120];\
		d+=128;	/* ints; bytes == 512 */

#else

#define	ASUM	sum+=d[0]+d[1]+d[2]+d[3]+d[4]+d[5]+d[6]+d[7]+d[8]+d[9]+\
		d[10]+d[11]+d[12]+d[13]+d[14]+d[15]+d[16]+d[17]+d[18]+d[19]+\
		d[20]+d[21]+d[22]+d[23]+d[24]+d[25]+d[26]+d[27]+d[28]+d[29]+\
		d[30]+d[31]+d[32]+d[33]+d[34]+d[35]+d[36]+d[37]+d[38]+d[39]+\
		d[40]+d[41]+d[42]+d[43]+d[44]+d[45]+d[46]+d[47]+d[48]+d[49]+\
		d[50]+d[51]+d[52]+d[53]+d[54]+d[55]+d[56]+d[57]+d[58]+d[59]+\
		d[60]+d[61]+d[62]+d[63]+d[64]+d[65]+d[66]+d[67]+d[68]+d[69]+\
		d[70]+d[71]+d[72]+d[73]+d[74]+d[75]+d[76]+d[77]+d[78]+d[79]+\
		d[80]+d[81]+d[82]+d[83]+d[84]+d[85]+d[86]+d[87]+d[88]+d[89]+\
		d[90]+d[91]+d[92]+d[93]+d[94]+d[95]+d[96]+d[97]+d[98]+d[99]+\
		d[100]+d[101]+d[102]+d[103]+d[104]+\
		d[105]+d[106]+d[107]+d[108]+d[109]+\
		d[110]+d[111]+d[112]+d[113]+d[114]+\
		d[115]+d[116]+d[117]+d[118]+d[119]+\
		d[120]+d[121]+d[122]+d[123]+d[124]+d[125]+d[126]+d[127];\
		d+=128;	/* ints; bytes == 512 */

#endif

#define	ONEKB	ASUM ASUM

	for (done = 0; done < howmuch; done += 1024) {
		ONEKB
	}
	return sum;
}


void taskproc(unsigned int thr) {
	long long int counter = 0;

	while (!start)
		usleep(100000);

	atomic_inc((atomic_t *) &actthreads);

	while (!stop) {
		if (count) {
			++counter;
			cacheload(datasize);
		}
		syscall(158);           /* sys_sched_yield() */
	}
	totalwork[thr] = counter;
	atomic_dec((atomic_t *) &actthreads);
	exit(0);
}



unsigned long long getmstics(void) {
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (0);

	return 1000 * (unsigned long long) tv.tv_sec + (unsigned long long) tv.tv_usec / 1000;
}


void usage(char *prg) {
	fprintf(stderr,
			"use: %s [--ntasks n {%d}] [--ttime s {%d}] [--size b {%d}] [--stksize b {%d}]\n"
			"\t[--stkalign b {%d}] [--max-eck k {%lf}] [--verbose] [--help]\n",
			prg, nbtasks, measure_time, datasize, stksize, stkalign, eck);
}


main(int argc, char **argv) {
	int i, status, avgwork, thrzero = 0, stkrsiz;
	long long int value = 0, avgvalue;
	double sqrdev;
	unsigned long long ts, te, sts, ste, ttime, sits, is, tcorr;
	char const *ustr = "";

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--ntasks") == 0) {
			if (++i < argc)
				nbtasks = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--ttime") == 0) {
			if (++i < argc)
				measure_time = atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--stksize") == 0) {
			if (++i < argc)
				stksize = atoi(argv[i]), stksize &= ~(CACHELINE_SIZE - 1);
			continue;
		}
		if (strcmp(argv[i], "--size") == 0) {
			if (++i < argc)
				datasize = atoi(argv[i]) * 1024;
			continue;
		}
		if (strcmp(argv[i], "--max-eck") == 0) {
			if (++i < argc)
				eck = atof(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--stkalign") == 0) {
			if (++i < argc)
				stkalign = atoi(argv[i]), stkalign &= ~(CACHELINE_SIZE - 1);
			continue;
		}
		if (strcmp(argv[i], "--verbose") == 0) {
			verbose = 1;
			continue;
		}
		if (strcmp(argv[i], "--help") == 0) {
			usage(argv[0]);
			exit(9);
		}
	}

	if (nbtasks > MAXTASKS)
		nbtasks = MAXTASKS;

	if (datasize)
		data = (int *) malloc(datasize);

	stkrsiz = stkalign + stksize;
	if (!(stacks = (char *) malloc((nbtasks + 1) * stkrsiz))) {
		perror("stack alloc");
		exit(1);
	}

	if (verbose) fprintf(stderr, "\ncreating %d tasks ...", nbtasks);
	for (i = 0; i < nbtasks; i++) {
		thr[i] = __clone((void *) &taskproc, stacks + (i + 1) * stkrsiz,
				CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND, (void *) i);
		if (thr[i] == -1) {
			perror("clone");
			exit(2);
		}
	}

	for (i = 0; i < nbtasks; i++)
		totalwork[i] = 0;

	if (verbose) fprintf(stderr, " ok\nwaiting for all tasks to start ...");

	start = 1;
	while (atomic_read(&actthreads) != nbtasks)
		usleep(100000);

	if (verbose) fprintf(stderr, " ok\nrunning test ...");

	count = 1;
	ts = getmstics();

	sleep(measure_time);

	count = 0;
	stop = 1;
	te = getmstics();

	if (verbose) fprintf(stderr, " ok\nwaiting completion ...");

	while (atomic_read(&actthreads) > 0)
		usleep(100000);

	for (i = 0; i < nbtasks; i++)
		wait(&status);

	if (verbose) fprintf(stderr, " ok\n");

	for (i = 0; i < nbtasks; i++) {
		value += totalwork[i];
		if (totalwork[i] == 0)
			++thrzero;
	}

	if (verbose) fprintf(stderr, "compensation loop ...");
	is = (value * 1000 * SSUMTIME) / (te - ts);
	do {
		sits = is;
		sts = getmstics();

		for (; is; is--)
			cacheload(datasize);

		ste = getmstics();
		is = 3 * (sits * 1000 * SSUMTIME) / (2 * (ste - sts));
	} while ((ste - sts) < 1000 * SSUMTIME);
	tcorr = ((ste - sts) * value) / sits;
	if (verbose) fprintf(stderr, " sits=%llu comptime=%llu corr=%llu value=%llu citt=%e\n",
			sits, ste - sts, tcorr, value, (double) (ste - sts) / ((double) sits * 1000.0));
	if ((double) tcorr > eck * (te - ts)) {
		if (verbose) fprintf(stderr, "measure unstable: corr{%llu} > (eck{%lf} * ttime{%llu})\n",
				tcorr, eck, te - ts);
		ustr = "*";
	}
	ttime = (te - ts) - tcorr;
	avgvalue = value / nbtasks;
	value *= 1000;
	value /= ttime;
	avgwork = (int) (value / nbtasks);

	for (i = 0, sqrdev = 0; i < nbtasks; i++) {
		double difvv = (double) (totalwork[i] - avgvalue);

		sqrdev += (difvv * difvv) / (double) avgvalue;
	}

	fprintf(stdout, "%s%d\t%lu\t%lld\t%e\t%d\t%d\t%.0f\n",
			ustr, nbtasks, (unsigned long) (te - ts), value, 1.0 / (double) value, avgwork, thrzero, sqrdev);

	exit(0);
}

