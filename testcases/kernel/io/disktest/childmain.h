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
* $Id: childmain.h,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: childmain.h,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
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

#define SEEK_FAILURE	1
#define ACCESS_FAILURE	2
#define DATA_MISCOMPARE	3

typedef struct action {
	op_t	oper;
	unsigned long trsiz;
	OFF_T	lba;
} action_t;

#ifdef WIN32
DWORD WINAPI ChildMain(child_args_t *);
#else
void *ChildMain(void *);
#endif
