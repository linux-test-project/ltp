/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
#ifndef _WRITE_LOG_H_
#define _WRITE_LOG_H_

/*
 * Constants defining the max size of various wlog_rec fields.  ANY SIZE
 * CHANGES HERE MUST BE REFLECTED IN THE WLOG_REC_DISK STRUCTURE DEFINED
 * BELOW.
 */

#define WLOG_MAX_PATH		128
#define WLOG_MAX_PATTERN	64
#define WLOG_MAX_HOST           8
#define WLOG_REC_MAX_SIZE	(sizeof(struct wlog_rec)+WLOG_MAX_PATH+WLOG_MAX_PATTERN+WLOG_MAX_HOST+2)

/*
 * User view of a write log record.  Note that this is not necessiliary
 * how the data is formatted on disk (signifigant compression occurrs), so
 * don't expect to od the write log file and see things formatted this way.
 */

struct wlog_rec {
    int	    w_pid;	    /* pid doing the write  	    */
    int	    w_offset;       /* file offset  	    	    */
    int	    w_nbytes;	    /* # bytes written	    	    */
    int	    w_oflags;       /* low-order open() flags	    */
    int	    w_done;	    /* 1 if io confirmed done	    */
    int	    w_async;	    /* 1 if async write	(writea)    */

    char    w_host[WLOG_MAX_HOST+1];		/* host doing write -	*/
						/* null terminated */
    int	    w_hostlen;				/* host name length	*/
    char    w_path[WLOG_MAX_PATH+1];		/* file written to -	*/
						/* null terminated */
    int	    w_pathlen;				/* file name length	*/
    char    w_pattern[WLOG_MAX_PATTERN+1];	/* pattern written -	*/
						/* null terminated */
    int	    w_patternlen;			/* pattern length	*/
};

#ifndef uint
#define uint	unsigned int
#endif

/*
 * On-disk structure of a wlog_rec.  Actually, the record consists of
 * 3 parts:  [wlog_rec_disk structure][variable length data][length]
 * where length is a 2 byte field containing the total record length
 * (including the 2 bytes).  It is used for scanning the logfile in reverse
 * order.
 *
 * The variable length data includes the path, host, and pattern (in that
 * order).  The lengths of these pieces of data are held in the
 * wlog_rec_disk structure.  Thus, the actual on-disk record looks like
 * this (top is lower byte offset):
 *
 *		struct wlog_rec_disk
 *		path    (w_pathlen bytes - not null terminated)
 *		host    (w_hostlen bytes - not null terminated)
 *		pattern (w_patternlen bytes - not null terminated)
 *		2-byte record length
 *
 * Another way of looking at it is:
 *
 * <struct wlog_rec_disk><path (wpathlen bytes)>-->
 * --><host (w_hostlen bytes)><pattern (w_patternlen bytes)><length (2 bytes)>
 *
 * The maximum length of this record is defined by the WLOG_REC_MAX_SIZE
 * record.  Note that the 2-byte record length forces this to be
 * <= 64k bytes.
 *
 * Note that there is lots of bit-masking done here.  The w_pathlen,
 * w_hostlen, and w_patternlen fields MUST have enough bits to hold
 * WLOG_MAX_PATH, WLOG_MAX_HOST, and WLOG_MAX_PATTERN bytes respectivly.
 */

struct wlog_rec_disk {
#ifdef CRAY
    uint    w_offset    : 44;	    /* file offset  	    	    */
    uint    w_extra0    : 20;       /* EXTRA BITS IN WORD 0         */
#else
    /* NB: sgi is pissy about fields > 32 bit, even cc -mips3 */
    uint    w_offset    : 32;	    /* file offset  	    	    */
    uint    w_extra0    : 32;       /* EXTRA BITS IN WORD 0         */
#endif

    uint    w_nbytes    : 32;	    /* # bytes written	    	    */
    uint    w_oflags	: 32;	    /* low-order open() flags	    */

    uint    w_pid       : 17;	    /* pid doing the write  	    */
    uint    w_pathlen	:  7;	    /* length of file path  	    */
    uint    w_patternlen:  6;	    /* length of pattern            */
    uint    w_hostlen   :  4;       /* length of host               */
    uint    w_done      :  1;	    /* 1 if io confirmed done	    */
    uint    w_async     :  1;	    /* 1 if async write	(writea)    */
    uint    w_extra2 	: 28;	    /* EXTRA BITS IN WORD 2 	    */
};

/*
 * write log file datatype.  wlog_open() initializes this structure
 * which is then passed around to the various wlog_xxx routines.
 */

struct wlog_file {
    int		w_afd;			/* append fd			*/
    int		w_rfd;			/* random-access fd		*/
    char	w_file[1024];		/* name of the write_log	*/
};

/*
 * return value defines for the user-supplied function to
 * wlog_scan_backward().
 */

#define WLOG_STOP_SCAN		0
#define WLOG_CONTINUE_SCAN	1

/*
 * wlog prototypes
 */

#if __STDC__
extern int	wlog_open(struct wlog_file *wfile, int trunc, int mode);
extern int	wlog_close(struct wlog_file *wfile);
extern int	wlog_record_write(struct wlog_file *wfile,
				  struct wlog_rec *wrec, long offset);
extern int	wlog_scan_backward(struct wlog_file *wfile, int nrecs,
				   int (*func)(struct wlog_rec *rec),
				   long data);
#else
int	wlog_open();
int	wlog_close();
int	wlog_record_write();
int	wlog_scan_backward();
#endif

extern char	Wlog_Error_String[];

#endif /* _WRITE_LOG_H_ */

