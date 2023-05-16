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
/*
 *
 * Lib i/o
 *
 * This file contains several functions to doing reads and writes.
 * It was written so that a single function could be called in a test
 * program and only a io type field value would have to change to
 * do different types of io.  There is even a couple of functions that
 * will allow you to parse a string to determine the iotype.
 *
 * This file contains functions for writing/reading to/from open files
 * Prototypes:
 *
 * Functions declared in this module - see individual function code for
 * usage comments:
 *
 *  int	 stride_bounds(int offset, int stride, int nstrides,
 *		      int bytes_per_stride, int *min, int *max);

 *  int  lio_write_buffer(int fd, int method, char *buffer, int size,
 *						char **errmsg, long wrd);
 *  int  lio_read_buffer(int fd, int method, char *buffer, int size,
 *						char **errmsg, long wrd);
 *
 *  #ifdef CRAY
 *  int  lio_wait4asyncio(int method, int fd, struct iosw **statptr)
 *  int  lio_check_asyncio(char *io_type, int size, struct iosw *status)
 *  #endif
 *  #ifdef sgi
 *  int  lio_wait4asyncio(int method, int fd, aiocb_t *aiocbp)
 *  int  lio_check_asyncio(char *io_type, int size, aiocb_t *aiocbp, int method)
 *  #endif
 *
 *  int  lio_parse_io_arg1(char *string)
 *  void lio_help1(char *prefix);
 *
 *  int  lio_parse_io_arg2(char *string, char **badtoken)
 *  void lio_help2(char *prefix);
 *
 *  int  lio_set_debug(int level);
 *
 *  char Lio_SysCall[];
 *  struct lio_info_type Lio_info1[];
 *  struct lio_info_type Lio_info2[];
 *
 *  Author : Richard Logan
 *
 */

#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#define _LARGEFILE64_SOURCE
#endif
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>
#include <stdint.h>
#ifdef CRAY
#include <sys/secparm.h>
#include <sys/iosw.h>
#include <sys/listio.h>
#else
/* for linux or sgi */
#include <sys/uio.h>		/* readv(2)/writev(2) */
#include <string.h>
#endif
#if defined(__linux__) || defined(__sun) || defined(__hpux) || defined(_AIX)
#if !defined(UCLINUX) && !defined(__UCLIBC__)
#include <aio.h>
#endif
#endif
#include <stdlib.h>		/* atoi, abs */

#include "tlibio.h"		/* defines LIO* marcos */
#include "random_range.h"

#ifndef PATH_MAX
#define PATH_MAX	MAXPATHLEN
#endif

#if 0				/* disabled until it's needed -- roehrich 6/11/97 */
#define BUG1_workaround	1	/* Work around a condition where aio_return gives
				 * a value of zero but there is no errno followup
				 * and the read/write operation actually did its
				 * job.   spr/pv 705244
				 */
#endif


/*
 * Define the structure as used in lio_parse_arg1 and lio_help1
 */
struct lio_info_type Lio_info1[] = {
	{"s", LIO_IO_SYNC, "sync i/o"},
	{"p", LIO_IO_ASYNC | LIO_WAIT_SIGACTIVE,
	 "async i/o using a loop to wait for a signal"},
	{"b", LIO_IO_ASYNC | LIO_WAIT_SIGPAUSE, "async i/o using pause"},
	{"a", LIO_IO_ASYNC | LIO_WAIT_RECALL,
	 "async i/o using recall/aio_suspend"},
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	{"r",
	 LIO_RANDOM | LIO_IO_TYPES | LIO_WAIT_TYPES,
	 "random sync i/o types and wait methods"},
	{"R",
	 LIO_RANDOM | LIO_IO_ATYPES | LIO_WAIT_ATYPES,
	 "random i/o types and wait methods"},
#else
	{"r",
	 LIO_RANDOM | LIO_IO_TYPES | LIO_WAIT_TYPES,
	 "random i/o types and wait methods"},
	{"R",
	 LIO_RANDOM | LIO_IO_TYPES | LIO_WAIT_TYPES,
	 "random i/o types and wait methods"},
#endif
	{"l", LIO_IO_SLISTIO | LIO_WAIT_RECALL, "single stride sync listio"},
	{"L", LIO_IO_ALISTIO | LIO_WAIT_RECALL,
	 "single stride async listio using recall"},
	{"X", LIO_IO_ALISTIO | LIO_WAIT_SIGPAUSE,
	 "single stride async listio using pause"},
	{"v", LIO_IO_SYNCV, "single buffer sync readv/writev"},
	{"P", LIO_IO_SYNCP, "sync pread/pwrite"},
};

/*
 * Define the structure used by lio_parse_arg2 and lio_help2
 */
struct lio_info_type Lio_info2[] = {
	{"sync", LIO_IO_SYNC, "sync i/o (read/write)"},
	{"async", LIO_IO_ASYNC, "async i/o (reada/writea/aio_read/aio_write)"},
	{"slistio", LIO_IO_SLISTIO, "single stride sync listio"},
	{"alistio", LIO_IO_ALISTIO, "single stride async listio"},
	{"syncv", LIO_IO_SYNCV, "single buffer sync readv/writev"},
	{"syncp", LIO_IO_SYNCP, "pread/pwrite"},
	{"active", LIO_WAIT_ACTIVE, "spin on status/control values"},
	{"recall", LIO_WAIT_RECALL,
	 "use recall(2)/aio_suspend(3) to wait for i/o to complete"},
	{"sigactive", LIO_WAIT_SIGACTIVE, "spin waiting for signal"},
	{"sigpause", LIO_WAIT_SIGPAUSE, "call pause(2) to wait for signal"},
/* nowait is a touchy thing, it's an accident that this implementation worked at all.  6/27/97 roehrich */
/*    { "nowait",    LIO_WAIT_NONE,	"do not wait for async io to complete" },*/
	{"random", LIO_RANDOM, "set random bit"},
	{"randomall",
	 LIO_RANDOM | LIO_IO_TYPES | LIO_WAIT_TYPES,
	 "all random i/o types and wait methods (except nowait)"},
};

char Lio_SysCall[PATH_MAX];	/* string containing last i/o system call */

static volatile int Received_signal = 0;	/* number of signals received */
static volatile int Rec_signal;
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
static volatile int Received_callback = 0;	/* number of callbacks received */
static volatile int Rec_callback;
#endif
static char Errormsg[PATH_MAX*2];
static int Debug_level = 0;

/***********************************************************************
 * stride_bounds()
 *
 * Determine the bounds of a strided request, normalized to offset.  Returns
 * the number of bytes needed to satisfy the request, and optionally sets
 * *min and *max to the mininum and maximum bytes referenced, normalized
 * around offset.
 *
 * Returns -1 on error - the only possible error conditions are illegal values
 * for nstrides and/or bytes_per_stride - both parameters must be >= 0.
 *
 * (maule, 11/16/95)
 ***********************************************************************/

int stride_bounds(int offset, int stride, int nstrides, int bytes_per_stride,
		int *min, int *max)
{
	int nbytes, min_byte, max_byte;

	/*
	 * sanity checks ...
	 */

	if (nstrides < 0 || bytes_per_stride < 0) {
		return -1;
	}

	if (stride == 0) {
		stride = bytes_per_stride;
	}

	/*
	 * Determine the # of bytes needed to satisfy the request.  This
	 * value, along with the offset argument, determines the min and max
	 * bytes referenced.
	 */

	nbytes = abs(stride) * (nstrides - 1) + bytes_per_stride;

	if (stride < 0) {
		max_byte = offset + bytes_per_stride - 1;
		min_byte = max_byte - nbytes + 1;
	} else {
		min_byte = offset;
		max_byte = min_byte + nbytes - 1;
	}

	if (min != NULL) {
		*min = min_byte;
	}

	if (max != NULL) {
		*max = max_byte;
	}

	return nbytes;
}

/***********************************************************************
 * This function will allow someone to set the debug level.
 ***********************************************************************/
int lio_set_debug(int level)
{
	int old;

	old = Debug_level;
	Debug_level = level;
	return old;
}

/***********************************************************************
 * This function will parse a string and return desired io-method.
 * Only the first character of the string is used.
 *
 * This function does not provide for meaningful option arguments,
 * but it supports current growfiles/btlk interface.
 *
 *  (rrl 04/96)
 ***********************************************************************/
