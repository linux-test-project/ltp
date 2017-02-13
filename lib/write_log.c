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
 * This module contains code for logging writes to files, and for
 * perusing the resultant logfile.  The main intent of all this is
 * to provide a 'write history' of a file which can be examined to
 * judge the state of a file (ie. whether it is corrupted or not) based
 * on the write activity.
 *
 * The main abstractions available to the user are the wlog_file, and
 * the wlog_rec.  A wlog_file is a handle encapsulating a write logfile.
 * It is initialized with the wlog_open() function.  This handle is
 * then passed to the various wlog_xxx() functions to provide transparent
 * access to the write logfile.
 *
 * The wlog_rec datatype is a structure which contains all the information
 * about a file write.  Examples include the file name, offset, length,
 * pattern, etc.  In addition there is a bit which is cleared/set based
 * on whether or not the write has been confirmed as complete.  This
 * allows the write logfile to contain information on writes which have
 * been initiated, but not yet completed (as in async io).
 *
 * There is also a function to scan a write logfile in reverse order.
 *
 * NOTE:	For target file analysis based on a write logfile, the
 * 		assumption is made that the file being written to is
 * 		locked from simultaneous access, so that the order of
 * 		write completion is predictable.  This is an issue when
 * 		more than 1 process is trying to write data to the same
 *		target file simultaneously.
 *
 * The history file created is a collection of variable length records
 * described by scruct wlog_rec_disk in write_log.h.  See that module for
 * the layout of the data on disk.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "write_log.h"

#ifndef BSIZE
#ifdef DEV_BSIZE
#define BSIZE DEV_BSIZE
#else
#warning DEV_BSIZE is not defined, defaulting to 512
#define BSIZE 512
#endif
#endif

#ifndef PATH_MAX
#define PATH_MAX          255
/*#define PATH_MAX pathconf("/", _PC_PATH_MAX)*/
#endif

char Wlog_Error_String[256];

#if __STDC__
static int wlog_rec_pack(struct wlog_rec *wrec, char *buf, int flag);
static int wlog_rec_unpack(struct wlog_rec *wrec, char *buf);
#else
static int wlog_rec_pack();
static int wlog_rec_unpack();
#endif

/*
 * Initialize a write logfile.  wfile is a wlog_file structure that has
 * the w_file field filled in.  The rest of the information in the
 * structure is initialized by the routine.
 *
 * The trunc flag is used to indicate whether or not the logfile should
 * be truncated if it currently exists.  If it is non-zero, the file will
 * be truncated, otherwise it will be appended to.
 *
 * The mode argument is the [absolute] mode which the file will be
 * given if it does not exist.  This mode is not affected by your process
 * umask.
 */

int wlog_open(struct wlog_file *wfile, int trunc, int mode)
{
	int omask, oflags;

	if (trunc)
		trunc = O_TRUNC;

	omask = umask(0);

	/*
	 * Open 1 file descriptor as O_APPEND
	 */

	oflags = O_WRONLY | O_APPEND | O_CREAT | trunc;
	wfile->w_afd = open(wfile->w_file, oflags, mode);
	umask(omask);

	if (wfile->w_afd == -1) {
		sprintf(Wlog_Error_String,
			"Could not open write_log - open(%s, %#o, %#o) failed:  %s\n",
			wfile->w_file, oflags, mode, strerror(errno));
		return -1;
	}

	/*
	 * Open the next fd as a random access descriptor
	 */

	oflags = O_RDWR;
	if ((wfile->w_rfd = open(wfile->w_file, oflags)) == -1) {
		sprintf(Wlog_Error_String,
			"Could not open write log - open(%s, %#o) failed:  %s\n",
			wfile->w_file, oflags, strerror(errno));
		close(wfile->w_afd);
		wfile->w_afd = -1;
		return -1;
	}

	return 0;
}

/*
 * Release all resources associated with a wlog_file structure allocated
 * with the wlog_open() call.
 */

int wlog_close(struct wlog_file *wfile)
{
	close(wfile->w_afd);
	close(wfile->w_rfd);
	return 0;
}

/*
 * Write a wlog_rec structure to a write logfile.  Offset is used to
 * control where the record will be written.  If offset is < 0, the
 * record will be appended to the end of the logfile.  Otherwise, the
 * record which exists at the indicated offset will be overlayed.  This
 * is so that we can record writes which are outstanding (with the w_done
 * bit in wrec cleared), but not completed, and then later update the
 * logfile when the write request completes (as with async io).  When
 * offset is >= 0, only the fixed length portion of the record is
 * rewritten.  See text in write_log.h for details on the format of an
 * on-disk record.
 *
 * The return value of the function is the byte offset in the logfile
 * where the record begins.
 *
 * Note:  It is the callers responsibility to make sure that the offset
 * parameter 'points' to a valid record location when a record is to be
 * overlayed.  This is guarenteed by saving the return value of a previous
 * call to wlog_record_write() which wrote the record to be overlayed.
 *
 * Note2:  The on-disk version of the wlog_rec is MUCH different than
 * the user version.  Don't expect to od the logfile and see data formatted
 * as it is in the wlog_rec structure.  Considerable data packing takes
 * place before the record is written.
 */

