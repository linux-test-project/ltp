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
* $Id: childmain.h,v 1.7 2008/03/24 10:33:53 subrata_modak Exp $
*
*/

#ifndef _CHILDMAIN_H
#define _CHILDMAIN_H 1

#define SEEK_FAILURE	1
#define ACCESS_FAILURE	2
#define DATA_MISCOMPARE	3

typedef enum mc_func {
	EXP,ACT,REREAD
} mc_func_t;

#define DMOFFSTR "Thread %d: First miscompare at byte offset %zd (0x%zX)\n"

#ifdef WINDOWS
#define DMSTR "Thread %d: Data miscompare at lba %I64d (0x%I64X)\n"
#define AFSTR "Thread %d: %s failed: seek %I64u, lba %I64u (0x%I64X), got = %ld, asked for = %ld, errno %lu\n"
#define SFSTR "Thread %d: seek failed seek %I64d, lba = %I64d, request pos = %I64d, seek pos = %I64d, errno %lu\n"
#define DMFILESTR "\n********** %s (Target: %s, LBA: %I64d, Offset: %d) **********\n"
DWORD WINAPI ChildMain(test_ll_t *);
#else
#define DMSTR "Thread %d: Data miscompare at lba %lld (0x%llX)\n"
#define AFSTR "Thread %d: %s failed: seek %llu, lba %lld (0x%llX), got = %ld, asked for = %ld, errno %lu\n"
#define SFSTR "Thread %d: seek failed seek %lld, lba = %lld, request pos = %lld, seek pos = %lld, errno %lu\n"
#define DMFILESTR "\n********** %s (Target: %s, LBA: %lld, Offset: %zd) **********\n"
void *ChildMain(void *);
#endif

#endif /* _CHILDMAIN_H */