int lio_parse_io_arg1(char *string)
{
	unsigned int ind;
	int found = 0;
	int mask = 0;

	/*
	 * Determine if token is a valid string.
	 */
	for (ind = 0; ind < sizeof(Lio_info1) / sizeof(struct lio_info_type);
	     ind++) {
		if (strcmp(string, Lio_info1[ind].token) == 0) {
			mask |= Lio_info1[ind].bits;
			found = 1;
			break;
		}
	}

	if (found == 0) {
		return -1;
	}

	return mask;

}

/***********************************************************************
 * This function will print a help message describing the characters
 * that can be parsed by lio_parse_io_arg1().
 * They will be printed one per line.
 *  (rrl 04/96)
 ***********************************************************************/
void lio_help1(char *prefix)
{
	unsigned int ind;

	for (ind = 0; ind < sizeof(Lio_info1) / sizeof(struct lio_info_type);
	     ind++) {
		printf("%s %s : %s\n", prefix, Lio_info1[ind].token,
		       Lio_info1[ind].desc);
	}

	return;
}

/***********************************************************************
 * This function will parse a string and return the desired io-method.
 * This function will take a comma separated list of io type and wait
 * method tokens as defined in Lio_info2[].  If a token does not match
 * any of the tokens in Lio_info2[], it will be coverted to a number.
 * If it was a number, those bits are also set.
 *
 *  (rrl 04/96)
 ***********************************************************************/
int lio_parse_io_arg2(char *string, char **badtoken)
{
	char *token = string;
	char *cc = token;
	char savecc;
	int found;
	int mask = 0;

	int tmp;
	unsigned int ind;
	char chr;

	if (token == NULL)
		return -1;

	for (;;) {
		for (; ((*cc != ',') && (*cc != '\0')); cc++) ;
		savecc = *cc;
		*cc = '\0';

		found = 0;

		/*
		 * Determine if token is a valid string or number and if
		 * so, add the bits to the mask.
		 */
		for (ind = 0;
		     ind < sizeof(Lio_info2) / sizeof(struct lio_info_type);
		     ind++) {
			if (strcmp(token, Lio_info2[ind].token) == 0) {
				mask |= Lio_info2[ind].bits;
				found = 1;
				break;
			}
		}

		/*
		 * If token does not match one of the defined tokens, determine
		 * if it is a number, if so, add the bits.
		 */
		if (!found) {
			if (sscanf(token, "%i%c", &tmp, &chr) == 1) {
				mask |= tmp;
				found = 1;
			}
		}

		*cc = savecc;

		if (!found) {	/* token is not valid */
			if (badtoken != NULL)
				*badtoken = token;
			return (-1);
		}

		if (savecc == '\0')
			break;

		token = ++cc;
	}

	return mask;
}

/***********************************************************************
 * This function will print a help message describing the tokens
 * that can be parsed by lio_parse_io_arg2().
 * It will print them one per line.
 *
 * (rrl 04/96)
 ***********************************************************************/
void lio_help2(char *prefix)
{
	unsigned int ind;

	for (ind = 0; ind < sizeof(Lio_info2) / sizeof(struct lio_info_type);
	     ind++) {
		printf("%s %s : %s\n", prefix, Lio_info2[ind].token,
		       Lio_info2[ind].desc);
	}
	return;
}

/***********************************************************************
 * This is an internal signal handler.
 * If the handler is called, it will increment the Received_signal
 * global variable.
 ***********************************************************************/
static void lio_async_signal_handler(int sig)
{
	if (Debug_level)
		printf
		    ("DEBUG %s/%d: received signal %d, a signal caught %d times\n",
		     __FILE__, __LINE__, sig, Received_signal + 1);

	Received_signal++;

	return;
}

#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
/***********************************************************************
 * This is an internal callback handler.
 * If the handler is called, it will increment the Received_callback
 * global variable.
 ***********************************************************************/
static void lio_async_callback_handler(union sigval sigval)
{
	if (Debug_level)
		printf
		    ("DEBUG %s/%d: received callback, nbytes=%ld, a callback called %d times\n",
		     __FILE__, __LINE__, (long)sigval.sival_int,
		     Received_callback + 1);

	Received_callback++;

	return;
}
#endif /* sgi */

/***********************************************************************
 * lio_random_methods
 * This function will randomly choose an io type and wait method
 * from set of io types and wait methods.  Since this information
 * is stored in a bitmask, it randomly chooses an io type from
 * the io type bits specified and does the same for wait methods.
 *
 * Return Value
 * This function will return a value with all non choosen io type
 * and wait method bits cleared.  The LIO_RANDOM bit is also
 * cleared.  All other bits are left unchanged.
 *
 * (rrl 04/96)
 ***********************************************************************/
int lio_random_methods(long curr_mask)
{
	int mask = 0;

	/* remove random select, io type, and wait method bits from curr_mask */
	mask = curr_mask & (~(LIO_IO_TYPES | LIO_WAIT_TYPES | LIO_RANDOM));

	/* randomly select io type from specified io types */
	mask = mask | random_bit(curr_mask & LIO_IO_TYPES);

	/* randomly select wait methods  from specified wait methods */
	mask = mask | random_bit(curr_mask & LIO_WAIT_TYPES);

	return mask;
}

static void wait4sync_io(int fd, int read)
{
	fd_set s;
	FD_ZERO(&s);
	FD_SET(fd, &s);

	select(fd + 1, read ? &s : NULL, read ? NULL : &s, NULL, NULL);
}

/***********************************************************************
 * Generic write function
 * This function can be used to do a write using write(2), writea(2),
 * aio_write(3), writev(2), pwrite(2),
 * or single stride listio(2)/lio_listio(3).
 * By setting the desired bits in the method
 * bitmask, the caller can control the type of write and the wait method
 * that will be used.  If no io type bits are set, write will be used.
 *
 * If async io was attempted and no wait method bits are set then the
 * wait method is: recall(2) for writea(2) and listio(2); aio_suspend(3) for
 * aio_write(3) and lio_listio(3).
 *
 * If multiple wait methods are specified,
 * only one wait method will be used. The order is predetermined.
 *
 * If the call specifies a signal and one of the two signal wait methods,
 * a signal handler for the signal is set.  This will reset an already
 * set handler for this signal.
 *
 * If the LIO_RANDOM method bit is set, this function will randomly
 * choose a io type and wait method from bits in the method argument.
 *
 * If an error is encountered, an error message will be generated
 * in a internal static buffer.  If errmsg is not NULL, it will
 * be updated to point to the static buffer, allowing the caller
 * to print the error message.
 *
 * Return Value
 *   If a system call fails, -errno is returned.
 *   If LIO_WAIT_NONE bit is set, the return value is the return value
 *   of the system call.
 *   If the io did not fail, the amount of data written is returned.
 *	If the size the system call say was written is different
 *	then what was asked to be written, errmsg is updated for
 *	this error condition.  The return value is still the amount
 *	the system call says was written.
 *
 * (rrl 04/96)
 ***********************************************************************/
