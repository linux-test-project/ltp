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
* $Id: io.c,v 1.6 2008/02/14 08:22:23 subrata_modak Exp $
*
*/

#ifdef WINDOWS
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#endif

#include "defs.h"
#include "main.h"
#include "io.h"

long Write(fd_t fd, const void *buf, const unsigned long trsiz)
{
	long tcnt;
#ifdef WINDOWS
	WriteFile(fd, buf, trsiz, &tcnt, NULL);
#else
	tcnt = write(fd, buf, trsiz);
#endif
	return (tcnt);
}

long Read(fd_t fd, void *buf, const unsigned long trsiz)
{
	long tcnt;
#ifdef WINDOWS
	ReadFile(fd, buf, trsiz, &tcnt, NULL);
#else
	tcnt = read(fd, buf, trsiz);
#endif
	return (tcnt);
}

#ifdef WINDOWS
/*
 * wrapper for file seeking in WINDOWS API to hind the ugle 32 bit
 * interface of SetFile Pointer
 */
OFF_T FileSeek64(HANDLE hf, OFF_T distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
	}

	return li.QuadPart;
}
#endif

OFF_T SeekEnd(fd_t fd)
{
	OFF_T return_lba;

#ifdef WINDOWS
	return_lba = (OFF_T) FileSeek64(fd, 0, FILE_END);
#else
	return_lba = (OFF_T) lseek64(fd, 0, SEEK_END);
#endif
	return (return_lba);
}

OFF_T Seek(fd_t fd, OFF_T lba)
{
	OFF_T return_lba;

#ifdef WINDOWS
	return_lba = (OFF_T) FileSeek64(fd, lba, FILE_BEGIN);
#else
	return_lba = (OFF_T) lseek64(fd, lba, SEEK_SET);
#endif
	return (return_lba);
}

fd_t Open(const char *filespec, const OFF_T flags)
{
	fd_t fd;
#ifdef WINDOWS
	unsigned long OPEN_FLAGS = 0, OPEN_DISPO = 0, OPEN_READ_WRITE =
	    0, OPEN_SHARE = 0;

	if ((flags & CLD_FLG_R) && !(flags & CLD_FLG_W)) {
		OPEN_READ_WRITE |= GENERIC_READ;
		OPEN_SHARE |= FILE_SHARE_READ;
	} else if (!(flags & CLD_FLG_R) && (flags & CLD_FLG_W)) {
		OPEN_READ_WRITE |= GENERIC_WRITE;
		OPEN_SHARE |= FILE_SHARE_WRITE;
	} else {
		OPEN_READ_WRITE |= (GENERIC_READ | GENERIC_WRITE);
		OPEN_SHARE |= (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}

#ifdef CLD_FLG_DIRECT
	if (flags & CLD_FLG_DIRECT)
		OPEN_FLAGS = FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
#endif
	OPEN_DISPO = OPEN_EXISTING;

#ifdef CLD_FLG_RANDOM
	if (flags & CLD_FLG_RANDOM)
		OPEN_FLAGS |= FILE_FLAG_RANDOM_ACCESS;
#endif
#ifdef CLD_FLG_LINEAR
	if (flags & CLD_FLG_LINEAR)
		OPEN_FLAGS |= FILE_FLAG_SEQUENTIAL_SCAN;
#endif
#ifdef CLD_FLG_FILE
	if (flags & CLD_FLG_FILE) {
		OPEN_FLAGS |= FILE_ATTRIBUTE_ARCHIVE;
		if (flags & CLD_FLG_W)
			OPEN_DISPO = OPEN_ALWAYS;
	}
#endif
	fd = CreateFile(filespec,
			OPEN_READ_WRITE,
			OPEN_SHARE, NULL, OPEN_DISPO, OPEN_FLAGS, NULL);
#else
	int OPEN_MASK = O_LARGEFILE;
	if ((flags & CLD_FLG_R) && !(flags & CLD_FLG_W))
		OPEN_MASK |= O_RDONLY;
	else if (!(flags & CLD_FLG_R) && (flags & CLD_FLG_W))
		OPEN_MASK |= O_WRONLY;
	else
		OPEN_MASK |= O_RDWR;
#ifdef CLD_FLG_FILE
	if (flags & CLD_FLG_FILE)
		OPEN_MASK |= O_CREAT;
#endif
#ifdef CLD_FLG_DIRECT
	if (flags & CLD_FLG_DIRECT)
		OPEN_MASK |= O_DIRECT;
#endif
	fd = open(filespec, OPEN_MASK, 00600);
#endif
	return (fd);
}

int Sync(fd_t fd)
{
#ifdef WINDOWS
	if (FlushFileBuffers(fd) != TRUE) {
		return -1;
	}
	return 0;
#else
	return fsync(fd);
#endif
}
