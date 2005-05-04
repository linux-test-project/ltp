/*
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
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
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

/*
 * The main purpose of this thread is track tie
 * during the test. A test will start the timer
 * thread the thread will complete when the
 * current time time(NULL) >= global_end_time
 */
#ifdef WINDOWS
DWORD WINAPI ChildTimer(test_ll_t *test)
#else
void *ChildTimer(void *vtest)
#endif
{
#ifndef WINDOWS
	test_ll_t *test = (test_ll_t *)vtest;
#endif
	extern time_t global_end_time;		/* overall end time of test */
	time_t local_end_time = 0;
	int i;

	child_args_t *args = test->args;
	test_env_t *env = test->env;

	if((args->flags & CLD_FLG_TMD)) {
			local_end_time = global_end_time;
	} else { /* force the longest time */
			for(i=0;i<(sizeof(time_t)*8)-1;i++) {
					local_end_time |= 0x1ULL<<i;
			}
	}

	PDBG4(DEBUG, test->args, "In timer %lu < %lu, %d\n", time(NULL), local_end_time, env->bContinue);
	do {
		Sleep(1000);
		PDBG4(DEBUG, test->args, "Continue timing %lu < %lu, %d\n", time(NULL), local_end_time, env->bContinue);
		if(args->flags & CLD_FLG_W) {
			if((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
				if(TST_OPER(args->test_state) == WRITER) {
					env->cycle_stats.wtime++;
				}
			} else {
				env->cycle_stats.wtime++;
			}
		} 
		if(args->flags & CLD_FLG_R) {
			if((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
				if(TST_OPER(args->test_state) == READER) {
					env->cycle_stats.rtime++;
				}
			} else {
				env->cycle_stats.rtime++;
			}
		}
		if((args->hbeat > 0) && ((env->cycle_stats.wtime % args->hbeat) || (env->cycle_stats.rtime % args->hbeat)) == 0) {
			print_stats(args, env, CYCLE);
		}
		/* If test failed don't continue to report stats */
		if(!(TST_STS(args->test_state))) { break; }
	} while((time(NULL) < local_end_time) && (env->bContinue));
	PDBG4(DEBUG, test->args, "Out of timer %lu < %lu, %d\n", time(NULL), local_end_time, env->bContinue);
	env->bContinue = FALSE;
	TEXIT(GETLASTERROR());
}
