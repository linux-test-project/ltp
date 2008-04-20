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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *      libjvmsim.c
 *
 * DESCRIPTION
 *      Provide threads to mimic real-time jvm activity
 *
 *      Note: this "library" is written as a single header file to simplify the
 *      creation of new test cases and keep the build system simple.  At some
 *      point it may make sense to break it up into .h and .c files and link to
 *      it as an object file.

 * USAGE:
 *      To be included in testcases.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-May-9:  Initial version by Darren Hart
 *
 *****************************************************************************/

#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <semaphore.h>
#include <librttest.h>

/* this mutex and cond pair are used by the gc alarm and collect threads */
static pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gc_cond;

/* this semaphore is used by posix signal dispatch, and any other simulated
 * by a sit and wait routine on a POSIX semaphore */
sem_t block_sem;

/* this mutex and cond pair are used by all threads simulated by a
 * sit and wait routine on a pthread mutex and cond pair */
static pthread_mutex_t block_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t block_cond;
#define SIT_AND_WAIT \
struct thread *t = (struct thread *)arg; \
int i = 0; \
int ret; \
nsec_t timeout; \
struct timespec ts_timeout; \
while (!thread_quit(t)) { \
	timeout = rt_gettime() + 50 * NS_PER_MS; \
	nsec_to_ts(timeout, &ts_timeout); \
	pthread_mutex_lock(&block_mutex); \
	ret = pthread_cond_timedwait(&block_cond, &block_mutex, &ts_timeout); \
	if (ret && ret != ETIMEDOUT) \
		printf("%s: %s\n", __FUNCTION__, strerror(ret)); \
	busy_work_us(50); /* this should only happen on exit atm */ \
	pthread_mutex_unlock(&block_mutex); \
	i++; \
} \
debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i); \
return NULL;

/* High level description of JVM threads
 * There may be other threads used for profiling, debug and trace
 * purposes depending on options and application.
 */

/* Main/Primordial  thread  (SCHED_OTHER prio)
 * o Creates other internal threads and runs user's main().
 * o Typically would create the initial RT thread but otherwise
 *   is effectively in a pthread_cond wait state doing no work
 * o Covered by testcase main() really (Not implemented here)
 */

/* GC Alarm  (RT FIFO 89)
 * o Does clock_nanosleep for 451us
 * o Upon awakening, does a pthread_mutex_lock(m) and then a
 *   pthread_cond_signal(s) to a GC collect thread waiting on signal(s)
 * o Repeat this process
 */
void *jvm_gc_alarm(void *arg)
{
	struct thread *t = (struct thread *)arg;
	int i = 0;
	while (!thread_quit(t)) {
		rt_nanosleep(451*NS_PER_US);
		pthread_mutex_lock(&gc_mutex);
		pthread_cond_signal(&gc_cond);
		pthread_mutex_unlock(&gc_mutex);
		i++;
	}
	/* do it once more to make sure the collect thread can exit */
	pthread_mutex_lock(&gc_mutex);
	pthread_cond_signal(&gc_cond);
	pthread_mutex_unlock(&gc_mutex);
	debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i);
	return NULL;
}

/* GC Collect (RT FIFO appPrio+1)
 * o The default should be to have one such thread. There is a JVM option that
 *   can control the number of such worker threads created but the common case
 *   we should only have one running.
 * o Does a pthread_cond_wait(m,s)
 * o On pthread_cond_signal() from the alarm thread sees if there is GC work to
 *   be performed (small/trivial calculation - i.e. noise)
 *   o If no work, then the server thread repeats this whole process
 *     Performs gc work (which I don't think you would want to emulate)
 *   o Repeats this process
 */
void *jvm_gc_collect(void *arg)
{
	struct thread *t = (struct thread *)arg;
	int i = 0;
	int ret;
	nsec_t timeout;
	struct timespec ts_timeout;

	while (!thread_quit(t)) {
		timeout = rt_gettime() + 50 * NS_PER_MS;
		nsec_to_ts(timeout, &ts_timeout);
		pthread_mutex_lock(&gc_mutex);
		ret = pthread_cond_timedwait(&gc_cond, &gc_mutex, &ts_timeout);
		if (ret && ret != ETIMEDOUT)
			printf("%s: %s\n", __FUNCTION__, strerror(ret));
                busy_work_us(50);
		pthread_mutex_unlock(&gc_mutex);
		i++;
	}
	debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i);
	return NULL;
}