int lio_write_buffer(int fd,		/* open file descriptor */
		int method,	/* contains io type and wait method bitmask */
		char *buffer,	/* pointer to buffer */
		int size,	/* the size of the io */
		int sig,	/* signal to use if async io */
		char **errmsg,	/* char pointer that will be updated to point to err message */
		long wrd)	/* to allow future features, use zero for now */
{
	int ret = 0;		/* syscall return or used to get random method */
	int totally_written = 0;/* as we cycle writes in case of partial writes, */
				/* we have to report up total bytes written */
	char *io_type;		/* Holds string of type of io */
	int omethod = method;
	int listio_cmd;		/* Holds the listio/lio_listio cmd */
#ifdef  CRAY
	struct listreq request;	/* Used when a listio is wanted */
	struct iosw status, *statptr[1];
#else
	/* for linux or sgi */
	struct iovec iov;	/* iovec for writev(2) */
#endif
#if defined (sgi)
	aiocb_t aiocbp;		/* POSIX aio control block */
	aiocb_t *aiolist[1];	/* list of aio control blocks for lio_listio */
	off64_t poffset;	/* pwrite(2) offset */
#endif
#if defined(__linux__) && !defined(__UCLIBC__)
	struct aiocb aiocbp;	/* POSIX aio control block */
	struct aiocb *aiolist[1];	/* list of aio control blocks for lio_listio */
	off64_t poffset;	/* pwrite(2) offset */
#endif
	/*
	 * If LIO_RANDOM bit specified, get new method randomly.
	 */
	if (method & LIO_RANDOM) {
		if (Debug_level > 3)
			printf("DEBUG %s/%d: method mask to choose from: %#o\n",
			       __FILE__, __LINE__, method);
		method = lio_random_methods(method);
		if (Debug_level > 2)
			printf("DEBUG %s/%d: random chosen method %#o\n",
			       __FILE__, __LINE__, method);
	}

	if (errmsg != NULL)
		*errmsg = Errormsg;

	Rec_signal = Received_signal;	/* get the current number of signals received */
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	Rec_callback = Received_callback;	/* get the current number of callbacks received */
#endif

#ifdef  CRAY
	memset(&status, 0x00, sizeof(struct iosw));
	memset(&request, 0x00, sizeof(struct listreq));
	statptr[0] = &status;
#else
	/* for linux or sgi */
	memset(&iov, 0x00, sizeof(struct iovec));
	iov.iov_base = buffer;
	iov.iov_len = size;
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
#if defined(sgi)
	memset(&aiocbp, 0x00, sizeof(aiocb_t));
#else
	memset(&aiocbp, 0x00, sizeof(struct aiocb));
#endif
	aiocbp.aio_fildes = fd;
	aiocbp.aio_nbytes = size;
	aiocbp.aio_buf = buffer;
/*    aiocbp.aio_offset = lseek( fd, 0, SEEK_CUR ); -- set below */
	aiocbp.aio_sigevent.sigev_notify = SIGEV_NONE;
	aiocbp.aio_sigevent.sigev_signo = 0;
#ifdef sgi
	aiocbp.aio_sigevent.sigev_func = NULL;
	aiocbp.aio_sigevent.sigev_value.sival_int = 0;
#elif defined(__linux__) && !defined(__UCLIBC__)
	aiocbp.aio_sigevent.sigev_notify_function = NULL;
	aiocbp.aio_sigevent.sigev_notify_attributes = 0;
#endif
	aiolist[0] = &aiocbp;

	if ((ret = lseek(fd, 0, SEEK_CUR)) == -1) {
		ret = 0;
		/* If there is an error and it is not ESPIPE then kick out the error.
		 * If the fd is a fifo then we have to make sure that
		 * lio_random_methods() didn't select pwrite/pread; if it did then
		 * switch to write/read.
		 */
		if (errno == ESPIPE) {
			if (method & LIO_IO_SYNCP) {
				if (omethod & LIO_RANDOM) {
					method &= ~LIO_IO_SYNCP;
					method |= LIO_IO_SYNC;
					if (Debug_level > 2)
						printf
						    ("DEBUG %s/%d: random chosen method switched to %#o for fifo\n",
						     __FILE__, __LINE__,
						     method);
				} else if (Debug_level) {
					printf
					    ("DEBUG %s/%d: pwrite will fail when it writes to a fifo\n",
					     __FILE__, __LINE__);
				}
			}
			/* else: let it ride */
		} else {
			sprintf(Errormsg,
				"%s/%d lseek(fd=%d,0,SEEK_CUR) failed, errno=%d  %s",
				__FILE__, __LINE__, fd, errno, strerror(errno));
			return -errno;
		}
	}
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	poffset = (off64_t) ret;
#endif
	aiocbp.aio_offset = ret;

#endif

	/*
	 * If the LIO_USE_SIGNAL bit is not set, only use the signal
	 * if the LIO_WAIT_SIGPAUSE or the LIO_WAIT_SIGACTIVE bits are bit.
	 * Otherwise there is not necessary a signal handler to trap
	 * the signal.
	 */
	if (sig && !(method & LIO_USE_SIGNAL) && !(method & LIO_WAIT_SIGTYPES)) {

		sig = 0;	/* ignore signal parameter */
	}
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	if (sig && (method & LIO_WAIT_CBTYPES))
		sig = 0;	/* ignore signal parameter */
#endif

	/*
	 * only setup signal hander if sig was specified and
	 * a sig wait method was specified.
	 * Doing this will change the handler for this signal.  The
	 * old signal handler will not be restored.
	 *** restoring the signal handler could be added ***
	 */

	if (sig && (method & LIO_WAIT_SIGTYPES)) {
#ifdef CRAY
		sigctl(SCTL_REG, sig, lio_async_signal_handler);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		aiocbp.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		aiocbp.aio_sigevent.sigev_signo = sig;
		sigset(sig, lio_async_signal_handler);
#endif /* sgi */
	}
#if defined(sgi)
	else if (method & LIO_WAIT_CBTYPES) {
		/* sival_int just has to be something that I can use
		 * to identify the callback, and "size" happens to be handy...
		 */
		aiocbp.aio_sigevent.sigev_notify = SIGEV_CALLBACK;
		aiocbp.aio_sigevent.sigev_func = lio_async_callback_handler;
		aiocbp.aio_sigevent.sigev_value.sival_int = size;
	}
#endif
#if defined(__linux__) && !defined(__UCLIBC__)
	else if (method & LIO_WAIT_CBTYPES) {
		/* sival_int just has to be something that I can use
		 * to identify the callback, and "size" happens to be handy...
		 */
		aiocbp.aio_sigevent.sigev_notify = SIGEV_THREAD;
		aiocbp.aio_sigevent.sigev_notify_function =
		    lio_async_callback_handler;
		aiocbp.aio_sigevent.sigev_notify_attributes =
		    (void *)(uintptr_t) size;
	}
#endif
	/*
	 * Determine the system call that will be called and produce
	 * the string of the system call and place it in Lio_SysCall.
	 * Also update the io_type char pointer to give brief description
	 * of system call.  Execute the system call and check for
	 * system call failure.  If sync i/o, return the number of
	 * bytes written/read.
	 */

	if ((method & LIO_IO_SYNC)
	    || (method & (LIO_IO_TYPES | LIO_IO_ATYPES)) == 0) {
		/*
		 * write(2) is used if LIO_IO_SYNC bit is set or not none
		 * of the LIO_IO_TYPES bits are set (default).
		 */

		sprintf(Lio_SysCall, "write(%d, buf, %d)", fd, size);
		io_type = "write";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}
		while (1) {
			if (((ret = write(fd, buffer, size)) == -1)
			    && errno != EAGAIN && errno != EINTR) {
				sprintf(Errormsg,
					"%s/%d write(%d, buf, %d) ret:-1, errno=%d %s",
					__FILE__, __LINE__, fd, size, errno,
					strerror(errno));
				return -errno;
			}

			if (ret != -1) {
				if (ret != size) {
					sprintf(Errormsg,
						"%s/%d write(%d, buf, %d) returned=%d",
						__FILE__, __LINE__,
						fd, size, ret);
					size -= ret;
					buffer += ret;
					totally_written += ret;
				} else {
					if (Debug_level > 1)
						printf
						    ("DEBUG %s/%d: write completed without error (ret %d)\n",
						     __FILE__, __LINE__, ret);

					return totally_written + ret;
				}
			}
			wait4sync_io(fd, 0);
		}

	}

	else if (method & LIO_IO_ASYNC) {
#ifdef CRAY
		sprintf(Lio_SysCall,
			"writea(%d, buf, %d, &status, %d)", fd, size, sig);
		io_type = "writea";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if ((ret = writea(fd, buffer, size, &status, sig)) == -1) {
			sprintf(Errormsg,
				"%s/%d writea(%d, buf, %d, &stat, %d) ret:-1, errno=%d %s",
				__FILE__, __LINE__,
				fd, size, sig, errno, strerror(errno));
			sigon();
			return -errno;
		}
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		sprintf(Lio_SysCall,
			"aio_write(fildes=%d, buf, nbytes=%d, signo=%d)", fd,
			size, sig);
		io_type = "aio_write";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if ((ret = aio_write(&aiocbp)) == -1) {
			sprintf(Errormsg,
				"%s/%d aio_write(fildes=%d, buf, nbytes=%d, signo=%d) ret:-1, errno=%d %s",
				__FILE__, __LINE__,
				fd, size, sig, errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}
#endif
	}
	/* LIO_IO_ASYNC */
	else if (method & LIO_IO_SLISTIO) {
#ifdef CRAY
		request.li_opcode = LO_WRITE;
		request.li_fildes = fd;
		request.li_buf = buffer;
		request.li_nbyte = size;
		request.li_status = &status;
		request.li_signo = sig;
		request.li_nstride = 0;
		request.li_filstride = 0;
		request.li_memstride = 0;

		listio_cmd = LC_WAIT;
		io_type = "listio(2) sync write";

		sprintf(Lio_SysCall,
			"listio(LC_WAIT, &req, 1) LO_WRITE, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if (listio(listio_cmd, &request, 1) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			sigon();
			return -errno;
		}

		if (Debug_level > 1)
			printf("DEBUG %s/%d: %s did not return -1\n",
			       __FILE__, __LINE__, Lio_SysCall);

		ret = lio_check_asyncio(io_type, size, &status);
		return ret;

#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))

		aiocbp.aio_lio_opcode = LIO_WRITE;
		listio_cmd = LIO_WAIT;
		io_type = "lio_listio(3) sync write";

		sprintf(Lio_SysCall,
			"lio_listio(LIO_WAIT, aiolist, 1, NULL) LIO_WRITE, fd:%d, nbyte:%d, sig:%d",
			fd, size, sig);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if (lio_listio(listio_cmd, aiolist, 1, NULL) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}

		if (Debug_level > 1)
			printf("DEBUG %s/%d: %s did not return -1\n",
			       __FILE__, __LINE__, Lio_SysCall);

		ret = lio_check_asyncio(io_type, size, &aiocbp, method);
		return ret;
#endif
	}
	/* LIO_IO_SLISTIO */
	else if (method & LIO_IO_ALISTIO) {
#ifdef CRAY
		request.li_opcode = LO_WRITE;
		request.li_fildes = fd;
		request.li_buf = buffer;
		request.li_nbyte = size;
		request.li_status = &status;
		request.li_signo = sig;
		request.li_nstride = 0;
		request.li_filstride = 0;
		request.li_memstride = 0;

		listio_cmd = LC_START;
		io_type = "listio(2) async write";

		sprintf(Lio_SysCall,
			"listio(LC_START, &req, 1) LO_WRITE, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if (listio(listio_cmd, &request, 1) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			sigon();
			return -errno;
		}
#endif
#if defined (sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		aiocbp.aio_lio_opcode = LIO_WRITE;
		listio_cmd = LIO_NOWAIT;
		io_type = "lio_listio(3) async write";

		sprintf(Lio_SysCall,
			"lio_listio(LIO_NOWAIT, aiolist, 1, NULL) LIO_WRITE, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if (lio_listio(listio_cmd, aiolist, 1, NULL) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}
#endif
	}
	/* LIO_IO_ALISTIO */
#ifndef CRAY
	else if (method & LIO_IO_SYNCV) {
		io_type = "writev(2)";

		sprintf(Lio_SysCall, "writev(%d, &iov, 1) nbyte:%d", fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}
		if ((ret = writev(fd, &iov, 1)) == -1) {
			sprintf(Errormsg,
				"%s/%d writev(%d, iov, 1) nbyte:%d ret:-1, errno=%d %s",
				__FILE__, __LINE__, fd, size, errno,
				strerror(errno));
			return -errno;
		}

		if (ret != size) {
			sprintf(Errormsg,
				"%s/%d writev(%d, iov, 1) nbyte:%d returned=%d",
				__FILE__, __LINE__, fd, size, ret);
		} else if (Debug_level > 1)
			printf
			    ("DEBUG %s/%d: writev completed without error (ret %d)\n",
			     __FILE__, __LINE__, ret);

		return ret;
	}			/* LIO_IO_SYNCV */
#endif

#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	else if (method & LIO_IO_SYNCP) {
		io_type = "pwrite(2)";

		sprintf(Lio_SysCall,
			"pwrite(%d, buf, %d, %lld)", fd, size,
			(long long)poffset);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}
		if ((ret = pwrite(fd, buffer, size, poffset)) == -1) {
			sprintf(Errormsg,
				"%s/%d pwrite(%d, buf, %d, %lld) ret:-1, errno=%d %s",
				__FILE__, __LINE__, fd, size,
				(long long)poffset, errno, strerror(errno));
			return -errno;
		}

		if (ret != size) {
			sprintf(Errormsg,
				"%s/%d pwrite(%d, buf, %d, %lld) returned=%d",
				__FILE__, __LINE__,
				fd, size, (long long)poffset, ret);
		} else if (Debug_level > 1)
			printf
			    ("DEBUG %s/%d: pwrite completed without error (ret %d)\n",
			     __FILE__, __LINE__, ret);

		return ret;
	}			/* LIO_IO_SYNCP */
