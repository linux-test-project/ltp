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
* $Id: main.c,v 1.4 2005/05/04 17:54:00 mridge Exp $
* $Log: main.c,v $
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.26  2005/05/03 16:24:38  yardleyb
* Added needed code changes to support windows
*
* Revision 1.25  2005/04/28 21:25:17  yardleyb
* Fixed up some issues with AIX compilation due to the change made
* in endian support in Linux.
*
* Revision 1.24  2005/01/12 02:40:11  yardleyb
* Fixed regression issue in the -h option
*
* Revision 1.23  2005/01/08 22:29:56  yardleyb
* Changed 'exit()' in threaded code to macro TEXIT
* to fix a possible seg fault
*
* Revision 1.22  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.21  2004/12/18 06:13:03  yardleyb
* Updated timer schema to more accurately use the time options.  Added
* fsync on write option to -If.
*
* Revision 1.20  2004/12/17 06:34:56  yardleyb
* removed -mf -ml.  These mark options cause to may issues when using
* random block size transfers.  Fixed -ma option for endian-ness.  Fixed
* false data misscompare during multiple cycles.
*
* Revision 1.19  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.18  2004/11/19 21:45:12  yardleyb
* Fixed issue with code added for -F option.  Cased disktest
* to SEG FAULT when cleaning up threads.
*
* Revision 1.17  2004/11/19 03:47:45  yardleyb
* Fixed issue were args data was not being copied from a
* clean source.
*
* Revision 1.16  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.15  2003/09/12 21:23:01  yardleyb
* The following isses have been fixed:
* - Updated to Version 1.12
* - Disktest will falsely detect a data miscompare
* when using random block sizes and random data
* - If the linear option is used while doing random
* block sizes and read/write/error checks, disktest
* will hang
* - Disktest will use the wrong transfer size on
* the last IO when using random block transfer size
* and the number of seeks are specified.
* - Total Reads and Writes not reported correctly
* - While running linear write/read tests while
* doing the heartbeat performance and you get an
* error on the 'write' side of the test, disktest
* does not exit
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
#include "timer.h"
#include "stats.h"

/* global */
child_args_t cleanArgs;
test_env_t cleanEnv;

void linear_read_write_test(test_ll_t *test)
{
	OFF_T *pVal1 = (OFF_T *)test->env->shared_mem;
	int i;

	if(test->args->flags & CLD_FLG_W) {
		test->env->bContinue = TRUE;
		*(pVal1 + OFF_WLBA) = test->args->start_lba;
		test->args->test_state = DIRCT_INC(test->args->test_state);
		test->args->test_state = SET_OPER_W(test->args->test_state);
		test->args->test_state = SET_wFST_TIME(test->args->test_state);
		srand(test->args->seed);	/* reseed so we can re create the same random transfers */
		if(test->args->flags & CLD_FLG_CYC)
			if(test->args->cycles == 0) {
				pMsg(INFO,test->args, "Starting write pass, cycle %lu\n", (unsigned long) test->env->pass_count);
			} else {
				pMsg(INFO,test->args, "Starting write pass, cycle %lu of %lu\n", (unsigned long) test->env->pass_count, test->args->cycles);
			}
		else {
			pMsg(INFO,test->args, "Starting write pass\n");
		}
		CreateTestChild(ChildTimer, test);
		for(i=0;i<test->args->t_kids;i++) {
			CreateTestChild(ChildMain, test);
		}
		/* Wait for the writers to finish */
		cleanUpTestChildren(test);
	}

	/* If the write test failed don't start the read test */
	if(!(TST_STS(test->args->test_state))) { return; }

	if(test->args->flags & CLD_FLG_R) {
		test->env->bContinue = TRUE;
		*(pVal1 + OFF_RLBA) = test->args->start_lba;
		test->args->test_state = DIRCT_INC(test->args->test_state);
		test->args->test_state = SET_OPER_R(test->args->test_state);
		test->args->test_state = SET_rFST_TIME(test->args->test_state);
		srand(test->args->seed);	/* reseed so we can re create the same random transfers */
		if(test->args->flags & CLD_FLG_CYC)
			if(test->args->cycles == 0) {
				pMsg(INFO,test->args, "Starting read pass, cycle %lu\n", (unsigned long) test->env->pass_count);
			} else {
				pMsg(INFO,test->args, "Starting read pass, cycle %lu of %lu\n", (unsigned long) test->env->pass_count, test->args->cycles);
			}
		else {
			pMsg(INFO,test->args, "Starting read pass\n");
		}
		CreateTestChild(ChildTimer, test);
		for(i=0;i<test->args->t_kids;i++) {
			CreateTestChild(ChildMain, test);
		}
		/* Wait for the readers to finish */
		cleanUpTestChildren(test);
	}
}

