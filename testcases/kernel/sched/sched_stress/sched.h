/*
 *
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* @BULL_COPYRIGHT@ */
/*@(#)63  1.1  testcase/bpsc_str/sched.h, pvt_kernel, pv_kernel 12/11/96 16:43:19*/
/*
 *
 * FUNCTIONS: Scheduler Test Suite
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*---------------------------------------------------------------------+
|                                sched.h                               |
| ==================================================================== |
|                                                                      |
| Description:  Simplistic test to verify the signal system function   |
|               calls:                                                 |
|                                                                      |
| Last update:   Ver. 1.2, 4/10/94 23:05:59                           |
|                                                                      |
| Change Activity                                                      |
|                                                                      |
|   Version  Date    Name  Reason                                      |
|    0.1     040294  DJK   Initial version for AIX 4.1                 |
|    0.2     010402  Manoj Iyer Ported to Linux                        |
|                                                                      |
+---------------------------------------------------------------------*/

#ifndef _H_COMMON
#define _H_COMMON

#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

#define DEFAULT_PRIORITY 70

#if 0
int  openlog (char *);
void logmsg (const char *, ...);
#endif
void sys_error (const char *, const char *, int);
void error (const char *, const char *, int);

#endif /* _H_COMMON */
