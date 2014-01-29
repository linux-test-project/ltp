/*
* $Id: timer.c,v 1.6 2009/02/26 12:02:23 subrata_modak Exp $
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
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
* $Id: timer.c,v 1.6 2009/02/26 12:02:23 subrata_modak Exp $
*
*/
#include <stdio.h>
#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include "getopt.h"
#else
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "globals.h"
#include "threading.h"
#include "sfunc.h"
#include "stats.h"
#include "signals.h"

/*
 * The main purpose of this thread is track time during the test. Along with
 * keeping track of read/write time. And check that each interval, that the
 * IO threads are making progress. The timer thread is started before any IO
 * threads and will complete either after all IO threads exit, the test fails,
 * or if a timed run, the run time is exceeded.
 */
#ifdef WINDOWS
DWORD WINAPI ChildTimer(test_ll_t * test)
#else
void *ChildTimer(void *vtest)
#endif
{
#ifndef WINDOWS
	test_ll_t *test = (test_ll_t *) vtest;
#endif
	time_t ioTimeoutCount = 0;
	time_t total_time = 0;
	OFF_T cur_total_io_count = 0;
	OFF_T last_total_io_count = 0;

	OFF_T tmp_io_count = 0;
	time_t run_time = 0;

	lvl_t msg_level = WARN;

	child_args_t *args = test->args;
	test_env_t *env = test->env;

	extern int signal_action;
	extern unsigned short glb_run;

#ifdef _DEBUG
	PDBG3(DBUG, args, "In timer %lu, %d\n", time(NULL), env->bContinue);
#endif
	do {
		Sleep(1000);
		run_time++;
#ifdef _DEBUG
		PDBG3(DBUG, args, "Continue timing %lu, %lu, %d\n", time(NULL),
		      run_time, env->bContinue);
#endif
		if (args->flags & CLD_FLG_W) {
			if ((args->flags & CLD_FLG_LINEAR)
			    && !(args->flags & CLD_FLG_NTRLVD)) {
				if (TST_OPER(args->test_state) == WRITER) {
					env->hbeat_stats.wtime++;
				}
			} else {
				env->hbeat_stats.wtime++;
			}
		}
		if (args->flags & CLD_FLG_R) {
			if ((args->flags & CLD_FLG_LINEAR)
			    && !(args->flags & CLD_FLG_NTRLVD)) {
				if (TST_OPER(args->test_state) == READER) {
					env->hbeat_stats.rtime++;
				}
			} else {
				env->hbeat_stats.rtime++;
			}
		}

		/*
		 * Check to see if we have made any IO progress in the last interval,
		 * if not incremment the ioTimeout timer, otherwise, clear it
		 */
		cur_total_io_count = env->global_stats.wcount
		    + env->cycle_stats.wcount
		    + env->hbeat_stats.wcount
		    + env->global_stats.rcount
		    + env->cycle_stats.rcount + env->hbeat_stats.rcount;

		if (cur_total_io_count == 0) {
			tmp_io_count = 1;
		} else {
			tmp_io_count = cur_total_io_count;
		}

		total_time = env->global_stats.rtime
		    + env->cycle_stats.rtime
		    + env->hbeat_stats.rtime
		    + env->global_stats.wtime
		    + env->cycle_stats.wtime + env->hbeat_stats.wtime;

#ifdef _DEBUG
		PDBG3(DBUG, args, "average number of seconds per IO: %0.8lf\n",
		      ((double)(total_time) / (double)(tmp_io_count)));
#endif

		if (cur_total_io_count == last_total_io_count) {	/* no IOs completed in interval */
			if (0 == (++ioTimeoutCount % args->ioTimeout)) {	/* no progress after modulo ioTimeout interval */
				if (args->flags & CLD_FLG_TMO_ERROR) {
					args->test_state =
					    SET_STS_FAIL(args->test_state);
					env->bContinue = FALSE;
					msg_level = ERR;
				}
				pMsg(msg_level, args,
				     "Possible IO hang condition, IO timeout reached, %lu seconds\n",
				     args->ioTimeout);
			}
#ifdef _DEBUG
			PDBG3(DBUG, args, "io timeout count: %lu\n",
			      ioTimeoutCount);
#endif
		} else {
			ioTimeoutCount = 0;
			last_total_io_count = cur_total_io_count;
#ifdef _DEBUG
			PDBG3(DBUG, args, "io timeout reset\n");
#endif
		}

		if (((args->hbeat > 0) && ((run_time % args->hbeat) == 0))
		    || (signal_action & SIGNAL_STAT)) {
			print_stats(args, env, HBEAT);
			update_cyc_stats(env);
			clear_stat_signal();
		}

		if (glb_run == 0) {
			break;
		}		/* global run flag cleared */
		if (signal_action & SIGNAL_STOP) {
			break;
		}
		/* user request to stop */
		if (args->flags & CLD_FLG_TMD) {	/* if timing */
			if (run_time >= args->run_time) {	/* and run time exceeded */
				break;
			}
		} else {	/* if not timing */
			if (env->kids <= 1) {	/* and the timer is the only child */
				break;
			}
		}
	} while (TRUE);
#ifdef _DEBUG
	PDBG3(DBUG, args, "Out of timer %lu, %lu, %d, %d\n", time(NULL),
	      run_time, env->bContinue, env->kids);
#endif

	if (args->flags & CLD_FLG_TMD) {	/* timed test, timer exit needs to stop io threads */
#ifdef _DEBUG
		PDBG3(DBUG, args,
		      "Setting bContinue to FALSE, timed test & timer exit\n");
#endif
		env->bContinue = FALSE;
	}

	TEXIT((uintptr_t) GETLASTERROR());
}

#ifdef _DEBUG
#ifdef WINDOWS
DWORD startTime;
DWORD endTime;

void setStartTime(void)
{
	startTime = GetTickCount();
}

void setEndTime(void)
{
	endTime = GetTickCount();
}

unsigned long getTimeDiff(void)
{
	return ((endTime - startTime) * 1000);	/* since we report in usecs, and windows is msec, multiply by 1000 */
}
#else
struct timeval tv_start;
struct timeval tv_end;

void setStartTime(void)
{
	gettimeofday(&tv_start, NULL);
}

void setEndTime(void)
{
	gettimeofday(&tv_end, NULL);
}

unsigned long getTimeDiff(void)
{
	return (tv_end.tv_usec - tv_start.tv_usec);
}

#endif
#endif
