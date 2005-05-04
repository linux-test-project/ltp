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
*
* $Id: main.h,v 1.4 2005/05/04 17:54:00 mridge Exp $
* $Log: main.h,v $
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.21  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.20  2004/12/18 06:13:03  yardleyb
* Updated timer schema to more accurately use the time options.  Added
* fsync on write option to -If.
*
* Revision 1.19  2004/12/17 06:34:56  yardleyb
* removed -mf -ml.  These mark options cause to may issues when using
* random block size transfers.  Fixed -ma option for endian-ness.  Fixed
* false data misscompare during multiple cycles.
*
* Revision 1.18  2004/11/20 04:43:42  yardleyb
* Minor code fixes.  Checking for alloc errors.
*
* Revision 1.17  2004/11/19 21:45:12  yardleyb
* Fixed issue with code added for -F option.  Cased disktest
* to SEG FAULT when cleaning up threads.
*
* Revision 1.16  2004/11/19 03:47:45  yardleyb
* Fixed issue were args data was not being copied from a
* clean source.
*
* Revision 1.15  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.14  2003/09/12 21:23:01  yardleyb
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
* Revision 1.13  2003/09/12 18:10:09  yardleyb
* Updated to version 1.11
* Code added to fix compile
* time warnings
*
* Revision 1.12  2003/01/13 21:33:31  yardleyb
* Added code to detect AIX volume size.
* Updated mask for random LBA to use start_lba offset
* Updated version to 1.1.10
*
* Revision 1.11  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.10  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.9  2002/03/30 01:32:14  yardleyb
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
* Revision 1.8  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
* Revision 1.7  2002/02/26 19:35:59  yardleyb
* Updates to parsing routines for user
* input.  Added multipliers for -S and
* -s command line arguments. Forced
* default seeks to default if performing
* a diskcache test.
*
* Revision 1.6  2002/02/20 18:45:00  yardleyb
* Set revision number v1.0.0
*
* Revision 1.5  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
*
* Revision 1.4  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.3  2002/01/31 20:12:21  yardleyb
* Updated the performance counters
* to reflect run time based on duty
* cycle not on total.
*
* Revision 1.2  2001/12/07 23:33:29  yardleyb
* Fixed bug where a false positive data
* miscompare could occur when running
* multi cycle testing with mark block
* enabled.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
* Revision 1.11  2001/10/10 00:18:26  yardleyb
* Updated revision to 0.9.2
*
* Revision 1.10  2001/10/10 00:17:14  yardleyb
* Added Copyright and GPL license text.
* Miner bug fixes throughout text.
*
* Revision 1.9  2001/10/01 23:30:58  yardleyb
* Increased overall disktest revision number.
*
* Revision 1.8  2001/10/01 23:18:15  yardleyb
* Changed all time related variables to time_t.
* Rearranged some of the CLD_FLGs
*
* Revision 1.7  2001/09/26 23:38:31  yardleyb
* Changed default seeks to 1000.
* Added version string.
*
* Revision 1.6  2001/09/22 03:42:56  yardleyb
* Anyother major update.  fixed bugs in IO routines,
* added more error checking on input.  Added heartbeat
* option, up-and-up and up-down-up seek routines, pass/fail
* output, pMsg level discription, veriable length data comare
* checking.  Lots of lint cleanup.
*
* Revision 1.5  2001/09/07 02:13:31  yardleyb
* Major rewrite of main IO function.  Fixed bug in duty cycle were percentages
* were off by one.  Got rid of some sloppy memory usage.  Major performance
* increase overall.
*
* Revision 1.4  2001/09/06 21:59:23  yardleyb
* Fixed a bug in the -L/-K where it ther ewere to many childern be created.
* also more code cleanup.  Changed 'loops' to 'seeks' throughout.
*
* Revision 1.3  2001/09/06 18:23:30  yardleyb
* Added duty cycle -D.  Updated usage. Added
* make option to create .tar.gz of all files
*
* Revision 1.2  2001/09/05 22:44:42  yardleyb
* Split out some of the special functions.
* added O_DIRECT -Id.  Updated usage.  Lots
* of clean up to functions.  Added header info
* to pMsg.
*
* Revision 1.1  2001/09/04 19:28:08  yardleyb
* Split usage out. Split header out.  Added usage text.
* Made signal handler one function. code cleanup.
*
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

#define VER_STR "v1.2.8"
#define BLKGETSIZE _IO(0x12,96)
#define BLKSSZGET  _IO(0x12,104)

#define DEV_NAME_LEN 80
#define MAX_ARG_LEN 160

#define BLK_SIZE	512
#define M_BLK_SIZE	512
#define ALIGNSIZE	4096	/* 4k alignment works in all cases ?? */

#if __WORDSIZE == 64
#define ALIGN(x, bs) (((OFF_T)x + ((OFF_T)bs - 1)) & ~((OFF_T)bs - 1))
#define BUFALIGN(x) (void *) (((unsigned long)x + (OFF_T)(ALIGNSIZE - 1)) & (OFF_T)~(ALIGNSIZE - 1))
#else
#define ALIGN(x, bs) ((x + (bs - 1)) & ~(bs - 1))
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

