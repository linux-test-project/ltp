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
* $Id: globals.h,v 1.6 2008/02/14 08:22:23 subrata_modak Exp $
* $Log: globals.h,v $
* Revision 1.6  2008/02/14 08:22:23  subrata_modak
* Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
*
* Revision 1.7  2006/04/21 23:10:43  yardleyb
* Major updates for v1_3_3 of disktest.  View README for details.
*
* Revision 1.6  2005/10/12 23:13:35  yardleyb
* Updates to code to support new function in disktest version 1.3.x.
* Actual changes are recorded in the README
*
* Revision 1.5  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
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
* Revision 1.2  2002/03/07 03:32:13  yardleyb
* Removed the use of global
* appname.  Set devname to
* init. value of "No Device"
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef _GLOBALS_H
#define _GLOBALS_H 1

#include "defs.h"
#include "threading.h"

/* global flags */
#define GLB_FLG_QUIET	0x00000001
#define GLB_FLG_SUPRESS	0x00000002
#define GLB_FLG_PERFP	0x00000004 /* forces alternate performance printing format */
#define GLB_FLG_KILL	0x00000008 /* will kill all threads to all targets when set */

#define PDBG1  if (gbl_dbg_lvl > 0) pMsg
#define PDBG2  if (gbl_dbg_lvl > 1) pMsg
#define PDBG3  if (gbl_dbg_lvl > 2) pMsg
#define PDBG4  if (gbl_dbg_lvl > 3) pMsg
#define PDBG5  if (gbl_dbg_lvl > 4) pMsg

extern unsigned int gbl_dbg_lvl;

void init_gbl_data(test_env_t *);
#ifdef WINDOWS
void PrintLastSystemError(unsigned long);
void GetSystemErrorString(unsigned long, void *);
#endif

#endif /* _GLOBALS_H */

