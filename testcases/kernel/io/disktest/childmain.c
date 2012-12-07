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
* $Id: childmain.c,v 1.11 2009/02/26 12:14:53 subrata_modak Exp $
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
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
#include "globals.h"
#include "main.h"
#include "sfunc.h"
#include "threading.h"
#include "io.h"
#include "dump.h"
#include "timer.h"
#include "signals.h"
#include "childmain.h"

/*
 * The following three functions are used to mutex LBAs that are in use by another
 * thread from any other thread performing an action on that lba.
 */
unsigned short action_in_use(const test_env_t * env, const action_t target)
{
	int i = 0;

	for (i = 0; i < env->action_list_entry; i++) {
		if ((target.lba == env->action_list[i].lba)	/* attempting same transfer start lba */
		    ||((target.lba < env->action_list[i].lba) && (target.lba + target.trsiz - 1) >= env->action_list[i].lba)	/* attempting transfer over an lba in use */
		    ) {
			/*
			 * The lba(s) we want to do IO to are in use by another thread,
			 * but since POSIX allows for multiple readers, we need to compare
			 * our action with the action being executed by the other thread
			 */
			switch (target.oper) {
			case WRITER:	/* if we want to write, we can't */
				return TRUE;
			case READER:	/* if we want to read, and a write is in progress, we can't */
				if (env->action_list[i].oper == WRITER) {
					return TRUE;
				}
				/* otherwise allow multiple readers */
				return FALSE;
			default:
				/* for all other operations, always assume inuse */
				return TRUE;
			}
		}
	}

	return FALSE;
}

void add_action(test_env_t * env, const child_args_t * args,
		const action_t target)
{

	if (env->action_list_entry == args->t_kids) {	/* we should never get here */
		printf
		    ("ATTEMPT TO ADD MORE ENTRIES TO LBA WRITE LIST THEN ALLOWED, CODE BUG!!!\n");
		abort();
	}

	env->action_list[env->action_list_entry++] = target;
}

void remove_action(test_env_t * env, const action_t target)
{
	int i = 0;

	if (env->action_list_entry == 0) {
		/* we should never get here */
		printf
		    ("ATTEMPT TO REMOVE ENTRIES FROM LBA WRITE LIST WHERE NONE EXIST, CODE BUG!!!\n");
		abort();
	}

	/* look for the removing target */
	while (target.lba != env->action_list[i].lba) {
		if (env->action_list_entry == i++) {
			printf
			    ("INDEX AND CURRENT LIST ENTRY, CODE BUG!!!!!!\n");
			abort();
		}
	}

	/* move eny other entries down */
	for (; i < env->action_list_entry - 1; i++) {
		env->action_list[i] = env->action_list[i + 1];
	}

	/* reduce the slot */
	env->action_list_entry--;
}

void decrement_io_count(const child_args_t * args, test_env_t * env,
			const action_t target)
{
	if (args->flags & CLD_FLG_LBA_SYNC) {
		remove_action(env, target);
	}
	if (target.oper == WRITER) {
		(env->wcount)--;
	} else {
		(env->rcount)--;
	}
}

/*
 * This function will write a special mark to LBA 0 of
 * a target, if an error occured on the target.  This
 * is so a trigger can be set, i.e. on an analyser.
 */
void write_error_mark(fd_t fd, char *data)
{
	OFF_T ActualBytePos = 0;
	long tcnt = 0;

	ActualBytePos = Seek(fd, 0);
	if (ActualBytePos != 0) {
		/* could not seek to LBA 0 */
		return;
	}

	memcpy(data, "DISKTEST ERROR OCCURRED",
	       strlen("DISKTEST ERROR OCCURRED"));
	tcnt = Write(fd, data, BLK_SIZE);
}

/*
 * Sets the test state correctly, and updates test flags
 * based on user parsed options
 */
void update_test_state(child_args_t * args, test_env_t * env,
		       const int this_thread_id, fd_t fd, char *data)
{
	extern unsigned short glb_run;
	extern unsigned long glb_flags;

	if (args->flags & CLD_FLG_ALLDIE) {
#ifdef _DEBUG
		PDBG4(DBUG, args,
		      "Thread %d: Setting bContinue to FALSE, io error, all die\n",
		      this_thread_id);
#endif
		args->test_state = SET_STS_FAIL(args->test_state);
		env->bContinue = FALSE;
	}
	if (glb_flags & GLB_FLG_KILL) {
#ifdef _DEBUG
		PDBG4(DBUG, args,
		      "Thread %d: Setting bContinue to FALSE, io error, global die\n",
		      this_thread_id);
#endif
		args->test_state = SET_STS_FAIL(args->test_state);
		env->bContinue = FALSE;
		glb_run = 0;
	}
	if ((args->flags & CLD_FLG_W) && (args->flags & CLD_FLG_ERR_MARK)) {
		write_error_mark(fd, data);
	}
}

