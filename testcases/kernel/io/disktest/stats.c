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
* $Id: stats.c,v 1.2 2008/02/14 08:22:24 subrata_modak Exp $
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
#include "sfunc.h"
#include "threading.h"
#include "stats.h"

void print_stats(child_args_t * args, test_env_t * env, statop_t operation)
{
	extern time_t global_start_time;	/* global pointer to overall start */
	extern unsigned long glb_flags;	/* global flags GLB_FLG_xxx */

	time_t curr_time = 0, hwrite_time = 0, hread_time = 0, write_time =
	    0, read_time = 0, gw_time = 0, gr_time = 0;
	fmt_time_t time_struct;

	curr_time = time(NULL);

	if ((curr_time - env->start_time) == 0)
		curr_time++;

	if ((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
		hread_time = env->hbeat_stats.rtime;
		hwrite_time = env->hbeat_stats.wtime;
		read_time = env->cycle_stats.rtime;
		write_time = env->cycle_stats.wtime;
		gr_time = env->global_stats.rtime;
		gw_time = env->global_stats.wtime;
	} else {
		hread_time = ((env->hbeat_stats.rtime * args->rperc) / 100);
		hwrite_time = ((env->hbeat_stats.wtime * args->wperc) / 100);
		read_time = ((env->cycle_stats.rtime * args->rperc) / 100);
		write_time = ((env->cycle_stats.wtime * args->wperc) / 100);
		gr_time =
		    (time_t) ((env->global_stats.rtime * args->rperc) / 100);
		gw_time =
		    (time_t) ((env->global_stats.wtime * args->wperc) / 100);
	}

	/* if one second really has not passed, then make it at least one second */
	if (hread_time == 0)
		hread_time++;
	if (hwrite_time == 0)
		hwrite_time++;
	if (read_time == 0)
		read_time++;
	if (write_time == 0)
		write_time++;
	if (gr_time == 0)
		gr_time++;
	if (gw_time == 0)
		gw_time++;

	if (glb_flags & GLB_FLG_PERFP) {
		if (args->flags & CLD_FLG_PRFTYPS) {
			printf("%s;", args->device);
		}
		switch (operation) {
		case HBEAT:	/* only display current HBEAT stats */
			if ((args->flags & CLD_FLG_XFERS)) {
				printf(CTRSTR, (env->hbeat_stats.rbytes),
				       (env->hbeat_stats.rcount));
				printf(CTWSTR, (env->hbeat_stats.wbytes),
				       (env->hbeat_stats.wcount));
			}
			if ((args->flags & CLD_FLG_TPUTS)) {
				printf(CTRRSTR,
				       ((double)(env->hbeat_stats.rbytes) /
					(double)(hread_time)),
				       ((double)(env->hbeat_stats.rcount) /
					(double)(hread_time)));
				printf(CTRWSTR,
				       ((double)(env->hbeat_stats.wbytes) /
					(double)(hwrite_time)),
				       ((double)(env->hbeat_stats.wcount) /
					(double)(hwrite_time)));
			}
			if ((args->flags & CLD_FLG_RUNT)) {
				printf("%lu;Rsecs;%lu;Wsecs;", hread_time,
				       hwrite_time);
			}
			break;
		case CYCLE:	/* only display current CYCLE stats */
			if ((args->flags & CLD_FLG_XFERS)) {
				printf(CTRSTR, (env->cycle_stats.rbytes),
				       (env->cycle_stats.rcount));
				printf(CTWSTR, (env->cycle_stats.wbytes),
				       (env->cycle_stats.wcount));
			}
			if ((args->flags & CLD_FLG_TPUTS)) {
				printf(CTRRSTR,
				       ((double)(env->cycle_stats.rbytes) /
					(double)(read_time)),
				       ((double)(env->cycle_stats.rcount) /
					(double)(read_time)));
				printf(CTRWSTR,
				       ((double)(env->cycle_stats.wbytes) /
					(double)(write_time)),
				       ((double)(env->cycle_stats.wcount) /
					(double)(write_time)));
			}
			if ((args->flags & CLD_FLG_RUNT)) {
				printf("%lu;Rsecs;%lu;Wsecs;", read_time,
				       write_time);
			}
			break;
		case TOTAL:	/* display total read and write stats */
			if ((args->flags & CLD_FLG_XFERS)) {
				printf(TCTRSTR, (env->global_stats.rbytes),
				       (env->global_stats.rcount));
				printf(TCTWSTR, (env->global_stats.wbytes),
				       (env->global_stats.wcount));
			}
			if ((args->flags & CLD_FLG_TPUTS)) {
				printf(TCTRRSTR,
				       ((double)(env->global_stats.rbytes) /
					(double)(gr_time)),
				       ((double)(env->global_stats.rcount) /
					(double)(gr_time)));
				printf(TCTRWSTR,
				       ((double)(env->global_stats.wbytes) /
					(double)(gw_time)),
				       ((double)(env->global_stats.wcount) /
					(double)(gw_time)));
			}
			if ((args->flags & CLD_FLG_RUNT)) {
				printf("%lu;secs;",
				       (curr_time - env->start_time));
			}
			break;
		default:
			pMsg(ERR, args, "Unknown stats display type.\n");
		}

		if (args->flags & CLD_FLG_PRFTYPS) {
			printf("\n");
		}
	} else {
		if ((args->flags & CLD_FLG_XFERS)) {
			switch (operation) {
			case HBEAT:	/* only display current HBEAT stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, HRTSTR,
					     (env->hbeat_stats.rbytes),
					     (env->hbeat_stats.rcount));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, HWTSTR,
					     (env->hbeat_stats.wbytes),
					     (env->hbeat_stats.wcount));
				}
				break;
			case CYCLE:	/* only display current CYCLE stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, CRTSTR,
					     (env->cycle_stats.rbytes),
					     (env->cycle_stats.rcount));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, CWTSTR,
					     (env->cycle_stats.wbytes),
					     (env->cycle_stats.wcount));
				}
				break;
			case TOTAL:	/* display total read and write stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, TRTSTR,
					     (env->global_stats.rcount),
					     (env->global_stats.rbytes));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, TWTSTR,
					     (env->global_stats.wcount),
					     (env->global_stats.wbytes));
				}
				break;
			default:
				pMsg(ERR, args,
				     "Unknown stats display type.\n");
			}
		}

		if ((args->flags & CLD_FLG_TPUTS)) {
			switch (operation) {
			case HBEAT:	/* only display current read stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, HRTHSTR,
					     ((double)env->hbeat_stats.rbytes /
					      (double)(hread_time)),
					     (((double)env->hbeat_stats.rbytes /
					       (double)hread_time) /
					      (double)1048576.),
					     ((double)env->hbeat_stats.rcount /
					      (double)(hread_time)));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, HWTHSTR,
					     ((double)env->hbeat_stats.wbytes /
					      (double)hwrite_time),
					     (((double)env->hbeat_stats.wbytes /
					       (double)hwrite_time) /
					      (double)1048576.),
					     ((double)env->hbeat_stats.wcount /
					      (double)hwrite_time));
				}
				break;
			case CYCLE:	/* only display current read stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, CRTHSTR,
					     ((double)env->cycle_stats.rbytes /
					      (double)(read_time)),
					     (((double)env->cycle_stats.rbytes /
					       (double)read_time) /
					      (double)1048576.),
					     ((double)env->cycle_stats.rcount /
					      (double)(read_time)));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, CWTHSTR,
					     ((double)env->cycle_stats.wbytes /
					      (double)write_time),
					     (((double)env->cycle_stats.wbytes /
					       (double)write_time) /
					      (double)1048576.),
					     ((double)env->cycle_stats.wcount /
					      (double)write_time));
				}
				break;
			case TOTAL:	/* display total read and write stats */
				if (args->flags & CLD_FLG_R) {
					pMsg(STAT, args, TRTHSTR,
					     ((double)env->global_stats.rbytes /
					      (double)gr_time),
					     (((double)env->global_stats.
					       rbytes / (double)gr_time) /
					      (double)1048576.),
					     ((double)env->global_stats.rcount /
					      (double)gr_time));
				}
				if (args->flags & CLD_FLG_W) {
					pMsg(STAT, args, TWTHSTR,
					     ((double)env->global_stats.wbytes /
					      (double)gw_time),
					     (((double)env->global_stats.
					       wbytes / (double)gw_time) /
					      (double)1048576.),
					     ((double)env->global_stats.wcount /
					      (double)gw_time));
				}
				break;
			default:
				pMsg(ERR, args,
				     "Unknown stats display type.\n");
			}
		}
		if (args->flags & CLD_FLG_RUNT) {
			switch (operation) {
			case HBEAT:	/* only display current cycle stats */
				if (args->flags & CLD_FLG_R) {
					time_struct = format_time(hread_time);
					pMsg(STAT, args,
					     "Heartbeat Read Time: %u seconds (%luh%lum%lus)\n",
					     hread_time, time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				if (args->flags & CLD_FLG_W) {
					time_struct = format_time(hwrite_time);
					pMsg(STAT, args,
					     "Heartbeat Write Time: %u seconds (%luh%lum%lus)\n",
					     hwrite_time, time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				break;
			case CYCLE:	/* only display current cycle stats */
				if (args->flags & CLD_FLG_R) {
					time_struct = format_time(read_time);
					pMsg(STAT, args,
					     "Cycle Read Time: %u seconds (%luh%lum%lus)\n",
					     read_time, time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				if (args->flags & CLD_FLG_W) {
					time_struct = format_time(write_time);
					pMsg(STAT, args,
					     "Cycle Write Time: %u seconds (%luh%lum%lus)\n",
					     write_time, time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				break;
			case TOTAL:
				if (args->flags & CLD_FLG_R) {
					time_struct = format_time(gr_time);
					pMsg(STAT, args,
					     "Total Read Time: %u seconds (%lud%luh%lum%lus)\n",
					     gr_time, time_struct.days,
					     time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				if (args->flags & CLD_FLG_W) {
					time_struct = format_time(gw_time);
					pMsg(STAT, args,
					     "Total Write Time: %u seconds (%lud%luh%lum%lus)\n",
					     gw_time, time_struct.days,
					     time_struct.hours,
					     time_struct.minutes,
					     time_struct.seconds);
				}
				time_struct =
				    format_time((curr_time -
						 global_start_time));
				pMsg(STAT, args,
				     "Total overall runtime: %u seconds (%lud%luh%lum%lus)\n",
				     (curr_time - global_start_time),
				     time_struct.days, time_struct.hours,
				     time_struct.minutes, time_struct.seconds);
				break;
			default:
				pMsg(ERR, args,
				     "Unknown stats display type.\n");
			}
		}
	}
}

void update_gbl_stats(test_env_t * env)
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

void update_cyc_stats(test_env_t * env)
{
	env->cycle_stats.wcount += env->hbeat_stats.wcount;
	env->cycle_stats.rcount += env->hbeat_stats.rcount;
	env->cycle_stats.wbytes += env->hbeat_stats.wbytes;
	env->cycle_stats.rbytes += env->hbeat_stats.rbytes;
	env->cycle_stats.wtime += env->hbeat_stats.wtime;
	env->cycle_stats.rtime += env->hbeat_stats.rtime;

	env->hbeat_stats.wcount = 0;
	env->hbeat_stats.rcount = 0;
	env->hbeat_stats.wbytes = 0;
	env->hbeat_stats.rbytes = 0;
	env->hbeat_stats.wtime = 0;
	env->hbeat_stats.rtime = 0;
}
