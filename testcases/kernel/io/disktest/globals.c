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
* $Id: globals.c,v 1.6 2009/02/26 12:02:22 subrata_modak Exp $
*
*/

#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#endif
#include <time.h>
#include <string.h>

#include "defs.h"
#include "globals.h"
#include "main.h"
#include "threading.h"
#include "sfunc.h"

/* Globals */
unsigned int gbl_dbg_lvl;	/* the global debugging level   */
unsigned long glb_flags;	/* global flags GLB_FLG_xxx */
time_t global_start_time;	/* global start time */
unsigned short glb_run = 1;	/* global run flag */

void init_gbl_data(test_env_t * env)
{
	env->kids = 0;
	env->shared_mem = NULL;
	env->data_buffer = NULL;
	env->bmp_siz = 0;
	env->pThreads = NULL;
	env->bContinue = TRUE;
	env->pass_count = 0;
	env->start_time = time(NULL);	/*      overall start time of test      */
	env->end_time = 0;	/*      overall end time of test        */
	memset(&env->global_stats, 0, sizeof(stats_t));
	memset(&env->cycle_stats, 0, sizeof(stats_t));
}

#ifdef WINDOWS
/*
void PrintLastSystemError(unsigned long ulErrorNum)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
		ulErrorNum,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
	    0,
		NULL
	);
	pMsg(INFO,"%s",lpMsgBuf);
	LocalFree(lpMsgBuf);
}
*/

void GetSystemErrorString(unsigned long ulErrorNum, void *buffer)
{
	/* Use Default language */
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		      FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      ulErrorNum,
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) & buffer, 0, NULL);
}
#endif