int wlog_record_write(struct wlog_file *wfile, struct wlog_rec *wrec,
			long offset)
{
	int reclen;
	char wbuf[WLOG_REC_MAX_SIZE + 2];

	/*
	 * If offset is -1, we append the record at the end of file
	 *
	 * Otherwise, we overlay wrec at the file offset indicated and assume
	 * that the caller passed us the correct offset.  We do not record the
	 * fname in this case.
	 */

	reclen = wlog_rec_pack(wrec, wbuf, (offset < 0));

	if (offset < 0) {
		/*
		 * Since we're writing a complete new record, we must also tack
		 * its length onto the end so that wlog_scan_backward() will work.
		 * Length is asumed to fit into 2 bytes.
		 */

		wbuf[reclen] = reclen / 256;
		wbuf[reclen + 1] = reclen % 256;
		reclen += 2;

		if (write(wfile->w_afd, wbuf, reclen) == -1) {
			sprintf(Wlog_Error_String,
				"Could not write log - write(%s, %s, %d) failed:  %s\n",
				wfile->w_file, wbuf, reclen, strerror(errno));
			return -1;
		} else {
			offset = lseek(wfile->w_afd, 0, SEEK_CUR) - reclen;
			if (offset == -1) {
				sprintf(Wlog_Error_String,
					"Could not reposition file pointer - lseek(%s, 0, SEEK_CUR) failed:  %s\n",
					wfile->w_file, strerror(errno));
				return -1;
			}
		}
	} else {
		if ((lseek(wfile->w_rfd, offset, SEEK_SET)) == -1) {
			sprintf(Wlog_Error_String,
				"Could not reposition file pointer - lseek(%s, %ld, SEEK_SET) failed:  %s\n",
				wfile->w_file, offset, strerror(errno));
			return -1;
		} else {
			if ((write(wfile->w_rfd, wbuf, reclen)) == -1) {
				sprintf(Wlog_Error_String,
					"Could not write log - write(%s, %s, %d) failed:  %s\n",
					wfile->w_file, wbuf, reclen,
					strerror(errno));
				return -1;
			}
		}
	}

	return offset;
}

/*
 * Function to scan a logfile in reverse order.  Wfile is a valid
 * wlog_file structure initialized by wlog_open().  nrecs is the number
 * of records to scan (all records are scanned if nrecs is 0).  func is
 * a user-supplied function to call for each record found.  The function
 * will be passed a single parameter - a wlog_rec structure .
 */

int wlog_scan_backward(struct wlog_file *wfile, int nrecs,
			int (*func)(), long data)
{
	int fd, leftover, nbytes, offset, recnum, reclen, rval;
	char buf[BSIZE * 32], *bufend, *cp, *bufstart;
	char albuf[WLOG_REC_MAX_SIZE];
	struct wlog_rec wrec;

	fd = wfile->w_rfd;

	/*
	 * Move to EOF.  offset will always hold the current file offset
	 */

	if ((lseek(fd, 0, SEEK_END)) == -1) {
		sprintf(Wlog_Error_String,
			"Could not reposition file pointer - lseek(%s, 0, SEEK_END) failed:  %s\n",
			wfile->w_file, strerror(errno));
		return -1;
	}
	offset = lseek(fd, 0, SEEK_CUR);
	if ((offset == -1)) {
		sprintf(Wlog_Error_String,
			"Could not reposition file pointer - lseek(%s, 0, SEEK_CUR) failed:  %s\n",
			wfile->w_file, strerror(errno));
		return -1;
	}

	bufend = buf + sizeof(buf);
	bufstart = buf;

	recnum = 0;
	leftover = 0;
	while ((!nrecs || recnum < nrecs) && offset > 0) {
		/*
		 * Check for beginning of file - if there aren't enough bytes
		 * remaining to fill buf, adjust bufstart.
		 */

		if ((unsigned int)offset + leftover < sizeof(buf)) {
			bufstart = bufend - (offset + leftover);
			offset = 0;
		} else {
			offset -= sizeof(buf) - leftover;
		}

		/*
		 * Move to the proper file offset, and read into buf
		 */
		if ((lseek(fd, offset, SEEK_SET)) == -1) {
			sprintf(Wlog_Error_String,
				"Could not reposition file pointer - lseek(%s, %d, SEEK_SET) failed:  %s\n",
				wfile->w_file, offset, strerror(errno));
			return -1;
		}

		nbytes = read(fd, bufstart, bufend - bufstart - leftover);

		if (nbytes == -1) {
			sprintf(Wlog_Error_String,
				"Could not read history file at offset %d - read(%d, %p, %d) failed:  %s\n",
				offset, fd, bufstart,
				(int)(bufend - bufstart - leftover),
				strerror(errno));
			return -1;
		}

		cp = bufend;
		leftover = 0;

		while (cp >= bufstart) {

			/*
			 * If cp-bufstart is not large enough to hold a piece
			 * of record length information, copy remainder to end
			 * of buf and continue reading the file.
			 */

			if (cp - bufstart < 2) {
				leftover = cp - bufstart;
				memcpy(bufend - leftover, bufstart, leftover);
				break;
			}

			/*
			 * Extract the record length.  We must do it this way
			 * instead of casting cp to an int because cp might
			 * not be word aligned.
			 */

			reclen = (*(cp - 2) * 256) + *(cp - 1);

			/*
			 * If cp-bufstart isn't large enough to hold a
			 * complete record, plus the length information, copy
			 * the leftover bytes to the end of buf and continue
			 * reading.
			 */

			if (cp - bufstart < reclen + 2) {
				leftover = cp - bufstart;
				memcpy(bufend - leftover, bufstart, leftover);
				break;
			}

			/*
			 * Adjust cp to point at the start of the record.
			 * Copy the record into wbuf so that it is word
			 * aligned and pass the record to the user supplied
			 * function.
			 */

			cp -= reclen + 2;
			memcpy(albuf, cp, reclen);

			wlog_rec_unpack(&wrec, albuf);

			/*
			 * Call the user supplied function -
			 * stop if instructed to.
			 */

			if ((rval = (*func) (&wrec, data)) == WLOG_STOP_SCAN) {
				break;
			}

			recnum++;

			if (nrecs && recnum >= nrecs)
				break;
		}
	}

	return 0;
}