/* GC Trace Thread (SCHED_OTHER)
 * o A thread that sleeps using pthread_cond_timedwait for 50 millis.
 * o It wakes up, releases the  mutex does a getrusage(RUSAGE_SELF, &times)
 *   call and some small calculations (~50us or so) then goes back to the
 *   timewait (grabbing mutex of course)
 */
void *jvm_gc_trace(void *arg)
{
	struct thread *t = (struct thread *)arg;
	pthread_cond_t c;
	pthread_condattr_t c_attr;
	pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
	nsec_t abs_timeout;
	struct timespec ts_timeout;
	struct rusage usage;
	int i, ret;

	/* use the same clock as the timeout value */
	pthread_condattr_init(&c_attr);
        if (pthread_condattr_setclock(&c_attr, CLOCK_MONOTONIC) != 0) {
                perror("pthread_condattr_setclock");
                exit(-1);
        }
        if (pthread_cond_init(&c, &c_attr) != 0) {
                perror("pthread_cond_init");
                exit(-1);
        }

	i=0;
	while (!thread_quit(t)) {
		pthread_mutex_lock(&m);
		abs_timeout = rt_gettime() + 50 * NS_PER_MS;
		nsec_to_ts(abs_timeout, &ts_timeout);
		do {
			ret = pthread_cond_timedwait(&c, &m, &ts_timeout);
		} while (ret == 0);
		if (ret != ETIMEDOUT)
			printf("%s: %s\n", __FUNCTION__, strerror(ret));
		getrusage(RUSAGE_SELF, &usage);
		busy_work_us(50);
		pthread_mutex_unlock(&m);
		i++;
	}
	pthread_condattr_destroy(&c_attr);
	debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i);
	return NULL;
}

/* GC Finalize (SCHED_OTHER)
 * o GC could dispatch work off to this thread depending on users
 *   application. But, for the common case I believe this thread should be
 *   in a wait state and can be ignored,
 */
/* sit and wait */
void *jvm_gc_finalize(void *arg)
{
SIT_AND_WAIT
}

/* GC Processor info thread (RT FIFO ??)
 * o JVM startup creates one such thread per processor and binds one
 *   thread per the individual processors (including CPU 0). Each threads
 *   tries to calculate its CPU frequency,  and when done,  the thread dies.
 * o For the common case, these threads have exited and so are not running.
 *   I do have comeback as to how long they run to perform the frequency
 *   calculation.  (will append that later)
 */
/* can ignore this one */
/*void *jvm_gc_cpu_info(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* POSIX Signal Dispatch (RT FIFO 87)
 * o should be in a sem_wait() state. Async signals such as SIGABORT, SIGQUIT,
 *   SIGSTOP are delegated off to this thread from a signal handler using
 *   sem_post().
 * o For the common case, this thread should be in a sem_wait() state and so
 *   doing nothing. I.E. can be ignored.
 */
/* good to have, sit and do nothing */
void *jvm_posix_signal_dispatch(void *arg)
{
	struct thread *t = (struct thread *)arg;
	int i = 0;
	nsec_t timeout;
	struct timespec ts_timeout;

	while (!thread_quit(t)) {
		clock_gettime(CLOCK_REALTIME, &ts_timeout);
		ts_to_nsec(&ts_timeout, &timeout);
		timeout += 50 * NS_PER_MS;
		nsec_to_ts(timeout, &ts_timeout);
		sem_timedwait(&block_sem, &ts_timeout);
		i++;
	}
	debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i);
	return NULL;
}

/* POSIX Signal Dispatch NH
 * o This thread is analogous to the previous thread. The reason we have 2 is
 *   that the former thread may touch the heap while this thread cannot.
 *   I.E. can be ignored.
 */
/*void *jvm_posix_signal_dispatch_nh(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* JIT Sampler (RT FIFO 1)
 * o Somewhat like the alarm thread and used by JIT.
 * o For configuration running AOT this thread is not running and can be
 *   ignored
 */
/* not needed with aot */
/*void *jvm_jit_sampler(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* JIT Compile (should be SCHED_OTHER - but think it is not)
 * o Does a JIT compile.
 * o For Configurations running AOT this thread is not running and can
 *   be ignored
 */
