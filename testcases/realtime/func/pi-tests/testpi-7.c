/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006, 2008
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
 *      testpi-7.c
 *
 * DESCRIPTION
 *      measure the latency involved with PI boosting.
 *
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *   2006-May-3: Initial version by Darren Hart <dvhltc@us.ibm.com>
 *   2006-May-4: Timing fixes reported by Vernon Mauery <vernux@us.ibm.com>
 *   2006-May-4: Made the med prio threads RT by Darren Hart <dvhltc@us.ibm.com>
 *   2006-May-5: Modified to use flagging by Vernon Mauery <vernux@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>

#define HIGH_PRIO 15
#define MED_PRIO 10
#define LOW_PRIO  5

#define ITERATIONS 100

#define MED_WORK_MS 20
#define NS_PER_MS 1000000

static int use_flag_mutex;
static int max_delay_us;
static int max_drop2grab_us;

static pthread_mutex_t pi_mutex;

// flagging details
typedef enum {
	LOW_START_CYCLE = 1,	// 1
	MED_START_WORK,		// 2
	HIGH_GRAB_MUTEX,	// 3
	LOW_DROP_MUTEX,		// 4
	END_OF_CYCLE,		// 5
	END_OF_GAME		// 6
} phase_t;

static volatile phase_t phase_flag = END_OF_CYCLE;

static pthread_mutex_t flag_mutex;

int med_threads = 0;
long iterations = ITERATIONS;

