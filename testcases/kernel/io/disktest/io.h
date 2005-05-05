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
* $Id: io.h,v 1.4 2005/05/05 19:46:55 mridge Exp $
* $Log: io.h,v $
* Revision 1.4  2005/05/05 19:46:55  mridge
* upgrade to 1.2.8
*
* Revision 1.8  2005/05/03 16:24:38  yardleyb
* Added needed code changes to support windows
*
* Revision 1.7  2004/11/03 22:50:23  yardleyb
* Added O_DIRECT ifdef for Linux on Power
*
* Revision 1.6  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
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
#ifndef IO_H_
#define IO_H_ 1

#ifdef WINDOWS
#include <windows.h> /* HANDLE */
#endif

#include "defs.h"

#ifdef PPC
#ifdef LINUX
// for linux on power O_DIRECT is only defined in asm/fcntl.h
#define O_DIRECT 0400000
#endif
#endif

#ifdef WINDOWS
#define CLOSE(fd) CloseHandle(fd)
typedef HANDLE fd_t;
#else
#include <stdio.h>
#define CLOSE(fd) close(fd)
typedef int fd_t;
#endif

fd_t Open(const char *, const OFF_T);
OFF_T Seek(fd_t, OFF_T);
OFF_T SeekEnd(fd_t);
long Write(fd_t, const void *, const unsigned long);
long Read(fd_t, void *, const unsigned long);
int Sync (fd_t);

#endif /* IO_H_ */

