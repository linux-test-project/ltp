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
* TODO:
* -- Header on first lba should include fclun, pattern, LBA, access count
* -- Add the following options:
*	butterfly: seek option: test will seek lba start/end/start+1/end-1/.../end-1/start+1/end/start
*	ping-pong: seek option: test will seek lba start/end/start/end/etc...
*	min seek: force a minimum seek disktance during any IO access
*	max seek: force a maximum seek distance during any IO access
*	WORO: all blocks will be written and read only once
*	WORM: all blocks will be written only once, but read many times
*	WRWR: a block will be written then read then written then read
*	count: number of times an lba is 'hit' with a read or a write
*	serialize: only one I/O request is ever outstanding no mater how many children
*	retry: number of times an I/O should be retried before counting as a failure
*	non-distructive: will read lba/write lba with XOR/bit inverted read data/then read lba to verify
*
*
* $Id: main.c,v 1.2 2003/04/17 15:21:57 robbiew Exp $
* $Log: main.c,v $
* Revision 1.2  2003/04/17 15:21:57  robbiew
* Updated to v1.1.10
*
* Revision 1.14  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.13  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.12  2002/04/03 20:16:07  yardleyb
* Added check for case when
* fixed pattern is 0
*
* Revision 1.11  2002/03/30 01:32:14  yardleyb
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
* Revision 1.10  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
* Revision 1.9  2002/02/28 04:25:45  yardleyb
* reworked threading code
* made locking code a macro.
*
* Revision 1.8  2002/02/26 19:35:59  yardleyb
* Updates to parsing routines for user
* input.  Added multipliers for -S and
* -s command line arguments. Forced
* default seeks to default if performing
* a diskcache test.
*
* Revision 1.7  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
*
* Revision 1.6  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.5  2002/01/31 20:19:05  yardleyb
* Updated print_stat time output
* to reflect 64bit printf formating
*
* Revision 1.4  2002/01/31 20:12:21  yardleyb
* Updated the performance counters
* to reflect run time based on duty
* cycle not on total.
*
* Revision 1.3  2001/12/07 23:33:29  yardleyb
* Fixed bug where a false positive data
* miscompare could occur when running
* multi cycle testing with mark block
* enabled.
*
* Revision 1.2  2001/12/04 21:45:25  yardleyb
* modified hours calculation
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
* Revision 1.12  2001/10/10 00:17:14  yardleyb
* Added Copyright and GPL license text.
* Miner bug fixes throughout text.
*
* Revision 1.11  2001/10/01 23:17:20  yardleyb
* Added performance option P for disk coverage.
* Minor code updates to get_next_lba.
*
* Revision 1.10  2001/09/26 23:37:50  yardleyb
* Added -v option to display version.
* seeks are now calculated based on start and stop block.
* fixed bug with linear seeking and multipule children.
* block size can use 'k' to mean *1024.
*
* Revision 1.9  2001/09/22 03:42:56  yardleyb
* Anyother major update.  fixed bugs in IO routines,
* added more error checking on input.  Added heartbeat
* option, up-and-up and up-down-up seek routines, pass/fail
* output, pMsg level discription, veriable length data comare
* checking.  Lots of lint cleanup.
*
* Revision 1.8  2001/09/07 02:13:31  yardleyb
* Major rewrite of main IO function.  Fixed bug in duty cycle were percentages
* were off by one.  Got rid of some sloppy memory usage.  Major performance
* increase overall.
*
* Revision 1.7  2001/09/06 21:59:23  yardleyb
* Fixed a bug in the -L/-K where it ther ewere to many childern be created.
* also more code cleanup.  Changed 'loops' to 'seeks' throughout.
*
* Revision 1.6  2001/09/06 18:23:30  yardleyb
* Added duty cycle -D.  Updated usage. Added
* make option to create .tar.gz of all files
*
* Revision 1.5  2001/09/05 22:44:42  yardleyb
* Split out some of the special functions.
* added O_DIRECT -Id.  Updated usage.  Lots
* of clean up to functions.  Added header info
* to pMsg.
*
* Revision 1.4  2001/09/04 19:28:07  yardleyb
* Split usage out. Split header out.  Added usage text.
* Made signal handler one function. code cleanup.
*
* Revision 1.3  2001/08/25 02:39:24  yardleyb
* Added support for interleaved reads & writes, -pi
* created pMsg function, which serializes output to
* stdout.
*
* Revision 1.2  2001/08/24 23:52:31  yardleyb
* Multipule changes to file.  fixed issue with file discriptor.
* Fixed core dump when noargs given.  lba is now passed to fill_buff
* as a ulong.
*
* Revision 1.1  2001/08/24 23:38:59  yardleyb
* Inital checkin.  First distro.
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
#include "main.h"
#include "usage.h"
#include "sfunc.h"
#include "parse.h"
#include "childmain.h"
#include "threading.h"
#include "dump.h"