#endif

	else {
		printf("DEBUG %s/%d: No I/O method chosen\n", __FILE__,
		       __LINE__);
		return -1;
	}

	/*
	 * wait for async io to complete.
	 */
#ifdef CRAY
	ret = lio_wait4asyncio(method, fd, statptr);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	ret = lio_wait4asyncio(method, fd, &aiocbp);
#endif

	/*
	 * If there was an error waiting for async i/o to complete,
	 * return the error value (errno) to the caller.
	 * Note: Errormsg should already have been updated.
	 */
	if (ret < 0) {
		return ret;
	}

	/*
	 * If i/o was not waited for (may not have been completed at this time),
	 * return the size that was requested.
	 */
	if (ret == 1)
		return size;

	/*
	 * check that async io was successful.
	 * Note:  if the there was an system call failure, -errno
	 * was returned and Errormsg should already have been updated.
	 * If amount i/o was different than size, Errormsg should already
	 * have been updated but the actual i/o size if returned.
	 */

#ifdef CRAY
	ret = lio_check_asyncio(io_type, size, &status);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	ret = lio_check_asyncio(io_type, size, &aiocbp, method);
#endif

	return ret;
}				/* end of lio_write_buffer */

/***********************************************************************
 * Generic read function
 * This function can be used to do a read using read(2), reada(2),
 * aio_read(3), readv(2), pread(2),
 * or single stride listio(2)/lio_listio(3).
 * By setting the desired bits in the method
 * bitmask, the caller can control the type of read and the wait method
 * that will be used.  If no io type bits are set, read will be used.
 *
 * If async io was attempted and no wait method bits are set then the
 * wait method is: recall(2) for reada(2) and listio(2); aio_suspend(3) for
 * aio_read(3) and lio_listio(3).
 *
 * If multiple wait methods are specified,
 * only one wait method will be used. The order is predetermined.
 *
 * If the call specifies a signal and one of the two signal wait methods,
 * a signal handler for the signal is set.  This will reset an already
 * set handler for this signal.
 *
 * If the LIO_RANDOM method bit is set, this function will randomly
 * choose a io type and wait method from bits in the method argument.
 *
 * If an error is encountered, an error message will be generated
 * in a internal static buffer.  If errmsg is not NULL, it will
 * be updated to point to the static buffer, allowing the caller
 * to print the error message.
 *
 * Return Value
 *   If a system call fails, -errno is returned.
 *   If LIO_WAIT_NONE bit is set, the return value is the return value
 *   of the system call.
 *   If the io did not fail, the amount of data written is returned.
 *	If the size the system call say was written is different
 *	then what was asked to be written, errmsg is updated for
 *	this error condition.  The return value is still the amount
 *	the system call says was written.
 *
 * (rrl 04/96)
 ***********************************************************************/
