/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006-2008
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
 *       librttest.h
 *
 * DESCRIPTION
 *      A set of commonly used convenience functions for writing
 *      threaded realtime test cases.
 *
 * USAGE:
 *       To be included in testcases.
 *
 * AUTHOR
 *        Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-Apr-26: Initial version by Darren Hart
 *      2006-May-08: Added atomic_{inc,set,get}, thread struct, debug function,
 *                      rt_init, buffered printing -- Vernon Mauery
 *      2006-May-09: improved command line argument handling
 *      2007-Jul-12: Added latency tracing functions -- Josh Triplett
 *      2007-Jul-26: Renamed to librttest.h -- Josh Triplett
 *      2009-Nov-4: TSC macros within another header -- Giuseppe Cavallaro
 *
 *****************************************************************************/

#ifndef LIBRTTEST_H
#define LIBRTTEST_H

#include <sys/syscall.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "list.h"

extern void setup(void);
extern void cleanup(int i);

extern int optind, opterr, optopt;
extern char *optarg;

#define _MAXTHREADS 256
#define CALIBRATE_LOOPS 100000

#define NS_PER_MS 1000000
#define NS_PER_US 1000
#define NS_PER_SEC 1000000000
#define US_PER_MS 1000
#define US_PER_SEC 1000000
#define MS_PER_SEC 1000

typedef unsigned long long nsec_t;

struct thread {
	struct list_head _threads;
	pthread_t pthread;
	pthread_attr_t attr;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	void *arg;
	void *(*func)(void*);
	int priority;
	int policy;
	int flags;
	int id;
};
typedef struct { volatile int counter; } atomic_t;

// flags for threads
#define THREAD_START 1
#define THREAD_QUIT  2
#define thread_quit(T) (((T)->flags) & THREAD_QUIT)

#define PRINT_BUFFER_SIZE (1024*1024*4)
#define ULL_MAX 18446744073709551615ULL // (1 << 64) - 1

extern pthread_mutex_t _buffer_mutex;
extern char * _print_buffer;
extern int _print_buffer_offset;
extern int _dbg_lvl;
extern double pass_criteria;

/* function prototypes */

/* atomic_add - add integer to atomic variable and returns a value.
 * i: integer value to add
 * v: pointer of type atomic_t
 */
static inline int atomic_add(int i, atomic_t *v)
{
	return __sync_add_and_fetch(&v->counter, i);
}

/* atomic_inc: atomically increment the integer passed by reference
 */
static inline int atomic_inc(atomic_t *v)
{
	return atomic_add(1, v);
}

/* atomic_get: atomically get the integer passed by reference
 */
//#define atomic_get(v) ((v)->counter)
static inline int atomic_get(atomic_t *v)
{
	return v->counter;
}

/* atomic_set: atomically get the integer passed by reference
 */
//#define atomic_set(i,v) ((v)->counter = (i))
static inline void atomic_set(int i, atomic_t *v)
{
	v->counter = i;
}

/* buffer_init: initialize the buffered printing system
 */
void buffer_init(void);

/* buffer_print: prints the contents of the buffer
 */
void buffer_print(void);

/* buffer_fini: destroy the buffer
 */
void buffer_fini(void);

/* debug: do debug prints at level L (see DBG_* below).  If buffer_init
 * has been called previously, this will print to the internal memory
 * buffer rather than to stderr.
 * L: debug level (see below) This will print if L is lower than _dbg_lvl
 * A: format string (printf style)
 * B: args to format string (printf style)
 */