void print_stats(child_args_t *args, op_t operation)
{
	extern unsigned long glb_flags;	/* global flags GLB_FLG_xxx */
	extern time_t global_start_time;/* global pointer to overall start */
	extern stats_t cycle_stats;		/* per cycle statistics */
	extern stats_t global_stats;	/* per cycle statistics */

	time_t curr_time = 0, write_time = 0, read_time = 0, gw_time = 0, gr_time = 0;
	fmt_time_t time_struct;

	curr_time = time(NULL);

	if((curr_time - global_start_time) == 0) curr_time++;

	if((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
		read_time = cycle_stats.rtime;
		write_time = cycle_stats.wtime;
		gr_time = global_stats.rtime;
		gw_time = global_stats.wtime;
	} else {
		read_time = ((cycle_stats.rtime * args->rperc) / 100);
		write_time = ((cycle_stats.wtime * args->wperc) / 100);
		gr_time = (time_t) ((global_stats.rtime * args->rperc) / 100);
		gw_time = (time_t) ((global_stats.wtime * args->wperc) / 100);
	}

	/* if one second really has not passed, then make it at least one second */
	if(read_time == 0) read_time++;
	if(write_time == 0) write_time++;
	if(gr_time == 0) gr_time++;
	if(gw_time == 0) gw_time++;

	if(glb_flags & GLB_FLG_PERFP) {
		printf("%s;", args->device);
		if(operation == ALL) {
			if((args->flags & CLD_FLG_XFERS)) {
				printf(TCTRSTR, (global_stats.rbytes), (global_stats.rcount));
				printf(TCTWSTR, (global_stats.wbytes), (global_stats.wcount));
			}
			if((args->flags & CLD_FLG_TPUTS)) {
				printf(TCTRRSTR, (double) ((global_stats.rbytes) / (read_time)), (double) ((global_stats.rcount) / (read_time)));
				printf(TCTRWSTR, (double) ((global_stats.wbytes) / (write_time)), (double) ((global_stats.wcount) / (write_time)));
			}
			if((args->flags & CLD_FLG_RUNT)) {
				printf("%lu;secs;",(curr_time - global_start_time));
			}
			printf("\n");
		} else {
			if((args->flags & CLD_FLG_XFERS)) {
				printf(CTRSTR, (cycle_stats.rbytes), (cycle_stats.rcount));
				printf(CTWSTR, (cycle_stats.wbytes), (cycle_stats.wcount));
			}
			if((args->flags & CLD_FLG_TPUTS)) {
				printf(CTRRSTR, (double) ((cycle_stats.rbytes) / (read_time)), (double) ((cycle_stats.rcount) / (read_time)));
				printf(CTRWSTR, (double) ((cycle_stats.wbytes) / (write_time)), (double) ((cycle_stats.wcount) / (write_time)));
			}
			if((args->flags & CLD_FLG_RUNT)) {
				printf("%lu;Rsecs;%lu;Wsecs;",read_time, write_time);
			}
			printf("\n");
		}
	} else {
		if((args->flags & CLD_FLG_XFERS)) {
			switch(operation) {
				case READER: /* only display current read stats */
					PDBG1(STAT, RTSTR, (cycle_stats.rbytes), (cycle_stats.rcount));
					break;
				case WRITER:  /* only display current write stats */
					PDBG1(STAT, WTSTR, (cycle_stats.wbytes), (cycle_stats.wcount));
					break;
				case ALL: /* display total read and write stats */
					if(args->flags & CLD_FLG_R) {
						pMsg(STAT, TRTSTR, (global_stats.rcount), (global_stats.rbytes));
					}
					if(args->flags & CLD_FLG_W) {
						pMsg(STAT, TWTSTR, (global_stats.wcount), (global_stats.wbytes));
					}
					break;
				default:
					pMsg(ERR, "Unknown stats display type.\n");
			}
		}

		if((args->flags & CLD_FLG_TPUTS)) {
			switch(operation) {
				case READER: /* only display current read stats */
					PDBG1(STAT, RTHSTR,
						((double) cycle_stats.rbytes / (double) (read_time)),
						(((double) cycle_stats.rbytes / (double) read_time) / (double) 1048576.),
						((double) cycle_stats.rcount / (double) (read_time)));
					break;
				case WRITER:  /* only display current write stats */
					PDBG1(STAT, WTHSTR,
						((double) cycle_stats.wbytes / (double) write_time),
						(((double) cycle_stats.wbytes / (double) write_time) / (double) 1048576.),
						((double) cycle_stats.wcount / (double) write_time));
					break;
				case ALL: /* display total read and write stats */
					if(args->flags & CLD_FLG_R) {
						pMsg(STAT, TRTHSTR,
							((double) global_stats.rbytes / (double) gr_time),
							(((double) global_stats.rbytes / (double) gr_time) / (double) 1048576.),
							((double) global_stats.rcount / (double) gr_time));
					}
					if(args->flags & CLD_FLG_W) {
						pMsg(STAT, TWTHSTR,
							((double) global_stats.wbytes / (double) gw_time),
							(((double) global_stats.wbytes / (double) gw_time) / (double) 1048576.),
							((double) global_stats.wcount / (double) gw_time));
					}
					break;
				default:
					pMsg(ERR, "Unknown stats display type.\n");
			}
		}
		if(args->flags & CLD_FLG_RUNT) {
			switch(operation) {
				case READER: /* only display current read stats */
					time_struct = format_time(read_time);
					PDBG1(STAT,"Read Time: %u seconds (%luh%lum%lus)\n", read_time, time_struct.hours, time_struct.minutes, time_struct.seconds);
					break;
				case WRITER:
					time_struct = format_time(write_time);
					PDBG1(STAT,"Write Time: %u seconds (%luh%lum%lus)\n", write_time, time_struct.hours, time_struct.minutes, time_struct.seconds);
					break;
				case ALL:
					if(args->flags & CLD_FLG_R) {
						time_struct = format_time(gr_time);
						pMsg(STAT,"Total Read Time: %u seconds (%luh%lum%lus)\n", gr_time, time_struct.hours, time_struct.minutes, time_struct.seconds);
					}
					if(args->flags & CLD_FLG_W) {
						time_struct = format_time(gw_time);
						pMsg(STAT,"Total Write Time: %u seconds (%luh%lum%lus)\n", gw_time, time_struct.hours, time_struct.minutes, time_struct.seconds);
					}
					time_struct = format_time((curr_time - global_start_time));
					pMsg(STAT,"Total overall runtime: %u seconds (%luh%lum%lus)\n", (curr_time - global_start_time), time_struct.hours, time_struct.minutes, time_struct.seconds);
					break;
				default:
					pMsg(ERR, "Unknown stats display type.\n");
			}
		}
	}
}

#ifdef WINDOWS
DWORD WINAPI ChildPS(child_args_t *args)
#else
void *ChildPS(void *vargs)
#endif
{
	extern BOOL bContinue;				/* global that when set to false will force exit of all threads */
	extern unsigned int gbl_dbg_lvl;	/* global flags GLB_FLG_xxx */
	extern stats_t cycle_stats;

	unsigned long exit_code = 0;
	time_t seconds = 0;

#ifndef WINDOWS
	child_args_t *args = (child_args_t *) vargs;
#endif

	if(gbl_dbg_lvl < 1) gbl_dbg_lvl++;

	do {
		Sleep(1000);
		seconds++;
		if(seconds == args->hbeat) {
			if(args->flags & CLD_FLG_W) {
				if((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
					if(TST_OPER(args->test_state) == WRITER) {
						cycle_stats.wtime += (time_t) args->hbeat;
						print_stats(args, WRITER);
					}
				} else {
					cycle_stats.wtime += (time_t) args->hbeat;
					print_stats(args, WRITER);
				}
			} 
			if(args->flags & CLD_FLG_R) {
				if((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
					if(TST_OPER(args->test_state) == READER) {
						cycle_stats.rtime += (time_t) args->hbeat;
						print_stats(args, READER);
					}
				} else {
					cycle_stats.rtime += (time_t) args->hbeat;
					print_stats(args, READER);
				}
			}
			seconds = 0;
		}
	} while(bContinue);
	TEXIT(exit_code);
}

void linear_read_write_test(child_args_t *args)
{
	extern void *shared_mem;
	extern OFF_T pass_count;
	extern stats_t cycle_stats;
	extern BOOL bContinue;				/* global that when set to false will force exit of all threads */

	OFF_T *pVal1 = (OFF_T *)shared_mem;
	time_t start_time;
	int i;

	if(args->flags & CLD_FLG_W) {
		bContinue = TRUE;
		*(pVal1 + OFF_WLBA) = args->start_lba;
		args->test_state = DIRCT_INC(args->test_state);
		args->test_state = SET_wFST_TIME(args->test_state);
		srand(args->seed);	/* reseed so we can re create the same random transfers */
		if(args->flags & CLD_FLG_CYC)
			pMsg(INFO,"Starting write pass %lu of %lu\n", (unsigned long) pass_count, args->cycles);
		else
			pMsg(INFO,"Starting write pass\n");
		args->test_state = SET_OPER_W(args->test_state);
		start_time = time(NULL);
		/*
		 * If we are using a heartbeat to print stats
		 * create a thread to do it for us.
		 */
		if(args->hbeat > 0) CreateChild(ChildPS, args);

		for(i=0;i<args->t_kids;i++) {
			CreateChild(ChildMain, args);
		}
		/* Wait for the writers to finish */
		clean_up();

		cycle_stats.wtime = time(NULL) - start_time;
		if(args->hbeat == 0) print_stats(args, WRITER);
	}

	if(args->flags & CLD_FLG_R) {
		bContinue = TRUE;
		*(pVal1 + OFF_RLBA) = args->start_lba;
		args->test_state = DIRCT_INC(args->test_state);
		args->test_state = SET_rFST_TIME(args->test_state);
		srand(args->seed);	/* reseed so we can re create the same random transfers */
		if(args->flags & CLD_FLG_CYC)
			pMsg(INFO,"Starting read pass %lu of %lu\n", (unsigned long) pass_count, args->cycles);
		else
			pMsg(INFO,"Starting read pass\n");
		args->test_state = SET_OPER_R(args->test_state);
		start_time = time(NULL);
		/*
		 * If we are using a heartbeat to print stats
		 * create a thread to do it for us.
		 */
		if(args->hbeat > 0) CreateChild(ChildPS, args);

		for(i=0;i<args->t_kids;i++) {
			CreateChild(ChildMain, args);
		}
		/* Wait for the writers to finish */
		clean_up();

		cycle_stats.rtime = time(NULL) - start_time;
		if(args->hbeat == 0) print_stats(args, READER);
	}
}

unsigned long init_data(child_args_t *args, unsigned char **data_buffer_unaligned)
{
	extern void *shared_mem;			/* global pointer to shared memory */
	extern unsigned char *data_buffer;	/* global pointer to shared memory */
	extern size_t bmp_siz;				/* size of bitmask */
	extern time_t global_start_time;	/* overall start time of test */
	extern time_t global_end_time;		/* overall end time of test */

	int i;
	OFF_T *pVal1;
	pid_t my_pid;

	unsigned long data_buffer_size;

	my_pid = GETPID();
	if(args->seed == 0) args->seed = my_pid;
	srand(args->seed);

	/* create bitmap to hold write/read context: each bit is an LBA */
	/* the stuff before BMP_OFFSET is the data for child/thread shared context */
	bmp_siz = (((((size_t)args->vsiz))/8) == 0) ? 1 : ((((size_t)args->vsiz))/8);
	if ((args->vsiz/8) != 0) bmp_siz += 1;	/* account for rounding error */

	/* We use that same data buffer for static data, so alloc here. */
	data_buffer_size = ((args->htrsiz*BLK_SIZE)*2);
	if((*data_buffer_unaligned = (unsigned char *) ALLOC(data_buffer_size+ALIGNSIZE)) == NULL) {
		pMsg(ERR, "Failed to allocate static data buffer memory.\n");
		return(-1);
	}
	data_buffer = (unsigned char *) BUFALIGN(*data_buffer_unaligned);

	if((shared_mem = (void *) ALLOC(bmp_siz+BMP_OFFSET)) == NULL) {
		pMsg(ERR, "Failed to allocate bitmap memory\n");
		return(-1);
	}
#ifdef WINDOWS
	if(CreateMutex(NULL, FALSE, "gbl") == NULL) {
		pMsg(ERR, "Failed to create semaphore, error = %u\n", GetLastError());
		return(GetLastError());
	}
	if(CreateMutex(NULL, FALSE, "data") == NULL) {
		pMsg(ERR, "Failed to create semaphore, error = %u\n", GetLastError());
		return(GetLastError());
	}
#endif

	memset((char *)shared_mem,0,bmp_siz+BMP_OFFSET);
	memset((unsigned char *)data_buffer,0,data_buffer_size);

	pVal1 = (OFF_T *)shared_mem;
	*(pVal1 + OFF_WLBA) = args->start_lba;
	*(pVal1 + OFF_RLBA) = args->start_lba;
	args->test_state = SET_STS_PASS(args->test_state);
	args->test_state = SET_wFST_TIME(args->test_state);
	args->test_state = SET_rFST_TIME(args->test_state);
	args->test_state = DIRCT_INC(args->test_state);
	if(args->flags & CLD_FLG_W)
		args->test_state = SET_OPER_W(args->test_state);
	else
		args->test_state = SET_OPER_R(args->test_state);

	/* prefill the data buffer with data for compares and writes */
	switch(args->flags & CLD_FLG_PTYPS) {
		case CLD_FLG_FPTYPE :
			for(i=0;i<sizeof(args->pattern);i++) {
				if((args->pattern & (((OFF_T) 0xff) << (((sizeof(args->pattern)-1)-i)*8))) != 0) break;
			}
			/* special case for pattern = 0 */
			if(i == sizeof(args->pattern)) i = 0;
			fill_buffer(data_buffer, data_buffer_size, &args->pattern, sizeof(args->pattern)-i, CLD_FLG_FPTYPE);
			break;
		case CLD_FLG_RPTYPE :
			fill_buffer(data_buffer, data_buffer_size, NULL, 0, CLD_FLG_RPTYPE);
			break;
		case CLD_FLG_CPTYPE :
			fill_buffer(data_buffer, data_buffer_size, 0, 0, CLD_FLG_CPTYPE);
		case CLD_FLG_LPTYPE :
			break;
		default :
			pMsg(WARN, "Unknown fill pattern\n");
			return(-1);
	}

	if(args->flags & CLD_FLG_TMD) {
		global_end_time = global_start_time + args->run_time;
	}

	return(0);
}

int main(int argc, char **argv)
{
	extern void *shared_mem;		/* global pointer to shared memory */
	extern size_t bmp_siz;			/* size of bitmask */
	extern OFF_T pass_count;		/* current pass */
	extern stats_t cycle_stats;		/* per cycle statistics */
	extern stats_t global_stats;	/* global statistics */
	extern time_t global_end_time;	/* overall end time of test */
	extern BOOL bContinue;			/* global that when set to false will force exit of all threads */

	time_t start_time;
	OFF_T *pVal1;
	unsigned char *data_buffer_unaligned = NULL;
	unsigned long ulRV;
	child_args_t args;
	char argstr[MAX_ARG_LEN];
	int i;
	
	init_gbl_data();
	
	memset(&args,0,sizeof(child_args_t));
	memset(argstr, 0, MAX_ARG_LEN);

	args.stop_lba = -1;
	args.stop_blk = -1;
	args.flags |= CLD_FLG_ALLDIE;

	for(i=1;i<argc-1;i++) {
		strncat(argstr, argv[i], (MAX_ARG_LEN-1)-strlen(argstr));
		strncat(argstr, " ", (MAX_ARG_LEN-1)-strlen(argstr));
	}

	if(fill_cld_args(argc, argv, &args) < 0) exit(1);
	if(make_assumptions(&args, argstr) < 0) exit(1);
	if(check_conclusions(&args) < 0) exit(1);
	if(args.flags & CLD_FLG_DUMP) {
		/*
		 * All we are doing is dumping filespec data to STDOUT, so
		 * we will do this here and be done.
		 */
		do_dump(&args);
		exit(0);
	} else {
		ulRV = init_data(&args, &data_buffer_unaligned);
		if(ulRV != 0) exit(ulRV);
		pVal1 = (OFF_T *)shared_mem;
	}

	pMsg(START, "Start args: %s\n", argstr);

	/*
	 * This loop takes care of passes
	 */
	do {
		if((args.flags & CLD_FLG_LINEAR) && !(args.flags & CLD_FLG_NTRLVD)) {
			linear_read_write_test(&args);
		} else {
			bContinue = TRUE;
			*(pVal1 + OFF_WLBA) = args.start_lba;
			args.test_state = DIRCT_INC(args.test_state);
			args.test_state = SET_wFST_TIME(args.test_state);
			args.test_state = SET_rFST_TIME(args.test_state);

			if(args.flags & CLD_FLG_CYC)
				pMsg(INFO,"Starting pass %lu of %lu\n", (unsigned long) pass_count, args.cycles);
			else
				pMsg(INFO,"Starting pass\n");

			start_time = time(NULL);
			/*
			 * If we are using a heartbeat to print stats
			 * create a thread to do it for us.
			 */
			if(args.hbeat > 0) CreateChild(ChildPS, &args);

			for(i=0;i<args.t_kids;i++) {
				CreateChild(ChildMain, &args);
			}
			/* Wait for the children to finish */
			clean_up();
			cycle_stats.wtime = time(NULL) - start_time;
			cycle_stats.rtime = time(NULL) - start_time;
			if(args.hbeat == 0) print_stats(&args, READER);
			if(args.hbeat == 0) print_stats(&args, WRITER);
		}
		/* 
		 * Reset all the start conditions in case
		 * we are doing more passes.
		 */
		memset((char *)shared_mem+BMP_OFFSET,0,bmp_siz);
		pass_count++;
		update_gbl_stats();
		if((args.flags & CLD_FLG_CYC) && (pass_count > args.cycles) && (args.cycles != 0))
			break;
		if((args.flags & CLD_FLG_TMD) && (time(NULL) >= global_end_time))
			break;
		if(!(args.flags & CLD_FLG_CYC) && !(args.flags & CLD_FLG_TMD)
			&& (args.flags & CLD_FLG_SKS)
			&& ((global_stats.rcount+global_stats.wcount) >= args.seeks))
			break;
	} while(TST_STS(args.test_state) == 1);

	print_stats(&args, ALL);

	FREE(data_buffer_unaligned);
	FREE(shared_mem);
#ifdef WINDOWS
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "gbl"));
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "data"));
#endif

	if(TST_STS(args.test_state)) {
		pMsg(END, "Test Done (Passed)\n");
		exit(0);
	} else {
		pMsg(END, "Test Done (Failed)\n");
		exit(1);
	}
}