int lio_read_buffer(int fd,	/* open file descriptor */
		int method,	/* contains io type and wait method bitmask*/
		char *buffer,	/* pointer to buffer */
		int size,	/* the size of the io */
		int sig,	/* signal to use if async io */
		char **errmsg,	/* char pointer that will be updated to point to err message */
		long wrd)	/* to allow future features, use zero for now */
{
	int ret = 0;		/* syscall return or used to get random method */
	char *io_type;		/* Holds string of type of io */
	int listio_cmd;		/* Holds the listio/lio_listio cmd */
	int omethod = method;
#ifdef  CRAY
	struct listreq request;	/* Used when a listio is wanted */
	struct iosw status, *statptr[1];
#else
	/* for linux or sgi */
	struct iovec iov;	/* iovec for readv(2) */
#endif
#ifdef sgi
	aiocb_t aiocbp;		/* POSIX aio control block */
	aiocb_t *aiolist[1];	/* list of aio control blocks for lio_listio */
	off64_t poffset;	/* pread(2) offset */
#endif
#if defined (__linux__) && !defined(__UCLIBC__)
	struct aiocb aiocbp;	/* POSIX aio control block */
	struct aiocb *aiolist[1];	/* list of aio control blocks for lio_listio */
	off64_t poffset;	/* pread(2) offset */
#endif

	/*
	 * If LIO_RANDOM bit specified, get new method randomly.
	 */
	if (method & LIO_RANDOM) {
		if (Debug_level > 3)
			printf("DEBUG %s/%d: method mask to choose from: %#o\n",
			       __FILE__, __LINE__, method);
		method = lio_random_methods(method);
		if (Debug_level > 2)
			printf("DEBUG %s/%d: random chosen method %#o\n",
			       __FILE__, __LINE__, method);
	}

	if (errmsg != NULL)
		*errmsg = Errormsg;

	Rec_signal = Received_signal;	/* get the current number of signals received */
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	Rec_callback = Received_callback;	/* get the current number of callbacks received */
#endif

#ifdef  CRAY
	memset(&status, 0x00, sizeof(struct iosw));
	memset(&request, 0x00, sizeof(struct listreq));
	statptr[0] = &status;
#else
	/* for linux or sgi */
	memset(&iov, 0x00, sizeof(struct iovec));
	iov.iov_base = buffer;
	iov.iov_len = size;
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
#if defined(sgi)
	memset(&aiocbp, 0x00, sizeof(aiocb_t));
#else
	memset(&aiocbp, 0x00, sizeof(struct aiocb));
#endif
	aiocbp.aio_fildes = fd;
	aiocbp.aio_nbytes = size;
	aiocbp.aio_buf = buffer;
/*    aiocbp.aio_offset = lseek( fd, 0, SEEK_CUR ); -- set below */
	aiocbp.aio_sigevent.sigev_notify = SIGEV_NONE;
	aiocbp.aio_sigevent.sigev_signo = 0;
#ifdef sgi
	aiocbp.aio_sigevent.sigev_func = NULL;
	aiocbp.aio_sigevent.sigev_value.sival_int = 0;
#elif defined(__linux__) && !defined(__UCLIBC__)
	aiocbp.aio_sigevent.sigev_notify_function = NULL;
	aiocbp.aio_sigevent.sigev_notify_attributes = 0;
#endif
	aiolist[0] = &aiocbp;

	if ((ret = lseek(fd, 0, SEEK_CUR)) == -1) {
		ret = 0;
		/* If there is an error and it is not ESPIPE then kick out the error.
		 * If the fd is a fifo then we have to make sure that
		 * lio_random_methods() didn't select pwrite/pread; if it did then
		 * switch to write/read.
		 */
		if (errno == ESPIPE) {
			if (method & LIO_IO_SYNCP) {
				if (omethod & LIO_RANDOM) {
					method &= ~LIO_IO_SYNCP;
					method |= LIO_IO_SYNC;
					if (Debug_level > 2)
						printf
						    ("DEBUG %s/%d: random chosen method switched to %#o for fifo\n",
						     __FILE__, __LINE__,
						     method);
				} else if (Debug_level) {
					printf
					    ("DEBUG %s/%d: pread will fail when it reads from a fifo\n",
					     __FILE__, __LINE__);
				}
			}
			/* else: let it ride */
		} else {
			sprintf(Errormsg,
				"%s/%d lseek(fd=%d,0,SEEK_CUR) failed, errno=%d  %s",
				__FILE__, __LINE__, fd, errno, strerror(errno));
			return -errno;
		}
	}
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	poffset = (off64_t) ret;
#endif
	aiocbp.aio_offset = ret;

#endif

	/*
	 * If the LIO_USE_SIGNAL bit is not set, only use the signal
	 * if the LIO_WAIT_SIGPAUSE or the LIO_WAIT_SIGACTIVE bits are set.
	 * Otherwise there is not necessarily a signal handler to trap
	 * the signal.
	 */
	if (sig && !(method & LIO_USE_SIGNAL) && !(method & LIO_WAIT_SIGTYPES)) {

		sig = 0;	/* ignore signal parameter */
	}
#if defined(sgi) || (defined(__linux__)&& !defined(__UCLIBC__))
	if (sig && (method & LIO_WAIT_CBTYPES))
		sig = 0;	/* ignore signal parameter */
#endif

	/*
	 * only setup signal hander if sig was specified and
	 * a sig wait method was specified.
	 * Doing this will change the handler for this signal.  The
	 * old signal handler will not be restored.
	 *** restoring the signal handler could be added ***
	 */

	if (sig && (method & LIO_WAIT_SIGTYPES)) {
#ifdef CRAY
		sigctl(SCTL_REG, sig, lio_async_signal_handler);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		aiocbp.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
		aiocbp.aio_sigevent.sigev_signo = sig;
		sigset(sig, lio_async_signal_handler);
#endif /* CRAY */
	}
#if defined(sgi)
	else if (method & LIO_WAIT_CBTYPES) {
		aiocbp.aio_sigevent.sigev_notify = SIGEV_CALLBACK;
		aiocbp.aio_sigevent.sigev_func = lio_async_callback_handler;
		/* sival_int just has to be something that I can use
		 * to identify the callback, and "size" happens to be handy...
		 */
		aiocbp.aio_sigevent.sigev_value.sival_int = size;
	}
#endif
#if defined(__linux__) && !defined(__UCLIBC__)
	else if (method & LIO_WAIT_CBTYPES) {
		aiocbp.aio_sigevent.sigev_notify = SIGEV_THREAD;
		aiocbp.aio_sigevent.sigev_notify_function =
		    lio_async_callback_handler;
		/* sival_int just has to be something that I can use
		 * to identify the callback, and "size" happens to be handy...
		 */
		aiocbp.aio_sigevent.sigev_notify_attributes =
		    (void *)(uintptr_t) size;
	}
#endif

	/*
	 * Determine the system call that will be called and produce
	 * the string of the system call and place it in Lio_SysCall.
	 * Also update the io_type char pointer to give brief description
	 * of system call.  Execute the system call and check for
	 * system call failure.  If sync i/o, return the number of
	 * bytes written/read.
	 */

	if ((method & LIO_IO_SYNC)
	    || (method & (LIO_IO_TYPES | LIO_IO_ATYPES)) == 0) {
		/*
		 * read(2) is used if LIO_IO_SYNC bit is set or not none
		 * of the LIO_IO_TYPES bits are set (default).
		 */

		sprintf(Lio_SysCall, "read(%d, buf, %d)", fd, size);
		io_type = "read";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		while (1) {
			if (((ret = read(fd, buffer, size)) == -1)
			    && errno != EINTR && errno != EAGAIN) {
				sprintf(Errormsg,
					"%s/%d read(%d, buf, %d) ret:-1, errno=%d %s",
					__FILE__, __LINE__, fd, size, errno,
					strerror(errno));
				return -errno;
			}

			if (ret == 0)
				return 0;
			if (ret != -1) {
				if (ret != size) {
					sprintf(Errormsg,
						"%s/%d read(%d, buf, %d) returned=%d",
						__FILE__, __LINE__,
						fd, size, ret);
					size -= ret;
					buffer += ret;
				} else {
					if (Debug_level > 1)
						printf
						    ("DEBUG %s/%d: read completed without error (ret %d)\n",
						     __FILE__, __LINE__, ret);

					return ret;
				}
			}
			wait4sync_io(fd, 1);
		}

	}

	else if (method & LIO_IO_ASYNC) {
#ifdef CRAY
		sprintf(Lio_SysCall,
			"reada(%d, buf, %d, &status, %d)", fd, size, sig);
		io_type = "reada";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if ((ret = reada(fd, buffer, size, &status, sig)) == -1) {
			sprintf(Errormsg,
				"%s/%d reada(%d, buf, %d, &stat, %d) ret:-1, errno=%d %s",
				__FILE__, __LINE__,
				fd, size, sig, errno, strerror(errno));
			sigon();
			return -errno;
		}
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		sprintf(Lio_SysCall,
			"aio_read(fildes=%d, buf, nbytes=%d, signo=%d)", fd,
			size, sig);
		io_type = "aio_read";

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if ((ret = aio_read(&aiocbp)) == -1) {
			sprintf(Errormsg,
				"%s/%d aio_read(fildes=%d, buf, nbytes=%d, signo=%d) ret:-1, errno=%d %s",
				__FILE__, __LINE__,
				fd, size, sig, errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}
#endif
	}
	/* LIO_IO_ASYNC */
	else if (method & LIO_IO_SLISTIO) {
#ifdef CRAY
		request.li_opcode = LO_READ;
		request.li_fildes = fd;
		request.li_buf = buffer;
		request.li_nbyte = size;
		request.li_status = &status;
		request.li_signo = sig;
		request.li_nstride = 0;
		request.li_filstride = 0;
		request.li_memstride = 0;

		listio_cmd = LC_WAIT;
		io_type = "listio(2) sync read";

		sprintf(Lio_SysCall,
			"listio(LC_WAIT, &req, 1) LO_READ, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if (listio(listio_cmd, &request, 1) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			sigon();
			return -errno;
		}

		if (Debug_level > 1)
			printf("DEBUG %s/%d: %s did not return -1\n",
			       __FILE__, __LINE__, Lio_SysCall);

		ret = lio_check_asyncio(io_type, size, &status);
		return ret;
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		aiocbp.aio_lio_opcode = LIO_READ;
		listio_cmd = LIO_WAIT;
		io_type = "lio_listio(3) sync read";

		sprintf(Lio_SysCall,
			"lio_listio(LIO_WAIT, aiolist, 1, NULL) LIO_READ, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if (lio_listio(listio_cmd, aiolist, 1, NULL) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}

		if (Debug_level > 1)
			printf("DEBUG %s/%d: %s did not return -1\n",
			       __FILE__, __LINE__, Lio_SysCall);

		ret = lio_check_asyncio(io_type, size, &aiocbp, method);
		return ret;
#endif
	}
	/* LIO_IO_SLISTIO */
	else if (method & LIO_IO_ALISTIO) {
#ifdef CRAY
		request.li_opcode = LO_READ;
		request.li_fildes = fd;
		request.li_buf = buffer;
		request.li_nbyte = size;
		request.li_status = &status;
		request.li_signo = sig;
		request.li_nstride = 0;
		request.li_filstride = 0;
		request.li_memstride = 0;

		listio_cmd = LC_START;
		io_type = "listio(2) async read";

		sprintf(Lio_SysCall,
			"listio(LC_START, &req, 1) LO_READ, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		sigoff();
		if (listio(listio_cmd, &request, 1) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			sigon();
			return -errno;
		}
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
		aiocbp.aio_lio_opcode = LIO_READ;
		listio_cmd = LIO_NOWAIT;
		io_type = "lio_listio(3) async read";

		sprintf(Lio_SysCall,
			"lio_listio(LIO_NOWAIT, aiolist, 1, NULL) LIO_READ, fd:%d, nbyte:%d",
			fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}

		if (sig)
			sighold(sig);
		if (lio_listio(listio_cmd, aiolist, 1, NULL) == -1) {
			sprintf(Errormsg,
				"%s/%d %s failed, fd:%d, nbyte:%d errno=%d %s",
				__FILE__, __LINE__, Lio_SysCall, fd, size,
				errno, strerror(errno));
			if (sig)
				sigrelse(sig);
			return -errno;
		}
#endif
	}
	/* LIO_IO_ALISTIO */
#ifndef CRAY
	else if (method & LIO_IO_SYNCV) {
		io_type = "readv(2)";

		sprintf(Lio_SysCall, "readv(%d, &iov, 1) nbyte:%d", fd, size);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}
		if ((ret = readv(fd, &iov, 1)) == -1) {
			sprintf(Errormsg,
				"%s/%d readv(%d, iov, 1) nbyte:%d ret:-1, errno=%d %s",
				__FILE__, __LINE__, fd, size, errno,
				strerror(errno));
			return -errno;
		}

		if (ret != size) {
			sprintf(Errormsg,
				"%s/%d readv(%d, iov, 1) nbyte:%d returned=%d",
				__FILE__, __LINE__, fd, size, ret);
		} else if (Debug_level > 1)
			printf
			    ("DEBUG %s/%d: readv completed without error (ret %d)\n",
			     __FILE__, __LINE__, ret);

		return ret;
	}			/* LIO_IO_SYNCV */
#endif

#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	else if (method & LIO_IO_SYNCP) {
		io_type = "pread(2)";

		sprintf(Lio_SysCall,
			"pread(%d, buf, %d, %lld)", fd, size,
			(long long)poffset);

		if (Debug_level) {
			printf("DEBUG %s/%d: %s\n", __FILE__, __LINE__,
			       Lio_SysCall);
		}
		if ((ret = pread(fd, buffer, size, poffset)) == -1) {
			sprintf(Errormsg,
				"%s/%d pread(%d, buf, %d, %lld) ret:-1, errno=%d %s",
				__FILE__, __LINE__, fd, size,
				(long long)poffset, errno, strerror(errno));
			return -errno;
		}

		if (ret != size) {
			sprintf(Errormsg,
				"%s/%d pread(%d, buf, %d, %lld) returned=%d",
				__FILE__, __LINE__,
				fd, size, (long long)poffset, ret);
		} else if (Debug_level > 1)
			printf
			    ("DEBUG %s/%d: pread completed without error (ret %d)\n",
			     __FILE__, __LINE__, ret);

		return ret;
	}			/* LIO_IO_SYNCP */
#endif

	else {
		printf("DEBUG %s/%d: No I/O method chosen\n", __FILE__,
		       __LINE__);
		return -1;
	}

	/*
	 * wait for async io to complete.
	 * Note: Sync io should have returned prior to getting here.
	 */
#ifdef CRAY
	ret = lio_wait4asyncio(method, fd, statptr);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	ret = lio_wait4asyncio(method, fd, &aiocbp);
#endif

	/*
	 * If there was an error waiting for async i/o to complete,
	 * return the error value (errno) to the caller.
	 * Note: Errormsg should already have been updated.
	 */
	if (ret < 0) {
		return ret;
	}

	/*
	 * If i/o was not waited for (may not have been completed at this time),
	 * return the size that was requested.
	 */
	if (ret == 1)
		return size;

	/*
	 * check that async io was successful.
	 * Note:  if the there was an system call failure, -errno
	 * was returned and Errormsg should already have been updated.
	 * If amount i/o was different than size, Errormsg should already
	 * have been updated but the actual i/o size if returned.
	 */

#ifdef CRAY
	ret = lio_check_asyncio(io_type, size, &status);
#endif
#if defined(sgi) || (defined(__linux__) && !defined(__UCLIBC__))
	ret = lio_check_asyncio(io_type, size, &aiocbp, method);
#endif

	return ret;
}				/* end of lio_read_buffer */

#if !defined(__sun) && !defined(__hpux) && !defined(_AIX)
/***********************************************************************
 * This function will check that async io was successful.
 * It can also be used to check sync listio since it uses the
 * same method.
 *
 * Return Values
 *  If status.sw_error is set, -status.sw_error is returned.
 *  Otherwise sw_count's field value is returned.
 *
 * (rrl 04/96)
 ***********************************************************************/
#ifdef CRAY
int lio_check_asyncio(char *io_type, int size, struct iosw *status)
#elif defined(sgi)
int lio_check_asyncio(char *io_type, int size, aiocb_t * aiocbp, int method)
#elif defined(__linux__) && !defined(__UCLIBC__)
int lio_check_asyncio(char *io_type, int size, struct aiocb *aiocbp, int method)
{
	int ret;

#ifdef CRAY
	if (status->sw_error) {
		sprintf(Errormsg,
			"%s/%d %s, sw_error set = %d %s, sw_count = %d",
			__FILE__, __LINE__, io_type,
			status->sw_error, strerror(status->sw_error),
			status->sw_count);
		return -status->sw_error;
	} else if (status->sw_count != size) {
		sprintf(Errormsg,
			"%s/%d %s, sw_count not as expected(%d), but actual:%d",
			__FILE__, __LINE__, io_type, size, status->sw_count);
	} else if (Debug_level > 1) {
		printf
		    ("DEBUG %s/%d: %s completed without error (sw_error == 0, sw_count == %d)\n",
		     __FILE__, __LINE__, io_type, status->sw_count);
	}

	return status->sw_count;

#else

	int cnt = 1;

	/* The I/O may have been synchronous with signal completion.  It doesn't
	 * make sense, but the combination could be generated.  Release the
	 * completion signal here otherwise it'll hang around and bite us
	 * later.
	 */
	if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL)
		sigrelse(aiocbp->aio_sigevent.sigev_signo);

	ret = aio_error(aiocbp);

	while (ret == EINPROGRESS) {
		ret = aio_error(aiocbp);
		++cnt;
	}
	if (cnt > 1) {
		sprintf(Errormsg,
			"%s/%d %s, aio_error had to loop on EINPROGRESS, cnt=%d; random method %#o; sigev_notify=%s",
			__FILE__, __LINE__, io_type, cnt, method,
			(aiocbp->aio_sigevent.sigev_notify ==
			 SIGEV_SIGNAL ? "signal" : aiocbp->aio_sigevent.
			 sigev_notify == SIGEV_NONE ? "none" :
#ifdef SIGEV_CALLBACK
			 aiocbp->aio_sigevent.sigev_notify ==
			 SIGEV_CALLBACK ? "callback" :
#endif
			 aiocbp->aio_sigevent.sigev_notify ==
			 SIGEV_THREAD ? "thread" : "unknown"));
		return -ret;
	}

	if (ret != 0) {
		sprintf(Errormsg,
			"%s/%d %s, aio_error = %d %s; random method %#o",
			__FILE__, __LINE__, io_type,
			ret, strerror(ret), method);
		return -ret;
	}
	ret = aio_return(aiocbp);
	if (ret != size) {
		sprintf(Errormsg,
			"%s/%d %s, aio_return not as expected(%d), but actual:%d",
			__FILE__, __LINE__, io_type, size, ret);

#ifdef BUG1_workaround
		if (ret == 0) {
			ret = size;
			if (Debug_level > 1) {
				printf
				    ("WARN %s/%d: %s completed with bug1_workaround (aio_error == 0, aio_return now == %d)\n",
				     __FILE__, __LINE__, io_type, ret);
			}
		}
#endif /* BUG1_workaround */

	} else if (Debug_level > 1) {
		printf
		    ("DEBUG %s/%d: %s completed without error (aio_error == 0, aio_return == %d)\n",
		     __FILE__, __LINE__, io_type, ret);
	}

	return ret;

#endif
}				/* end of lio_check_asyncio */
#endif