unsigned long init_data(test_ll_t *test, unsigned char **data_buffer_unaligned)
{
	extern time_t global_start_time;	/* overall start time of test */
	extern time_t global_end_time;		/* overall end time of test */

	int i;
	OFF_T *pVal1;

	unsigned long data_buffer_size;

	if(test->args->seed == 0) test->args->seed = test->args->pid;
	srand(test->args->seed);

	/* create bitmap to hold write/read context: each bit is an LBA */
	/* the stuff before BMP_OFFSET is the data for child/thread shared context */
	test->env->bmp_siz = (((((size_t)test->args->vsiz))/8) == 0) ? 1 : ((((size_t)test->args->vsiz))/8);
	if ((test->args->vsiz/8) != 0) test->env->bmp_siz += 1;	/* account for rounding error */

	/* We use that same data buffer for static data, so alloc here. */
	data_buffer_size = ((test->args->htrsiz*BLK_SIZE)*2);
	if((*data_buffer_unaligned = (unsigned char *) ALLOC(data_buffer_size+ALIGNSIZE)) == NULL) {
		pMsg(ERR,test->args,  "Failed to allocate static data buffer memory.\n");
		return(-1);
	}
	test->env->data_buffer = (unsigned char *) BUFALIGN(*data_buffer_unaligned);

	if((test->env->shared_mem = (void *) ALLOC(test->env->bmp_siz+BMP_OFFSET)) == NULL) {
		pMsg(ERR, test->args, "Failed to allocate bitmap memory\n");
		return(-1);
	}
#ifdef WINDOWS
	if(CreateMutex(NULL, FALSE, "gbl") == NULL) {
		pMsg(ERR, test->args, "Failed to create semaphore, error = %u\n", GetLastError());
		return(GetLastError());
	}
	if(CreateMutex(NULL, FALSE, "data") == NULL) {
		pMsg(ERR, test->args, "Failed to create semaphore, error = %u\n", GetLastError());
		return(GetLastError());
	}
#endif

	memset(test->env->shared_mem,0,test->env->bmp_siz+BMP_OFFSET);
	memset(test->env->data_buffer,0,data_buffer_size);

	pVal1 = (OFF_T *)test->env->shared_mem;
	*(pVal1 + OFF_WLBA) = test->args->start_lba;
	*(pVal1 + OFF_RLBA) = test->args->start_lba;
	test->args->test_state = SET_STS_PASS(test->args->test_state);
	test->args->test_state = SET_wFST_TIME(test->args->test_state);
	test->args->test_state = SET_rFST_TIME(test->args->test_state);
	test->args->test_state = DIRCT_INC(test->args->test_state);
	if(test->args->flags & CLD_FLG_W)
		test->args->test_state = SET_OPER_W(test->args->test_state);
	else
		test->args->test_state = SET_OPER_R(test->args->test_state);

	/* prefill the data buffer with data for compares and writes */
	switch(test->args->flags & CLD_FLG_PTYPS) {
		case CLD_FLG_FPTYPE :
			for(i=0;i<sizeof(test->args->pattern);i++) {
				if((test->args->pattern & (((OFF_T) 0xff) << (((sizeof(test->args->pattern)-1)-i)*8))) != 0) break;
			}
			/* special case for pattern = 0 */
			if(i == sizeof(test->args->pattern)) i = 0;
			fill_buffer(test->env->data_buffer, data_buffer_size, &test->args->pattern, sizeof(test->args->pattern)-i, CLD_FLG_FPTYPE);
			break;
		case CLD_FLG_RPTYPE :
			fill_buffer(test->env->data_buffer, data_buffer_size, NULL, 0, CLD_FLG_RPTYPE);
			break;
		case CLD_FLG_CPTYPE :
			fill_buffer(test->env->data_buffer, data_buffer_size, 0, 0, CLD_FLG_CPTYPE);
		case CLD_FLG_LPTYPE :
			break;
		default :
			pMsg(WARN, test->args, "Unknown fill pattern\n");
			return(-1);
	}

	if(test->args->flags & CLD_FLG_TMD) {
		global_end_time = global_start_time + test->args->run_time;
	}

	return(0);
}

