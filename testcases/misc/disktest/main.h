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
* $Id: main.h,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: main.h,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
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

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#else
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <wait.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "defs.h"

#define VER_STR "Version 0.9.9b4"
#define BLKGETSIZE _IO(0x12,96)
#define BLKSSZGET  _IO(0x12,104)

#define BLK_SIZE	512
#define M_BLK_SIZE	512
#define T_MAX_SIZE	M_BLK_SIZE*BLK_SIZE*2
#define ALIGNSIZE	4096	/* 4k alignment works in all cases ?? */
#define ALIGN(x, bs) ((x + (bs - 1)) & ~(bs - 1))
//#define ALIGN2(x, bs) ((x/bs) * bs)
#define BUFALIGN(x) (void *) (((unsigned int)x + (ALIGNSIZE - 1)) & ~(ALIGNSIZE - 1))
#define MASK(x,y) (x & y)

/* each is a 64b number.  offsets are in 8B*offset placement */
#define OFF_TST_STAT	0	/* offset in memseg were global test status lives */
#define OFF_RLBA	1	/* offset in memseg of current read LBA */
#define OFF_WLBA	2	/* offset in memseg of current write LBA */
#define OFF_WCOUNT	3	/* offset in memseg were total write count lives */
#define OFF_RCOUNT	4	/* offset in memseg were total read count lives */
#define OFF_RBYTES	5	/* bytes read */
#define OFF_WBYTES	6	/* bytes written */

#define BMP_OFFSET	7*sizeof(OFF_T)		/* bitmap starts here */

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

#define DBUF_SIZE	(1*T_MAX_SIZE)
#define OFF_DATA	(0*T_MAX_SIZE)

#define CLD_FLG_CMPR	0x000000001	/* will cause readers to compare data read */
#define CLD_FLG_MBLK	0x000000002	/* will add header info to fist block, fc lun, lba, etc */
#define CLD_FLG_SQNCE	0x000000004	/* forces IOs to be queued one at a time */
#define CLD_FLG_RTRSIZ	0x000000008	/* Ignore weither a block has been written */

/* Perforamnce Flags */
#define CLD_FLG_XFERS	0x000000010	/* reports # of transfers */
#define CLD_FLG_TPUTS	0x000000020	/* reports calculated throughtput */
#define CLD_FLG_RUNT	0x000000040	/* reports run time */

/* Seek Flags */
#define CLD_FLG_RANDOM	0x000000100	/* child seeks are random */
#define CLD_FLG_LINEAR	0x000000200	/* child seeks are linear */
#define CLD_FLG_PP		0x000000400	/* ping pong seeks: start/end/start+1/end-1 */
#define CLD_FLG_NTRLVD	0x000000800	/* reads and writes are mixed during sequential IO */
#define CLD_FLG_SKTYPS	(CLD_FLG_RANDOM|CLD_FLG_LINEAR|CLD_FLG_PP)

/* IO Type Flags */
#define CLD_FLG_RAW		0x000001000	/* child IO is to a RAW device */
#define CLD_FLG_BLK		0x000002000	/* child IO is to a BLOCK device */
#define CLD_FLG_FILE	0x000004000	/* child IO is to a file */
#define CLD_FLG_DIRECT	0x000008000	/* child IO has direct disk access */
#define CLD_FLG_IOTYPS	(CLD_FLG_RAW|CLD_FLG_BLK|CLD_FLG_FILE|CLD_FLG_DIRECT)

/* Pattern Flags */
#define CLD_FLG_RPTYPE	0x000010000	/* random pattern */
#define CLD_FLG_FPTYPE	0x000020000	/* fixed pattern */
#define CLD_FLG_CPTYPE	0x000040000	/* counting pattern */
#define CLD_FLG_LPTYPE	0x000080000	/* lba pattern */
#define CLD_FLG_PTYPS	(CLD_FLG_RPTYPE|CLD_FLG_FPTYPE|CLD_FLG_CPTYPE|CLD_FLG_LPTYPE)

/* Duration Flags */
#define CLD_FLG_TMD		0x000100000	/* set if using time */
#define CLD_FLG_SKS		0x000200000	/* set if seeks are used */
#define CLD_FLG_CYC		0x000400000	/* set if cycles are used */
#define CLD_FLG_DUTY	0x000800000	/* set if a duty cycle is used while running */

#define CLD_FLG_LBA_RNG	0x001000000	/* write multipule read multipule, must define multiple */
#define CLD_FLG_BLK_RNG	0x002000000	/* write once read multiple, must define multiple */
#define CLD_FLG_ALLDIE	0x004000000	/* will force all children to die on any error if set */

#define CLD_FLG_LUNU	0x010000000	/* seek start/end and then start/end */
#define CLD_FLG_LUND	0x020000000	/* seek start/end and then end/start */
#define CLD_FLG_W		0x040000000	/* there are child writers */
#define CLD_FLG_R		0x080000000	/* there are child readers */

/* startup defaults */
#define TRSIZ	1		/* default transfer size in blocks */
#define VSIZ	2000	/* default volume capacity in LBAs */
#define SEEKS	1000	/* default seeks */
#define KIDS	4		/* default number of children */

typedef enum op {
	WRITER,READER,NONE
} op_t;

typedef struct child_args {
	char *device;			/* pointer to device name */
	OFF_T vsiz;				/* volume size in blocks */
	unsigned long ltrsiz;	/* low bound of transfer size in blocks */
	unsigned long htrsiz;	/* high bound of transfer size in blocks */
	unsigned long offset;	/* lba offset */
	OFF_T pattern;			/* pattern data */
	time_t run_time;		/* run time in seconds */
	OFF_T seeks;			/* number of seeks */
	unsigned long cycles;	/* number of cycles */
	OFF_T start_blk;		/* starting transfer block */
	OFF_T stop_blk;			/* ending transfer block */
	OFF_T start_lba;		/* starting LBA */
	OFF_T stop_lba;			/* ending LBA */
	unsigned int retries;	/* number of retries */
	time_t hbeat;			/* Statistics will be reported every hbeats seconds */
	OFF_T flags;			/* common flags that a child uses */
	unsigned long rcount;	/* number of reads a child should perform, 0 is unbound  */
	unsigned long wcount;	/* number of writes a child should perform, 0 is unbound  */
	short rperc;			/* percent of IO that should be reads */
	short wperc;			/* percent of IO that should be write */
	unsigned char t_kids;	/* total children, max is 256 */
	unsigned int cmp_lng;	/* how much of the data should be compared */
	short mrk_flag;			/* how the tranfsers should be marked */
} child_args_t;

#endif /* _DISKTEST_H */