/***********************************************************************
 *
 * This function will wait for async io to complete.
 * If multiple wait methods are specified, the order is predetermined
 * to LIO_WAIT_RECALL,
 * LIO_WAIT_ACTIVE, LIO_WAIT_SIGPAUSE, LIO_WAIT_SIGACTIVE,
 * then LIO_WAIT_NONE.
 *
 * If no wait method was specified the default wait method is: recall(2)
 * or aio_suspend(3), as appropriate.
 *
 * Return Values
 *	<0: errno of failed recall
 *	0 : async io was completed
 *	1 : async was not waited for, io may not have completed.
 *
 * (rrl 04/96)
 ***********************************************************************/
#ifdef CRAY
int lio_wait4asyncio(int method, int fd, struct iosw **statptr)
#elif defined(sgi)
int lio_wait4asyncio(int method, int fd, aiocb_t * aiocbp)
#elif defined(__linux__) && !defined(__UCLIBC__)
int lio_wait4asyncio(int method, int fd, struct aiocb *aiocbp)
{
	int cnt;
#ifdef sgi
	int ret;
	const aiocb_t *aioary[1];
#endif
#if defined(__linux__)&& !defined(__UCLIBC__)
	int ret;
	const struct aiocb *aioary[1];
#endif

	if ((method & LIO_WAIT_RECALL)
#if defined(sgi) || (defined(__linux__)&& !defined(__UCLIBC__))
	    || (method & LIO_WAIT_CBSUSPEND)
	    || (method & LIO_WAIT_SIGSUSPEND)
#endif
	    || ((method & LIO_WAIT_TYPES) == 0)) {
		/*
		 * If method has LIO_WAIT_RECALL bit set or method does
		 * not have any wait method bits set (default), use recall/aio_suspend.
		 */
#ifdef CRAY
		if (Debug_level > 2)
			printf("DEBUG %s/%d: wait method : recall\n", __FILE__,
			       __LINE__);
		sigon();
		if (recall(fd, 1, statptr)) {
			sprintf(Errormsg,
				"%s/%d recall(%d, 1, stat) failed, errno:%d %s",
				__FILE__, __LINE__, fd, errno, strerror(errno));
			return -errno;
		}
#else
		if (Debug_level > 2)
			printf
			    ("DEBUG %s/%d: wait method : aio_suspend, sigev_notify=%s\n",
			     __FILE__, __LINE__,
			     (aiocbp->aio_sigevent.sigev_notify ==
			      SIGEV_SIGNAL ? "signal" : aiocbp->aio_sigevent.
			      sigev_notify == SIGEV_NONE ? "none" :
#ifdef SIGEV_CALLBACK
			      aiocbp->aio_sigevent.sigev_notify ==
			      SIGEV_CALLBACK ? "callback" :
#endif
			      aiocbp->aio_sigevent.sigev_notify ==
			      SIGEV_THREAD ? "thread" : "unknown"));

		aioary[0] = aiocbp;
		ret = aio_suspend(aioary, 1, NULL);
		if ((ret == -1) && (errno == EINTR)) {
			if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL) {
				if (Debug_level > 2) {
					printf
					    ("DEBUG %s/%d: aio_suspend received EINTR, sigev_notify=SIGEV_SIGNAL -- ok\n",
					     __FILE__, __LINE__);
				}
			} else {
				sprintf(Errormsg,
					"%s/%d aio_suspend received EINTR, sigev_notify=%s, not ok\n",
					__FILE__, __LINE__,
					(aiocbp->aio_sigevent.sigev_notify ==
					 SIGEV_SIGNAL ? "signal" : aiocbp->
					 aio_sigevent.sigev_notify ==
					 SIGEV_NONE ? "none" :
#ifdef SIGEV_CALLBACK
					 aiocbp->aio_sigevent.sigev_notify ==
					 SIGEV_CALLBACK ? "callback" :
#endif
					 aiocbp->aio_sigevent.sigev_notify ==
					 SIGEV_THREAD ? "thread" : "unknown"));
				return -errno;
			}
		} else if (ret) {
			sprintf(Errormsg,
				"%s/%d aio_suspend(fildes=%d, aioary, 1, NULL) failed, errno:%d %s",
				__FILE__, __LINE__, fd, errno, strerror(errno));
			return -errno;
		}
#endif

	} else if (method & LIO_WAIT_ACTIVE) {
		if (Debug_level > 2)
			printf("DEBUG %s/%d: wait method : active\n", __FILE__,
			       __LINE__);
#ifdef CRAY
		sigon();
		/*
		 * loop until sw_flag, sw_count or sw_error field elements
		 * change to non-zero.
		 */
		cnt = 0;
		while ((*statptr)->sw_flag == 0 &&
		       (*statptr)->sw_count == 0 && (*statptr)->sw_error == 0) {
			cnt++;
		}
#else
		/* loop while aio_error() returns EINPROGRESS */
		cnt = 0;
		while (1) {
			ret = aio_error(aiocbp);
			if (ret != EINPROGRESS) {
				break;
			}
			++cnt;
		}

#endif
		if (Debug_level > 5 && cnt && (cnt % 50) == 0)
			printf("DEBUG %s/%d: wait active cnt = %d\n",
			       __FILE__, __LINE__, cnt);

	} else if (method & LIO_WAIT_SIGPAUSE) {
		if (Debug_level > 2)
			printf("DEBUG %s/%d: wait method : sigpause\n",
			       __FILE__, __LINE__);
#ifdef sgi
		/* note: don't do the sigon() for CRAY in this case.  why? -- roehrich 6/11/97 */
		if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL)
			sigrelse(aiocbp->aio_sigevent.sigev_signo);
		else {
			printf("DEBUG %s/%d: sigev_notify != SIGEV_SIGNAL\n",
			       __FILE__, __LINE__);
			return -1;
		}