#ifdef WINDOWS
DWORD WINAPI threadedMain(test_ll_t *test)
#else
void *threadedMain(void *vtest)
#endif
{
#ifndef WINDOWS
	test_ll_t *test = (test_ll_t *) vtest;
#endif

	extern time_t global_end_time;

	OFF_T *pVal1;
	unsigned char *data_buffer_unaligned = NULL;
	unsigned long ulRV;
	int i;
	unsigned char *sharedMem;

	test->args->pid = GETPID();

	init_gbl_data(test->env);

	if(make_assumptions(test->args) < 0) { TEXIT(GETLASTERROR()); }
	if(check_conclusions(test->args) < 0) { TEXIT(GETLASTERROR()); }
	if(test->args->flags & CLD_FLG_DUMP) {
		/*
		 * All we are doing is dumping filespec data to STDOUT, so
		 * we will do this here and be done.
		 */
		do_dump(test->args);
		TEXIT(GETLASTERROR());
	} else {
		ulRV = init_data(test, &data_buffer_unaligned);
		if(ulRV != 0) { TEXIT(ulRV); }
		pVal1 = (OFF_T *)test->env->shared_mem;
	}

	pMsg(START, test->args, "Start args: %s\n", test->args->argstr);

	/*
	 * This loop takes care of passes
	 */
	do {
		test->env->pass_count++;
		sharedMem = test->env->shared_mem;
		memset(sharedMem+BMP_OFFSET,0,test->env->bmp_siz);
		if((test->args->flags & CLD_FLG_LINEAR) && !(test->args->flags & CLD_FLG_NTRLVD)) {
			linear_read_write_test(test);
		} else {
			test->env->bContinue = TRUE;
			*(pVal1 + OFF_WLBA) = test->args->start_lba;
			test->args->test_state = DIRCT_INC(test->args->test_state);
			test->args->test_state = SET_wFST_TIME(test->args->test_state);
			test->args->test_state = SET_rFST_TIME(test->args->test_state);

			if(test->args->flags & CLD_FLG_CYC)
				if(test->args->cycles == 0) {
					pMsg(INFO,test->args, "Starting pass %lu\n", (unsigned long) test->env->pass_count);
				} else {
					pMsg(INFO,test->args, "Starting pass %lu of %lu\n", (unsigned long) test->env->pass_count, test->args->cycles);
				}
			else {
				pMsg(INFO,test->args, "Starting pass\n");
			}

			CreateTestChild(ChildTimer, test);
			for(i=0;i<test->args->t_kids;i++) {
				CreateTestChild(ChildMain, test);
			}
			/* Wait for the children to finish */
			cleanUpTestChildren(test);
		}
		if((test->args->hbeat == 0) || ((test->args->hbeat > 0) &&
		  ((test->env->cycle_stats.wtime % test->args->hbeat) || (test->env->cycle_stats.rtime % test->args->hbeat)) != 0)) {
			print_stats(test->args, test->env, CYCLE);
		}
		update_gbl_stats(test->env);
		print_stats(test->args, test->env, TOTAL);
		if((test->args->flags & CLD_FLG_CYC) && (test->env->pass_count >= test->args->cycles) && (test->args->cycles != 0)) {
			break;
		}
		if((test->args->flags & CLD_FLG_TMD) && (time(NULL) >= global_end_time)) {
			break;
		}
		if(!(test->args->flags & CLD_FLG_CYC) && !(test->args->flags & CLD_FLG_TMD)
			&& (test->args->flags & CLD_FLG_SKS)
			&& ((test->env->global_stats.rcount+test->env->global_stats.wcount) >= test->args->seeks)) {
			break;
		}
	} while(TST_STS(test->args->test_state));

	FREE(data_buffer_unaligned);
	FREE(test->env->shared_mem);
#ifdef WINDOWS
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "gbl"));
	CloseHandle(OpenMutex(SYNCHRONIZE, TRUE, "data"));
