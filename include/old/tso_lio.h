// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2026
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
#if defined(__linux__) && !defined(__UCLIBC__)
#define LIO_IO_TYPES            00061   /* all io types */
#define LIO_IO_ATYPES           00077   /* all io types */
#endif
#if defined(__sun) || defined(__hpux) || defined(_AIX) || defined(__UCLIBC__)
#define LIO_IO_TYPES            00021   /* all io types except pread/pwrite */
#endif /* linux */
#ifdef CRAY
#define LIO_IO_TYPES            00017   /* all io types */
#endif /* CRAY */

#ifndef LIO_IO_ATYPES
#define LIO_IO_ATYPES LIO_IO_TYPES
#endif

#define LIO_WAIT_NONE           00010000 /* return asap -- use with care */
#define LIO_WAIT_ACTIVE         00020000 /* spin looking at iosw fields, or EINPROGRESS */
#define LIO_WAIT_RECALL         00040000 /* call recall(2)/aio_suspend(3) */
#define LIO_WAIT_SIGPAUSE       00100000 /* call pause */
#define LIO_WAIT_SIGACTIVE      00200000 /* spin waiting for signal */
#if defined(sgi) || defined(__linux__)
#define LIO_WAIT_CBSUSPEND      00400000 /* aio_suspend waiting for callback */
#define LIO_WAIT_SIGSUSPEND     01000000 /* aio_suspend waiting for signal */
#define LIO_WAIT_ATYPES         01760000 /* all async wait types, except nowait */
#define LIO_WAIT_TYPES          00020000 /* all sync wait types (sorta) */
#endif /* sgi */
#if defined(__sun) || defined(__hpux) || defined(_AIX)
#define LIO_WAIT_TYPES          00300000 /* all wait types, except nowait */
#endif /* linux */
#ifdef CRAY
#define LIO_WAIT_TYPES          00360000 /* all wait types, except nowait */
#endif /* CRAY */

/* meta wait io  */
/*  00  000 0000 */

#if defined(sgi) || defined(__linux__)
/* all callback wait types */
#define LIO_WAIT_CBTYPES	(LIO_WAIT_CBSUSPEND)
/* all signal wait types */
#define LIO_WAIT_SIGTYPES	(LIO_WAIT_SIGPAUSE|LIO_WAIT_SIGACTIVE|LIO_WAIT_SIGSUSPEND)
/* all aio_{read,write} or lio_listio */
#define LIO_IO_ASYNC_TYPES	(LIO_IO_ASYNC|LIO_IO_SLISTIO|LIO_IO_ALISTIO)
#endif /* sgi */
#if defined(__sun) || defined(__hpux) || defined(_AIX)
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
#if defined (sgi)
#include <aio.h>
int  lio_wait4asyncio(int method, int fd, aiocb_t *aiocbp);
int  lio_check_asyncio(char *io_type, int size, aiocb_t *aiocbp, int method);
#endif /* sgi */
#if defined(__linux__) && !defined(__UCLIBC__)
#include <aio.h>
int  lio_wait4asyncio(int method, int fd, struct aiocb *aiocbp);
int  lio_check_asyncio(char *io_type, int size, struct aiocb *aiocbp, int method);
#endif

/*
 * Define the structure that contains the infomation that is used
 * by the parsing and help functions.
 */
struct lio_info_type {
    char *token;
    int  bits;
    char *desc;
};


