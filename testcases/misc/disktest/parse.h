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
* $Id: parse.h,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: parse.h,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
*
* Revision 1.1  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
*/

#ifndef _PARSE_H
#define _PARSE_H

#ifdef WIN32
#include "getopt.h"
#define BLKDEVICESTR "PHYSICALDRIVE"
#else
#define BLKDEVICESTR "DEV"
#endif

#include "main.h"
#include "defs.h"

void fill_cld_args(int argc, char **argv, child_args_t *args);
void make_assumptions(child_args_t *args);
int check_conclusions(child_args_t *args);

#endif
