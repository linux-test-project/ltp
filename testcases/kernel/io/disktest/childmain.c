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
* $Id: childmain.c,v 1.2 2003/04/17 15:21:56 robbiew Exp $
* $Log: childmain.c,v $
* Revision 1.2  2003/04/17 15:21:56  robbiew
* Updated to v1.1.10
*
* Revision 1.10  2003/01/13 21:33:31  yardleyb
* Added code to detect AIX volume size.
* Updated mask for random LBA to use start_lba offset
* Updated version to 1.1.10
*
* Revision 1.9  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.8  2002/03/30 01:32:14  yardleyb
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
* Revision 1.7  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
* Revision 1.6  2002/02/28 04:25:45  yardleyb
* reworked threading code
* made locking code a macro.
*
* Revision 1.4  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
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
* Revision 1.1  2001/12/04 18:52:33  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include "getopt.h"
#else
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "main.h"
#include "sfunc.h"
#include "threading.h"
#include "childmain.h"
#include "io.h"
#include "dump.h"

action_t get_next_action(child_args_t *args, const OFF_T mask)
{
	extern void *shared_mem;		/* global pointer to shared memory */
	extern time_t global_end_time;	/* overall end time of test */
	extern stats_t cycle_stats;		/* per cycle statistics */
	
	OFF_T *pVal1 = (OFF_T *)shared_mem;
	OFF_T *tmpLBA;
	unsigned char *wbitmap = (unsigned char *)shared_mem + BMP_OFFSET;

	short blk_written = 1;
	unsigned long i;
	action_t target;
	short direct = 0;
	BOOL bChangeState = FALSE;

	/* pick an operation */
	if(args->flags & CLD_FLG_RANDOM) {
		if((((cycle_stats.wcount)*100)/(((cycle_stats.rcount)+1)+(cycle_stats.wcount))) >= (args->wperc)) {
			args->test_state = SET_OPER_R(args->test_state);
		} else {
			args->test_state = SET_OPER_W(args->test_state);
		}
	} else if((args->flags & CLD_FLG_NTRLVD) && !TST_wFST_TIME(args->test_state)) {
		if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W))
			args->test_state = CNG_OPER(args->test_state);
	}
	target.oper = TST_OPER(args->test_state);

	/* pick a transfer length */
	if(!(args->flags & CLD_FLG_RTRSIZ)) {
		target.trsiz = args->ltrsiz;
	} else {
		do {
			if((args->flags & CLD_FLG_SKS) && (((cycle_stats.wcount)+(cycle_stats.rcount)) >= args->seeks))
				break;
			if((args->flags & CLD_FLG_TMD) && (time(NULL) >= global_end_time))
				break;
			target.trsiz = (rand()&0xFF) + args->ltrsiz;
		} while(target.trsiz > args->htrsiz);
	}

	/* pick an lba */
	if(args->vsiz == args->htrsiz) { /* diskcache test */
		target.lba = args->start_lba;
	} else if (args->flags & CLD_FLG_LINEAR) {
		tmpLBA = (target.oper == WRITER) ? pVal1+OFF_WLBA : pVal1+OFF_RLBA;
		direct = (TST_DIRCTN(args->test_state)) ? 1 : -1;
		if((target.oper == WRITER) && TST_wFST_TIME(args->test_state)) {
			*(tmpLBA) = args->start_lba;
		} else if((target.oper == READER) && TST_rFST_TIME(args->test_state)) {
			*(tmpLBA) = args->start_lba;
		} else if((TST_DIRCTN(args->test_state)) && ((*(tmpLBA)+(target.trsiz-1)) <= args->stop_lba)) {
		} else if(!(TST_DIRCTN(args->test_state)) && (*(tmpLBA) >= args->start_lba)) {
		} else {
			bChangeState = TRUE;
		}
		if(bChangeState) {			
			if (args->flags & CLD_FLG_LUNU) {
				*(tmpLBA) = args->start_lba;
				if((args->flags & CLD_FLG_CYC) && (target.oper == WRITER)) {
					target.oper = NONE;
				}
			} else if (args->flags & CLD_FLG_LUND) {
				args->test_state = DIRCT_CNG(args->test_state);
				direct = (TST_DIRCTN(args->test_state)) ? 1 : -1;
				*(tmpLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
				if((args->flags & CLD_FLG_CYC) && (direct > 0)) {
					target.oper = NONE;
				}
			}
		}
		target.lba = *(tmpLBA);
	} else if (args->flags & CLD_FLG_RANDOM) {
		do {
			if((args->flags & CLD_FLG_SKS) && (((cycle_stats.wcount)+(cycle_stats.rcount)) >= args->seeks))
				break;
			if((args->flags & CLD_FLG_TMD) && (time(NULL) >= global_end_time))
				break;
			target.lba = (Rand64()&mask) + args->start_lba;
			target.lba = ALIGN(target.lba, target.trsiz);
		} while((target.lba+target.trsiz) > args->stop_lba);
		/*
		 * check to see if we should start a new cycle. We use
		 * rcount as the measure.  That is if rcount is at least
		 * 80% of the total seeks, then we will say that
		 * a cycle has been completed.
		 */
		if(!(args->flags & (CLD_FLG_TMD|CLD_FLG_SKS)) && (args->flags & CLD_FLG_W) && (args->flags & CLD_FLG_R) &&
			(((cycle_stats.rcount*100)/(args->seeks+1)) >= 80)) {
				target.oper = NONE;
		}
	}

	if(!(args->flags & CLD_FLG_NTRLVD)
		&& !(args->flags & CLD_FLG_RANDOM)
		&& (args->flags & CLD_FLG_W)
		&& (args->flags & CLD_FLG_R)) {
			if(((target.oper == WRITER) ? cycle_stats.wcount : cycle_stats.rcount) >= (args->seeks/2)) {
				target.oper = NONE;
			}
	}

	/* get out if exceeded one of the following */
	if((args->flags & CLD_FLG_SKS)
		&& (((cycle_stats.wcount)+(cycle_stats.rcount)) >= args->seeks)) {
			target.oper = NONE;
	}

	if((args->flags & CLD_FLG_TMD)
		&& (time(NULL) >= global_end_time)) {
			target.oper = NONE;
	}

	if((target.oper == READER) && (args->flags & CLD_FLG_CMPR) && (args->flags & CLD_FLG_W)) {
		for(i=0;i<target.trsiz;i++) {
			if((*(wbitmap+(((target.lba-args->start_lba)+i)/8))&(0x80>>(((target.lba-args->start_lba)+i)%8))) == 0) {
				blk_written = 0;
				break;
			}
		}
	}

	if((target.oper == NONE) || (target.oper == WRITER)); /* get out now */
	else if(!(args->flags & CLD_FLG_W) || !(args->flags & CLD_FLG_CMPR)); /* read only */
	else if((args->flags & CLD_FLG_W) && blk_written);	/* read lba that has been written */
	else if((args->flags & CLD_FLG_LINEAR))
		/*
		 * sit and spin, as with linear, reads must come after writes
		 * since this is a read, we will wait here till the write finishes
		 */
		for(i=0;i<target.trsiz;i++)
			while((*(wbitmap+(((target.lba-args->start_lba)+i)/8))&(0x80>>(((target.lba-args->start_lba)+i)%8))) == 0)
				Sleep(1);
	else {
		/* should have been a random reader, but blk not written, so make me a writer */
		target.oper = WRITER;
		args->test_state = SET_OPER_W(args->test_state);
	}

#ifdef WINDOWS
	PDBG5(DEBUG, "%I64d, %I64d, %I64d, %I64d\n", cycle_stats.wcount, cycle_stats.rcount, args->seeks, args->stop_lba);
#else
	PDBG5(DEBUG, "%lld, %lld, %lld, %lld\n", cycle_stats.wcount, cycle_stats.rcount, args->seeks, args->stop_lba);
#endif

	if(target.oper == WRITER) {
		(cycle_stats.wcount)++;
		if((args->flags & CLD_FLG_LUND))
			*(pVal1+OFF_RLBA) = *(pVal1+OFF_WLBA);
		*(pVal1+OFF_WLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
		if(TST_wFST_TIME(args->test_state)) args->test_state = CLR_wFST_TIME(args->test_state);
	}
	if(target.oper == READER) {
		(cycle_stats.rcount)++;
		*(pVal1+OFF_RLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
		if(TST_rFST_TIME(args->test_state)) args->test_state = CLR_rFST_TIME(args->test_state);
	}
	return target;
}

void miscompare_dump(const unsigned char *expected, const unsigned char *actual, const size_t buf_len, OFF_T tPosition)
{
	FILE *fpDumpFile;
	pid_t my_pid;
	char obuff[80];

	my_pid = GETPID();

	obuff[0] = 0;
	sprintf(obuff, "dump_%d.dat", my_pid);
	fpDumpFile = fopen(obuff, "a");

	if(fpDumpFile) fprintf(fpDumpFile, "\n\n\n");
    pMsg(ERR, DMSTR, tPosition, tPosition);
	if(fpDumpFile) fprintf(fpDumpFile, DMSTR, tPosition, tPosition);
	pMsg(ERR, "EXPECTED:\n");
	if(fpDumpFile) fprintf(fpDumpFile, "********** EXPECTED: **********\n\n\n");
	dump_data(stdout, expected, 16, 16, FMT_STR);
	if(fpDumpFile) dump_data(fpDumpFile, expected, buf_len, 16, FMT_STR);
	pMsg(ERR, "ACTUAL:\n");
	if(fpDumpFile) fprintf(fpDumpFile, "\n\n\n********** ACTUAL: **********\n\n\n\n");
	dump_data(stdout, actual, 16, 16, FMT_STR);
	if(fpDumpFile) dump_data(fpDumpFile, actual, buf_len, 16, FMT_STR);
	if(fpDumpFile) fclose(fpDumpFile);
}

/*
* This function is really the main function for a thread
* Once here, this function will act as if it
* were 'main' for that thread.
*/
#ifdef WINDOWS
DWORD WINAPI ChildMain(child_args_t *args)
#else
void *ChildMain(void *vargs)
#endif
{
	extern void *shared_mem;			/* global pointer to shared memory */
	extern BOOL bContinue;				/* global that when set to false will force exit of all threads */
	extern OFF_T pass_count;			/* current pass */
	extern unsigned char *data_buffer;	/* global pointer to shared memory */
	extern stats_t cycle_stats;		/* per cycle statistics */

	unsigned char *buf1 = NULL, *buffer1 = NULL; /* 'buf' is the aligned 'buffer' */
	unsigned char *buf2 = NULL, *buffer2 = NULL; /* 'buf' is the aligned 'buffer' */
	unsigned char *wbitmap = (unsigned char *)shared_mem + BMP_OFFSET;
	unsigned long ulLastError;

	action_t target = { NONE, -1, 0};
	unsigned int i;
	OFF_T ActualBytePos=0, TargetBytePos=0, mask=1;
	long tcnt=0;
	int exit_code=0;
	char filespec[DEV_NAME_LEN];
	fd_t fd;

#ifdef WINDOWS
	HANDLE MutexGBL, MutexDATA;

	if((MutexGBL = OpenMutex(SYNCHRONIZE, TRUE, "gbl")) == NULL) {
		pMsg(ERR, "Failed to open semaphore, error = %u\n", GetLastError());
		args->test_state = SET_STS_FAIL(args->test_state);
		TEXIT(GETLASTERROR());
	}
	if((MutexDATA = OpenMutex(SYNCHRONIZE, TRUE, "data")) == NULL) {
		pMsg(ERR, "Failed to open semaphore, error = %u\n", GetLastError());
		args->test_state = SET_STS_FAIL(args->test_state);
		TEXIT(GETLASTERROR());
	}
#else
	static pthread_mutex_t MutexGBL = PTHREAD_MUTEX_INITIALIZER;
	static pthread_mutex_t MutexDATA = PTHREAD_MUTEX_INITIALIZER;
	child_args_t *args = (child_args_t *) vargs;
#endif

	strncpy(filespec, args->device, DEV_NAME_LEN);

	fd = Open(filespec, args->flags);
	if(INVALID_FD(fd)) {
		pMsg(ERR, "Error = %u\n",GETLASTERROR());
		pMsg(ERR, "could not open %s.\n",args->device);
		args->test_state = SET_STS_FAIL(args->test_state);
		TEXIT(GETLASTERROR());
	}

	/* Create aligned memory buffers for sending IO. */
	if ((buffer1 = (unsigned char *) ALLOC(((args->htrsiz*BLK_SIZE)+ALIGNSIZE))) == NULL) {
		perror("allocation failed, buffer1");
		args->test_state = SET_STS_FAIL(args->test_state);
		CLOSE(fd);
		TEXIT(errno);
	}
	buf1 = (unsigned char *) BUFALIGN(buffer1);

	if ((buffer2 = (unsigned char *) ALLOC(((args->htrsiz*BLK_SIZE)+ALIGNSIZE))) == NULL) {
		perror("allocation failed, buffer2");
		FREE(buffer1);
		args->test_state = SET_STS_FAIL(args->test_state);
		CLOSE(fd);
		TEXIT(errno);
	}
	buf2 = (unsigned char *) BUFALIGN(buffer2);

	/*  set up lba mask of all 1's with value between vsiz and 2*vsiz */
	while(mask <= (args->stop_lba - args->start_lba)) { mask = mask<<1; }
	mask -= 1;

	while(bContinue) {
		LOCK(MutexGBL);
		target = get_next_action(args, mask);
#ifdef WINDOWS
		PDBG5(DEBUG, "%s, %I64d, %lu\n", (target.oper) ? "READ" : "WRITE", target.lba, (target.trsiz*BLK_SIZE));
#else
		PDBG5(DEBUG, "%s, %lld, %lu\n", (target.oper) ? "READ" : "WRITE", target.lba, (target.trsiz*BLK_SIZE));
#endif
		UNLOCK(MutexGBL);

		if(target.oper == NONE) {
			bContinue = FALSE;
            break;
		}

		TargetBytePos=(OFF_T) (target.lba*BLK_SIZE);
		ActualBytePos=Seek(fd, TargetBytePos);
		if(ActualBytePos != TargetBytePos) {
			pMsg(ERR, SFSTR,(target.oper == WRITER) ? (cycle_stats.wcount) : (cycle_stats.rcount),target.lba,TargetBytePos,ActualBytePos);
			args->test_state = SET_STS_FAIL(args->test_state);
			if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
			exit_code = SEEK_FAILURE;
			continue;
		}

		if(target.oper == WRITER) {
			if(args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba), sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, data_buffer, target.trsiz*BLK_SIZE);
			}
			if(args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz*BLK_SIZE, &(target.lba), pass_count, args->mrk_flag);
			}
			tcnt = Write(fd, buf2, target.trsiz*BLK_SIZE);
		}
		if(target.oper == READER) {
			tcnt = Read(fd, buf1, target.trsiz*BLK_SIZE);
		}
		if(tcnt != (long) target.trsiz*BLK_SIZE) {
			ulLastError = GETLASTERROR();
			pMsg(ERR, AFSTR, (target.oper) ? "Read" : "Write", (target.oper) ? (cycle_stats.rcount) : (cycle_stats.wcount),target.lba,target.lba,tcnt,target.trsiz*BLK_SIZE);
			args->test_state = SET_STS_FAIL(args->test_state);
			if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
			exit_code = ACCESS_FAILURE;
			continue;
		}

		/* update bytes transfered and bitmap */
		LOCK(MutexDATA);
		if(target.oper == WRITER) {
			(cycle_stats.wbytes) += target.trsiz*BLK_SIZE;
			for(i=0;i<target.trsiz;i++) {
				*(wbitmap+(((target.lba-args->start_lba)+i)/8)) |= 0x80>>(((target.lba-args->start_lba)+i)%8);
			}
		} else {
			(cycle_stats.rbytes) += target.trsiz*BLK_SIZE;
		}
		UNLOCK(MutexDATA);

		/* data compare routine.  Act as if we were to write, but just compare */
		if((target.oper == READER) && (args->flags & CLD_FLG_CMPR)) {
			/* This is very SLOW!!! */
			if((args->cmp_lng == 0) || (args->cmp_lng > target.trsiz*BLK_SIZE)) {
				args->cmp_lng = target.trsiz*BLK_SIZE;
			}
			if(args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba), sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, data_buffer, target.trsiz*BLK_SIZE);
			}
			if(args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz*BLK_SIZE, &(target.lba), pass_count, args->mrk_flag);
			}
			if(memcmp(buf2, buf1, args->cmp_lng) != 0) {
				/* data miscompare !!! */
				LOCK(MutexGBL);
				miscompare_dump(buf2, buf1, args->htrsiz*BLK_SIZE, target.lba);
				args->test_state = SET_STS_FAIL(args->test_state);
				if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
				UNLOCK(MutexGBL);
				exit_code = DATA_MISCOMPARE;
				continue;
			}
		}
	}

	FREE(buffer1);
	FREE(buffer2);
	CLOSE(fd);
	TEXIT(exit_code);
}

