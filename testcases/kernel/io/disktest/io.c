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
* $Id: io.c,v 1.5 2005/05/04 17:54:00 mridge Exp $
* $Log: io.c,v $
* Revision 1.5  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.8  2005/05/03 16:24:38  yardleyb
* Added needed code changes to support windows
*
* Revision 1.7  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.6  2002/05/31 18:47:59  yardleyb
* Updates to -pl -pL options.
* Fixed test status to fail on
* failure to open filespec.
* Version set to 1.1.9
*
* Revision 1.5  2002/04/24 01:45:31  yardleyb
* Minor Fixes:
* Read/write time could exceeds overall time
* Heartbeat options sometimes only displayed once
* Cleanup time for large number of threads was very long (windows)
* If heartbeat specified, now checks for performance option also
* No IO was performed when -S0:0 and -pr specified
*
* Revision 1.4  2002/04/03 20:17:16  yardleyb
* Fixed return value for Read/Write
* to be size_t not unsigned long
*
* Revision 1.3  2002/03/30 01:32:14  yardleyb
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
* Revision 1.2  2002/03/07 03:35:31  yardleyb
* Added Open routine
*
* Revision 1.1  2002/02/28 04:21:35  yardleyb
* Split out Read, Write, and Seek
* Initial Checkin
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
	return(tcnt);
}

long Read(fd_t fd, void *buf, const unsigned long trsiz)
{
	long tcnt;
#ifdef WINDOWS
		ReadFile(fd, buf, trsiz, &tcnt, NULL);
#else
		tcnt = read(fd, buf, trsiz);
#endif
	return(tcnt);
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

   if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
   {
      li.QuadPart = -1;
   }

   return li.QuadPart;
}
#endif

OFF_T SeekEnd(fd_t fd)
{
	OFF_T return_lba;

#ifdef WINDOWS
	return_lba=(OFF_T) FileSeek64(fd, 0, FILE_END);
#else
	return_lba=(OFF_T) lseek64(fd, 0, SEEK_END);
#endif
	return(return_lba);
}

OFF_T Seek(fd_t fd, OFF_T lba)
{
	OFF_T return_lba;

#ifdef WINDOWS
	return_lba=(OFF_T) FileSeek64(fd, lba, FILE_BEGIN);
#else
	return_lba=(OFF_T) lseek64(fd,lba,SEEK_SET);
#endif
	return(return_lba);
}

fd_t Open(const char *filespec, const OFF_T flags)
{
	fd_t fd;
#ifdef WINDOWS
	unsigned long OPEN_FLAGS = 0, OPEN_DISPO = 0, OPEN_READ_WRITE = 0, OPEN_SHARE = 0;

	if((flags & CLD_FLG_R) && !(flags & CLD_FLG_W)) {
		OPEN_READ_WRITE |= GENERIC_READ;
		OPEN_SHARE |= FILE_SHARE_READ;
	} else if(!(flags & CLD_FLG_R) && (flags & CLD_FLG_W)) {
		OPEN_READ_WRITE |= GENERIC_WRITE;
		OPEN_SHARE |= FILE_SHARE_WRITE;
	} else {
		OPEN_READ_WRITE |= (GENERIC_READ | GENERIC_WRITE);
		OPEN_SHARE |= (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}

#ifdef CLD_FLG_DIRECT
	if(flags & CLD_FLG_DIRECT) OPEN_FLAGS = FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
#endif
	OPEN_DISPO = OPEN_EXISTING;

#ifdef CLD_FLG_RANDOM
	if(flags & CLD_FLG_RANDOM) OPEN_FLAGS |= FILE_FLAG_RANDOM_ACCESS;
#endif
#ifdef CLD_FLG_LINEAR
	if(flags & CLD_FLG_LINEAR) OPEN_FLAGS |= FILE_FLAG_SEQUENTIAL_SCAN;
#endif
#ifdef CLD_FLG_FILE
	if(flags & CLD_FLG_FILE) {
		OPEN_FLAGS |= FILE_ATTRIBUTE_ARCHIVE;
		if(flags & CLD_FLG_W)
			OPEN_DISPO = OPEN_ALWAYS;
	}
#endif
	fd = CreateFile(filespec,
		OPEN_READ_WRITE,
		OPEN_SHARE,
		NULL,
		OPEN_DISPO,
		OPEN_FLAGS,
		NULL);
#else
	int OPEN_MASK = O_LARGEFILE;
	if((flags & CLD_FLG_R) && !(flags & CLD_FLG_W)) OPEN_MASK |= O_RDONLY;
	else if(!(flags & CLD_FLG_R) && (flags & CLD_FLG_W)) OPEN_MASK |= O_WRONLY;
	else OPEN_MASK |= O_RDWR;
#ifdef CLD_FLG_FILE
	if(flags & CLD_FLG_FILE) OPEN_MASK |= O_CREAT;
#endif
#ifdef CLD_FLG_DIRECT
	if(flags & CLD_FLG_DIRECT) OPEN_MASK |= O_DIRECT;
#endif
	fd = open(filespec,OPEN_MASK,00600);
#endif
	return(fd);
}

int Sync (fd_t fd) {
#ifdef WINDOWS
	if(FlushFileBuffers(fd) != TRUE) {
		return -1;
	}
	return 0;
#else
	return fsync(fd);
#endif
}

