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
* $Id: main.c,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: main.c,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
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
#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include "getopt.h"
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#ifndef WIN32
#include <wait.h>
#include <unistd.h>
#endif
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

void print_stats(time_t start_time, child_args_t *args)
{
	extern void *shared_mem;			/* global pointer to shared memory */
	extern unsigned long glb_flags;		/* global flags GLB_FLG_xxx */
	extern char *devname;				/* global pointer to device name */

	OFF_T *wcount = (OFF_T *)shared_mem + OFF_WCOUNT;
	OFF_T *rcount = (OFF_T *)shared_mem + OFF_RCOUNT;
	OFF_T *rbytes = (OFF_T *)shared_mem + OFF_RBYTES;
	OFF_T *wbytes = (OFF_T *)shared_mem + OFF_WBYTES;
	time_t curr_time;

	curr_time = time(NULL);

	if((args->flags & CLD_FLG_TMD) && ((curr_time - start_time) > args->run_time))
		curr_time = start_time + args->run_time;

	/* if one second really has not passed, then make it at least one second */
	if((curr_time - start_time) == 0)
		curr_time++;

	if(glb_flags & GLB_FLG_PERFP) {
		printf("%s;", devname);
		if((args->flags & CLD_FLG_XFERS)) {
#ifdef WIN32
			printf("%I64d;Rbytes;%I64d;Rxfers;", (*rbytes), (*rcount));
			printf("%I64d;Wbytes;%I64d;Wxfers;", (*wbytes), (*wcount));
#else
			printf("%lld;Rbytes;%lld;Rxfers;", (*rbytes), (*rcount));
			printf("%lld;Wbytes;%lld;Wxfers;", (*wbytes), (*wcount));
#endif
		}
		if((args->flags & CLD_FLG_TPUTS)) {
#ifdef WIN32
			printf("%I64d;RB/s;%I64d;RIOPS;", ((*rbytes) / (curr_time - start_time)), ((*rcount) / (curr_time - start_time)));
			printf("%I64d;WB/s;%I64d;WIOPS;", ((*wbytes) / (curr_time - start_time)), ((*wcount) / (curr_time - start_time)));
#else
			printf("%lld;RB/s;%lld;RIOPS;", ((*rbytes) / (curr_time - start_time)), ((*rcount) / (curr_time - start_time)));
			printf("%lld;WB/s;%lld;WIOPS;", ((*wbytes) / (curr_time - start_time)), ((*wcount) / (curr_time - start_time)));
#endif
		}
		if((args->flags & CLD_FLG_RUNT)) {
			printf("%lu;secs;",(curr_time - start_time));
		}
		printf("\n");
	} else {
		if((args->flags & CLD_FLG_XFERS)) {
			if (args->flags & CLD_FLG_R) {
#ifdef WIN32
				pMsg(STAT, "%I64d bytes read in %I64d transfers.\n",
#else
				pMsg(STAT, "%lld bytes read in %lld transfers.\n",
#endif
					(*rbytes), (*rcount));
			}
			if (args->flags & CLD_FLG_W) {
#ifdef WIN32
				pMsg(STAT, "%I64d bytes written in %I64d transfers.\n",
#else
				pMsg(STAT, "%lld bytes written in %lld transfers.\n",
#endif
					(*wbytes), (*wcount));
			}
		}
		if((args->flags & CLD_FLG_TPUTS)) {
			if (args->flags & CLD_FLG_R) {
#ifdef WIN32
				pMsg(STAT, "Read Throughput %I64dB/s (%I64dMB/s), IOPS %I64d/s.\n",
#else
				pMsg(STAT, "Read Throughput %lldB/s (%lldMB/s), IOPS %lld/s.\n",
#endif
					((*rbytes) / (OFF_T) (curr_time - start_time)),
					(((*rbytes) / (curr_time - start_time)) / (OFF_T) (1024 * 1024)),
					((*rcount) / (OFF_T) (curr_time - start_time)));
			}
			if (args->flags & CLD_FLG_W) {
#ifdef WIN32
				pMsg(STAT, "Write Throughput %I64dB/s (%I64dMB/s), IOPS %I64d/s.\n",
#else
				pMsg(STAT, "Write Throughput %lldB/s (%lldMB/s), IOPS %lld/s.\n",
#endif
					((*wbytes) / (OFF_T) (curr_time - start_time)),
					(((*wbytes) / (curr_time - start_time)) / (OFF_T) (1024 * 1024)),
					((*wcount) / (OFF_T) (curr_time - start_time)));
			}
		}
		if((args->flags & CLD_FLG_RUNT)) {
			pMsg(STAT,"Runtime %u seconds (%luh%lum%lus)\n", (curr_time - start_time), ((curr_time - start_time)/3600), (((curr_time - start_time)%3600)/60), ((curr_time - start_time)%60));
		}
	}
}

int main(int argc, char **argv)
{
	extern void *shared_mem;			/* global pointer to shared memory */
	extern char *devname;				/* global pointer to device name */
	extern int kids;					/* global number of current child processes */
	extern unsigned char *data_buffer;	/* global pointer to shared memory */
	extern size_t bmp_siz;				/* size of bitmask */
	extern size_t seed;					/* random seed */
	extern OFF_T pass_count;			/* current pass */

	int i;
	time_t start_time;
	OFF_T *pVal1, *test_state;
	unsigned char *data_buffer_unaligned;
	child_args_t args;
	pid_t my_pid;
	char argstr[80];

#ifdef WIN32
	HANDLE hSem;
#endif
	init_gbl_data(argv);
	
#ifdef WIN32
	my_pid = _getpid();
#else
	my_pid = getpid();
#endif

	memset(&args,0,sizeof(child_args_t));
	memset(argstr, 0, 80);

	args.stop_lba = -1;
	args.flags |= CLD_FLG_ALLDIE;

	for(i=1;i<argc-1;i++) {
		strncat(argstr, argv[i], 79-strlen(argstr));
		strncat(argstr, " ", 79-strlen(argstr));
	}
	devname = argv[argc-1];

	fill_cld_args(argc, argv, &args);
	make_assumptions(&args);
	if(check_conclusions(&args) < 0) exit(1);
	
	if(seed == -1) seed = my_pid;
	srand(seed);

	if(args.flags & CLD_FLG_DUTY) normalize_percs(&args);

	/* create bitmap to hold write/read context: each bit is an LBA */
	/* the stuff before BMP_OFFSET is the data for child/thread shared context */
	bmp_siz = (((((size_t)args.vsiz))/8) == 0) ? 1 : ((((size_t)args.vsiz))/8);
	if ((args.vsiz/8) != 0) bmp_siz += 1;	/* account for rounding error */

	/*
	 * We use that same data buffer for static data, so alloc here.
	 */
	if((data_buffer_unaligned = (unsigned char *) malloc(DBUF_SIZE+ALIGNSIZE)) == NULL) {
		pMsg(ERR, "Failed to allocate static data buffer memory.\n");
		exit(1);
	}
	data_buffer = (unsigned char *) BUFALIGN(data_buffer_unaligned);

	if((shared_mem = (void *) malloc(bmp_siz+BMP_OFFSET)) == NULL) {
		pMsg(ERR, "Failed to allocate bitmap memory\n");
		exit(1);
	}
#ifdef WIN32
	if((hSem = CreateMutex(NULL, FALSE, "gbl")) == NULL) {
		pMsg(ERR, "Failed to create semaphore, error = %u\n", GetLastError());
		exit(GetLastError());
	}
	if((hSem = CreateMutex(NULL, FALSE, "data")) == NULL) {
		pMsg(ERR, "Failed to create semaphore, error = %u\n", GetLastError());
		exit(GetLastError());
	}
#endif

	memset((char *)shared_mem,0,bmp_siz+BMP_OFFSET);
	memset((unsigned char *)data_buffer,0,DBUF_SIZE);

	pVal1 = (OFF_T *)shared_mem;
	*(pVal1 + OFF_WLBA) = args.start_lba;
	*(pVal1 + OFF_RLBA) = args.start_lba;
	*(pVal1 + OFF_TST_STAT) = SET_STS_PASS(*(pVal1 + OFF_TST_STAT));
	*(pVal1 + OFF_TST_STAT) = SET_wFST_TIME(*(pVal1 + OFF_TST_STAT));
	*(pVal1 + OFF_TST_STAT) = SET_rFST_TIME(*(pVal1 + OFF_TST_STAT));
	*(pVal1 + OFF_TST_STAT) = DIRCT_INC(*(pVal1 + OFF_TST_STAT));
	if(args.flags & CLD_FLG_W)
		*(pVal1 + OFF_TST_STAT) = SET_OPER_W(*(pVal1 + OFF_TST_STAT));
	else
		*(pVal1 + OFF_TST_STAT) = SET_OPER_R(*(pVal1 + OFF_TST_STAT));

	/*
	* prefill the data buffer with data for compares and writes
	* larges transfer size is T_MAX_SIZE, so fill to that size.
	*/
	
	switch(args.flags & CLD_FLG_PTYPS) {
		case CLD_FLG_FPTYPE :
			for(i=0;i<sizeof(args.pattern);i++) {
				if((args.pattern & (((OFF_T) 0xff) << (((sizeof(args.pattern)-1)-i)*8))) != 0) break;
			}
			fill_buffer(data_buffer+OFF_DATA, T_MAX_SIZE, &args.pattern, sizeof(args.pattern)-i, CLD_FLG_FPTYPE);
			break;
		case CLD_FLG_RPTYPE :
			fill_buffer(data_buffer+OFF_DATA, T_MAX_SIZE/8, &seed, sizeof(pid_t), CLD_FLG_RPTYPE);
			break;
		case CLD_FLG_CPTYPE :
			fill_buffer(data_buffer+OFF_DATA, T_MAX_SIZE, 0, 0, CLD_FLG_CPTYPE);
		case CLD_FLG_LPTYPE :
			break;
		default :
			pMsg(WARN, "Unknown fill pattern\n");
			exit(1);
	}
	
	pMsg(START, "Start args: %s\n", argstr);

	start_time = time(NULL);

	/*
	 * This loop takes care of passes
	 */
	do {
		for(i=0;i<args.t_kids;i++) {
			CreateChild(&args);
		}

		/*
		* If we missed any zombied kids along the way, then we will clean
		* them up here.  We also wait in this loop while the kids do
		* their work.  'clean_up' does nothing if there are no
		* zombied children.
		*/
		while (kids) {
			Sleep(1000);
			if((args.hbeat > 0) && (((time(NULL) - start_time) % args.hbeat) == 0))
				print_stats(start_time, &args);
			clean_up(NULL);
		}

		memset((char *)shared_mem+BMP_OFFSET,0,bmp_siz);
		pass_count++;
		*(pVal1 + OFF_TST_STAT) = SET_wFST_TIME(*(pVal1 + OFF_TST_STAT));
		*(pVal1 + OFF_TST_STAT) = SET_rFST_TIME(*(pVal1 + OFF_TST_STAT));
	} while((args.flags & CLD_FLG_CYC) && (pass_count <= args.cycles));

	if(args.hbeat == 0)
		print_stats(start_time, &args);

	test_state = (OFF_T *)shared_mem + OFF_TST_STAT;
	if(TST_STS(*test_state))
		pMsg(END, "Test Done (Passed)\n");
	else
		pMsg(END, "Test Done (Failed)\n");

	free(data_buffer_unaligned);
#ifdef WIN32
	free(shared_mem);
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "gbl"));
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "data"));
#endif
	exit(0);
}