#define CLD_FLG_CMPR	0x000000001ULL	/* will cause readers to compare data read */
#define CLD_FLG_MBLK	0x000000002ULL	/* will add header info to fist block, fc lun, lba, etc */
#define CLD_FLG_SQNCE	0x000000004ULL	/* forces IOs to be queued one at a time */
#define CLD_FLG_RTRSIZ	0x000000008ULL	/* Ignore weither a block has been written */

/* Perforamnce Flags */
#define CLD_FLG_XFERS	0x000000010ULL	/* reports # of transfers */
#define CLD_FLG_TPUTS	0x000000020ULL	/* reports calculated throughtput */
#define CLD_FLG_RUNT	0x000000040ULL	/* reports run time */
#define CLD_FLG_PCYC	0x000000080ULL	/* report cycle data */
#define CLD_FLG_PRFTYPS	(CLD_FLG_XFERS|CLD_FLG_TPUTS|CLD_FLG_RUNT|CLD_FLG_PCYC)

/* Seek Flags */
#define CLD_FLG_RANDOM	0x000000100ULL	/* child seeks are random */
#define CLD_FLG_LINEAR	0x000000200ULL	/* child seeks are linear */
#define CLD_FLG_NTRLVD	0x000000400ULL	/* reads and writes are mixed during sequential IO */
#define CLD_FLG_SKTYPS	(CLD_FLG_RANDOM|CLD_FLG_LINEAR)

#define CLD_FLG_VSIZ	0x000000800ULL	/* Volume size specified on cli */

/* IO Type Flags */
#define CLD_FLG_RAW		0x000001000ULL	/* child IO is to a RAW device */
#define CLD_FLG_BLK		0x000002000ULL	/* child IO is to a BLOCK device */
#define CLD_FLG_FILE	0x000004000ULL	/* child IO is to a file */
#define CLD_FLG_DIRECT	0x000008000ULL	/* child IO has direct disk access */
#define CLD_FLG_IOTYPS	(CLD_FLG_RAW|CLD_FLG_BLK|CLD_FLG_FILE|CLD_FLG_DIRECT)

/* Pattern Flags */
#define CLD_FLG_RPTYPE	0x000010000ULL	/* random pattern */
#define CLD_FLG_FPTYPE	0x000020000ULL	/* fixed pattern */
#define CLD_FLG_CPTYPE	0x000040000ULL	/* counting pattern */
#define CLD_FLG_LPTYPE	0x000080000ULL	/* lba pattern */
#define CLD_FLG_PTYPS	(CLD_FLG_RPTYPE|CLD_FLG_FPTYPE|CLD_FLG_CPTYPE|CLD_FLG_LPTYPE)

/* Duration Flags */
#define CLD_FLG_TMD		0x000100000ULL	/* set if using time */
#define CLD_FLG_SKS		0x000200000ULL	/* set if seeks are used */
#define CLD_FLG_CYC		0x000400000ULL	/* set if cycles are used */
#define CLD_FLG_DUTY	0x000800000ULL	/* set if a duty cycle is used while running */

#define CLD_FLG_LBA_RNG	0x001000000ULL	/* write multipule read multipule, must define multiple */
#define CLD_FLG_BLK_RNG	0x002000000ULL	/* write once read multiple, must define multiple */
#define CLD_FLG_ALLDIE	0x004000000ULL	/* will force all children to die on any error if set */
#define CLD_FLG_DUMP	0x008000000ULL	/* will dump formatted data */

#define CLD_FLG_LUNU	0x010000000ULL	/* seek start/end and then start/end */
#define CLD_FLG_LUND	0x020000000ULL	/* seek start/end and then end/start */
#define CLD_FLG_W		0x040000000ULL	/* there are child writers */
#define CLD_FLG_R		0x080000000ULL	/* there are child readers */

#define CLD_FLG_FSLIST	0x100000000ULL	/* the filespec is a list of targets */
#define CLD_FLG_HBEAT	0x200000000ULL	/* if performance heartbeat is being used */
#define CLD_FLG_WFSYNC	0x400000000ULL	/* do an fsync on every write for file IO */

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

typedef enum op {
	WRITER,READER,NONE
} op_t;

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
	unsigned long offset;		/* lba offset */
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
	short mrk_flag;				/* how the tranfsers should be marked */
	OFF_T test_state;			/* current test state */
	unsigned int seed;			/* test seed */
	pid_t pid;					/* the process_id used for this environment */
} child_args_t;

typedef struct test_env {
	void *shared_mem;           /* global pointer to shared memory */
	unsigned char *data_buffer; /* global data buffer */
	size_t bmp_siz;             /* size of bitmask */
	BOOL bContinue;             /* global that when set to false will force exit of all threads */
	OFF_T pass_count;           /* hit counters */
	stats_t cycle_stats;        /* per cycle statistics */
	unsigned short kids;		/* number of test child processes */
	thread_struct_t *pThreads;  /* List of child test processes */
	stats_t global_stats;       /* overall statistics for test */
} test_env_t;

typedef struct test_ll {
	test_env_t *env;			/* pointer to the environment structure */
	child_args_t *args;			/* pointer to the argument structure */
	hThread_t hThread;
	struct test_ll *next;		/* pointer to the next test */
} test_ll_t;

#endif /* _DISKTEST_H */
