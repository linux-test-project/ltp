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
* $Id: globals.c,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: globals.c,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "defs.h"
#include "globals.h"
#include "main.h"
#include "threading.h"

/* global flags */
#define GLB_FLG_QUIET	0x00000001
#define GLB_FLG_PERFP	0x00000002

/* Globals */
unsigned long glb_flags;	/* global flags GLB_FLG_xxx */
unsigned short kids;		/* global number of current child processes */
void *shared_mem;			/* global pointer to shared memory */
unsigned char *data_buffer;	/* global pointer to shared memory */
char *appname;				/* global pointer to application name string */
char *devname;				/* global pointer to device name */
size_t bmp_siz;				/* size of bitmask */
thread_struct_t *pThreads;	/* Global List of child processes */
size_t seed;				/* random seed */
BOOL bContinue;				/* global that when set to false will force exit of all threads */
OFF_T pass_count;			/* hit counters */

void init_gbl_data(char **argv)
{
	glb_flags = 0;
	kids = 0;
	shared_mem = NULL;
	data_buffer = NULL;
	appname = argv[0];
	devname = NULL;
	bmp_siz = 0;
	pThreads = NULL;
	seed = -1;
	bContinue = TRUE;
	pass_count = 1;
}

