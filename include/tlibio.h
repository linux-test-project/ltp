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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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

#define LIO_IO_SYNC             00001   /* read/write */
#define LIO_IO_ASYNC            00002   /* reada/writea/aio_write/aio_read */
#define LIO_IO_SLISTIO          00004   /* single stride sync listio */
#define LIO_IO_ALISTIO          00010   /* single stride async listio */
#define LIO_IO_SYNCV            00020   /* single-buffer readv/writev */
#define LIO_IO_SYNCP            00040   /* pread/pwrite */

#ifdef sgi
#define LIO_IO_ATYPES           00077   /* all io types */
#define LIO_IO_TYPES            00061   /* all io types, non-async */
#endif /* sgi */
#ifdef linux
#define LIO_IO_TYPES            00021   /* all io types */
#endif /* linux */
#ifdef CRAY
#define LIO_IO_TYPES            00017   /* all io types */
#endif /* CRAY */

#define LIO_WAIT_NONE           00010000 /* return asap -- use with care */
#define LIO_WAIT_ACTIVE         00020000 /* spin looking at iosw fields, or EINPROGRESS */
#define LIO_WAIT_RECALL         00040000 /* call recall(2)/aio_suspend(3) */
#define LIO_WAIT_SIGPAUSE       00100000 /* call pause */
#define LIO_WAIT_SIGACTIVE      00200000 /* spin waiting for signal */
#ifdef sgi
#define LIO_WAIT_CBSUSPEND      00400000 /* aio_suspend waiting for callback */
#define LIO_WAIT_SIGSUSPEND     01000000 /* aio_suspend waiting for signal */
#define LIO_WAIT_ATYPES         01760000 /* all async wait types, except nowait */
#define LIO_WAIT_TYPES          00020000 /* all sync wait types (sorta) */
#endif /* sgi */
#ifdef linux
#define LIO_WAIT_TYPES          00300000 /* all wait types, except nowait */
#endif /* linux */
#ifdef CRAY
#define LIO_WAIT_TYPES          00360000 /* all wait types, except nowait */
#endif /* CRAY */

/* meta wait io  */
/*  00  000 0000 */

#ifdef sgi
/* all callback wait types */
#define LIO_WAIT_CBTYPES	(LIO_WAIT_CBSUSPEND)
/* all signal wait types */
#define LIO_WAIT_SIGTYPES	(LIO_WAIT_SIGPAUSE|LIO_WAIT_SIGACTIVE|LIO_WAIT_SIGSUSPEND)
/* all aio_{read,write} or lio_listio */
#define LIO_IO_ASYNC_TYPES	(LIO_IO_ASYNC|LIO_IO_SLISTIO|LIO_IO_ALISTIO)
#endif /* sgi */
#ifdef linux
/* all signal wait types */
#define LIO_WAIT_SIGTYPES	(LIO_WAIT_SIGPAUSE)
#endif /* linux */
#ifdef CRAY
/* all signal wait types */
#define LIO_WAIT_SIGTYPES	(LIO_WAIT_SIGPAUSE|LIO_WAIT_SIGACTIVE)
#endif /* CRAY */

/*
 * This bit provides a way to randomly pick an io type and wait method.
 * lio_read_buffer() and lio_write_buffer() functions will call
 * lio_random_methods() with the given method.
 */
#define LIO_RANDOM              010000000

/*
 * This bit provides a way for the programmer to use async i/o with
 * signals and to use their own signal handler.  By default,
 * the signal will only be given to the system call if the wait
 * method is LIO_WAIT_SIGPAUSE or LIO_WAIT_SIGACTIVE.
 * Whenever these wait methods are used, libio signal handler
 * will be used.
 */
#define LIO_USE_SIGNAL          020000000

/*
 * prototypes/structures for functions in the libio.c module.  See comments
 * in that module, or man page entries for information on the individual
 * functions.
 */

int  stride_bounds(int offset, int stride, int nstrides,
		      int bytes_per_stride, int *min_byte, int *max_byte);

int  lio_set_debug(int level);
int  lio_parse_io_arg1(char *string);
void lio_help1(char *prefex);
int  lio_parse_io_arg2(char *string, char **badtoken);
void lio_help2(char *prefex);
int  lio_write_buffer(int fd, int method, char *buffer, int size,
		      int sig, char **errmsg, long wrd);

int  lio_read_buffer(int fd, int method, char *buffer, int size,
		     int sig, char **errmsg, long wrd);
int  lio_random_methods(long mask);

#if CRAY
#include <sys/iosw.h>
int  lio_wait4asyncio(int method, int fd, struct iosw **statptr);
int  lio_check_asyncio(char *io_type, int size, struct iosw *status);
#endif /* CRAY */
#ifdef sgi
#include <aio.h>
int  lio_wait4asyncio(int method, int fd, aiocb_t *aiocbp);
int  lio_check_asyncio(char *io_type, int size, aiocb_t *aiocbp, int method);
#endif /* sgi */

/*
 * Define the structure that contains the infomation that is used
 * by the parsing and help functions.
 */
struct lio_info_type {
    char *token;
    int  bits;
    char *desc;
};


