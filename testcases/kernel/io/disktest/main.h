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
* $Id: main.h,v 1.5 2008/02/14 08:22:23 subrata_modak Exp $
*
*/

#ifndef _DISKTEST_H
#define _DISKTEST_H

#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#else
#include <pthread.h>
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
#include "lapi/abisize.h"

#define VER_STR "v1.4.2"
#define BLKGETSIZE _IO(0x12,96)		/* IOCTL for getting the device size */
#define BLKSSZGET  _IO(0x12,104)	/* ALT IOCTL for getting the device size */

#define DEV_NAME_LEN		80		/* max character for target name */
#define MAX_ARG_LEN			160		/* max length of command line arguments for startarg display */
#define HOSTNAME_SIZE		16		/* number of hostname characters used in mark header */
#define BLK_SIZE			512		/* default size of an LBA in bytes */
#define ALIGNSIZE			4096	/* memory alignment size in bytes */
#define DEFAULT_IO_TIMEOUT	120		/* the default number of seconds before IO timeout */

/* the new way we align */
#define ALIGN(x, y) (((long long unsigned)x/(long long unsigned)y)*(long long unsigned)y)

#ifdef TST_ABI64
/* the old way we use to align */
/* #define ALIGN(x, bs) (((OFF_T)x + ((OFF_T)bs - 1)) & ~((OFF_T)bs - 1)) */
#define BUFALIGN(x) (void *) (((unsigned long)x + (OFF_T)(ALIGNSIZE - 1)) & (OFF_T)~(ALIGNSIZE - 1))
#else
/* the old way we use to align */
/* #define ALIGN(x, bs) ((x + (bs - 1)) & ~(bs - 1)) */
#define BUFALIGN(x) (void *) (((unsigned long)x + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1))
#endif

#define MASK(x,y) (x & y)

/* each is a 64b number.  offsets are in 8B*offset placement */
#define OFF_RLBA	0	/* offset in memseg of current read LBA */
#define OFF_WLBA	1	/* offset in memseg of current write LBA */

#define BMP_OFFSET	2*sizeof(OFF_T)		/* bitmap starts here */

#define TST_STS(x)			(x & 0x1)	/* current testing status */
#define SET_STS_PASS(x)		(x | 0x01)
#define SET_STS_FAIL(x)		(x & ~0x01)
#define TST_wFST_TIME(x)	(x & 0x02)	/* first write lba access */
#define SET_wFST_TIME(x)	(x | 0x02)
#define CLR_wFST_TIME(x)	(x & ~0x02)
#define TST_rFST_TIME(x)	(x & 0x04)	/* first read lba access */
#define SET_rFST_TIME(x)	(x | 0x04)
#define CLR_rFST_TIME(x)	(x & ~0x04)
#define TST_DIRCTN(x)	(x & 0x08)		/* lba inc/dec 1 is inc, 0 is dec */
#define DIRCT_INC(x)    (x | 0x08)
#define DIRCT_DEC(x)    (x & ~0x08)
#define DIRCT_CNG(x)    (x & 0x08) ? (x & ~0x08) : (x | 0x08)
#define TST_OPER(x) (short) ((x & 0x10) >> 4)	/* last transfer operation (write = 0, read = 1) */
#define SET_OPER_R(x)	(x | 0x10)
#define SET_OPER_W(x)	(x & ~0x10)
#define CNG_OPER(x)		(x & 0x10) ? (x & ~0x10) : (x | 0x10)

#define CLD_FLG_CMPR		0x0000000000000001ULL	/* will cause readers to compare data read */
#define CLD_FLG_MBLK		0x0000000000000002ULL	/* will add header info to first block, fc lun, lba, etc */
#define CLD_FLG_OFFSET		0x0000000000000004ULL	/* specifies that an offset up to 2^31 LBAs has been given */
#define CLD_FLG_RTRSIZ		0x0000000000000008ULL	/* Ignore weither a block has been written */

/* Perforamnce Flags */
#define CLD_FLG_XFERS		0x0000000000000010ULL	/* reports # of transfers */
#define CLD_FLG_TPUTS		0x0000000000000020ULL	/* reports calculated throughtput */
#define CLD_FLG_RUNT		0x0000000000000040ULL	/* reports run time */
#define CLD_FLG_PCYC		0x0000000000000080ULL	/* report cycle data */
#define CLD_FLG_PRFTYPS	(CLD_FLG_XFERS|CLD_FLG_TPUTS|CLD_FLG_RUNT|CLD_FLG_PCYC)

/* Seek Flags */
#define CLD_FLG_RANDOM		0x0000000000000100ULL	/* child seeks are random */
#define CLD_FLG_LINEAR		0x0000000000000200ULL	/* child seeks are linear */
#define CLD_FLG_NTRLVD		0x0000000000000400ULL	/* reads and writes are interleaved */
#define CLD_FLG_SKTYPS	(CLD_FLG_RANDOM|CLD_FLG_LINEAR)

