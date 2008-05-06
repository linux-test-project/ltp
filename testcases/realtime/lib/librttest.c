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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *       librttest.c
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
 *      2007-Jul-12: Added latency tracing functions and I/O helper functions
 *                                              -- Josh triplett
 *	2008-Jan-10: Added RR thread support to tests -- Chirag Jog
 *
 *****************************************************************************/

#include <librttest.h>
#include <libstats.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

static LIST_HEAD(_threads);
static atomic_t _thread_count = {-1};

pthread_mutex_t _buffer_mutex;
char * _print_buffer = NULL;
int _print_buffer_offset = 0;
int _dbg_lvl = 0;

static int _use_pi = 1;

/* function implementations */
void rt_help(void)
{
	printf("librt standard options:\n");
	printf("  -b(0,1)	1:enable buffered output, 0:diable buffered output\n");
	printf("  -p(0,1)	0:don't use pi mutexes, 1:use pi mutexes\n");
	printf("  -v[0-4]	0:no debug, 1:DBG_ERR, 2:DBG_WARN, 3:DBG_INFO, 4:DBG_DEBUG\n");
	printf("  -s		Enable saving stats data (default disabled)\n");
}

int rt_init(const char *options, int (*parse_arg)(int option, char *value), int argc, char *argv[])
{
	int use_buffer = 1;
	size_t i;
	int c;
	opterr = 0;
	char *all_options, *opt_ptr;
	static const char my_options[] = "b:p:v:s";

	if (options) {
		opt_ptr = all_options = (char *)malloc(sizeof(my_options) + strlen(options) + 1);
		for (i=0; i<strlen(options); i++) {
			char opt = options[i];
			if (opt != ':' && !strchr(my_options, opt)) {
				*opt_ptr++ = opt;
				if (options[i+1] == ':') {
					*opt_ptr++ = ':';
					i++;
				}
			} else {
				printf("Programmer error -- argument -%c already used by librt.h\n", opt);
			}
		}
		strcpy(opt_ptr, my_options);
	} else {
		all_options = (char *)my_options;
	}

	while ((c = getopt(argc, argv, all_options)) != -1) {
		switch (c) {
			case 'b':
				use_buffer = atoi(optarg);
				break;
			case 'p':
				_use_pi = atoi(optarg);
				break;
			case 'v':
				_dbg_lvl = atoi(optarg);
				break;
			case 's':
				save_stats = 1;
				break;
			default:
				if (parse_arg) {
					if (!parse_arg(c, optarg)) {
						printf("option -%c not recognized\n", optopt);
					}
				}
		}
	}
	if (!_use_pi)
		printf("Priority Inheritance has been disabled for this run.\n");
	if (use_buffer)
		buffer_init();

	/*
	 * atexit() order matters here - buffer_print() will be called before
	 * buffer_fini().
	 */
	atexit(buffer_fini);
	atexit(buffer_print);
	return 0;
}

void buffer_init(void)
{
	_print_buffer = (char *)malloc(PRINT_BUFFER_SIZE);
	if (!_print_buffer)
		fprintf(stderr, "insufficient memory for print buffer - printing directly to stderr\n");
	else
		memset(_print_buffer, 0, PRINT_BUFFER_SIZE);
}

void buffer_print(void)
{
	if (_print_buffer) {
		fprintf(stderr, "%s", _print_buffer);
		memset(_print_buffer, 0, PRINT_BUFFER_SIZE);
		_print_buffer_offset = 0;
	}
}

void buffer_fini(void)
{
	if (_print_buffer)
		free(_print_buffer);
	_print_buffer = NULL;
}

void cleanup(int i) {
       printf("Test terminated with asynchronous signal\n");
       buffer_print();
       buffer_fini();
       if (i)
               exit (i);
}

void setup()
{
       signal(SIGINT,cleanup);
       signal(SIGQUIT,cleanup);
       signal(SIGTERM,cleanup);
}


