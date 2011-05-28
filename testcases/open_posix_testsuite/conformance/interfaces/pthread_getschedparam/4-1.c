/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
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
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  27/05/2011
 *
 *   Test assertion 4 - EINTR is not returned.   Do this be creating two
 *   in SCHED_FIFO, one thread spins until a signal is sent from the
 *   other thread.
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <posixtest.h>

#define ERR_MSG(f, rc)	printf("Failed: function: %s status: %s(%u)\n", \
						f, strerror(rc), rc)

typedef void * (*tfunc)(void *);

/* Fleg, indicates when signal is sent */
static int sent;

#define SIG SIGHUP

/*
 * Used to verify a signal is pending
 * Note that this will pop the signal from the stack.
 */
#if VERIFY_PENDING
static void verify(void)
{
	sigset_t sig;
	int s = 0;

	sigemptyset(&sig);
	sigaddset(&sig, SIG);
	rc = sigwait(&sig, &s);
	printf("sigwait: rc = %d, s = %d\n", rc, s);
}
#endif

static void *test_thread(void *data)
{
	struct sched_param sp;
	int policy;
	int rc;
	long status;


	/*
	 * Wait until the signal is posted
	 */
	while (!sent)
		sched_yield();

#if  VERIFY_PENDING
	verify();
#endif

	rc = pthread_getschedparam(pthread_self(), &policy, &sp);
	switch (rc) {
	case EINTR:
		status = PTS_FAIL;
		ERR_MSG("pthread_getschedparam", rc);
		break;
	case 0:
		status = PTS_PASS;
		break;
	default:
		ERR_MSG("pthread_getschedparam", rc);
		status = PTS_UNRESOLVED;
		break;
	}

	return (void *) status;
}

static void *kill_thread(void *data)
{
	pthread_t *t = data;
	int rc;

	rc = pthread_kill(*t, SIG);

	/* Tell test thread signal is sent */
	sent = 1;

	if (rc) {
		ERR_MSG("pthread_kill()", rc);
		return (void *) PTS_UNRESOLVED;
	}

	return (void *) PTS_PASS;
}

static int create_thread(int prio, tfunc f, void *data, pthread_t *tid)
{
	int rc;
	char *func;
	struct sched_param sp;
	pthread_attr_t attr;

	func = "pthread_attr_init()";
	rc = pthread_attr_init(&attr);
	if (rc != 0)
		goto done;

	func = "pthread_attr_setschedpolicy()";
	rc = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setinheritsched()";
	rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setschedparam()";
	sp.sched_priority = prio;
	rc = pthread_attr_setschedparam(&attr, &sp);
	if (rc != 0)
		goto error;

	rc = pthread_create(tid, &attr, f, data);
	if (rc) {
		ERR_MSG("pthread_create()", rc);
		goto error;
	}

	pthread_attr_destroy(&attr);

	return 0;

error:
	pthread_attr_destroy(&attr);
done:
	ERR_MSG(func, rc);
	return -1;
}



int main(void)
{
	int status;
	int rc;
	void *r1;
	void *r2;
	char *name;
	pthread_t t1;
	pthread_t t2;
	sigset_t sig;

	status = PTS_UNRESOLVED;

	name = "sigaddmask()";
	sigemptyset(&sig);
	rc = sigaddset(&sig, SIG);
	if (rc)
		goto done;

	name = "pthread_sigmask()";
	rc = pthread_sigmask(SIG_BLOCK, &sig, NULL);
	if (rc)
		goto done;

	name = "pthread_create() - test_thread";
	rc = create_thread(25, test_thread, NULL, &t1);
	if (rc)
		goto done;

	name = "pthread_create() - kill_thread";
	rc = create_thread(25, kill_thread, &t1, &t2);
	if (rc)
		goto done;

	name = "pthread_join() - test_thread";
	rc = pthread_join(t1, &r1);
	if (rc)
		goto done;

	name = "pthread_join() - kill_thread";
	rc = pthread_join(t2, &r2);
	if (rc)
		goto done;

	status = PTS_PASS;
	if ((long) r1 != PTS_PASS)
		status = (long) r1;
	if ((long) r2 != PTS_PASS)
		status = (long) r2;

	if (status == PTS_PASS)
		printf("Test PASS\n");

	return status;

done:
	ERR_MSG(name, rc);
	return PTS_UNRESOLVED;

}