#define CLD_FLG_VSIZ		0x0000000000000800ULL	/* Volume size is user specified */

/* IO Type Flags */
#define CLD_FLG_RAW			0x0000000000001000ULL	/* child IO is to a raw/character device */
#define CLD_FLG_BLK			0x0000000000002000ULL	/* child IO is to a block device */
#define CLD_FLG_FILE		0x0000000000004000ULL	/* child IO is to a file */
#define CLD_FLG_DIRECT		0x0000000000008000ULL	/* child IO has direct disk access */
#define CLD_FLG_IOTYPS	(CLD_FLG_RAW|CLD_FLG_BLK|CLD_FLG_FILE|CLD_FLG_DIRECT)

/* Pattern Flags */
#define CLD_FLG_RPTYPE		0x0000000000010000ULL	/* random pattern */
#define CLD_FLG_FPTYPE		0x0000000000020000ULL	/* fixed pattern */
#define CLD_FLG_CPTYPE		0x0000000000040000ULL	/* counting pattern */
#define CLD_FLG_LPTYPE		0x0000000000080000ULL	/* lba pattern */
#define CLD_FLG_PTYPS	(CLD_FLG_RPTYPE|CLD_FLG_FPTYPE|CLD_FLG_CPTYPE|CLD_FLG_LPTYPE)

/* Duration Flags */
#define CLD_FLG_TMD			0x0000000000100000ULL	/* set if using time */
#define CLD_FLG_SKS			0x0000000000200000ULL	/* set if seeks are used */
#define CLD_FLG_CYC			0x0000000000400000ULL	/* set if cycles are used */
#define CLD_FLG_DUTY		0x0000000000800000ULL	/* set if a duty cycle is used while running */

#define CLD_FLG_LBA_RNG		0x0000000001000000ULL	/* write multipule read multipule, must define multiple */
#define CLD_FLG_BLK_RNG		0x0000000002000000ULL	/* write once read multiple, must define multiple */
#define CLD_FLG_ALLDIE		0x0000000004000000ULL	/* will force all children to die on any error if set */
#define CLD_FLG_DUMP		0x0000000008000000ULL	/* will dump formatted data */

#define CLD_FLG_LUNU		0x0000000010000000ULL	/* seek start/end and then start/end */
#define CLD_FLG_LUND		0x0000000020000000ULL	/* seek start/end and then end/start */
#define CLD_FLG_W			0x0000000040000000ULL	/* there are child writers */
#define CLD_FLG_R			0x0000000080000000ULL	/* there are child readers */

#define CLD_FLG_FSLIST		0x0000000100000000ULL	/* the filespec is a list of targets */
#define CLD_FLG_HBEAT		0x0000000200000000ULL	/* if performance heartbeat is being used */
#define CLD_FLG_WFSYNC		0x0000000400000000ULL	/* do an fsync on write for file IO */
#define FLAG_NOT_DEFINED	0x0000000800000000ULL	/* NOT DEFINED */

#define CLD_FLG_WRITE_ONCE	0x0000001000000000ULL	/* only write once to each LBA */
#define CLD_FLG_ERR_REREAD	0x0000002000000000ULL	/* On miscompare, reread the miscompare transfer */
#define CLD_FLG_LBA_SYNC	0x0000004000000000ULL	/* LBA syncronizion */
#define CLD_FLG_IO_SERIAL	0x0000008000000000ULL	/* serialize IO at the IO operation level */

#define CLD_FLG_MRK_LBA		0x0000010000000000ULL	/* enable adding LBA to mark data */
#define CLD_FLG_MRK_PASS	0x0000020000000000ULL	/* enable adding pass count to mark data */
#define CLD_FLG_MRK_TIME	0x0000040000000000ULL	/* enable adding start time to mark data */
#define CLD_FLG_MRK_SEED	0x0000080000000000ULL	/* enable adding seed to mark data */
#define CLD_FLG_MRK_HOST	0x0000100000000000ULL	/* enable adding hostname to mark data */
#define CLD_FLG_MRK_TARGET	0x0000200000000000ULL	/* enable adding target name to mark data */
#define CLD_FLG_MRK_ALL	(CLD_FLG_MRK_LBA|CLD_FLG_MRK_PASS|CLD_FLG_MRK_TIME|CLD_FLG_MRK_SEED|CLD_FLG_MRK_HOST|CLD_FLG_MRK_TARGET)

#define CLD_FLG_ALT_MARK	0x0000400000000000ULL	/* override time marker, with data in alt_mark, in mark header */
#define CLD_FLG_ERR_MARK	0x0000800000000000ULL	/* On error, write a special MARKER to LBA 0 on the target */

