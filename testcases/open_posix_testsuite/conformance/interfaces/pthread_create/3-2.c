/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 * This sample test aims to check the following assertion:
 *
 * Once the thread has been created, subsequent changes to the
 * thread attribute object don't affect the running thread.

 * The steps are:
 * -> stack grow
 *    -> create a thread with minimal stack size
 *    -> change the stack size to a bigger value
 *    -> check that the thread stack size did not change.
 * -> stack decrease
 *    -> create a thread with a known stack size (> minimum)
 *    -> change the stack size to the min value
 *    -> check that the thread stack size did not change.
 * -> sched policy/param change
 *    -> create a new thread with a known policy
 *    -> change the policy in the thread attribute and check the thread policy did not change
 *    -> change the schedparam in the thread attribute and check the thread priority did not change
 *    -> change the policy in the running thread and check the thread attribute did not change.
 *    -> change the priority in the running thread and check the thread attribute did not change.

 * The test fails if one of the checking fails

 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  *
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  *
  * The other file defines the functions
  * void output_init()
  * void output(char * string, ...)
  *
  * Those may be used to output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "../testfrmw/threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

static sem_t semsync[2];		/* These semaphores will only be used in child process! */

static unsigned int sc;

/* The overflow function is used to test the stack overflow */
static void *overflow(void *arg)
{
	void *current;
	long stacksize = sysconf(_SC_THREAD_STACK_MIN);	/* make sure we touch the current stack memory */

	int ret = 0;

	if (arg == NULL) {
		/* first call */

		/* Synchronize with the parent */
		do {
			ret = sem_wait(&semsync[0]);
		}
		while ((ret == -1) && (errno == EINTR));
		if (ret == -1) {
			UNRESOLVED(errno, "Failed to wait for the semaphore");
		}

		/* Go to recursion */
		current = overflow(&current);

		/* Terminated */
		do {
			ret = sem_post(&semsync[1]);
		}
		while ((ret == -1) && (errno == EINTR));
		if (ret == -1) {
			UNRESOLVED(errno, "Failed to post the semaphore");
		}

		/* Terminate the overflow thread */
		return current;
	}

	/* we cast the pointers into long, which might be a problem on some architectures... */
	if (((long)arg) < ((long)&current)) {
		/* the stack is growing up */
		if (((long)&current) - ((long)arg) >= stacksize) {
			output
			    ("Growing up stack started below %p and we are currently up to %p\n",
			     arg, &current);
			return NULL;
		}
	} else {
		/* the stack is growing down */
		if (((long)arg) - ((long)&current) >= stacksize) {
			output
			    ("Growing down stack started upon %p and we are currently down to %p\n",
			     arg, &current);
			return NULL;
		}
	}

	/* We are not yet overflowing, so we loop */
	return overflow(arg);
}

/* The following function will return
  0 if a thread created with the attribute ta is able to fill the stack up to {minstacksize}.
  1 if the operation failed.
  2 if an error prevented the test to complete

  If newsize is not 0, the stack size in ta will be set to this value once the thread is created.

*/
static int test_stack(pthread_attr_t * ta, size_t newsize)
{
	pid_t child, ctrl;
	int status;
	int ret;

#if VERBOSE > 0
	fflush(stdout);
#endif

	child = fork();		/* We'll test the feature in another process as this test may segfault */

	if (child == -1) {
		output("Failed to fork (%s)\n", strerror(errno));
		return 2;
	}

	if (child != 0) {	/* father */
		/* Just wait for the child and check its return value */
		ctrl = waitpid(child, &status, 0);
		if (ctrl != child) {
			output("Failed to wait for process termination (%s)\n",
			       strerror(errno));
			return 2;
		}

		if (WIFEXITED(status)) {	/* The process exited */
			if (WEXITSTATUS(status) == 0) {
				return 0;
			}	/* We were able to fill the stack */
			if (WEXITSTATUS(status) == PTS_UNRESOLVED) {
				output
				    ("The child process returned unresolved status\n");
				return 2;
			} else {
				output("The child process returned: %i\n",
				       WEXITSTATUS(status));
				return 2;
			}
		} else {
#if VERBOSE > 4
			output("The child process did not return\n");
			if (WIFSIGNALED(status))
				output("It was killed with signal %i\n",
				       WTERMSIG(status));
			else
				output("neither was it killed. (status = %i)\n",
				       status);
#endif
		}

		return 1;
	}

	/* else */
	/* this is the new process */
	{
		pthread_t th;
		void *rc;
		int detach;

		/* Semaphore to force the child to wait */
		ret = sem_init(&semsync[0], 0, 0);
		if (ret == -1) {
			UNRESOLVED(errno, "Unable to init a semaphore");
		}
		/* Semaphore to detect thread ending */
		ret = sem_init(&semsync[1], 0, 0);
		if (ret == -1) {
			UNRESOLVED(errno, "Unable to init a semaphore");
		}

		ret = pthread_create(&th, ta, overflow, NULL);	/* Create a new thread with the same attributes */
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to create a thread in the new process");
		}

		/* If we were asked to perform a change on ta, do it now. */
		if (newsize) {
			ret = pthread_attr_setstacksize(ta, newsize);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to set the new stack size");
			}
		}

		/* Ok the child can run now */
		do {
			ret = sem_post(&semsync[0]);
		}
		while ((ret == -1) && (errno == EINTR));
		if (ret == -1) {
			UNRESOLVED(errno, "Failed to post the semaphore");
		}

		/* Wait for its termination */
		do {
			ret = sem_wait(&semsync[1]);
		}
		while ((ret == -1) && (errno == EINTR));
		if (ret == -1) {
			UNRESOLVED(errno, "Failed to wait for the semaphore");
		}

		if (ta != NULL) {
			ret = pthread_attr_getdetachstate(ta, &detach);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to get detach state from the thread attribute");
			}
		} else {
			detach = PTHREAD_CREATE_JOINABLE;	/* default */
		}

		if (detach == PTHREAD_CREATE_JOINABLE) {
			ret = pthread_join(th, &rc);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to join a thread");
			}
			if (rc != NULL) {
				UNRESOLVED((int)(long)rc,
					   "The overflow function returned an unexpected value");
			}
		}

		/* Terminate the child process here */
		exit(0);
	}
}

