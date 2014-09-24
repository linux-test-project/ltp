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
 *      async_handler_jk.c
 *
 * DESCRIPTION
 *     This test mimics an async event handler in a real-time JVM
 *     An async event server thread is created that goes to sleep waiting
 *     to be woken up to do some work.
 *
 *     A user thread is created that simulates the firing of an event by
 *     signalling the async handler thread to do some work.
 *
 * USAGE:
 *     Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      John Kacur <jkacur@ca.ibm.com>
 *
 * HISTORY
 *     2006-Nov-20: Initial Version by John Kacur <jkacur@ca.ibm.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <librttest.h>
#include <libstats.h>

// This is the normal priority for an event handler if not specified.
#define NORMAL_PRIORITY	43
#define THREAD_FLAG_SUSPENDED 8
#define PASS_US 100

long start, end;

/* Function Prototypes */
void *async_event_server(void *arg);
void *handler1(void *arg);

void usage(void)
{
	rt_help();
	printf("async_handler_jk specific options:\n");
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

void *async_event_server(void *arg)
{
	int err = 0;
	struct thread *thread = ((struct thread *)arg);

	thread->func = NULL;	// entrypoint
	thread->flags |= THREAD_FLAG_SUSPENDED;

	for (;;) {
		if ((err = pthread_mutex_lock(&thread->mutex)))
			return (void *)(intptr_t) err;

		/* Go to sleep and wait for work */
		while (thread->flags & THREAD_FLAG_SUSPENDED)
			pthread_cond_wait(&thread->cond, &thread->mutex);

		pthread_mutex_unlock(&thread->mutex);

		/* The JVM would be able to dynamically choose a handler */
		thread->func = handler1;

		if (thread->func != NULL)
			thread->func(arg);

		// Reset Priority to original async server priority
		set_thread_priority(thread->pthread, thread->priority);

		thread->flags |= THREAD_FLAG_SUSPENDED;
	}			// Go back to sleep and wait for next command
}

void *user_thread(void *arg)
{
	struct thread *thread = ((struct thread *)arg);
	struct thread *server = (struct thread *)thread->arg;

	start = rt_gettime();

	/* Change the async server thread priority to be the priority
	   of the user_thread. (event thread) */
	set_thread_priority(server->pthread, thread->priority);

	/* Clear the THREAD_FLAG_SUSPENDED flag of the server before signal */
	server->flags &= ~THREAD_FLAG_SUSPENDED;

	/* Signal the async server thread - simulates firing of an event */
	pthread_cond_broadcast(&server->cond);

	return NULL;
}

void *handler1(void *arg)
{
	end = rt_gettime();
	return NULL;
}

int main(int argc, char *argv[])
{
	int aes_id;		// asynchronous event server id
	int user_id;		// User thread - that fires the event
	long delta;
	struct thread *server;
	setup();

	pass_criteria = PASS_US;
	rt_init("h", parse_args, argc, argv);

	aes_id = create_fifo_thread(async_event_server, NULL, 83);
	server = get_thread(aes_id);

	user_id =
	    create_fifo_thread(user_thread, (void *)server, NORMAL_PRIORITY);

	usleep(1000);
	pthread_detach(server->pthread);
	join_thread(user_id);
	join_threads();
	delta = (end - start) / NS_PER_US;

	printf("delta = %ld us\n", delta);
	printf("\nCriteria: latencies < %d\n", (int)pass_criteria);
	printf("Result: %s\n", delta > pass_criteria ? "FAIL" : "PASS");

	return 0;
}