static volatile int _debug_count = 0;
#define debug(L,A,B...) do {\
	if ((L) <= _dbg_lvl) {\
		pthread_mutex_lock(&_buffer_mutex);\
		if (_print_buffer) {\
			if (PRINT_BUFFER_SIZE - _print_buffer_offset < 1000)\
				buffer_print();\
			_print_buffer_offset += snprintf(&_print_buffer[_print_buffer_offset],\
					PRINT_BUFFER_SIZE - _print_buffer_offset, "%06d: "A, _debug_count++, ##B);\
		} else {\
			fprintf(stderr, "%06d: "A, _debug_count++, ##B);\
		}\
		pthread_mutex_unlock(&_buffer_mutex);\
	}\
} while (0)
#define DBG_ERR  1
#define DBG_WARN 2
#define DBG_INFO 3
#define DBG_DEBUG 4

/* rt_help: print help for standard args */
void rt_help(void);

/* rt_init_long: initialize librttest
 * options: pass in an getopt style string of options -- e.g. "ab:cd::e:"
 *          if this or parse_arg is null, no option parsing will be done
 *          on behalf of the calling program (only internal args will be parsed)
 * longopts: a pointer to the first element of an array of struct option, the
 *           last entry must be set to all zeros.
 * parse_arg: a function that will get called when one of the above
 *            options is encountered on the command line.  It will be passed
 *            the option -- e.g. 'b' -- and the value.  Something like:
 *            // assume we passed "af:z::" to rt_init
 *            int parse_my_options(int option, char *value) {
 *                int handled = 1;
 *                switch (option) {
 *                    case 'a':
 *                        alphanum = 1;
 *                        break;
 *                    case 'f':
 *                    // we passed f: which means f has an argument
 *                        freedom = strcpy(value);
 *                        break;
 *                    case 'z':
 *                    // we passed z:: which means z has an optional argument
 *                        if (value)
 *                            zero_size = atoi(value);
 *                        else
 *                            zero_size++;
 *                    default:
 *                        handled = 0;
 *                }
 *                return handled;
 *            }
 * argc: passed from main
 * argv: passed from main
 */
int rt_init_long(const char *options, const struct option *longopts,
		 int (*parse_arg)(int option, char *value),
		 int argc, char *argv[]);

/* rt_init: same as rt_init_long with no long options */
int rt_init(const char *options, int (*parse_arg)(int option, char *value),
	    int argc, char *argv[]);

int create_thread(void*(*func)(void*), void *arg, int prio, int policy);

/* create_fifo_thread: spawn a SCHED_FIFO thread with priority prio running
 * func as the thread function with arg as it's parameter.
 * func:
 * arg: argument to func
 * prio: 1-100, 100 being highest priority
 */
int create_fifo_thread(void*(*func)(void*), void *arg, int prio);

/* create_rr_thread: spawn a SCHED_RR thread with priority prio running
 * func as the thread function with arg as it's parameter.
 * func:
 * arg: argument to func
 * prio: 1-100, 100 being highest priority
 */
int create_rr_thread(void*(*func)(void*), void *arg, int prio);

/* create_other_thread: spawn a SCHED_OTHER thread
 * func as the thread function with arg as it's parameter.
 * func:
 * arg: argument to func
 */
int create_other_thread(void*(*func)(void*), void *arg);

/* Change the priority of a running thread */
int set_thread_priority(pthread_t pthread, int prio);

/* Change the priority of the current context (usually called from main())
 * and its policy to SCHED_FIFO
 * prio: 1-99
 */
int set_priority(int prio);

/* all_threads_quit: signal all threads to quit */
void all_threads_quit(void);

/* join_threads: wait for all threads to finish
 * (calls all_threads_quit interally)
 */
void join_threads(void);

/* get_thread: return a struct thread pointer from the list */
struct thread * get_thread(int i);

/* signal thread i to quit and then call join */
void join_thread(int i);

/* return the delta in ts_delta
 * ts_end > ts_start
 * if ts_delta is not null, the difference will be returned in it
 */
void ts_minus(struct timespec *ts_end, struct timespec *ts_start, struct timespec *ts_delta);

/* return the sum in ts_sum
 * all arguments are not null
 */
void ts_plus(struct timespec *ts_a, struct timespec *ts_b, struct timespec *ts_sum);

/* put a ts into proper form (nsec < NS_PER_SEC)
 * ts must not be null
 */
void ts_normalize(struct timespec *ts);

/* convert nanoseconds to a timespec
 * ts must not be null
 */
void nsec_to_ts(nsec_t ns, struct timespec *ts);

/* convert a timespec to nanoseconds
 * ts must not be null
 */
int ts_to_nsec(struct timespec *ts, nsec_t *ns);

/* return difference in microseconds */
unsigned long long tsc_minus(unsigned long long tsc_start, unsigned long long tsc_end);

/* rt_nanosleep: sleep for ns nanoseconds using clock_nanosleep
 */
void rt_nanosleep(nsec_t ns);

/* rt_nanosleep: sleep until absolute time ns given in
 * nanoseconds using clock_nanosleep
 */
void rt_nanosleep_until(nsec_t ns);

/* rt_gettime: get CLOCK_MONOTONIC time in nanoseconds
 */
nsec_t rt_gettime(void);

/* busy_work_ms: do busy work for ms milliseconds
 */
void *busy_work_ms(int ms);

/* busy_work_us: do busy work for us microseconds
 */
void *busy_work_us(int us);

/* init_pi_mutex: initialize a pthread mutex to have PI support
 */
void init_pi_mutex(pthread_mutex_t *m);

/* latency_trace_enable: Enable latency tracing via sysctls.
 */
void latency_trace_enable(void);

/* latency_trace_start: Start tracing latency; call before running test.
 */
void latency_trace_start(void);

/* latency_trace_stop: Stop tracing latency; call immediately after observing
 * excessive latency and stopping test.
 */
void latency_trace_stop(void);

/* latency_trace_print: Print latency trace information from
 * /proc/latency_trace.
 */
void latency_trace_print(void);

#endif /* LIBRTTEST_H */