#ifdef _DEBUG
#ifdef _DEBUG_PRINTMAP
void print_lba_bitmap(const test_env_t * env)
{
	unsigned char *wbitmap = (unsigned char *)env->shared_mem + BMP_OFFSET;
	int i;

	for (i = 0; i < (env->bmp_siz - 1); i++) {
		printf("%02x", *(wbitmap + i));
	}
	printf("\n");
}
#endif
#endif

action_t get_next_action(child_args_t * args, test_env_t * env,
			 const OFF_T mask)
{

	OFF_T *pVal1 = (OFF_T *) env->shared_mem;
	OFF_T *tmpLBA;
	OFF_T guessLBA;
	unsigned char *wbitmap = (unsigned char *)env->shared_mem + BMP_OFFSET;

	short blk_written = 0;
	unsigned long i;
	action_t target = { NONE, 0, 0 };
	short direct = 0;

	/* pick an operation */
	target.oper = env->lastAction.oper;
	if ((args->flags & CLD_FLG_LINEAR) && !(args->flags & CLD_FLG_NTRLVD)) {
		target.oper = TST_OPER(args->test_state);
	} else if ((args->flags & CLD_FLG_RANDOM)
		   && !(args->flags & CLD_FLG_NTRLVD)) {
		if ((((env->wcount) * 100) /
		     (((env->rcount) + 1) + (env->wcount))) >= (args->wperc)) {
			target.oper = READER;
		} else {
			target.oper = WRITER;
		}
#ifdef _DEBUG
		PDBG4(DBUG, args, "W:%.2f%% R:%.2f%%\n",
		      100 * ((double)(env->wcount) /
			     ((double)env->rcount + (double)env->wcount)),
		      100 * ((double)(env->rcount) /
			     ((double)env->wcount + (double)env->rcount)));
#endif
	} else if ((args->flags & CLD_FLG_NTRLVD)
		   && !TST_wFST_TIME(args->test_state)) {
		if ((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			target.oper =
			    (env->lastAction.oper == WRITER) ? READER : WRITER;
		}
	} else if (target.oper == NONE) {
		/* if still no decision for an operation, do the basics */
		target.oper = (args->flags & CLD_FLG_W) ? WRITER : READER;
	}

	/* pick a transfer length */
	if (!(args->flags & CLD_FLG_RTRSIZ)) {
		target.trsiz = args->ltrsiz;
	} else {
		if ((args->flags & CLD_FLG_NTRLVD) &&
		    (args->flags & CLD_FLG_W) &&
		    (args->flags & CLD_FLG_R) &&
		    (env->lastAction.trsiz != 0) && (target.oper == READER)) {
			target.trsiz = env->lastAction.trsiz;
		} else {
			do {
				target.trsiz = (rand() & 0xFFF) + args->ltrsiz;
				if ((args->flags & CLD_FLG_SKS)
				    && (((env->wcount) + (env->rcount)) >=
					args->seeks))
					break;
			} while (target.trsiz > args->htrsiz);
		}
	}

	/* pick an lba */
	if (args->start_blk == args->stop_blk) {	/* diskcache test */
		target.lba = args->start_lba + args->offset;
	} else if (args->flags & CLD_FLG_LINEAR) {
		tmpLBA =
		    (target.oper ==
		     WRITER) ? pVal1 + OFF_WLBA : pVal1 + OFF_RLBA;
		direct = (TST_DIRCTN(args->test_state)) ? 1 : -1;
		if ((target.oper == WRITER) && TST_wFST_TIME(args->test_state)) {
			*(tmpLBA) = args->start_lba + args->offset;
		} else if ((target.oper == READER)
			   && TST_rFST_TIME(args->test_state)) {
			*(tmpLBA) = args->start_lba + args->offset;
		} else if ((TST_DIRCTN(args->test_state))
			   && ((*(tmpLBA) + (target.trsiz - 1)) <=
			       args->stop_lba)) {
		} else if (!(TST_DIRCTN(args->test_state))
			   && (*(tmpLBA) >= (args->start_lba + args->offset))) {
		} else {
			if (args->flags & CLD_FLG_LUNU) {
				*(tmpLBA) = args->start_lba + args->offset;
				if ((args->flags & CLD_FLG_CYC)
				    && (target.oper == WRITER)) {
					target.oper = NONE;
				}
			} else if (args->flags & CLD_FLG_LUND) {
				args->test_state = DIRCT_CNG(args->test_state);
				direct =
				    (TST_DIRCTN(args->test_state)) ? 1 : -1;
				*(tmpLBA) +=
				    (OFF_T) direct *(OFF_T) target.trsiz;
				if ((args->flags & CLD_FLG_CYC) && (direct > 0)) {
					target.oper = NONE;
				}
			}
		}
		target.lba = *(tmpLBA);
	} else if (args->flags & CLD_FLG_RANDOM) {
		if ((args->flags & CLD_FLG_NTRLVD)
		    && (args->flags & CLD_FLG_W)
		    && (args->flags & CLD_FLG_R)
		    && (target.oper == READER)) {
			target.lba = env->lastAction.lba;
		} else {
			do {
				target.lba =
				    (Rand64() & mask) + args->start_lba;
			} while (target.lba > args->stop_lba);

			guessLBA =
			    ALIGN(target.lba, target.trsiz) + args->offset;
			if (guessLBA > args->stop_lba) {
				target.lba = guessLBA = args->stop_lba;
			}
			if (target.lba != guessLBA) {
				if ((target.lba - guessLBA) <=
				    ((guessLBA + target.trsiz) - target.lba)) {
					target.lba = guessLBA;
				} else if ((guessLBA + target.trsiz) >
					   args->stop_lba) {
					target.lba = guessLBA;
				} else {
					target.lba = guessLBA + target.trsiz;
				}
			}
			if ((target.lba + (target.trsiz - 1)) > args->stop_lba) {
				target.lba -= target.trsiz;
			}
		}
	}
	if ((args->flags & CLD_FLG_LBA_SYNC) && (action_in_use(env, target))) {
		target.oper = RETRY;
	}

	if (!(args->flags & CLD_FLG_NTRLVD)
	    && !(args->flags & CLD_FLG_RANDOM)
	    && (args->flags & CLD_FLG_W)
	    && (args->flags & CLD_FLG_R)) {
		if (((target.oper == WRITER) ? env->wcount : env->rcount) >=
		    (args->seeks / 2)) {
			target.oper = NONE;
		}
	}

	/* get out if exceeded one of the following */
	if ((args->flags & CLD_FLG_SKS)
	    && (((env->wcount) + (env->rcount)) >= args->seeks)) {
		target.oper = NONE;
	}

	/*
	 * check the bitmask to see if we can read,
	 * if the bitmask is set for the block of LBAs,
	 * then we are OK to read
	 *
	 * only matters of error checking or write once
	 */
	blk_written = 1;
	if (args->flags & (CLD_FLG_CMPR | CLD_FLG_WRITE_ONCE)) {
		for (i = 0; i < target.trsiz; i++) {
			if ((*
			     (wbitmap +
			      (((target.lba - args->offset - args->start_lba) +
				i) / 8)) & (0x80 >> (((target.lba -
						       args->offset -
						       args->start_lba) +
						      i) % 8))) == 0) {
				blk_written = 0;
				break;
			}
		}
	}

	/* get out, nothing to do */
	if ((target.oper == NONE) || (target.oper == RETRY)) ;
	/* get out, read only, or not comparing */
	else if (!(args->flags & CLD_FLG_W)) ;
	/* get out, we are a writer, write once enabled, and block not written */
	else if ((target.oper == WRITER) && (args->flags & CLD_FLG_WRITE_ONCE)
		 && !blk_written) ;
	/* get out, we are a writer and not write once */
	else if ((target.oper == WRITER)
		 && !(args->flags & CLD_FLG_WRITE_ONCE)) ;
	/* get out, we are a reader, and blocks written */
	else if ((target.oper == READER) && blk_written) ;
	else if ((args->flags & CLD_FLG_LINEAR)
		 || ((args->flags & CLD_FLG_NTRLVD)
		     && (args->flags & CLD_FLG_RANDOM))) {
		if (!blk_written) {
			/*
			 * if we are linear and not interleaved and on the read pass
			 * with random transfer sizes, and we hit the limit of the
			 * random write transfer lengths, because blk_written was
			 * false, then we cannot do any more reads unless we start
			 * over at start_lba+offset.
			 */
			if ((args->flags & CLD_FLG_LINEAR) &&
			    !(args->flags & CLD_FLG_NTRLVD) &&
			    (args->flags & CLD_FLG_RTRSIZ) &&
			    (target.oper == READER)) {
				tmpLBA = pVal1 + OFF_RLBA;
				*(tmpLBA) = args->start_lba + args->offset;
				target.lba = *(tmpLBA);
			} else {
				/*
				 * we must retry, as we can't start the read, since the write
				 * has not happened yet.
				 */
				target.oper = RETRY;
			}
		}
	} else if ((target.oper == READER) && (args->flags & CLD_FLG_CMPR)
		   && !blk_written) {
		/* should have been a random reader, but blk not written, and running with compare, so make me a writer */
		target.oper = WRITER;
		args->test_state = SET_OPER_W(args->test_state);
		/* if we switched to a writer, then we have to check action_in_use again */
		if ((args->flags & CLD_FLG_LBA_SYNC)
		    && (action_in_use(env, target))) {
			target.oper = RETRY;
		}
	} else {
		/* should have been a random writer, but blk already written, so make me a reader */
		target.oper = READER;
		args->test_state = SET_OPER_R(args->test_state);
		/* if we switched to a reader, then no need to check action_in_use again */
	}

#ifdef _DEBUG
#ifdef WINDOWS
	PDBG5(DBUG, args, "%I64d, %I64d, %I64d, %I64d\n", env->wcount,
	      env->rcount, args->seeks, args->stop_lba);
#else
	PDBG5(DBUG, args, "%lld, %lld, %lld, %lld\n", env->wcount, env->rcount,
	      args->seeks, args->stop_lba);
#endif
#endif

	if (target.oper == WRITER) {
		(env->wcount)++;
		if ((args->flags & CLD_FLG_LUND))
			*(pVal1 + OFF_RLBA) = *(pVal1 + OFF_WLBA);
		*(pVal1 + OFF_WLBA) += (OFF_T) direct *(OFF_T) target.trsiz;
		if (TST_wFST_TIME(args->test_state))
			args->test_state = CLR_wFST_TIME(args->test_state);
		env->lastAction = target;
		if (args->flags & CLD_FLG_LBA_SYNC) {
			add_action(env, args, target);
		}
	}
	if (target.oper == READER) {
		(env->rcount)++;
		*(pVal1 + OFF_RLBA) += (OFF_T) direct *(OFF_T) target.trsiz;
		if (TST_rFST_TIME(args->test_state))
			args->test_state = CLR_rFST_TIME(args->test_state);
		env->lastAction = target;
		if (args->flags & CLD_FLG_LBA_SYNC) {
			add_action(env, args, target);
		}
	}

	return target;
}

void miscompare_dump(const child_args_t * args, const char *data,
		     const size_t buf_len, OFF_T tPosition, const size_t offset,
		     mc_func_t oper, const int this_thread_id)
{
	FILE *fpDumpFile;
	char obuff[80];

	obuff[0] = 0;
	sprintf(obuff, "dump_%d.dat", args->pid);
	fpDumpFile = fopen(obuff, "a");

	if (oper == EXP) {
		if (fpDumpFile)
			fprintf(fpDumpFile, "\n\n\n");
		if (fpDumpFile)
			fprintf(fpDumpFile, "Execution string: %s\n",
				args->argstr);
		if (fpDumpFile)
			fprintf(fpDumpFile, "Target: %s\n", args->device);
		if (fpDumpFile)
			fprintf(fpDumpFile, DMSTR, this_thread_id, tPosition,
				tPosition);
		if (fpDumpFile)
			fprintf(fpDumpFile, DMOFFSTR, this_thread_id, offset,
				offset);
		pMsg(ERR, args, "EXPECTED:\n");
		if (fpDumpFile)
			fprintf(fpDumpFile, DMFILESTR, "EXPECTED", args->device,
				tPosition, offset);
	} else if (oper == ACT) {
		pMsg(ERR, args, "ACTUAL:\n");
		if (fpDumpFile)
			fprintf(fpDumpFile, DMFILESTR, "ACTUAL", args->device,
				tPosition, offset);
	} else if (oper == REREAD) {
		pMsg(ERR, args, "REREAD ACTUAL:\n");
		if (fpDumpFile)
			fprintf(fpDumpFile, DMFILESTR, "REREAD ACTUAL",
				args->device, tPosition, offset);
	}

	dump_data(stdout, data, 16, 16, offset, FMT_STR);
	if (fpDumpFile)
		dump_data(fpDumpFile, data, buf_len, 16, 0, FMT_STR);
	if (fpDumpFile)
		fclose(fpDumpFile);
}

/*
 * called after all the checks have been made to verify
 * that the io completed successfully.
 */
void complete_io(test_env_t * env, const child_args_t * args,
		 const action_t target)
{
	unsigned char *wbitmap = (unsigned char *)env->shared_mem + BMP_OFFSET;
	int i = 0;

	if (target.oper == WRITER) {
		(env->hbeat_stats.wbytes) += target.trsiz * BLK_SIZE;
		env->hbeat_stats.wcount++;
		for (i = 0; i < target.trsiz; i++) {
			*(wbitmap +
			  (((target.lba - args->offset - args->start_lba) +
			    i) / 8)) |=
		  0x80 >> (((target.lba - args->offset - args->start_lba) + i) %
			   8);
		}
	} else {
		(env->hbeat_stats.rbytes) += target.trsiz * BLK_SIZE;
		env->hbeat_stats.rcount++;
	}
	if (args->flags & CLD_FLG_LBA_SYNC) {
		remove_action(env, target);
	}
}

/*
* This function is really the main function for a thread
* Once here, this function will act as if it
* were 'main' for that thread.
*/
#ifdef WINDOWS
DWORD WINAPI ChildMain(test_ll_t * test)
#else
void *ChildMain(void *vtest)
#endif
{
#ifndef WINDOWS
	test_ll_t *test = (test_ll_t *) vtest;
#endif

	child_args_t *args = test->args;
	test_env_t *env = test->env;

	static int thread_id = 0;
	int this_thread_id = thread_id++;
	char *buf1 = NULL, *buffer1 = NULL;	/* 'buf' is the aligned 'buffer' */
	char *buf2 = NULL, *buffer2 = NULL;	/* 'buf' is the aligned 'buffer' */
	unsigned long ulLastError;
	unsigned long delayTime;

	action_t target = { NONE, 0, 0 };
	unsigned int i;
	OFF_T ActualBytePos = 0, TargetBytePos = 0, mask = 1, delayMask = 1;
	long tcnt = 0;
	int exit_code = 0, rv = 0;
	char filespec[DEV_NAME_LEN];
	fd_t fd;

	unsigned int retries = 0;
	BOOL is_retry = FALSE;
	lvl_t msg_level = WARN;
	int SET_CHAR = 0;	/* when data buffers are cleared, using memset, use this */

	extern unsigned long glb_flags;
	extern unsigned short glb_run;
	extern int signal_action;

#ifdef WINDOWS
	HANDLE MutexMISCOMP;

	if ((MutexMISCOMP = OpenMutex(SYNCHRONIZE, TRUE, "gbl")) == NULL) {
		pMsg(ERR, args,
		     "Thread %d: Failed to open semaphore, error = %u\n",
		     this_thread_id, GetLastError());
		args->test_state = SET_STS_FAIL(args->test_state);
		TEXIT(GETLASTERROR());
	}
#else
	static pthread_mutex_t MutexMISCOMP = PTHREAD_MUTEX_INITIALIZER;
#endif

	/*
	 * For some messages, the error level will change, based on if
	 * the test should continue on error, or stop on error.
	 */
	if ((args->flags & CLD_FLG_ALLDIE) || (glb_flags & GLB_FLG_KILL)) {
		msg_level = ERR;
	}

	target.oper = TST_OPER(args->test_state);

	strncpy(filespec, args->device, DEV_NAME_LEN);

	fd = Open(filespec, args->flags);
	if (INVALID_FD(fd)) {
		pMsg(ERR, args, "Thread %d: could not open %s, errno = %u.\n",
		     this_thread_id, args->device, GETLASTERROR());
		args->test_state = SET_STS_FAIL(args->test_state);
		TEXIT((uintptr_t) GETLASTERROR());
	}

	/* Create aligned memory buffers for sending IO. */
	if ((buffer1 =
	     (char *)ALLOC(((args->htrsiz * BLK_SIZE) + ALIGNSIZE))) == NULL) {
		pMsg(ERR, args,
		     "Thread %d: Memory allocation failure for IO buffer, errno = %u\n",
		     this_thread_id, GETLASTERROR());
		args->test_state = SET_STS_FAIL(args->test_state);
		CLOSE(fd);
		TEXIT((uintptr_t) GETLASTERROR());
	}
	memset(buffer1, SET_CHAR, ((args->htrsiz * BLK_SIZE) + ALIGNSIZE));
	buf1 = (char *)BUFALIGN(buffer1);

	if ((buffer2 =
	     (char *)ALLOC(((args->htrsiz * BLK_SIZE) + ALIGNSIZE))) == NULL) {
		pMsg(ERR, args,
		     "Thread %d: Memory allocation failure for IO buffer, errno = %u\n",
		     this_thread_id, GETLASTERROR());
		FREE(buffer1);
		args->test_state = SET_STS_FAIL(args->test_state);
		CLOSE(fd);
		TEXIT((uintptr_t) GETLASTERROR());
	}
	memset(buffer2, SET_CHAR, ((args->htrsiz * BLK_SIZE) + ALIGNSIZE));
	buf2 = (char *)BUFALIGN(buffer2);

	/*  set up lba mask of all 1's with value between vsiz and 2*vsiz */
	while (mask <= (args->stop_lba - args->start_lba)) {
		mask = mask << 1;
	}
	mask -= 1;

	/*  set up delay mask of all 1's with value between delayTimeMin and 2*delayTimeMax */
	while (delayMask <= (args->delayTimeMax - args->delayTimeMin)) {
		delayMask = delayMask << 1;
	}
	delayMask -= 1;

	while (env->bContinue) {
		if (!is_retry) {
			retries = args->retries;
#ifdef _DEBUG
			PDBG5(DBUG, args,
			      "Thread %d: lastAction: oper: %d, lba: %lld, trsiz: %ld\n",
			      this_thread_id, target.oper, target.lba,
			      target.trsiz);
#endif
			do {
				if (signal_action & SIGNAL_STOP) {
					break;
				}	/* user request to stop */
				if (glb_run == 0) {
					break;
				}	/* global request to stop */
				LOCK(env->mutexs.MutexACTION);
				target = get_next_action(args, env, mask);
				UNLOCK(env->mutexs.MutexACTION);
				/* this thread has to retry, so give up the reset of my time slice */
				if (target.oper == RETRY) {
					Sleep(0);
				}
			} while ((env->bContinue) && (target.oper == RETRY));	/* we failed to get an action, and were asked to retry */

#ifdef _DEBUG
			PDBG5(DBUG, args,
			      "Thread %d: nextAction: oper: %d, lba: %lld, trsiz: %ld\n",
			      this_thread_id, target.oper, target.lba,
			      target.trsiz);
#endif

			/*
			 * Delay delayTime msecs before continuing, for simulated
			 * processing time, requested by user
			 */

			if (args->delayTimeMin == args->delayTimeMax) {	/* static delay time */
				/* only sleep if delay is greater then zero */
				if (args->delayTimeMin > 0) {
					Sleep(args->delayTimeMin);
				}
			} else {	/* random delay time between min & max */
				do {
					delayTime =
					    (unsigned long)(rand() & delayMask)
					    + args->delayTimeMin;
				} while (delayTime > args->delayTimeMax);
#ifdef _DEBUG
				PDBG3(DBUG, args,
				      "Thread %d: Delay time = %lu\n",
				      this_thread_id, delayTime);
#endif
				Sleep(delayTime);
			}
		}
#ifdef _DEBUG
		if (target.oper == NONE) {	/* nothing left to do */
			PDBG3(DBUG, args,
			      "Thread %d: Setting break, oper is NONE\n",
			      this_thread_id);
		}
#endif

		if (target.oper == NONE) {
			break;
		}		/* nothing left so stop */
		if (signal_action & SIGNAL_STOP) {
			break;
		}		/* user request to stop */
		if (env->bContinue == FALSE) {
			break;
		}		/* internal request to stop */
		if (glb_run == 0) {
			break;
		}
		/* global request to stop */
		TargetBytePos = (OFF_T) (target.lba * BLK_SIZE);
		ActualBytePos = Seek(fd, TargetBytePos);
		if (ActualBytePos != TargetBytePos) {
			ulLastError = GETLASTERROR();
			pMsg(msg_level, args, SFSTR, this_thread_id,
			     (target.oper ==
			      WRITER) ? (env->wcount) : (env->rcount),
			     target.lba, TargetBytePos, ActualBytePos,
			     ulLastError);
			if (retries-- > 1) {	/* request to retry on error, decrement retry */
				pMsg(INFO, args,
				     "Thread %d: Retry after seek failure, retry count: %u\n",
				     this_thread_id, retries);
				is_retry = TRUE;
				Sleep(args->retry_delay);
			} else {
				exit_code = SEEK_FAILURE;
				is_retry = FALSE;
				LOCK(env->mutexs.MutexACTION);
				update_test_state(args, env, this_thread_id, fd,
						  buf2);
				decrement_io_count(args, env, target);
				UNLOCK(env->mutexs.MutexACTION);
			}
			continue;
		}

		if (target.oper == WRITER) {
			if (args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba),
					    sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, env->data_buffer,
				       target.trsiz * BLK_SIZE);
			}
			if (args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz * BLK_SIZE,
					    &(target.lba), args, env);
			}
#ifdef _DEBUG
			setStartTime();
#endif
			if (args->flags & CLD_FLG_IO_SERIAL) {
				LOCK(env->mutexs.MutexIO);
				tcnt = Write(fd, buf2, target.trsiz * BLK_SIZE);
				UNLOCK(env->mutexs.MutexIO);
			} else {
				tcnt = Write(fd, buf2, target.trsiz * BLK_SIZE);
			}

#ifdef _DEBUG
			setEndTime();
			PDBG5(DBUG, args, "Thread %d: I/O Time: %ld usecs\n",
			      this_thread_id, getTimeDiff());
#endif
			if (args->flags & CLD_FLG_WFSYNC) {
				rv = 0;
				/* if need to sync, then only have one thread do it */
				LOCK(env->mutexs.MutexACTION);
				if (0 ==
				    (env->hbeat_stats.wcount %
				     args->sync_interval)) {
#ifdef _DEBUG
					PDBG3(DBUG, args,
					      "Thread %d: Performing sync, write IO count %llu\n",
					      this_thread_id,
					      env->hbeat_stats.wcount);
#endif
					rv = Sync(fd);
					if (0 != rv) {
						exit_code = GETLASTERROR();
						pMsg(msg_level, args,
						     "Thread %d: fsync error = %d\n",
						     this_thread_id, exit_code);
						is_retry = FALSE;
						update_test_state(args, env,
								  this_thread_id,
								  fd, buf2);
						decrement_io_count(args, env,
								   target);
					}
				}
				UNLOCK(env->mutexs.MutexACTION);

				if (0 != rv) {	/* sync error, so don't count the write */
					continue;
				}
			}
		}

		if (target.oper == READER) {
			memset(buf1, SET_CHAR, target.trsiz * BLK_SIZE);
#ifdef _DEBUG
			setStartTime();
#endif
			if (args->flags & CLD_FLG_IO_SERIAL) {
				LOCK(env->mutexs.MutexIO);
				tcnt = Read(fd, buf1, target.trsiz * BLK_SIZE);
				UNLOCK(env->mutexs.MutexIO);
			} else {
				tcnt = Read(fd, buf1, target.trsiz * BLK_SIZE);
			}
#ifdef _DEBUG
			setEndTime();
			PDBG5(DBUG, args, "Thread %d: I/O Time: %ld usecs\n",
			      this_thread_id, getTimeDiff());
#endif
		}

		if (tcnt != (long)target.trsiz * BLK_SIZE) {
			ulLastError = GETLASTERROR();
			pMsg(msg_level, args, AFSTR, this_thread_id,
			     (target.oper) ? "Read" : "Write",
			     (target.oper) ? (env->rcount) : (env->wcount),
			     target.lba, target.lba, tcnt,
			     target.trsiz * BLK_SIZE, ulLastError);
			if (retries-- > 1) {	/* request to retry on error, decrement retry */
				pMsg(INFO, args,
				     "Thread %d: Retry after transfer failure, retry count: %u\n",
				     this_thread_id, retries);
				is_retry = TRUE;
				Sleep(args->retry_delay);
			} else {
				exit_code = ACCESS_FAILURE;
				is_retry = FALSE;
				LOCK(env->mutexs.MutexACTION);
				update_test_state(args, env, this_thread_id, fd,
						  buf2);
				decrement_io_count(args, env, target);
				UNLOCK(env->mutexs.MutexACTION);
			}
			continue;
		}

		/* data compare routine.  Act as if we were to write, but just compare */
		if ((target.oper == READER) && (args->flags & CLD_FLG_CMPR)) {
			/* This is very SLOW!!! */
			if ((args->cmp_lng == 0)
			    || (args->cmp_lng > target.trsiz * BLK_SIZE)) {
				args->cmp_lng = target.trsiz * BLK_SIZE;
			}
			if (args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba),
					    sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, env->data_buffer,
				       target.trsiz * BLK_SIZE);
			}
			if (args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz * BLK_SIZE,
					    &(target.lba), args, env);
			}
			if (memcmp(buf2, buf1, args->cmp_lng) != 0) {
				/* data miscompare, this takes lots of time, but its OK... !!! */
				LOCK(MutexMISCOMP);
				pMsg(ERR, args, DMSTR, this_thread_id,
				     target.lba, target.lba);
				/* find the actual byte that started the miscompare */
				for (i = 0; i < args->htrsiz * BLK_SIZE; i++) {
					if (*(buf2 + i) != *(buf1 + i)) {
						pMsg(ERR, args, DMOFFSTR,
						     this_thread_id, i, i);
						break;
					}
				}
				miscompare_dump(args, buf2,
						args->htrsiz * BLK_SIZE,
						target.lba, i, EXP,
						this_thread_id);
				miscompare_dump(args, buf1,
						args->htrsiz * BLK_SIZE,
						target.lba, i, ACT,
						this_thread_id);
				/* perform a reread of the target, if requested */
				if (args->flags & CLD_FLG_ERR_REREAD) {
					ActualBytePos = Seek(fd, TargetBytePos);
					if (ActualBytePos == TargetBytePos) {
						memset(buf1, SET_CHAR,
						       target.trsiz * BLK_SIZE);
#ifdef _DEBUG
						setStartTime();
#endif
						tcnt =
						    Read(fd, buf1,
							 target.trsiz *
							 BLK_SIZE);
#ifdef _DEBUG
						setEndTime();
						PDBG5(DBUG, args,
						      "Thread %d: ReRead I/O Time: %ld usecs\n",
						      this_thread_id,
						      getTimeDiff());
#endif
						if (tcnt !=
						    (long)target.trsiz *
						    BLK_SIZE) {
							pMsg(ERR, args,
							     "Thread %d: ReRead after data miscompare failed on transfer.\n",
							     this_thread_id);
							pMsg(ERR, args, AFSTR,
							     this_thread_id,
							     "ReRead",
							     (target.
							      oper) ? (env->
								       rcount)
							     : (env->wcount),
							     target.lba,
							     target.lba, tcnt,
							     target.trsiz *
							     BLK_SIZE);
						}
						miscompare_dump(args, buf1,
								args->htrsiz *
								BLK_SIZE,
								target.lba, i,
								REREAD,
								this_thread_id);
					} else {
						pMsg(ERR, args,
						     "Thread %d: ReRead after data miscompare failed on seek.\n",
						     this_thread_id);
						pMsg(ERR, args, SFSTR,
						     this_thread_id,
						     (target.oper ==
						      WRITER) ? (env->
								 wcount)
						     : (env->rcount),
						     target.lba, TargetBytePos,
						     ActualBytePos);
					}
				}
				UNLOCK(MutexMISCOMP);

				exit_code = DATA_MISCOMPARE;
				is_retry = FALSE;
				LOCK(env->mutexs.MutexACTION);
				update_test_state(args, env, this_thread_id, fd,
						  buf2);
				decrement_io_count(args, env, target);
				UNLOCK(env->mutexs.MutexACTION);
				continue;
			}
		}

		/* update stats, bitmap, and release LBA */
		LOCK(env->mutexs.MutexACTION);
		complete_io(env, args, target);
		UNLOCK(env->mutexs.MutexACTION);

		is_retry = FALSE;
	}

