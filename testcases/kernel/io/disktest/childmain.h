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
* $Id: childmain.h,v 1.5 2005/05/04 17:54:00 mridge Exp $
* $Log: childmain.h,v $
* Revision 1.5  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
*
* Revision 1.4  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
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
* Revision 1.2  2001/12/07 23:33:29  yardleyb
* Fixed bug where a false positive data
* miscompare could occur when running
* multi cycle testing with mark block
* enabled.
*
* Revision 1.1  2001/12/04 18:52:33  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef _CHILDMAIN_H
#define _CHILDMAIN_H 1

#define SEEK_FAILURE	1
#define ACCESS_FAILURE	2
#define DATA_MISCOMPARE	3

typedef struct action {
	op_t	oper;
	unsigned long trsiz;
	OFF_T	lba;
} action_t;

#ifdef WINDOWS
#define DMSTR "Data miscompare at lba %I64d (0x%I64X)\n"
#define AFSTR "%s failed: seek %I64u, lba %I64u (0x%I64X), got = %ld, asked for = %ld\n"
#define SFSTR "seek failed seek %I64d, lba = %I64d, request pos = %I64d, seek pos = %I64d\n"
DWORD WINAPI ChildMain(test_ll_t *);
#else
#define DMSTR "Data miscompare at lba %lld (0x%llX)\n"
#define AFSTR "%s failed: seek %llu, lba %lld (0x%llX), got = %ld, asked for = %ld\n"
#define SFSTR "seek failed seek %lld, lba = %lld, request pos = %lld, seek pos = %lld\n"
void *ChildMain(void *);
#endif

#endif /* _CHILDMAIN_H */