int create_thread(void*(*func)(void*), void *arg, int prio, int policy)
{
	struct sched_param param;
	int id, ret;
	struct thread *thread;

	id = atomic_inc(&_thread_count);

	thread = malloc(sizeof(struct thread));
	if (!thread)
		return -1;

	list_add_tail(&thread->_threads, &_threads);
	pthread_cond_init(&thread->cond, NULL);	// Accept the defaults
	init_pi_mutex(&thread->mutex);
	thread->id = id;
	thread->priority = prio;
	thread->policy = policy;
	thread->flags = 0;
	thread->arg = arg;
	thread->func = func;
	param.sched_priority = prio;

	pthread_attr_init(&thread->attr);
	pthread_attr_setinheritsched(&thread->attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedparam(&thread->attr, &param);
	pthread_attr_setschedpolicy(&thread->attr, thread->policy);

	if ((ret = pthread_create(&thread->pthread, &thread->attr, func, (void*)thread))) {
		printf("pthread_create failed: %d (%s)\n", ret, strerror(ret));
		list_del(&thread->_threads);
		pthread_attr_destroy(&thread->attr);
		free(thread);
		return -1;
	}
	pthread_attr_destroy(&thread->attr);

	return id;
}

int create_fifo_thread(void*(*func)(void*), void *arg, int prio)
{
	return create_thread(func, arg, prio, SCHED_FIFO);
}
int create_rr_thread(void*(*func)(void*), void *arg, int prio)
{
	return create_thread(func, arg, prio, SCHED_RR);
}
int create_other_thread(void*(*func)(void*), void *arg)
{
	return create_thread(func, arg, 0, SCHED_OTHER);
}

int set_thread_priority(pthread_t pthread, int prio)
{
	struct sched_param sched_param;
	sched_param.sched_priority = prio;
	int policy;

	policy = (prio > 0) ? SCHED_FIFO : SCHED_OTHER;

	return pthread_setschedparam(pthread, policy, &sched_param);
}

int set_priority(int prio)
{
	struct sched_param sp;
	int ret = 0;

	sp.sched_priority = prio;
	if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) {
		perror("sched_setscheduler");
		ret = -1;
	}
	return ret;
}

void join_thread(int i)
{
	struct thread *p, *t = NULL;
	list_for_each_entry(p, &_threads, _threads) {
		if (p->id == i) {
			t = p;
			break;
		}
	}
	if (t) {
		t->flags |= THREAD_QUIT;
		if(t->pthread)
			pthread_join(t->pthread, NULL);
		list_del(&t->_threads);
	}
}

void all_threads_quit(void)
{
	struct thread *p;
	list_for_each_entry(p, &_threads, _threads) {
			p->flags |= THREAD_QUIT;
	}
}

void join_threads(void)
{
	all_threads_quit();
	struct thread *p, *t;
	list_for_each_entry_safe(p, t, &_threads, _threads) {
		if(p->pthread)
			pthread_join(p->pthread, NULL);
		list_del(&p->_threads);
	}
}

struct thread *get_thread(int i) {
	struct thread *p;
	list_for_each_entry(p, &_threads, _threads) {
		if (p->id == i) {
			return p;
		}
	}
	return NULL;
}

void ts_minus(struct timespec *ts_end, struct timespec *ts_start, struct timespec *ts_delta)
{
	if (ts_end == NULL || ts_start == NULL || ts_delta == NULL) {
		printf("ERROR in %s: one or more of the timespecs is NULL", __FUNCTION__);
		return;
	}

	ts_delta->tv_sec = ts_end->tv_sec - ts_start->tv_sec;
	ts_delta->tv_nsec = ts_end->tv_nsec - ts_start->tv_nsec;
	ts_normalize(ts_delta);
}

void ts_plus(struct timespec *ts_a, struct timespec *ts_b, struct timespec *ts_sum)
{
	if (ts_a == NULL || ts_b == NULL || ts_sum == NULL) {
		printf("ERROR in %s: one or more of the timespecs is NULL", __FUNCTION__);
		return;
	}

	ts_sum->tv_sec = ts_a->tv_sec + ts_b->tv_sec;
	ts_sum->tv_nsec = ts_a->tv_nsec + ts_b->tv_nsec;
	ts_normalize(ts_sum);
}