#ifdef _DEBUG
#ifdef _DEBUG_PRINTMAP
	LOCK(env->mutexs.MutexACTION);
	print_lba_bitmap(env);
	UNLOCK(env->mutexs.MutexACTION);
#endif
#endif

	FREE(buffer1);
	FREE(buffer2);

	if ((args->flags & CLD_FLG_W) && !(args->flags & CLD_FLG_RAW)) {
#ifdef _DEBUG
		PDBG5(DBUG, args, "Thread %d: starting sync\n", this_thread_id);
#endif
		if (Sync(fd) < 0) {	/* just sync, should not matter the device type */
			exit_code = GETLASTERROR();
			pMsg(ERR, args, "Thread %d: fsync error = %d\n",
			     this_thread_id, exit_code);
			args->test_state = SET_STS_FAIL(args->test_state);
		}
#ifdef _DEBUG
		PDBG5(DBUG, args, "Thread %d: finished sync\n", this_thread_id);
#endif
	}

	if (CLOSE(fd) < 0) {	/* check return status on close */
		exit_code = GETLASTERROR();
		pMsg(ERR, args, "Thread %d: close error = %d\n", this_thread_id,
		     exit_code);
		args->test_state = SET_STS_FAIL(args->test_state);
	}

	TEXIT((uintptr_t) exit_code);
}
