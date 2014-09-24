/*
* Disktest
* Copyright (c) International Business Machines Corp., 2005
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: signals.c,v 1.1 2008/02/14 08:22:23 subrata_modak Exp $
*/
#ifdef WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <signal.h>
#include "threading.h"
#include "signals.h"

/*
 * global variable used to indicate what signal
 * (if any) has been caught
 */
int handled_signal = -1;
int signal_action = SIGNAL_NONE;

/*
 * mutex to be used whenever accessing the above
 * global data
 */
#ifdef WINDOWS
HANDLE sig_mutex;
#else
pthread_mutex_t sig_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef WINDOWS
void sig_handler(int sig)
#else
void *sig_handler(void *arg)
#endif
{
#ifndef WINDOWS
	sigset_t signal_set;
	int sig;
	int rv;

	/* wait for any and all signals */
	sigfillset(&signal_set);
#ifdef AIX
	/* except in AIX, can't sigwait on this signals */
	sigdelset(&signal_set, SIGKILL);
	sigdelset(&signal_set, SIGWAITING);
	sigdelset(&signal_set, SIGSTOP);
#endif

	for (;;) {
		rv = sigwait(&signal_set, &sig);
#endif

		switch (sig) {
		case SIGQUIT:
			LOCK(sig_mutex);
			handled_signal = SIGQUIT;
			signal_action |= SIGNAL_STOP;
			UNLOCK(sig_mutex);
			break;

		case SIGINT:
			LOCK(sig_mutex);
			handled_signal = SIGINT;
			signal_action |= SIGNAL_STOP;
			UNLOCK(sig_mutex);
			break;

		case SIGTERM:
			LOCK(sig_mutex);
			handled_signal = SIGTERM;
			signal_action |= SIGNAL_STOP;
			UNLOCK(sig_mutex);
			break;

		case SIGHUP:
			LOCK(sig_mutex);
			handled_signal = SIGHUP;
			signal_action |= SIGNAL_STOP;
			UNLOCK(sig_mutex);
			break;

		case SIGUSR1:
			LOCK(sig_mutex);
			handled_signal = SIGUSR1;
			signal_action |= SIGNAL_STAT;
			UNLOCK(sig_mutex);
			break;

			/* whatever you need to do for other signals */
		default:
			LOCK(sig_mutex);
			handled_signal = 0;
			UNLOCK(sig_mutex);
			break;
		}
#ifndef WINDOWS
	}
	return NULL;
#endif
}

void setup_sig_mask(void)
{
#ifndef WINDOWS
	sigset_t signal_set;
	pthread_t sig_thread;
#endif

#ifdef WINDOWS
	if ((sig_mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
		return;
	}
#endif

	/* block all signals */
#ifdef WINDOWS
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGUSR1, sig_handler);
#else
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGINT);
	sigaddset(&signal_set, SIGHUP);
	sigaddset(&signal_set, SIGQUIT);
	sigaddset(&signal_set, SIGTERM);
	sigaddset(&signal_set, SIGUSR1);

#ifdef AIX
	sigthreadmask(SIG_SETMASK, &signal_set, NULL);
#else
	pthread_sigmask(SIG_SETMASK, &signal_set, NULL);
#endif

	/* create the signal handling thread */
	pthread_create(&sig_thread, NULL, sig_handler, NULL);
#endif
}

void clear_stat_signal(void)
{
	if (signal_action & SIGNAL_STAT) {
		LOCK(sig_mutex);
		signal_action &= ~SIGNAL_STAT;
		UNLOCK(sig_mutex);
	}
}