typedef struct {
	pthread_barrier_t bar;
	int policy;
	struct sched_param param;
} testdata_t;

static void *schedtest(void *arg)
{
	testdata_t *td = (testdata_t *) arg;
	int newpol, ret = 0;
	struct sched_param newparam;

	/* Read the current sched policy & param */
	ret =
	    pthread_getschedparam(pthread_self(), &(td->policy), &(td->param));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to read current thread policy / param");
	}

	/* sync 1 */
	ret = pthread_barrier_wait(&(td->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to synchronize on the barrier");
	}

	/* sync 2 */
	ret = pthread_barrier_wait(&(td->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to synchronize on the barrier");
	}

	/* Read the current sched policy & param */
	ret =
	    pthread_getschedparam(pthread_self(), &(td->policy), &(td->param));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to read current thread policy / param");
	}

	/* sync 3 */
	ret = pthread_barrier_wait(&(td->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to synchronize on the barrier");
	}

	/* sync 4 */
	ret = pthread_barrier_wait(&(td->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to synchronize on the barrier");
	}

	/* Change the current sched policy & param */
	if (td->policy == SCHED_RR)
		newpol = SCHED_FIFO;
	else
		newpol = SCHED_RR;

	newparam.sched_priority = sched_get_priority_max(newpol);

	if (newparam.sched_priority == td->param.sched_priority)
		newparam.sched_priority--;

	ret = pthread_setschedparam(pthread_self(), newpol, &newparam);
#if VERBOSE > 0
	if (ret != 0)
		output
		    ("Changing the current thread sched policy failed with error: %s\n",
		     strerror(ret));
#endif
#if VERBOSE > 2
	else
		output("Executing thread scheduling policy changed\n");
#endif

	/* sync 5 */
	ret = pthread_barrier_wait(&(td->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to synchronize on the barrier");
	}

	/* Post the sem in case of a detached thread */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	return NULL;
}

int main(void)
{
	int ret = 0;
	int do_stack_tests;
	int do_sched_tests;

	/* Initialize output routine */
	output_init();

	/* Test abilities */
	do_sched_tests = (sysconf(_SC_THREAD_PRIORITY_SCHEDULING) > 0 ? 1 : 0);
	do_stack_tests = (test_stack(NULL, 0) == 0 ? 1 : 0);
#if VERBOSE > 0
	output
	    ("Test starting\n Stack tests %s be executed.\n Sched tests %s be executed.\n",
	     do_stack_tests ? "will" : "won't",
	     do_sched_tests ? "will" : "won't");
#endif

	/* Initialize thread attribute objects */
	scenar_init();

	for (sc = 0; sc < NSCENAR; sc++) {
#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", sc,
		       scenarii[sc].descr);
#endif

		if (do_stack_tests) {
			/* stack grow test */
			/* We need the thread attribute to specify a minimal stack */
			if ((scenarii[sc].altstack == 0)
			    && (scenarii[sc].altsize == 1)) {
#if VERBOSE > 2
				output("Processing stack grow test\n");
#endif

				ret =
				    test_stack(&scenarii[sc].ta,
					       2 *
					       sysconf(_SC_THREAD_STACK_MIN));

				if (ret == 0) {
					if (scenarii[sc].guard == 2) {
						FAILED
						    ("Changing the stacksize after the thread was created changed the running thread stack size");
					}
#if VERBOSE > 2
					else
						output
						    ("We were able to overflow the stack, but the guard area is unknow or null\n");
#endif

				}

				if ((ret != 2) && (scenarii[sc].result == 1)) {
					UNRESOLVED(-1,
						   "An error was expected but the thread creation succeeded");
				}
#if VERBOSE > 2
				if ((ret == 1)) {
					output("Stack grow test passed\n");
				}

				if ((ret == 2) && (scenarii[sc].result == 2)) {
					output
					    ("Something went wrong -- we don't care in this case\n");
				}
#endif

				if ((ret == 2) && (scenarii[sc].result == 0)) {
					UNRESOLVED(-1,
						   "An unexpected error occured\n");
				}

				/* Ok, set back the thread attribute object to a correct value */
				ret =
				    pthread_attr_setstacksize(&scenarii[sc].ta,
							      sysconf
							      (_SC_THREAD_STACK_MIN));
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to set stacksize back");
				}
			}

			/* stack decrease test */
			if ((scenarii[sc].altstack == 0)
			    && (scenarii[sc].altsize == 0)) {
#if VERBOSE > 2
				output("Processing stack decrease test\n");
#endif

				ret =
				    test_stack(&scenarii[sc].ta,
					       sysconf(_SC_THREAD_STACK_MIN));

				if (ret == 1) {
					FAILED
					    ("Decreasing the stack size after thread is created had an influence on the thread");
				}

				if ((ret == 0) && (scenarii[sc].result == 1)) {
					UNRESOLVED(-1,
						   "An error was expected but the thread creation succeeded");
				}

				if ((ret == 2) && (scenarii[sc].result == 0)) {
					UNRESOLVED(-1,
						   "An unexpected error occured\n");
				}
#if VERBOSE > 2
				if (ret == 0)
					output("Stack decrease test passed.\n");
				else
					output
					    ("Something failed but we don't care here.\n");
#endif
			}

		}
		/* if do_stack_tests */
		if (do_sched_tests) {
			/* Sched policy/param change test */
			if (scenarii[sc].explicitsched != 0) {	/* We need a specified policy */
				pthread_t child;
				int policy_ori, newpol_max;
				struct sched_param param_ori, tmp;

				testdata_t td;

#if VERBOSE > 2
				output
				    ("Processing sched policy/param change test\n");
#endif

				/* Backup the scenario object */
				ret =
				    pthread_attr_getschedpolicy(&
								(scenarii
								 [sc].ta),
								&policy_ori);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to read sched policy from thread attribute");
				}
				ret =
				    pthread_attr_getschedparam(&
							       (scenarii
								[sc].ta),
							       &param_ori);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to read sched param from thread attribute");
				}

				/* Initialize the barrier */
				ret = pthread_barrier_init(&(td.bar), NULL, 2);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to initialize the barrier");
				}

				/* Create a new thread with this scenario attribute */
				ret =
				    pthread_create(&child, &(scenarii[sc].ta),
						   schedtest, &td);
				if (ret != 0) {
					if (scenarii[sc].result == 0) {
						UNRESOLVED(ret,
							   "Failed to create a thread");
					}
#if VERBOSE > 2
					if (scenarii[sc].result == 2) {
						output
						    ("The thread creation failed -- we don't care\n");
					}
					if (scenarii[sc].result == 1) {
						output
						    ("The thread creation failed as expected\n");
					}
#endif
				} else {	/* Thread created */

					if (scenarii[sc].result == 1) {
						UNRESOLVED(-1,
							   "The thread was created where an error was expected");
					}
#if VERBOSE > 2
					if (scenarii[sc].result == 2)
						output("Thread is created\n");
#endif

					/* sync 1 */
					ret = pthread_barrier_wait(&(td.bar));
					if ((ret != 0)
					    && (ret !=
						PTHREAD_BARRIER_SERIAL_THREAD))
					{
						UNRESOLVED(ret,
							   "Failed to synchronize on the barrier");
					}

					/* Check the new thread reports the attributes */
					if (td.policy != policy_ori) {
						FAILED
						    ("The new thread does not report the scheluling policy that was specified");
					}

					if (td.param.sched_priority !=
					    param_ori.sched_priority) {
						FAILED
						    ("The new thread does not report the scheduling priority that was specified at creation");
					}

					/* Change the thread attribute object policy & param */
					if (policy_ori == SCHED_RR) {
						ret =
						    pthread_attr_setschedpolicy
						    (&(scenarii[sc].ta),
						     SCHED_FIFO);
						newpol_max =
						    sched_get_priority_max
						    (SCHED_FIFO);
					} else {
						ret =
						    pthread_attr_setschedpolicy
						    (&(scenarii[sc].ta),
						     SCHED_RR);
						newpol_max =
						    sched_get_priority_max
						    (SCHED_RR);
					}
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Failed to change the attribute object");
					}

					if (newpol_max ==
					    param_ori.sched_priority)
						newpol_max--;

					tmp.sched_priority = newpol_max;

					ret =
					    pthread_attr_setschedparam(&
								       (scenarii
									[sc].ta),
								       &tmp);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Failed to set the attribute sched param");
					}

					/* sync 2 */
					ret = pthread_barrier_wait(&(td.bar));
					if ((ret != 0)
					    && (ret !=
						PTHREAD_BARRIER_SERIAL_THREAD))
					{
						UNRESOLVED(ret,
							   "Failed to synchronize on the barrier");
					}

					/* sync 3 */
					ret = pthread_barrier_wait(&(td.bar));
					if ((ret != 0)
					    && (ret !=
						PTHREAD_BARRIER_SERIAL_THREAD))
					{
						UNRESOLVED(ret,
							   "Failed to synchronize on the barrier");
					}

					/* Check if the thread saw the change (should not) */
					if (td.policy != policy_ori) {
						FAILED
						    ("The new thread does not report the scheluling policy that was specified");
					}

					if (td.param.sched_priority !=
					    param_ori.sched_priority) {
						FAILED
						    ("The new thread does not report the scheduling priority that was specified at creation");
					}

					/* Check what we can see for the child thread from here */
					ret =
					    pthread_getschedparam(child,
								  &(td.policy),
								  &(td.param));
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Failed to read child thread policy / param");
					}

					if (td.policy != policy_ori) {
						FAILED
						    ("The child thread does not report the scheduling policy that was specified at creation");
					}

					if (td.param.sched_priority !=
					    param_ori.sched_priority) {
						FAILED
						    ("The child thread does not report the scheduling priority that was specified at creation");
					}

					/* Restore the thread attribute */
					ret =
					    pthread_attr_setschedpolicy(&
									(scenarii
									 [sc].ta),
									policy_ori);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to read sched policy from thread attribute");
					}
					ret =
					    pthread_attr_setschedparam(&
								       (scenarii
									[sc].ta),
								       &param_ori);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to read sched param from thread attribute");
					}

					/* sync 4 */
					ret = pthread_barrier_wait(&(td.bar));
					if ((ret != 0)
					    && (ret !=
						PTHREAD_BARRIER_SERIAL_THREAD))
					{
						UNRESOLVED(ret,
							   "Failed to synchronize on the barrier");
					}

					/* sync 5 */
					ret = pthread_barrier_wait(&(td.bar));
					if ((ret != 0)
					    && (ret !=
						PTHREAD_BARRIER_SERIAL_THREAD))
					{
						UNRESOLVED(ret,
							   "Failed to synchronize on the barrier");
					}

					/* check if the thread attribute reports a change (should not) */
					ret =
					    pthread_attr_getschedpolicy(&
									(scenarii
									 [sc].ta),
									&
									(td.policy));
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to read sched policy from thread attribute");
					}
					ret =
					    pthread_attr_getschedparam(&
								       (scenarii
									[sc].ta),
								       &
								       (td.param));
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to read sched param from thread attribute");
					}

					if (td.policy != policy_ori) {
						FAILED
						    ("The child thread does not report the scheduling policy that was specified at creation");
					}

					if (td.param.sched_priority !=
					    param_ori.sched_priority) {
						FAILED
						    ("The child thread does not report the scheduling priority that was specified at creation");
					}

					/* Wait for the sem and join eventually the thread */
					do {
						ret =
						    sem_wait(&scenarii[sc].sem);
					}
					while ((ret == -1) && (errno == EINTR));
					if (ret == -1) {
						UNRESOLVED(errno,
							   "Failed to wait for the semaphore");
					}

					if (scenarii[sc].detached == 0) {
						ret = pthread_join(child, NULL);
						if (ret != 0) {
							UNRESOLVED(ret,
								   "Unable to join a thread");
						}
					}
#if VERBOSE > 2
					output
					    ("Sched policy/param change test passed\n");
#endif
				}	/* thread created */
			}

			/* We could also test if the inheritsched does not influence the new thread */

		}		/* if do_sched_tests */
	}

	scenar_fini();
#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
#endif

	PASSED;
}