/* not needed with aot */
/*void *jvm_jit_compile(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* SigQuit Thread (SCHED_OTHER but the priority will likely be raised)
 * o SIGQUIT helper thread. The thread is in a pthread_cond_wait() so,
 *   for our purposes, should not be doing any work.
 */
/* good to have  - sit there and do nothing until exit*/
void *jvm_sigquit(void *arg)
{
SIT_AND_WAIT
}

/* Async EventServerThreads (priority described below)
 * o A pool of threads that are used to service hander fire requests and bind
 *   to handler.
 * o 6  or so threads are created in a wait state at priority 83 (I believe).
 *   The priority of these theads is changed to the appropriate RT priority
 *   before they exit the wait state and they go back to prio 83 when they are
 *   done servicing the request.
 * o For the common case, these threads should just be waiting.
 */
void *jvm_async_event_server(void *arg)
{
SIT_AND_WAIT
}

/* Async EventServerThreads NH
 * o Same as previous (# threads may be different) but used for handlers that
 *   can't access the Java heap.
 */
/*void *jvm_async_event_server_nh(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* Timer Dispatch (RT FIFO 83 currently)
 * o A thread used as part of implementation for Timers that need to fire at
 *   certain times.
 * o In a wait state if no timers or handlers are created (default)
 */
void *jvm_timer_dispatch(void *arg)
{
SIT_AND_WAIT
}

/* Timer Dispatch NH
 * o same as previous except for threads that cannot access the Java heap.
 */
/*void *jvm_timer_dispatch_nh(void *arg)
{
	//struct thread *t = (struct thread *)arg;
}*/

/* RAS trace thread (SCHED_OTHER)
 * o in support of some JVM tracing activities
 * o is in a wait state
 */
void *jvm_ras_trace(void *arg)
{
SIT_AND_WAIT
}

/* cleanup and wakeup blocked threads at join_threads() time */
void *jvmsim_monitor(void *arg) {
	struct thread *t = (struct thread *)arg;
	int i = 0;

	while (!thread_quit(t)) {
		usleep(10*US_PER_MS);
		i++;
	}

	/* unblock all the sit and wait threads */
	pthread_mutex_lock(&block_mutex);
	pthread_cond_broadcast(&block_cond);
	pthread_mutex_unlock(&block_mutex);

	sem_post(&block_sem); /* allow jvm_posix_signal_dispatch() to exit */

	debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i);
	return NULL;
}

/* FIXME: the priorities used below will need to be adjusted
 * and librt.h should be made to provide access to the highest
 * prio running thread - perhaps with a listener interface... (so the gc
 * can be boosted to app+1 prio.
 */

/* convenience functions for starting groups of jvm simulation threads */
void jvmsim_init(void)
{
	pthread_condattr_t c_attr;
	sem_init(&block_sem, 0, 0);

        if (pthread_condattr_setclock(&c_attr, CLOCK_MONOTONIC) != 0) {
                perror("pthread_condattr_setclock");
                exit(-1);
        }
        if (pthread_cond_init(&gc_cond, &c_attr) != 0) {
                perror("pthread_cond_init");
                exit(-1);
        }
        if (pthread_cond_init(&block_cond, &c_attr) != 0) {
                perror("pthread_cond_init");
                exit(-1);
        }

	create_other_thread(jvmsim_monitor, (void *)0);
	create_fifo_thread(jvm_gc_alarm, (void *)0, 89);
	/* The gc collect thread varies - this can be modelled better */
	create_fifo_thread(jvm_gc_collect, (void *)0, 82);
	create_other_thread(jvm_gc_trace, (void *)12);
	create_other_thread(jvm_gc_finalize, (void *)0);
	create_fifo_thread(jvm_posix_signal_dispatch, (void *)0, 87);
	create_other_thread(jvm_sigquit, (void *)88);
	create_fifo_thread(jvm_async_event_server, (void *)0, 85);
	create_fifo_thread(jvm_async_event_server, (void *)0, 85);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_async_event_server, (void *)0, 83);
	create_fifo_thread(jvm_timer_dispatch, (void *)0, 85);
	create_fifo_thread(jvm_timer_dispatch, (void *)0, 83);
	create_other_thread(jvm_ras_trace, (void *)0);
}

