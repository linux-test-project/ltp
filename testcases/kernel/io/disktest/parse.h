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
* $Id: parse.h,v 1.4 2005/05/04 17:54:00 mridge Exp $
* $Log: parse.h,v $
* Revision 1.4  2005/05/04 17:54:00  mridge
* Update to version 1.2.8
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
* Revision 1.4  2002/04/02 01:18:12  yardleyb
* Added ifdef for AIX and raw
* device path
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
* Revision 1.2  2002/03/07 03:38:52  yardleyb
* Added dump function from command
* line.  Created formatted dump output
* for Data miscomare and command line.
* Can now leave off filespec the full
* path header as it will be added based
* on -I.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef _PARSE_H
#define _PARSE_H

#ifdef WINDOWS
#include "getopt.h"
#define BLKDEVICESTR "\\\\"
#else
#ifdef _AIX
#define RAWDEVICESTR "rhdisk"
#else
#define RAWDEVICESTR "raw"
#endif
#define BLKDEVICESTR "DEV"
#endif




#include "main.h"
#include "defs.h"

#ifdef WINDOWS
#define DEV_BLK_HEADER "\\\\.\\"
#define DEV_RAW_HEADER "\\\\.\\"
#else
#define DEV_BLK_HEADER "/dev/"
#ifdef AIX
#define DEV_BLK_HEADER "/dev/"
#else
#define DEV_RAW_HEADER "/dev/raw/"
#endif
#endif

int fill_cld_args(int, char **, child_args_t *);
int make_assumptions(child_args_t *);
int check_conclusions(child_args_t *);

#endif
