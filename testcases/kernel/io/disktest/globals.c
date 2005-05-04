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
* $Id: globals.c,v 1.4 2005/05/04 17:54:00 mridge Exp $
* $Log: globals.c,v $
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.9  2004/12/18 06:13:03  yardleyb
* Updated timer schema to more accurately use the time options.  Added
* fsync on write option to -If.
*
* Revision 1.8  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.7  2002/03/30 01:32:14  yardleyb
* Major Changes:
*
* Added Dumping routines for
* data miscompares,
*
* Updated performance output
* based on command line.  Gave
* one decimal in MB/s output.
*
* Rewrote -pL IO routine to show
* correct stats.  Now show pass count
* when using -C.
*
* Minor Changes:
*
* Code cleanup to remove the plethera
* if #ifdef for windows/unix functional
* differences.
*
* Revision 1.6  2002/03/07 03:32:13  yardleyb
* Removed the use of global
* appname.  Set devname to
* init. value of "No Device"
*
* Revision 1.5  2002/02/28 02:05:36  yardleyb
* added global varibale start_time.
*
* Revision 1.4  2002/02/20 18:38:15  yardleyb
* Removed headers for ipc,
* sem and shared memory
* that are no longer used
*
* Revision 1.3  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.2  2001/12/07 23:33:29  yardleyb
* Fixed bug where a false positive data
* miscompare could occur when running
* multi cycle testing with mark block
* enabled.
*
* Revision 1.1  2001/12/04 18:51:05  yardleyb
* Checkin of new source files and removal
* of outdated source
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
time_t global_start_time;	/*	overall start time of test	*/
time_t global_end_time;		/*	overall end time of test	*/
unsigned int gbl_dbg_lvl;	/*	the global debugging level	*/
unsigned long glb_flags;    /*	global flags GLB_FLG_xxx	*/

void init_gbl_data(test_env_t *env)
{
	env->kids = 0;
	env->shared_mem = NULL;
	env->data_buffer = NULL;
	env->bmp_siz = 0;
	env->pThreads = NULL;
	env->bContinue = TRUE;
	env->pass_count = 0;
	memset(&env->global_stats, 0, sizeof(stats_t));
	memset(&env->cycle_stats, 0, sizeof(stats_t));
}

void update_gbl_stats(test_env_t *env)
{
	env->global_stats.wcount += env->cycle_stats.wcount;
	env->global_stats.rcount += env->cycle_stats.rcount;
	env->global_stats.wbytes += env->cycle_stats.wbytes;
	env->global_stats.rbytes += env->cycle_stats.rbytes;
	env->global_stats.wtime += env->cycle_stats.wtime;
	env->global_stats.rtime += env->cycle_stats.rtime;

	env->cycle_stats.wcount = 0;
	env->cycle_stats.rcount = 0;
	env->cycle_stats.wbytes = 0;
	env->cycle_stats.rbytes = 0;
	env->cycle_stats.wtime = 0;
	env->cycle_stats.rtime = 0;
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
	FormatMessage( 
	    FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
		ulErrorNum,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &buffer,
	    0,
		NULL 
	);
}
#endif
