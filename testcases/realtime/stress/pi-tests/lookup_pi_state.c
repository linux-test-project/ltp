/******************************************************************************
 *
 *	 Copyright © International Business Machines	Corp., 2006, 2008
 *
 *	 This program is free software;	you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY;	without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See
 *	 the GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program;	if not, write to the Free Software
 *	 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *			lookup_pi_state.c
 *
 * DESCRIPTION
 *			 A test to reproduce a bug in lookup_pi_state()
 *
 * USAGE:
 *			Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *			Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *  2006-May-18:	Initial version by Darren Hart <dvhltc@us.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <librttest.h>

#define NUM_SLAVES 20
#define SLAVE_PRIO 89

pthread_mutex_t MM;
pthread_mutex_t MS;
pthread_mutex_t MT;
pthread_cond_t CM;
pthread_cond_t CS;
pthread_cond_t CT;

atomic_t slave_order_a = { 0 };
atomic_t slave_order_b = { 0 };
atomic_t slave_order_c = { 0 };

void usage(void)
{
	rt_help();
	printf("lookup_pi_state specific options:\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *slave_thread(void *arg)
{
	struct thread *t = (struct thread *)arg;
	int id = (intptr_t) t->arg;
// 3
	pthread_mutex_lock(&MS);
// 4,5
	if (atomic_inc(&slave_order_a) == NUM_SLAVES) {
		printf("Slave thread %d notifying master\n", id);
		pthread_mutex_lock(&MM);	// make sure the master thread is waiting
		pthread_cond_signal(&CM);
		pthread_mutex_unlock(&MM);
	}
	printf("Slave thread %d waiting on CS,MS\n", id);
	pthread_cond_wait(&CS, &MS);	// docs are contradictory on if this
	// should be MS or MM

	if (atomic_inc(&slave_order_b) <= 6) {
// 10,11
		;
		// do nothing, just terminate
	} else {
// 12
		pthread_cond_wait(&CS, &MS);
// 17
	}
	pthread_mutex_unlock(&MS);
	atomic_inc(&slave_order_c);
	printf("Slave thread %d terminating\n", id);
	return NULL;
}

void *master_thread(void *arg)
{
	int i;
	struct timespec ts_abs_timeout;
	struct thread *t = (struct thread *)arg;
// 1
	pthread_mutex_lock(&MM);
	for (i = 0; i < NUM_SLAVES; i++) {
		create_fifo_thread(slave_thread, (void *)(intptr_t) i,
				   SLAVE_PRIO);
	}
// 2
	printf("Master waiting till slaves wait()\n");
	pthread_cond_wait(&CM, &MM);
	printf("Master awoken\n");
// 6
	pthread_mutex_lock(&MS);
// 7
	printf("Master doing 3 signals\n");
	pthread_cond_signal(&CS);
	pthread_cond_signal(&CS);
	pthread_cond_signal(&CS);
// 8
	printf("Master doing 3 broadcasts\n");
	pthread_cond_broadcast(&CS);
	pthread_cond_broadcast(&CS);
	pthread_cond_broadcast(&CS);

	/* if we should timedwait on MS, then we don't need to unlock it here */
	pthread_mutex_unlock(&MS);

	printf("Master waiting 10 seconds\n");
	clock_gettime(CLOCK_REALTIME, &ts_abs_timeout);
	ts_abs_timeout.tv_sec += 10;
	/*
	 * docs say CS and MS, but that doesn't seem correct
	 *
	 * XXX (garrcoop): then that's a documentation or implementation bug.
	 * Duh... FIX IT!
	 */
	pthread_cond_timedwait(&CM, &MM, &ts_abs_timeout);
// 13
	pthread_mutex_unlock(&MM);
// 14
	printf("Master doing notify of all remaining slaves\n");
	pthread_mutex_lock(&MS);
	pthread_cond_broadcast(&CS);
// 15
	/*
	 * docs say MM, but that doesn't make sense..
	 *
	 * XXX (garrcoop): comments above apply here too
	 */
	pthread_mutex_unlock(&MS);
// 16
	pthread_mutex_lock(&MT);
	clock_gettime(CLOCK_REALTIME, &ts_abs_timeout);
	ts_abs_timeout.tv_sec += 2;
	pthread_cond_timedwait(&CT, &MT, &ts_abs_timeout);
// 18
	while (!thread_quit(t))
		usleep(10);

	printf("All slaves have terminated\n");

	return NULL;
}

int main(int argc, char *argv[])
{
	init_pi_mutex(&MM);
	init_pi_mutex(&MS);
	init_pi_mutex(&MT);
	setup();

	pthread_cond_init(&CM, NULL);
	pthread_cond_init(&CS, NULL);
	pthread_cond_init(&CT, NULL);

	rt_init("h", parse_args, argc, argv);

	create_other_thread(master_thread, NULL);

	/* wait for the slaves to quit */
	while (atomic_get(&slave_order_c) < NUM_SLAVES)
		usleep(10);

	join_threads();

	return 0;
}