void ts_normalize(struct timespec *ts)
{
	if (ts == NULL) {
		/* FIXME: write a real error logging system */
		printf("ERROR in %s: ts is NULL\n", __FUNCTION__);
		return;
	}

	/* get the abs(nsec) < NS_PER_SEC */
	while (ts->tv_nsec > NS_PER_SEC) {
		ts->tv_sec++;
		ts->tv_nsec -= NS_PER_SEC;
	}
	while (ts->tv_nsec < -NS_PER_SEC) {
		ts->tv_sec--;
		ts->tv_nsec += NS_PER_SEC;
	}

	/* get the values to the same polarity */
	if (ts->tv_sec > 0 && ts->tv_nsec < 0) {
		ts->tv_sec--;
		ts->tv_nsec += NS_PER_SEC;
	}
	if (ts->tv_sec < 0 && ts->tv_nsec > 0) {
		ts->tv_sec++;
		ts->tv_nsec -= NS_PER_SEC;
	}
}

int ts_to_nsec(struct timespec *ts, nsec_t *ns)
{
	struct timespec t;
	if (ts == NULL) {
		/* FIXME: write a real error logging system */
		printf("ERROR in %s: ts is NULL\n", __FUNCTION__);
		return -1;
	}
	t.tv_sec = ts->tv_sec;
	t.tv_nsec = ts->tv_nsec;
	ts_normalize(&t);

	if (t.tv_sec <= 0 && t.tv_nsec < 0) {
		printf("ERROR in %s: ts is negative\n", __FUNCTION__);
		return -1;
	}

	*ns = (nsec_t)ts->tv_sec*NS_PER_SEC + ts->tv_nsec;
	return 0;
}

void nsec_to_ts(nsec_t ns, struct timespec *ts)
{
	if (ts == NULL) {
		/* FIXME: write a real error logging system */
		printf("ERROR in %s: ts is NULL\n", __FUNCTION__);
		return;
	}
	ts->tv_sec = ns/NS_PER_SEC;
	ts->tv_nsec = ns%NS_PER_SEC;
}

void rt_nanosleep(nsec_t ns) {
	struct timespec ts_sleep, ts_rem;
	int rc;
	nsec_to_ts(ns, &ts_sleep);
	rc = clock_nanosleep(CLOCK_MONOTONIC, 0, &ts_sleep, &ts_rem);
	/* FIXME: when should we display the remainder ? */
	if (rc != 0) {
		printf("WARNING: rt_nanosleep() returned early by %d s %d ns\n",
			(int)ts_rem.tv_sec, (int)ts_rem.tv_nsec);
	}
}

nsec_t rt_gettime(void) {
	struct timespec ts;
	nsec_t ns;
	int rc;

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rc != 0) {
		printf("ERROR in %s: clock_gettime() returned %d\n", __FUNCTION__, rc);
		perror("clock_gettime() failed");
		return 0;
	}

	ts_to_nsec(&ts, &ns);
	return ns;
}

void *busy_work_ms(int ms)
{
	busy_work_us(ms*US_PER_MS);
	return NULL;
}

void *busy_work_us(int us)
{
	int i;
	int scale;
	double pi_scaled;
	double pi_value;
	nsec_t start, now;
	int delta; /* time in us */
	volatile double a=16, b=1.0, c=5.0, d=4, e=1.0, f=239.0;

	scale = us * ITERS_PER_US;
	pi_scaled = 0;
	start = rt_gettime();
	for (i = 0; i < scale; i++) {
		double pi = a*atan(b/c) - d*atan(e/f);
		pi_scaled += pi;
	}
	pi_value = pi_scaled / scale;
	now = rt_gettime();
	delta = (now - start)/NS_PER_US;
	/* uncomment to tune to your machine */
        /* printf("busy_work_us requested: %dus  actual: %dus\n", us, delta); */
	return NULL;
}

