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
 *      libjvmsim.h
 *
 * DESCRIPTION
 *      Provide threads to mimic real-time jvm activity
 *
 *      Note: this "library" is written as a single header file to simplify the
 *      creation of new test cases and keep the build system simple.  At some
 *      point it may make sense to break it up into .h and .c files and link to
 *      it as an object file.

 * USAGE:
 *      Compilation : To be included in testcases.
 *
 * AUTHOR
 *      Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *      2006-May-9:  Initial version by Darren Hart
 *
 *****************************************************************************/

#ifndef LIBJVMSIM_H
#define LIBJVMSIM_H

#define SIT_AND_WAIT \
struct thread *t = (struct thread *)arg; \
int i = 0; \
while (!thread_quit(t)) { \
	pthread_mutex_lock(&block_mutex); \
	pthread_cond_wait(&block_cond, &block_mutex); \
	busy_work_us(50); /* this should only happen on exit atm */ \
	pthread_mutex_unlock(&block_mutex); \
	i++; \
} \
debug(DBG_INFO, "JVMSIM: %s thread woke %d times\n", __FUNCTION__, i); \
return NULL;

/* Function Prototypes */
void *jvm_gc_alarm(void *arg);
void *jvm_gc_collect(void *arg);
void *jvm_gc_trace(void *arg);
void *jvm_gc_finalize(void *arg);
void *jvm_posix_signal_dispatch(void *arg);
void *jvm_sigquit(void *arg);
void *jvm_async_event_server(void *arg);
void *jvm_timer_dispatch(void *arg);
void *jvm_ras_trace(void *arg);
void *jvmsim_monitor(void *arg);
void jvmsim_init();

#endif /* LIBJVMSIM_H */
