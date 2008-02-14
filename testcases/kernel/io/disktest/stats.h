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
* $Id: stats.h,v 1.2 2008/02/14 08:22:24 subrata_modak Exp $
*
*/

#ifndef _STATS_H
#define _STATS_H

#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "defs.h"

#ifdef WINDOWS
#define CTRSTR "%I64d;Rbytes;%I64d;Rxfers;"
#define CTWSTR "%I64d;Wbytes;%I64d;Wxfers;"
#define TCTRSTR "%I64d;TRbytes;%I64d;TRxfers;"
#define TCTWSTR "%I64d;TWbytes;%I64d;TWxfers;"
#define HRTSTR "%I64d bytes read in %I64d transfers during heartbeat.\n"
#define HWTSTR "%I64d bytes written in %I64d transfers during heartbeat.\n"
#define CRTSTR "%I64d bytes read in %I64d transfers during cycle.\n"
#define CWTSTR "%I64d bytes written in %I64d transfers during cycle.\n"
#define TRTSTR "Total bytes read in %I64d transfers: %I64d\n"
#define TWTSTR "Total bytes written in %I64d transfers: %I64d\n"
#else
#define CTRSTR "%lld;Rbytes;%lld;Rxfers;"
#define CTWSTR "%lld;Wbytes;%lld;Wxfers;"
#define TCTRSTR "%lld;TRbytes;%lld;TRxfers;"
#define TCTWSTR "%lld;TWbytes;%lld;TWxfers;"
#define HRTSTR "%lld bytes read in %lld transfers during heartbeat.\n"
#define HWTSTR "%lld bytes written in %lld transfers during heartbeat.\n"
#define CRTSTR "%lld bytes read in %lld transfers during cycle.\n"
#define CWTSTR "%lld bytes written in %lld transfers during cycle.\n"
#define TRTSTR "Total bytes read in %lld transfers: %lld\n"
#define TWTSTR "Total bytes written in %lld transfers: %lld\n"
#endif
#define HRTHSTR "Heartbeat read throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define HWTHSTR "Heartbeat write throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define CRTHSTR "Cycle read throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define CWTHSTR "Cycle write throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define TRTHSTR "Total read throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define TWTHSTR "Total write throughput: %.1fB/s (%.2fMB/s), IOPS %.1f/s.\n"
#define CTRRSTR "%.1f;RB/s;%.1f;RIOPS;"
#define CTRWSTR "%.1f;WB/s;%.1f;WIOPS;"
#define TCTRRSTR "%.1f;TRB/s;%.1f;TRIOPS;"
#define TCTRWSTR "%.1f;TWB/s;%.1f;TWIOPS;"

typedef enum statop {
	HBEAT,CYCLE,TOTAL
} statop_t;


void print_stats(child_args_t *, test_env_t *, statop_t);
void update_gbl_stats(test_env_t *);
void update_cyc_stats(test_env_t *);

#endif /* _STATS_H */
