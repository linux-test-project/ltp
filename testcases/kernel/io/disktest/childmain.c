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
* $Id: childmain.c,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: childmain.c,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
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
#ifdef WIN32
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

action_t get_next_action(child_args_t *args, const time_t end_time, const OFF_T mask)
{
	extern void *shared_mem;	/* global pointer to shared memory */
	extern OFF_T pass_count;	/* current pass */
	
	OFF_T *pVal1 = (OFF_T *)shared_mem;
	OFF_T *test_state = (OFF_T *)shared_mem + OFF_TST_STAT;
	OFF_T *wcount = (OFF_T *)shared_mem + OFF_WCOUNT;
	OFF_T *rcount = (OFF_T *)shared_mem + OFF_RCOUNT;
	OFF_T *tmpLBA;
	unsigned char *wbitmap = (unsigned char *)shared_mem + BMP_OFFSET;

	short blk_written = 1;
	unsigned long i;
	action_t target;
	short direct = 0;
	BOOL bChangeState = FALSE;

	/* pick an operation */
	if(args->flags & CLD_FLG_RANDOM) {
		if((((*wcount)*100)/(((*rcount)+1)+(*wcount))) >= (args->wperc)) {
			*test_state = SET_OPER_R(*test_state);
		} else {
			*test_state = SET_OPER_W(*test_state);
		}
	} else if((args->flags & CLD_FLG_NTRLVD) && !TST_wFST_TIME(*test_state)) {
		if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W))
			*test_state = CNG_OPER(*test_state);
	}
	target.oper = TST_OPER(*test_state);

	/* pick a transfer length */
	if(!(args->flags & CLD_FLG_RTRSIZ)) {
		target.trsiz = args->ltrsiz;
	} else {
		do {
			if((args->flags & CLD_FLG_SKS) && (((*wcount)+(*rcount)) >= (args->seeks*pass_count)))
				break;
			if((args->flags & CLD_FLG_TMD) && (time(NULL) > end_time))
				break;
			target.trsiz = (rand()&0xFF) + args->ltrsiz;
		} while(target.trsiz > args->htrsiz);
	}

	/* pick an lba */
	if(args->start_lba == args->stop_lba) { /* diskcache test */
		target.lba = args->start_lba;
	} else if (args->flags & CLD_FLG_LINEAR) {
			tmpLBA = (target.oper == WRITER) ? pVal1+OFF_WLBA : pVal1+OFF_RLBA;
		direct = (TST_DIRCTN(*test_state)) ? 1 : -1;
		if((target.oper == WRITER) && TST_wFST_TIME(*test_state)) {
			*(tmpLBA) = args->start_lba;
		} else if((target.oper == READER) && TST_rFST_TIME(*test_state)) {
			*(tmpLBA) = args->start_lba;
		} else if((TST_DIRCTN(*test_state)) && ((*(tmpLBA)+target.trsiz) <= args->stop_lba)) {
		} else if(!(TST_DIRCTN(*test_state)) && (*(tmpLBA) >= args->start_lba)) {
		} else {
			bChangeState = TRUE;
		}
		if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
			if(((target.oper == WRITER) && ((*wcount) >= ((args->seeks/2)*pass_count))) ||
				((target.oper == READER) && ((*rcount) >= ((args->seeks/2)*pass_count)))) {
				bChangeState = TRUE;
			}
		}
		if(bChangeState) {			
			if (args->flags & CLD_FLG_LUNU) {
				*(tmpLBA) = args->start_lba;
				if(!(args->flags & CLD_FLG_NTRLVD)) {
					if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W))
						*test_state = CNG_OPER(*test_state);
					target.oper = TST_OPER(*test_state);
				}
				if((target.oper == WRITER) && (args->flags & CLD_FLG_CYC)) {
					target.oper = NONE;
				}
			} else if (args->flags & CLD_FLG_LUND) {
				*test_state = DIRCT_CNG(*test_state);
				direct = (TST_DIRCTN(*test_state)) ? 1 : -1;
				if(!(args->flags & CLD_FLG_NTRLVD)) {
					if((args->flags & CLD_FLG_R) && (args->flags & CLD_FLG_W)) {
						*test_state = CNG_OPER(*test_state);
						target.oper = TST_OPER(*test_state);
						tmpLBA = (target.oper == WRITER) ? pVal1+OFF_WLBA : pVal1+OFF_RLBA;
					} else {
						*(tmpLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
					}
				} else {
					*(tmpLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
				}
				if((direct > 0) && (args->flags & CLD_FLG_CYC)) {
					target.oper = NONE;
				}
			}
		}
		target.lba = *(tmpLBA);
	}
	else if (args->flags & CLD_FLG_RANDOM) {
		do {
			if((args->flags & CLD_FLG_SKS) && (((*wcount)+(*rcount)) >= (args->seeks*pass_count)))
				break;
			if((args->flags & CLD_FLG_TMD) && (time(NULL) > end_time))
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
			(((*(rcount)*100)/((args->seeks+1)*pass_count)) >= 80)) {
				target.oper = NONE;
		}
	}

	/* get out if exceeded one of the following */
	if((args->flags & CLD_FLG_SKS) && (((*wcount)+(*rcount)) >= (args->seeks*pass_count))) {
		target.oper = NONE;
	}
	if((args->flags & CLD_FLG_TMD) && (time(NULL) > end_time)) {
		target.oper = NONE;
	}

	if((target.oper == READER) && (args->flags & CLD_FLG_CMPR) && (args->flags & CLD_FLG_W)) {
		for(i=0;i<target.trsiz;i++) {
			if((*(wbitmap+((target.lba+i)/8))&(0x80>>((target.lba+i)%8))) == 0) {
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
			while((*(wbitmap+((target.lba+i)/8))&(0x80>>((target.lba+i)%8))) == 0)
				Sleep(10);
	else {
		/* should have been a random reader, but blk not written, so make me a writer */
		target.oper = WRITER;
		*test_state = SET_OPER_W(*test_state);
	}

#ifdef _DEBUG
#ifdef WIN32
	printf("%I64d, %I64d, %I64d, %I64d, %I64d\n", *(wcount), *(rcount), args->seeks, args->stop_lba, pass_count);
#else
	printf("%lld, %lld, %lld, %lld, %lld\n", *(wcount), *(rcount), args->seeks, args->stop_lba, pass_count);
#endif
#endif

	if(target.oper == WRITER) {
		(*wcount)++;
		if((args->flags & CLD_FLG_LUND))
			*(pVal1+OFF_RLBA) = *(pVal1+OFF_WLBA);
		*(pVal1+OFF_WLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
		if(TST_wFST_TIME(*test_state)) *test_state = CLR_wFST_TIME(*test_state);
	}
	if(target.oper == READER) {
		(*rcount)++;
		*(pVal1+OFF_RLBA) += (OFF_T) direct * (OFF_T) target.trsiz;
		if(TST_rFST_TIME(*test_state)) *test_state = CLR_rFST_TIME(*test_state);
	}
	return target;
}

/*
* This function is really the main function for a child
* Once here, this function will act as if it
* were 'main' for that child, and therefore a child will
* never return from this function, only exit.
* args is shared so the children should NEVER update them.
*/
#ifdef WIN32
DWORD WINAPI ChildMain(child_args_t *args)
#else
void *ChildMain(void *vargs)
#endif
{
	extern void *shared_mem;			/* global pointer to shared memory */
	extern BOOL bContinue;				/* global that when set to false will force exit of all threads */
	extern OFF_T pass_count;			/* current pass */
	extern unsigned char *data_buffer;	/* global pointer to shared memory */

#ifndef WIN32
	static pthread_mutex_t global = PTHREAD_MUTEX_INITIALIZER;
	static pthread_mutex_t data = PTHREAD_MUTEX_INITIALIZER;
	child_args_t *args = (child_args_t *) vargs;
#endif
	unsigned char *buf1 = NULL, *buffer1 = NULL; /* 'buf' is the aligned 'buffer' */
	unsigned char *buf2 = NULL, *buffer2 = NULL; /* 'buf' is the aligned 'buffer' */
	OFF_T *wcount = (OFF_T *)shared_mem + OFF_WCOUNT;
	OFF_T *rcount = (OFF_T *)shared_mem + OFF_RCOUNT;
	OFF_T *rbytes = (OFF_T *)shared_mem + OFF_RBYTES;
	OFF_T *wbytes = (OFF_T *)shared_mem + OFF_WBYTES;
	OFF_T *test_state = (OFF_T *)shared_mem + OFF_TST_STAT;
	unsigned char *wbitmap = (unsigned char *)shared_mem + BMP_OFFSET;

	time_t end_time = 0;
	action_t target = { NONE, -1, 0};
	unsigned int i;
	OFF_T lba2=0, lba3=0, mask=1;
	long tcnt=0;
	int exit_code=0;

#ifdef WIN32
	DWORD OPEN_FLAGS = FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
	DWORD OPEN_DISPO = OPEN_EXISTING;
	HANDLE hFileHandle, hSemGBL, hSemDATA;

	if(args->flags & CLD_FLG_RANDOM) OPEN_FLAGS |= FILE_FLAG_RANDOM_ACCESS;
	if(args->flags & CLD_FLG_LINEAR) OPEN_FLAGS |= FILE_FLAG_SEQUENTIAL_SCAN;
	if(args->flags & CLD_FLG_FILE) {
		OPEN_FLAGS |= FILE_ATTRIBUTE_ARCHIVE;
		OPEN_DISPO = OPEN_ALWAYS;
	}

	if((hSemGBL = OpenMutex(SYNCHRONIZE, TRUE, "gbl")) == NULL) {
		pMsg(ERR, "Failed to open semaphore, error = %u\n", GetLastError());
		exit(GetLastError());
	}
	if((hSemDATA = OpenMutex(SYNCHRONIZE, TRUE, "data")) == NULL) {
		pMsg(ERR, "Failed to open semaphore, error = %u\n", GetLastError());
		exit(GetLastError());
	}

	hFileHandle = CreateFile(args->device, 
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_DISPO,
						OPEN_FLAGS,
						NULL);

	if(hFileHandle == INVALID_HANDLE_VALUE) {
		pMsg(ERR, "Error = %u\n",GetLastError());
		ExitThread(GetLastError());
		return(GetLastError());
	}
#else
	int fd = -1; /* file discipter */
#ifdef _AIX
	int OPEN_MASK = O_RDWR|O_LARGEFILE; /* RDWR and LARGEFILE */
	if(args->flags & CLD_FLG_FILE) OPEN_MASK |= O_CREAT;
	if(args->flags & CLD_FLG_DIRECT) OPEN_MASK |= O_DIRECT;
#else
	int OPEN_MASK = 02|0100000; /* O_RDWR and O_LARGEFILE */
	if(args->flags & CLD_FLG_FILE) OPEN_MASK |= 0100; /* O_CREAT */
	if(args->flags & CLD_FLG_DIRECT) OPEN_MASK |= 040000; /* O_DIRECT Linux */
#endif
	if((fd = open(args->device,OPEN_MASK,00600)) == -1) {
		*test_state = SET_STS_FAIL(*test_state);
		pMsg(ERR, "could not open %s.\n",args->device);
		perror(args->device);
		exit(errno);
	}
	pthread_cleanup_push(clean_up, &exit_code);
#endif

	/* Create aligned memory buffers for sending IO. */
	if ((buffer1 = (unsigned char *) malloc((T_MAX_SIZE+ALIGNSIZE))) == NULL) {
		perror("allocation failed, buffer1");
		*test_state = SET_STS_FAIL(*test_state);
		exit(errno);
	}
	buf1 = (unsigned char *) BUFALIGN(buffer1);

	if ((buffer2 = (unsigned char *) malloc((T_MAX_SIZE+ALIGNSIZE))) == NULL) {
		perror("allocation failed, buffer2");
		*test_state = SET_STS_FAIL(*test_state);
		exit(errno);
	}
	buf2 = (unsigned char *) BUFALIGN(buffer2);

	/*  set up lba mask of all 1's with value between vsiz and 2*vsiz */
	while(mask <= args->stop_lba) {	mask = mask<<1; }
	mask -= 1;

	if(args->flags & CLD_FLG_TMD) {
		end_time = time(NULL) + args->run_time;
	}

	for(;;) {
		if(!bContinue) break;	/* someone said to die */
#ifdef WIN32
		WaitForSingleObject(hSemGBL, INFINITE);
#else
		pthread_mutex_lock(&global);
#endif
		target = get_next_action(args, end_time, mask);

#ifdef _DEBUG
#ifdef WIN32
		printf("%s, %I64d, %lu\n", (target.oper) ? "READ" : "WRITE", target.lba, (target.trsiz*BLK_SIZE));
#else
		printf("%s, %lld, %lu\n", (target.oper) ? "READ" : "WRITE", target.lba, (target.trsiz*BLK_SIZE));
#endif
#endif

#ifdef WIN32
		ReleaseMutex(hSemGBL);
#else
		pthread_mutex_unlock(&global);
#endif

		if(target.oper == NONE) break;

		lba3=(OFF_T) (target.lba*BLK_SIZE);
#ifdef WIN32
		lba2=FileSeek64(hFileHandle, lba3, FILE_BEGIN);
#else
		lba2=(OFF_T) lseek64(fd,lba3,SEEK_SET);
#endif
		if(lba2 != lba3) {
#ifdef WIN32
			pMsg(ERR, "seek failed seek %I64d, lba = %I64d, request pos = %I64d, seek pos = %I64d\n",(target.oper == WRITER) ? (*wcount) : (*rcount),target.lba,lba3,lba2);
#else
			pMsg(ERR, "seek failed seek %lld, lba = %lld, request pos = %lld, seek pos = %lld\n",(target.oper == WRITER) ? (*wcount) : (*rcount),target.lba,lba3,lba2);
#endif
			*test_state = SET_STS_FAIL(*test_state);
			if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
			exit_code = SEEK_FAILURE;
			continue;
		}

		if(target.oper == WRITER) {
			if(args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba), sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, data_buffer+OFF_DATA, target.trsiz*BLK_SIZE);
			}
			if(args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz*BLK_SIZE, &(target.lba), pass_count, args->mrk_flag);
			}
#ifdef WIN32
			WriteFile(hFileHandle, buf2, target.trsiz*BLK_SIZE, &tcnt, NULL);
#else
			tcnt = write(fd,buf2,target.trsiz*BLK_SIZE);
#endif
		}
		if(target.oper == READER) {
#ifdef WIN32
			ReadFile(hFileHandle, buf1, target.trsiz*BLK_SIZE, &tcnt, NULL);
#else
			tcnt = read(fd,buf1,target.trsiz*BLK_SIZE);
#endif
		}
		if(tcnt != (long) target.trsiz*BLK_SIZE) {
#ifdef WIN32
			pMsg(ERR, "access failed: seek %I64u, lba %I64u (0x%I64x), got = %ld, asked for = %ld\n",(target.oper == WRITER) ? (*wcount) : (*rcount),target.lba,target.lba,tcnt,target.trsiz*BLK_SIZE);
#else
			pMsg(ERR, "access failed: seek %llu, lba %lld (0x%llx), got = %ld, asked for = %ld\n",(target.oper == WRITER) ? (*wcount) : (*rcount),target.lba,target.lba,tcnt,target.trsiz*BLK_SIZE);
#endif
			*test_state = SET_STS_FAIL(*test_state);
			if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
			exit_code = ACCESS_FAILURE;
			continue;
		}

		/* update bytes transfered and bitmap */
#ifdef WIN32
		WaitForSingleObject(hSemDATA, INFINITE);
#else
		pthread_mutex_lock(&data);
#endif
		if(target.oper == WRITER) {
			(*wbytes) += target.trsiz*BLK_SIZE;
			for(i=0;i<target.trsiz;i++) {
				*(wbitmap+(((target.lba)+i)/8)) |= 0x80>>(((target.lba)+i)%8);
			}
		} else {
			(*rbytes) += target.trsiz*BLK_SIZE;
		}
#ifdef WIN32
		ReleaseMutex(hSemDATA);
#else
		pthread_mutex_unlock(&data);
#endif

		/* data compare routine.  Act as if we were to write, but just compare */
		if((target.oper == READER) && (args->flags & CLD_FLG_CMPR)) {
			/* This is very SLOW!!! */
			if((args->cmp_lng == 0) || (args->cmp_lng > target.trsiz*BLK_SIZE)) {
				args->cmp_lng = target.trsiz*BLK_SIZE;
			}
			if(args->flags & CLD_FLG_LPTYPE) {
				fill_buffer(buf2, target.trsiz, &(target.lba), sizeof(OFF_T), CLD_FLG_LPTYPE);
			} else {
				memcpy(buf2, data_buffer+OFF_DATA, target.trsiz*BLK_SIZE);
			}
			if(args->flags & CLD_FLG_MBLK) {
				mark_buffer(buf2, target.trsiz*BLK_SIZE, &(target.lba), pass_count, args->mrk_flag);
			}
			if(memcmp(buf2, buf1, args->cmp_lng) != 0) {
				/* data miscompare !!! */
#ifdef WIN32
				WaitForSingleObject(hSemGBL, INFINITE);
				pMsg(ERR, "Data miscompare at lba %I64d (0x%I64x)\n", target.lba, target.lba);
#else
				pthread_mutex_lock(&global);
				pMsg(ERR, "Data miscompare at lba %lld (0x%llx)\n", target.lba, target.lba);
#endif
				pMsg(ERR, "EXPECTED: ");
				for(i=0;i<args->cmp_lng;i++)
					printf("%02x",*(buf2+i));
				printf("\n");
				pMsg(ERR, "ACTUAL:   ");
				for(i=0;i<args->cmp_lng;i++)
					printf("%02x",*(buf1+i));
				printf("\n");
				*test_state = SET_STS_FAIL(*test_state);
#ifdef WIN32
				ReleaseMutex(hSemGBL);
#else
				pthread_mutex_unlock(&global);
#endif
				if(args->flags & CLD_FLG_ALLDIE) bContinue = FALSE;
				exit_code = DATA_MISCOMPARE;
				continue;
			}
		}

	}

	free(buffer1);
	free(buffer2);
#ifdef WIN32
	CloseHandle(hFileHandle);
	ExitThread((unsigned long) exit_code);
	return((unsigned long) exit_code);
#else
	close(fd);
	pthread_exit(&exit_code); /* ChildMain (Good Exit) */
	pthread_cleanup_pop(0);
#endif
}

