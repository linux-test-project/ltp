/*
 *   Copyright (c) International Business Machines  Corp., 2001
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
 */
/*
 * FUNCTIONS: Scheduler Test Suite
 */

/*---------------------------------------------------------------------+
|                                sched.c                               |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the signal system function   |
|               calls:                                                 |
|                                                                      |
| Last update:   Ver. 1.2, 4/10/94 23:06:22                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     040294  DJK   Initial version for AIX 4.1                 |
|    0.2     010402  Manoj Iyer Ported to Linux			       |
|                                                                      |
+---------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "sched.h"

#if 0
extern FILE *logfile;

/*---------------------------------------------------------------------+
|                              openlog ()                              |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
int openlog(char *filename)
{

	if (filename == NULL)
		error("passed bad file name to openlog()", __FILE__, __LINE__);

	/*
	 * Open the log file...
	 */
	if ((logfile = fopen(filename, "a")) == (FILE *) NULL)
		sys_error("fopen failed", __FILE__, __LINE__);

	return (0);
}

/*---------------------------------------------------------------------+
|                               logmsg ()                              |
| ==================================================================== |
|                                                                      |
| Function:  ...                                                       |
|                                                                      |
+---------------------------------------------------------------------*/
void logmsg(const char *args, ...)
{
	fprintf(logfile, args);
	fflush(logfile);
}
#endif

/*---------------------------------------------------------------------+
|                             sys_error ()                             |
| ==================================================================== |
|                                                                      |
| Function:  Creates system error message and calls error ()           |
|                                                                      |
+---------------------------------------------------------------------*/
void sys_error(const char *msg, const char *file, int line)
{
	char syserr_msg[256];

	sprintf(syserr_msg, "%s: %s\n", msg, strerror(errno));
	error(syserr_msg, file, line);
}

/*---------------------------------------------------------------------+
|                               error ()                               |
| ==================================================================== |
|                                                                      |
| Function:  Prints out message and exits...                           |
|                                                                      |
+---------------------------------------------------------------------*/
void error(const char *msg, const char *file, int line)
{
	fprintf(stderr, "ERROR [file: %s, line: %d] %s\n", file, line, msg);
	exit(-1);
}
