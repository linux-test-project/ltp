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
* $Id: globals.h,v 1.1 2002/02/21 16:49:04 robbiew Exp $
* $Log: globals.h,v $
* Revision 1.1  2002/02/21 16:49:04  robbiew
* Relocated disktest to /kernel/io/.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef _GLOBALS_H
#define _GLOBALS_H

/* global flags */
#define GLB_FLG_QUIET	0x00000001
#define GLB_FLG_PERFP	0x00000002

#ifndef WIN32
/* semaphore lock definitions */

#define TOTAL_SEMS	6

#endif

void init_gbl_data(char **argv);

#endif /* _GLOBALS_H */