void init_pi_mutex(pthread_mutex_t *m)
{
	pthread_mutexattr_t attr;
	int ret;
	int protocol;

	if ((ret = pthread_mutexattr_init(&attr)) != 0) {
		printf("Failed to init mutexattr: %d (%s)\n", ret, strerror(ret));
	};
	if (_use_pi && (ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT)) != 0) {
		printf("Can't set protocol prio inherit: %d (%s)\n", ret, strerror(ret));
	}
	if ((ret = pthread_mutexattr_getprotocol(&attr, &protocol)) != 0) {
		printf("Can't get mutexattr protocol: %d (%s)\n", ret, strerror(ret));
	}
	if ((ret = pthread_mutex_init(m, &attr)) != 0) {
		printf("Failed to init mutex: %d (%s)\n", ret, strerror(ret));
	}

	/* FIXME: does any of this need to be destroyed ? */
}

/* Write the entirety of data.  Complain if unable to do so. */
static void write_or_complain(int fd, const void *data, size_t len)
{
	const char *remaining = data;

	while (len > 0) {
		ssize_t ret = write(fd, remaining, len);
		if (ret <= 0) {
			if (errno != EAGAIN && errno != EINTR) {
				perror("write");
				return;
			}
		} else {
			remaining += ret;
			len -= ret;
		}
	}
}

/* Write the given data to the existing file specified by pathname.  Complain
 * if unable to do so. */
static void write_file(const char *pathname, const void *data, size_t len)
{
	int fd = open(pathname, O_WRONLY);
	if (fd < 0) {
		printf("Failed to open file \"%s\": %d (%s)\n",
		       pathname, errno, strerror(errno));
		return;
	}

	write_or_complain(fd, data, len);

	if (close(fd) < 0) {
		printf("Failed to close file \"%s\": %d (%s)\n",
		       pathname, errno, strerror(errno));
	}
}

/* Write the given '\0'-terminated string to the existing file specified by
 * pathname.  Complain if unable to do so. */
static void write_string_to_file(const char *pathname, const char *string)
{
	write_file(pathname, string, strlen(string));
}

static void read_and_print(const char *pathname, int output_fd)
{
	char data[4096];
	int fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		printf("Failed to open file \"%s\": %d (%s)\n",
		       pathname, errno, strerror(errno));
		return;
	}

	while (1) {
		ssize_t ret = read(fd, data, sizeof(data));
		if (ret < 0) {
			if (errno != EAGAIN && errno != EINTR) {
				printf("Failed to read from file \"%s\": %d (%s)\n",
				       pathname, errno, strerror(errno));
				break;
			}
		} else if (ret == 0)
			break;
		else
			write_or_complain(output_fd, data, ret);
	}

	if (close(fd) < 0) {
		printf("Failed to close file \"%s\": %d (%s)\n",
		       pathname, errno, strerror(errno));
	}
}

void latency_trace_enable(void)
{
	printf("Enabling latency tracer.\n");
	write_string_to_file("/proc/sys/kernel/trace_use_raw_cycles", "1");
	write_string_to_file("/proc/sys/kernel/trace_all_cpus", "1");
	write_string_to_file("/proc/sys/kernel/trace_enabled", "1");
	write_string_to_file("/proc/sys/kernel/trace_freerunning", "1");
	write_string_to_file("/proc/sys/kernel/trace_print_on_crash", "0");
	write_string_to_file("/proc/sys/kernel/trace_user_triggered", "1");
	write_string_to_file("/proc/sys/kernel/trace_user_trigger_irq", "-1");
	write_string_to_file("/proc/sys/kernel/trace_verbose", "0");
	write_string_to_file("/proc/sys/kernel/preempt_thresh", "0");
	write_string_to_file("/proc/sys/kernel/wakeup_timing", "0");
	write_string_to_file("/proc/sys/kernel/mcount_enabled", "1");
	write_string_to_file("/proc/sys/kernel/preempt_max_latency", "0");
}

#ifndef PR_SET_TRACING
#define PR_SET_TRACING 0
#endif

void latency_trace_start(void)
{
	if(prctl(PR_SET_TRACING, 1) < 0)
		perror("Failed to start tracing");
}

void latency_trace_stop(void)
{
	if(prctl(PR_SET_TRACING, 0) < 0)
		perror("Failed to stop tracing");
}

void latency_trace_print(void)
{
	read_and_print("/proc/latency_trace", STDOUT_FILENO);
}