#endif
		pause();

	} else if (method & LIO_WAIT_SIGACTIVE) {
		if (Debug_level > 2)
			printf("DEBUG %s/%d: wait method : sigactive\n",
			       __FILE__, __LINE__);
#ifdef CRAY
		sigon();
#else
		if (aiocbp->aio_sigevent.sigev_notify == SIGEV_SIGNAL)
			sigrelse(aiocbp->aio_sigevent.sigev_signo);
		else {
			printf("DEBUG %s/%d: sigev_notify != SIGEV_SIGNAL\n",
			       __FILE__, __LINE__);
			return -1;
		}
#endif
		/* loop waiting for signal */
		while (Received_signal == Rec_signal) {
#ifdef CRAY
			sigon();
#else
			sigrelse(aiocbp->aio_sigevent.sigev_signo);
#endif
		}

	} else if (method & LIO_WAIT_NONE) {
		if (Debug_level > 2)
			printf("DEBUG %s/%d: wait method : none\n", __FILE__,
			       __LINE__);
		/* It's broken because the aiocb/iosw is an automatic variable in
		 * lio_{read,write}_buffer, so when the function returns and the
		 * I/O completes there will be nowhere to write the I/O status.
		 * It doesn't cause a problem on unicos--probably because of some
		 * compiler quirk, or an accident.  It causes POSIX async I/O
		 * to core dump some threads.   spr/pv 705909.  6/27/97 roehrich
		 */
		sprintf(Errormsg,
			"%s/%d LIO_WAIT_NONE was selected (this is broken)\n",
			__FILE__, __LINE__);
#ifdef CRAY
		sigon();
#endif
/*        return 1;*/
		return -1;
	} else {
		if (Debug_level > 2)
			printf("DEBUG %s/%d: no wait method was chosen\n",
			       __FILE__, __LINE__);
		return -1;
	}

	return 0;

}				/* end of lio_wait4asyncio */

