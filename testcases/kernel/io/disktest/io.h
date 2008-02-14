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
* $Id: io.h,v 1.5 2008/02/14 08:22:23 subrata_modak Exp $
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
/* for linux on power 2.4 O_DIRECT is only defined in asm/fcntl.h */
#ifndef O_DIRECT
#define O_DIRECT 0400000
#endif
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

