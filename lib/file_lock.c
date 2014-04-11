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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <string.h>		/* memset, strerror */
#include "file_lock.h"

#ifndef EFSEXCLWR
#define EFSEXCLWR	503
#endif

/*
 * String containing the last system call.
 *
 */
char Fl_syscall_str[128];

static char errmsg[256];

/***********************************************************************
 *
 * Test interface to the fcntl system call.
 * It will loop if the LOCK_NB flags is NOT set.
 ***********************************************************************/
int file_lock(int fd, int flags, char **errormsg)
{
	register int cmd, ret;
	struct flock flocks;

	memset(&flocks, 0, sizeof(struct flock));

	if (flags & LOCK_NB)
		cmd = F_SETLK;
	else
		cmd = F_SETLKW;

	flocks.l_whence = 0;
	flocks.l_start = 0;
	flocks.l_len = 0;

	if (flags & LOCK_UN)
		flocks.l_type = F_UNLCK;
	else if (flags & LOCK_EX)
		flocks.l_type = F_WRLCK;
	else if (flags & LOCK_SH)
		flocks.l_type = F_RDLCK;
	else {
		errno = EINVAL;
		if (errormsg != NULL) {
			sprintf(errmsg,
				"Programmer error, called file_lock with in valid flags\n");
			*errormsg = errmsg;
		}
		return -1;
	}

	sprintf(Fl_syscall_str,
		"fcntl(%d, %d, &flocks): type:%d whence:%d, start:%lld len:%lld\n",
		fd, cmd, flocks.l_type, flocks.l_whence,
		(long long)flocks.l_start, (long long)flocks.l_len);

	while (1) {
		ret = fcntl(fd, cmd, &flocks);

		if (ret < 0) {
			if (cmd == F_SETLK)
				switch (errno) {
					/* these errors are okay */
				case EACCES:	/* Permission denied */
				case EINTR:	/* interrupted system call */
#ifdef EFILESH
				case EFILESH:	/* file shared */
#endif
				case EFSEXCLWR:	/* File is write protected */
					continue;	/* retry getting lock */
				}
			if (errormsg != NULL) {
				sprintf(errmsg,
					"fcntl(%d, %d, &flocks): errno:%d %s\n",
					fd, cmd, errno, strerror(errno));
				*errormsg = errmsg;
			}
			return -1;
		}
		break;
	}

	return ret;

}				/* end of file_lock */

/***********************************************************************
 *
 * Test interface to the fcntl system call.
 * It will loop if the LOCK_NB flags is NOT set.
 ***********************************************************************/
int record_lock(int fd, int flags, int start, int len, char **errormsg)
{
	register int cmd, ret;
	struct flock flocks;

	memset(&flocks, 0, sizeof(struct flock));

	if (flags & LOCK_NB)
		cmd = F_SETLK;
	else
		cmd = F_SETLKW;

	flocks.l_whence = 0;
	flocks.l_start = start;
	flocks.l_len = len;

	if (flags & LOCK_UN)
		flocks.l_type = F_UNLCK;
	else if (flags & LOCK_EX)
		flocks.l_type = F_WRLCK;
	else if (flags & LOCK_SH)
		flocks.l_type = F_RDLCK;
	else {
		errno = EINVAL;
		if (errormsg != NULL) {
			sprintf(errmsg,
				"Programmer error, called record_lock with in valid flags\n");
			*errormsg = errmsg;
		}
		return -1;
	}

	sprintf(Fl_syscall_str,
		"fcntl(%d, %d, &flocks): type:%d whence:%d, start:%lld len:%lld\n",
		fd, cmd, flocks.l_type, flocks.l_whence,
		(long long)flocks.l_start, (long long)flocks.l_len);

	while (1) {
		ret = fcntl(fd, cmd, &flocks);

		if (ret < 0) {
			if (cmd == F_SETLK)
				switch (errno) {
					/* these errors are okay */
				case EACCES:	/* Permission denied */
				case EINTR:	/* interrupted system call */
#ifdef EFILESH
				case EFILESH:	/* file shared */
#endif
				case EFSEXCLWR:	/* File is write protected */
					continue;	/* retry getting lock */
				}
			if (errormsg != NULL) {
				sprintf(errmsg,
					"fcntl(%d, %d, &flocks): errno:%d %s\n",
					fd, cmd, errno, strerror(errno));
				*errormsg = errmsg;
			}
			return -1;
		}
		break;
	}

	return ret;

}				/* end of record_lock */