#endif

	if(TST_STS(test->args->test_state)) {
		pMsg(END, test->args, "Test Done (Passed)\n");
	} else {
		pMsg(END, test->args, "Test Done (Failed)\n");
	}
	TEXIT(GETLASTERROR());
}

/*
 * Creates a new test structure and adds it to the list of
 * test structures already available.  Allocate all memory
 * needed my the new test.
 *
 * Returns the newly created test structure
 */
test_ll_t *getNewTest(test_ll_t *testList) {
	test_ll_t *pNewTest;

	if((pNewTest = (test_ll_t *)ALLOC(sizeof(test_ll_t))) == NULL) {
		pMsg(ERR, &cleanArgs, "%d : Could not allocate memory for new test.\n", GETLASTERROR());
		return NULL;
	}

	memset(pNewTest, 0, sizeof(test_ll_t));

	if((pNewTest->args = (child_args_t *)ALLOC(sizeof(child_args_t))) == NULL) {
		pMsg(ERR, &cleanArgs, "%d : Could not allocate memory for new test.\n", GETLASTERROR());
		FREE(pNewTest);
		return NULL;
	}
	if((pNewTest->env = (test_env_t *)ALLOC(sizeof(test_env_t))) == NULL) {
		pMsg(ERR, &cleanArgs, "%d : Could not allocate memory for new test.\n", GETLASTERROR());
		FREE(pNewTest->args);
		FREE(pNewTest);
		return NULL;
	}
	memcpy(pNewTest->args, &cleanArgs, sizeof(child_args_t));
	memcpy(pNewTest->env, &cleanEnv, sizeof(test_env_t));

	pNewTest->next = testList;
	testList = pNewTest;
	return pNewTest;
}

test_ll_t *run() {
	test_ll_t *newTest = NULL, *lastTest = NULL;
	
	if(cleanArgs.flags & CLD_FLG_FSLIST) {
		char *filespec = cleanArgs.device;
		char *aFilespec = NULL; 
		FILE *file = NULL;

		if((aFilespec = (char *)ALLOC(80)) == NULL) {
			pMsg(ERR, &cleanArgs, "Could not allocate memory to read file");
			return newTest;
		}

		file = fopen(filespec, "r");
			if(file == NULL) {
				pMsg(
					ERR,
					&cleanArgs,
					"%s is not a regular file, could not be opened for reading, or was not found.",
					filespec);

				return newTest;
			}

		while(!feof(file)) {
			memset(aFilespec, 0, 80);
			fscanf(file, "%79s", aFilespec);
			if(aFilespec[0] != 0) { /* if we read something useful */
				lastTest = newTest;
				newTest = getNewTest(lastTest);
				if(newTest != lastTest) {
					memset(newTest->args->device, 0, DEV_NAME_LEN);
					strncpy(newTest->args->device, aFilespec, strlen(aFilespec));
					createChild(threadedMain, newTest);
				} else {
					newTest = lastTest;
					break;
				}
			}
		}
	
		fclose(file);
		FREE(aFilespec);
	} else {
		newTest = getNewTest(newTest);
		if(newTest != NULL) {
			createChild(threadedMain, newTest);
		}
	}

	return newTest;
}

int main(int argc, char **argv)
{
	extern time_t global_start_time;
	extern time_t global_end_time;
	extern unsigned long glb_flags;	/* global flags GLB_FLG_xxx */
	int i;

	global_start_time = time(NULL);
	global_end_time	= 0;
	glb_flags = 0;

	strncpy(cleanArgs.device, "No filespec", strlen("No filespec"));
	cleanArgs.stop_lba = -1;
	cleanArgs.stop_blk = -1;
	cleanArgs.flags |= CLD_FLG_ALLDIE;

	for(i=1;i<argc-1;i++) {
		strncat(cleanArgs.argstr, argv[i], (MAX_ARG_LEN-1)-strlen(cleanArgs.argstr));
		strncat(cleanArgs.argstr, " ", (MAX_ARG_LEN-1)-strlen(cleanArgs.argstr));
	}

	if(fill_cld_args(argc, argv, &cleanArgs) < 0) exit(1);

	cleanUp(run());

	return 0;
}