void usage(void)
{
	rt_help();
	printf("testpi-7 specific options:\n");
	printf("  -i#     #: number of iterations\n");
	printf("  -f      #: Use flag mutex\n");
	printf("  -x#     #:number of mid priority threads\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'f':
		use_flag_mutex = 0;
		break;
	case 'h':
		usage();
		exit(0);
	case 'i':
		iterations = atoi(v);
		break;
	case 'x':
		med_threads = atoi(v);
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

phase_t _read_flag(const char *s, int l)
{
	phase_t ret;
	if (use_flag_mutex)
		pthread_mutex_lock(&flag_mutex);
	ret = phase_flag;
	debug(DBG_DEBUG, "%s:%d: read_flag = %s (%d)\n", s, l,
	      (ret == LOW_START_CYCLE ? "LOW_START_CYCLE" : ret ==
	       MED_START_WORK ? "MED_START_WORK" : ret ==
	       HIGH_GRAB_MUTEX ? "HIGH_GRAB_MUTEX" : ret ==
	       LOW_DROP_MUTEX ? "LOW_DROP_MUTEX" : ret ==
	       END_OF_CYCLE ? "END_OF_CYCLE" : "ERROR"), ret);
	//debug(DBG_DEBUG, "%s:%d: read_flag = %d\n", s, l, ret);
	if (use_flag_mutex)
		pthread_mutex_unlock(&flag_mutex);
	return ret;
}

void _write_flag(const char *s, int l, phase_t new_flag)
{
	if (use_flag_mutex)
		pthread_mutex_lock(&flag_mutex);
	if (phase_flag != END_OF_GAME) {
		if (new_flag != phase_flag && new_flag != (phase_flag + 1)
		    && !(new_flag == LOW_START_CYCLE
			 && phase_flag == END_OF_CYCLE))
			printf("YOU'RE HOSED: new_flag=%d, phase_flag=%d\n",
			       new_flag, phase_flag);
		phase_flag = new_flag;
		debug(DBG_DEBUG, "phase_flag: %s set it to %d\n", s,
		      phase_flag);
		debug(DBG_DEBUG, "%s:%d: write_flag = %s (%d)\n", s, l,
		      (new_flag ==
		       LOW_START_CYCLE ? "LOW_START_CYCLE" : new_flag ==
		       MED_START_WORK ? "MED_START_WORK" : new_flag ==
		       HIGH_GRAB_MUTEX ? "HIGH_GRAB_MUTEX" : new_flag ==
		       LOW_DROP_MUTEX ? "LOW_DROP_MUTEX" : new_flag ==
		       END_OF_CYCLE ? "END_OF_CYCLE" : "ERROR"), new_flag);
		//debug(DBG_DEBUG, "%s:%d: write_flag = %d\n", s, l, new_flag);
	}
	if (use_flag_mutex)
		pthread_mutex_unlock(&flag_mutex);
}

#define read_flag(A) _read_flag(__FUNCTION__,__LINE__)
#define write_flag(A) _write_flag(__FUNCTION__,__LINE__,A)

#define while_not_flag(A,B) while (read_flag() != (A) && !thread_quit(B))

static nsec_t low_drop_time;
void *low_prio_rt_thread(void *arg)
{
	struct thread *t = (struct thread *)arg;
	while (!thread_quit(t)) {
		while_not_flag(LOW_START_CYCLE, t)
		    rt_nanosleep(1 * NS_PER_MS);
		debug(DBG_INFO, "low try mutex\n");
		pthread_mutex_lock(&pi_mutex);
		debug(DBG_INFO, "low grab mutex\n");
		write_flag(MED_START_WORK);
		rt_nanosleep(1 * NS_PER_MS);
		while_not_flag(LOW_DROP_MUTEX, t) {
			//printf("!"); fflush(NULL);
			rt_nanosleep(1);
		}
		debug(DBG_INFO, "low drop mutex\n");
		low_drop_time = rt_gettime();
		pthread_mutex_unlock(&pi_mutex);
		while_not_flag(END_OF_CYCLE, t) {
			//printf("@"); fflush(NULL);
			rt_nanosleep(1 * NS_PER_MS);
		}
	}
	debug(DBG_INFO, "low prio thread finished (flags=%#x)\n", t->flags);
	return NULL;
}

void *med_prio_thread(void *arg)
{
	static atomic_t m_flag = { 0 };
	struct thread *t = (struct thread *)arg;
#define MP "\t\t\t"
	while (!thread_quit(t)) {
		int i_am_the_one;
		phase_t f;
		while_not_flag(MED_START_WORK, t) {
			//printf("."); fflush(NULL);
			rt_nanosleep(1 * NS_PER_MS);
		}
		if ((i_am_the_one = atomic_inc(&m_flag)) == 1) {
			debug(DBG_INFO, MP "thread %d writing flag\n", t->id);
			write_flag(HIGH_GRAB_MUTEX);
		}

		debug(DBG_DEBUG, MP "ready to start work\n");
		write_flag(HIGH_GRAB_MUTEX);
		while (((f = read_flag()) == HIGH_GRAB_MUTEX
			|| f == LOW_DROP_MUTEX) && !thread_quit(t)) {
			busy_work_ms(MED_WORK_MS);
			//printf("-"); fflush(NULL);
		}
		debug(DBG_DEBUG, MP "done working -- time to sleep\n");
		if (i_am_the_one == 1) {
			debug(DBG_INFO, MP "thread %d resetting m_flag\n",
			      t->id);
			atomic_set(0, &m_flag);
		}
	}
	debug(DBG_INFO, "med prio thread finished\n");
	return NULL;
#undef MP
}

void *high_prio_rt_thread(void *arg)
{
	int delta_us;
	int i;
	nsec_t start, now;
	struct thread *t = (struct thread *)arg;
	long iterations = (long)t->arg;

#define HP "\t\t\t\t\t"
	for (i = 0; i < iterations; i++) {
		debug(DBG_INFO, "Staring iteration %d\n", i);
		write_flag(LOW_START_CYCLE);
		while_not_flag(HIGH_GRAB_MUTEX, t) {
			//printf("a"); fflush(NULL);
			rt_nanosleep(10 * NS_PER_MS);
		}
		debug(DBG_INFO, HP "high try mutex\n");
		write_flag(LOW_DROP_MUTEX);
		start = rt_gettime();
		pthread_mutex_lock(&pi_mutex);
		now = rt_gettime();
		debug(DBG_INFO, HP "high grab mutex\n");
		write_flag(END_OF_CYCLE);
		debug(DBG_INFO, HP "high drop mutex\n");
		delta_us = (now - start) / NS_PER_US;
		if (delta_us > max_delay_us)
			max_delay_us = delta_us;
		debug(DBG_WARN, "high prio delay time: %d us\n", delta_us);
		delta_us = (now - low_drop_time) / NS_PER_US;
		if (delta_us > max_drop2grab_us)
			max_drop2grab_us = delta_us;
		debug(DBG_WARN, "low drop to high grab time: %d us\n",
		      delta_us);
		pthread_mutex_unlock(&pi_mutex);
		rt_nanosleep(10 * NS_PER_MS);
	}
	all_threads_quit();
	write_flag(END_OF_GAME);
	debug(DBG_INFO, HP "high prio done\n");
#undef HP
	return NULL;
}

int main(int argc, char *argv[])
{
	int i, numcpus;
	setup();

	rt_init("hfi:x:", parse_args, argc, argv);

	if (!med_threads) {
		printf
		    ("This test requires that at least NRCPUS medium priority threads run\n");
		printf
		    ("If it is run bound to a single CPU, you can specify -x 1\n");
		printf("No User input , using default value for NRCPUS");
		numcpus = sysconf(_SC_NPROCESSORS_ONLN);
		med_threads = numcpus;
	}
	printf(" flag mutex: %s\n", use_flag_mutex ? "enabled" : "disabled");
	printf(" iterations: %ld\n", iterations);
	printf("med threads: %d\n", med_threads);

	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);

	max_delay_us = 0;
	max_drop2grab_us = 0;

	init_pi_mutex(&pi_mutex);

	create_fifo_thread(low_prio_rt_thread, NULL, LOW_PRIO);
	create_fifo_thread(high_prio_rt_thread, (void *)iterations, HIGH_PRIO);
	for (i = 0; i < med_threads; i++) {
		create_fifo_thread(med_prio_thread, NULL, MED_PRIO);
	}

	while (phase_flag != END_OF_GAME)
		usleep(100);
	join_threads();
	cleanup(0);

	printf("High priority lock aquisition maximum delay: %dus\n",
	       max_delay_us);
	printf
	    ("Low priority lock drop to high priority acqusistion time: %dus\n",
	     max_drop2grab_us);
	printf("\n");

	return 0;
}
