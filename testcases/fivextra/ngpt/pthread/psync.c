/*
 * Test thread contention for a monitor (mutex and cond var)
 *
 * This simulates the way Java implements synchronized methods.
 *
 * Compile as follows:
 *
 * 	cc -g -D_REENTRANT psync.c -lpthread -o psync
 *
 * history:	Jun 24 2003 - created by RC Paulsen <rpaulsen@us.ibm.com>
 *		Aug 25 2003 - added sched_yield to thread_routine (RCP)
 */
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>	// for sched_yield()
#include <stdio.h>

unsigned int	thd_cnt=0;	/* number of threads created */

/****************************************************************************
 * monitor used to control a synchronized method
 ***************************************************************************/

/*
 * a monitor
 */
typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t sig;
	volatile int busy;		/* synchronized method in-use flag */
} monitor_t;

/*
 * create a monitor 
 */
monitor_t *make_monitor()
{
	monitor_t *mon=(monitor_t*)malloc(sizeof(monitor_t));
	// Old glibc versions had bug in pthreads requiring the use of
	// PTHREAD_MUTEX_ERRORCHECK_NP
	// pthread_mutexattr_t mattr;
	// pthread_mutexattr_init(&mattr);
	// pthread_mutexattr_setkind_np(&mattr, PTHREAD_MUTEX_ERRORCHECK_NP);
	// pthread_mutex_init(&mon->lock,&mattr);
	pthread_mutex_init(&mon->lock,NULL);
	pthread_cond_init(&mon->sig,NULL);
	mon->busy=0;
	return mon;
}

/*
 * acquire a monitor
 */
void get_monitor(monitor_t *mon)
{
	pthread_mutex_lock(&mon->lock);
	while (mon->busy)
		pthread_cond_wait(&mon->sig,&mon->lock);
	mon->busy=1;
}

/*
 * release a monitor
 */
void rel_monitor(monitor_t *mon)
{
	mon->busy=0;
	pthread_cond_signal(&mon->sig);
	pthread_mutex_unlock(&mon->lock);
}

/****************************************************************************
 * shared data with an associated synchronized method
 ***************************************************************************/

/*
 * the data
 */
typedef struct {
	monitor_t *mon;		/* the synchronization controls */
	volatile long *data;	/* the shared data */
} shared_t;

/*
 * create a shared object
 */
shared_t *make_shared()
{
	shared_t *shr=(shared_t*)malloc(sizeof(shared_t));
	shr->data=(long*)malloc(sizeof(long));
	*(shr->data)=0;
	shr->mon=make_monitor();
	return shr;
}

/*
 * read the shared data
 */
long get_data(shared_t *shr)
{
	return *(shr->data);
}

/*
 * a synchronized method that accesses the shared data and tempts other threads
 * to mess with it when they shouldn't.
 */
void sync_method(shared_t *shr)
{
	long new_val;
	get_monitor(shr->mon);		/* enter synch method */
	new_val=++*(shr->data);		/* mess with the shared data */
	sched_yield();			/* encourage other threads */
	usleep(1);			/* encourage other threads */
	if (new_val!=*(shr->data)) {	/* ensure data is safe */
		fprintf(stderr,
			"Shared data incremented by non-owner of monitor!\n");
		fprintf(stderr,"Thread %u expected %ld, saw %ld\n",
			pthread_self(),new_val,*(shr->data));
		exit(1);
	}
	rel_monitor(shr->mon);	/* leave synch method */
}

/****************************************************************************
 * thread that invokes the synchronized method
 ***************************************************************************/

/*
 * the thread routine
 */
typedef void* thread_routine_t(void*);
static void* thread_routine(void *arg) {

	shared_t* shr = (shared_t*)arg;
	pthread_t me = pthread_self();
	long i;

	printf("thread %u started\n",me);

	/* contend for the synchronzed method */
	for (i=1;;++i) {			/* forever */
		sync_method(shr);
		if (i==1)
			printf("Thread %u ran at least once\n",me);
		sched_yield();
	} /* forever */

	return 0; /* never gets here */
}

/*
 * make a detached thread (utility function for make_threads)
 */
pthread_t make_detached_thread(thread_routine_t *thd_rtn, void* arg) {

	pthread_attr_t	detach;
	pthread_t	tid;
	int		rc;

	/* set up thread attributes */
	assert(0==pthread_attr_init(&detach));
	assert(0==
		pthread_attr_setdetachstate(&detach,PTHREAD_CREATE_DETACHED));

	/* create the thread */
	rc=pthread_create(&tid,&detach,thd_rtn,arg);
	if (rc) {
		fprintf(stderr,"%u threads created\n",thd_cnt);
		fprintf(stderr,"pthread_create rc=%d\n",rc);
		perror("pthread_create failed");
		exit(1);
	}

	return tid;
}

/* 
 * create/start all threads. these contend for synchronized method.
 */
void make_threads(shared_t *shr, int num_thds) {

	for (thd_cnt=0; thd_cnt<num_thds; ++thd_cnt)
		make_detached_thread(thread_routine, shr);

	printf("All %u threads created!\n",thd_cnt);
}

/****************************************************************************
 * main
 ***************************************************************************/

int main(int argc, char **argv) {

	int num_threads=30;	/* default for cmd line arg #1 */
	int sleep_time=30; 	/* default for cmd line arg #2 */

	/* cmd line arg #1:  number of threads to create */
	if (argc >= 2) num_threads=atoi(argv[1]);
	if (num_threads <= 0) {
		fprintf(stderr,"Invalid number of threads (%s) specified\n",argv[1]);
		exit(1);
	}
	printf("Creating %d threads.\n",num_threads);

	/* cmd line arg #2:  number of seconds to let testcase run */
	if (argc == 3) sleep_time=atoi(argv[2]);
	if (sleep_time <= 0) {
		fprintf(stderr,"Invalid sleep time (%s) specified\n",argv[2]);
		exit(2);
	}

	/* an object with a synchronized method */
	shared_t *shr = make_shared();

	/* start all threads (which contend for synchronized method) */
	make_threads(shr, num_threads);

	/* let test run for a while */
	printf("Sleeping %d seconds.\n",sleep_time);
	sleep(sleep_time);
	printf("shared data incremented to %ld\n",get_data(shr));
	exit(0);
}