/*
 * The following 2 routines are used to pack and unpack the user
 * visible wlog_rec structure to/from a character buffer which is
 * stored or read from the write logfile.  Any changes to either of
 * these routines must be reflected in the other.
 */

static int wlog_rec_pack(struct wlog_rec *wrec, char *buf, int flag)
{
	char *file, *host, *pattern;
	struct wlog_rec_disk *wrecd;

	wrecd = (struct wlog_rec_disk *)buf;

	wrecd->w_pid = (uint) wrec->w_pid;
	wrecd->w_offset = (uint) wrec->w_offset;
	wrecd->w_nbytes = (uint) wrec->w_nbytes;
	wrecd->w_oflags = (uint) wrec->w_oflags;
	wrecd->w_done = (uint) wrec->w_done;
	wrecd->w_async = (uint) wrec->w_async;

	wrecd->w_pathlen = (wrec->w_pathlen > 0) ? (uint) wrec->w_pathlen : 0;
	wrecd->w_hostlen = (wrec->w_hostlen > 0) ? (uint) wrec->w_hostlen : 0;
	wrecd->w_patternlen =
	    (wrec->w_patternlen > 0) ? (uint) wrec->w_patternlen : 0;

	/*
	 * If flag is true, we should also pack the variable length parts
	 * of the wlog_rec.  By default, we only pack the fixed length
	 * parts.
	 */

	if (flag) {
		file = buf + sizeof(struct wlog_rec_disk);
		host = file + wrecd->w_pathlen;
		pattern = host + wrecd->w_hostlen;

		if (wrecd->w_pathlen > 0)
			memcpy(file, wrec->w_path, wrecd->w_pathlen);

		if (wrecd->w_hostlen > 0)
			memcpy(host, wrec->w_host, wrecd->w_hostlen);

		if (wrecd->w_patternlen > 0)
			memcpy(pattern, wrec->w_pattern, wrecd->w_patternlen);

		return (sizeof(struct wlog_rec_disk) +
			wrecd->w_pathlen + wrecd->w_hostlen +
			wrecd->w_patternlen);
	} else {
		return sizeof(struct wlog_rec_disk);
	}
}

static int wlog_rec_unpack(struct wlog_rec *wrec, char *buf)
{
	char *file, *host, *pattern;
	struct wlog_rec_disk *wrecd;

	memset((char *)wrec, 0x00, sizeof(struct wlog_rec));
	wrecd = (struct wlog_rec_disk *)buf;

	wrec->w_pid = wrecd->w_pid;
	wrec->w_offset = wrecd->w_offset;
	wrec->w_nbytes = wrecd->w_nbytes;
	wrec->w_oflags = wrecd->w_oflags;
	wrec->w_hostlen = wrecd->w_hostlen;
	wrec->w_pathlen = wrecd->w_pathlen;
	wrec->w_patternlen = wrecd->w_patternlen;
	wrec->w_done = wrecd->w_done;
	wrec->w_async = wrecd->w_async;

	if (wrec->w_pathlen > 0) {
		file = buf + sizeof(struct wlog_rec_disk);
		memcpy(wrec->w_path, file, wrec->w_pathlen);
	}

	if (wrec->w_hostlen > 0) {
		host = buf + sizeof(struct wlog_rec_disk) + wrec->w_pathlen;
		memcpy(wrec->w_host, host, wrec->w_hostlen);
	}

	if (wrec->w_patternlen > 0) {
		pattern = buf + sizeof(struct wlog_rec_disk) +
		    wrec->w_pathlen + wrec->w_hostlen;
		memcpy(wrec->w_pattern, pattern, wrec->w_patternlen);
	}

	return 0;
}