#endif /* ifndef linux */
#endif

#if UNIT_TEST
/***********************************************************************
 * The following code is provided as unit test.
 * Just define add "-DUNIT_TEST=1" to the cc line.
 *
 * (rrl 04/96)
 ***********************************************************************/
struct unit_info_t {
	int method;
	int sig;
	char *str;
} Unit_info[] = {
	{
	LIO_IO_SYNC, 0, "sync io"}, {
	LIO_IO_SYNCV, 0, "sync readv/writev"}, {
	LIO_IO_SYNCP, 0, "sync pread/pwrite"}, {
	LIO_IO_ASYNC, 0, "async io, def wait"}, {
	LIO_IO_SLISTIO, 0, "sync listio"}, {
	LIO_IO_ALISTIO, 0, "async listio, def wait"}, {
	LIO_IO_ASYNC | LIO_WAIT_ACTIVE, 0, "async active"}, {
	LIO_IO_ASYNC | LIO_WAIT_RECALL, 0, "async recall/suspend"}, {
	LIO_IO_ASYNC | LIO_WAIT_SIGPAUSE, SIGUSR1, "async sigpause"}, {
	LIO_IO_ASYNC | LIO_WAIT_SIGACTIVE, SIGUSR1, "async sigactive"}, {
	LIO_IO_ALISTIO | LIO_WAIT_ACTIVE, 0, "async listio active"}, {
	LIO_IO_ALISTIO | LIO_WAIT_RECALL, 0, "async listio recall"}, {
	LIO_IO_ALISTIO | LIO_WAIT_SIGACTIVE, SIGUSR1, "async listio sigactive"},
	{
	LIO_IO_ALISTIO | LIO_WAIT_SIGPAUSE, SIGUSR1, "async listio sigpause"},
	{
	LIO_IO_ASYNC, SIGUSR2, "async io, def wait, sigusr2"}, {
LIO_IO_ALISTIO, SIGUSR2, "async listio, def wait, sigusr2"},};

int main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;

	int fd;
	char *err;
	char buffer[4096];
	int size = 4096;
	int ret;
	int ind;
	int iter = 3;
	int method;
	int exit_status = 0;
	int c;
	int i;
	char *symbols = NULL;
	int die_on_err = 0;

	while ((c = getopt(argc, argv, "s:di:")) != -1) {
		switch (c) {
		case 's':
			symbols = optarg;
			break;
		case 'd':
			++die_on_err;
			break;
		case 'i':
			iter = atoi(optarg);
			break;
		}
	}

	if ((fd =
	     open("unit_test_file", O_CREAT | O_RDWR | O_TRUNC, 0777)) == -1) {
		perror
		    ("open(unit_test_file, O_CREAT|O_RDWR|O_TRUNC, 0777) failed");
		exit(1);
	}

	Debug_level = 9;

	if (symbols != NULL) {
		if ((method = lio_parse_io_arg2(symbols, &err)) == -1) {
			printf
			    ("lio_parse_io_arg2(%s, &err) failed, bad token starting at %s\n",
			     symbols, err);
			if (die_on_err)
				exit(1);
		} else
			printf("lio_parse_io_arg2(%s, &err) returned %#o\n",
			       symbols, method);

		exit_status = 0;
		for (ind = 0; ind < iter; ind++) {
			memset(buffer, 'A', 4096);
			if (lseek(fd, 0, 0) == -1) {
				printf("lseek(fd,0,0), %d, failed, errno %d\n",
				       __LINE__, errno);
				++exit_status;
			}
			if ((ret = lio_write_buffer(fd, method, buffer,
						    size, SIGUSR1, &err,
						    0)) != size) {
				printf
				    ("lio_write_buffer returned -1, err = %s\n",
				     err);
			} else
				printf("lio_write_buffer returned %d\n", ret);

			memset(buffer, 'B', 4096);
			if (lseek(fd, 0, 0) == -1) {
				printf("lseek(fd,0,0), %d, failed, errno %d\n",
				       __LINE__, errno);
				++exit_status;
			}
			if ((ret = lio_read_buffer(fd, method, buffer,
						   size, SIGUSR2, &err,
						   0)) != size) {
				printf
				    ("lio_read_buffer returned -1, err = %s\n",
				     err);
			} else
				printf("lio_read_buffer returned %d\n", ret);

			for (i = 0; i < 4096; ++i) {
				if (buffer[i] != 'A') {
					printf("  buffer[%d] = %d\n", i,
					       buffer[i]);
					++exit_status;
					break;
				}
			}

			if (exit_status)
				exit(exit_status);

		}

		unlink("unit_test_file");
		exit(0);
	}

	for (ind = 0; ind < sizeof(Unit_info) / sizeof(struct unit_info_t);
	     ind++) {

		printf("\n********* write %s ***************\n",
		       Unit_info[ind].str);
		if (lseek(fd, 0, 0) == -1) {
			printf("lseek(fd,0,0), %d, failed, errno %d\n",
			       __LINE__, errno);
			++exit_status;
		}

		memset(buffer, 'A', 4096);
		if ((ret = lio_write_buffer(fd, Unit_info[ind].method, buffer,
					    size, Unit_info[ind].sig, &err,
					    0)) != size) {
			printf
			    (">>>>> lio_write_buffer(fd,0%x,buffer,%d,%d,err,0) returned -1,\n   err = %s\n",
			     Unit_info[ind].method, size, Unit_info[ind].sig,
			     err);
			++exit_status;
			if (die_on_err)
				exit(exit_status);
		} else {
			printf("lio_write_buffer returned %d\n", ret);
		}

		printf("\n********* read %s ***************\n",
		       Unit_info[ind].str);
		if (lseek(fd, 0, 0) == -1) {
			printf("lseek(fd,0,0), %d, failed, errno %d\n",
			       __LINE__, errno);
			++exit_status;
		}
		memset(buffer, 'B', 4096);
		if ((ret = lio_read_buffer(fd, Unit_info[ind].method, buffer,
					   size, Unit_info[ind].sig, &err,
					   0)) != size) {
			printf
			    (">>>>> lio_read_buffer(fd,0%x,buffer,%d,%d,err,0) returned -1,\n   err = %s\n",
			     Unit_info[ind].method, size, Unit_info[ind].sig,
			     err);
			++exit_status;
			if (die_on_err)
				exit(exit_status);
		} else {
			printf("lio_read_buffer returned %d\n", ret);
		}

		for (i = 0; i < 4096; ++i) {
			if (buffer[i] != 'A') {
				printf("  buffer[%d] = %d\n", i, buffer[i]);
				++exit_status;
				if (die_on_err)
					exit(exit_status);
				break;
			}
		}

		fflush(stdout);
		fflush(stderr);
		sleep(1);

	}

	unlink("unit_test_file");

	exit(exit_status);
}
#endif