#define CLD_FLG_TMO_ERROR	0x0001000000000000ULL	/* make an IO TIMEOUT warning, fail the IO test */
#define CLD_FLG_UNIQ_WRT	0x0002000000000000ULL	/* garentees that every write is unique */

/* startup defaults */
#define TRSIZ	1		/* default transfer size in blocks */
#define VSIZ	2000	/* default volume capacity in LBAs */
#define SEEKS	1000	/* default seeks */
#define KIDS	4		/* default number of children */

#ifdef WINDOWS
typedef HANDLE hThread_t;
#else
typedef pthread_t hThread_t;
#endif

typedef struct thread_struct {
	hThread_t hThread;
	BOOL bCanBeJoined;
	struct thread_struct *next; /* pointer to next thread */
} thread_struct_t;

typedef struct stats {
	OFF_T wcount;
	OFF_T rcount;
	OFF_T wbytes;
	OFF_T rbytes;
	time_t wtime;
	time_t rtime;
} stats_t;

typedef struct child_args {
	char device[DEV_NAME_LEN];	/* device name */
	char argstr[MAX_ARG_LEN];	/* human readable argument string /w assumtions */
	OFF_T vsiz;					/* volume size in blocks */
	unsigned long ltrsiz;		/* low bound of transfer size in blocks */
	unsigned long htrsiz;		/* high bound of transfer size in blocks */
	long offset;				/* the lba offset to shift IO alignment by */
	OFF_T pattern;				/* pattern data */
	time_t run_time;			/* run time in seconds */
	OFF_T seeks;				/* number of seeks */
	unsigned long cycles;		/* number of cycles */
	OFF_T start_blk;			/* starting transfer block */
	OFF_T stop_blk;				/* ending transfer block */
	OFF_T start_lba;			/* starting LBA */
	OFF_T stop_lba;				/* ending LBA */
	unsigned int retries;		/* number of retries */
	time_t hbeat;				/* Statistics will be reported every hbeats seconds */
	unsigned long long flags;	/* common flags that a child uses */
	unsigned long rcount;		/* number of reads a child should perform, 0 is unbound  */
	unsigned long wcount;		/* number of writes a child should perform, 0 is unbound  */
	short rperc;				/* percent of IO that should be reads */
	short wperc;				/* percent of IO that should be write */
	unsigned short t_kids;		/* total children, max is 64k */
	unsigned int cmp_lng;		/* how much of the data should be compared */
	OFF_T test_state;			/* current test state */
	unsigned int seed;			/* test seed */
	pid_t pid;					/* the process_id used for this environment */
	OFF_T alt_mark;				/* alternate marker the start time */
	unsigned long delayTimeMin;	/* the minimum time (msec) to delay on each IO */
	unsigned long delayTimeMax;	/* the maximum time (msec) to delay on each IO */
	time_t ioTimeout;			/* the time (sec) before failure do to possible hung IO */
	unsigned long sync_interval;/* number of write IOs before issuing a sync */
	long retry_delay;			/* number of msec to wait before retrying an IO */
} child_args_t;

typedef struct mutexs {
#ifdef WINDOWS
	HANDLE MutexACTION;			/* mutex for the entire target device */
	HANDLE MutexIO;				/* mutex for the IO to the device */
#else
	pthread_mutex_t MutexACTION; /* mutex for the entire target device */
	pthread_mutex_t MutexIO;	/* mutex for the IO to the device */
#endif
} mutexs_t;

typedef struct test_env {
	void *shared_mem;           /* global pointer to shared memory */
	unsigned char *data_buffer; /* global data buffer */
	size_t bmp_siz;             /* size of bitmask */
	BOOL bContinue;             /* global that when set to false will force exit for this environment */
	OFF_T pass_count;           /* pass counters */
	stats_t hbeat_stats;        /* per heartbeat statistics */
	stats_t cycle_stats;        /* per cycle statistics */
	stats_t global_stats;       /* per env statistics */
	OFF_T rcount;				/* number of read IO operations */
	OFF_T wcount;				/* number of write IO operations */
	unsigned short kids;		/* number of test child processes */
	thread_struct_t *pThreads;  /* List of child test processes */
	time_t start_time;			/*	overall start time of test	*/
	time_t end_time;			/*	overall end time of test	*/
	action_t lastAction;		/* when interleaving tests, tells the threads whcih action was last */
	action_t *action_list;		/* pointer to list of actions that are currently in use */
	int action_list_entry;		/* where in the action_list we are */
	mutexs_t mutexs;
} test_env_t;

typedef struct test_ll {
	test_env_t *env;			/* pointer to the environment structure */
	child_args_t *args;			/* pointer to the argument structure */
	hThread_t hThread;
	struct test_ll *next;		/* pointer to the next test */
} test_ll_t;

#endif /* _DISKTEST_H */
