/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: tag_report.h,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
#ifndef _TAG_REPORT_H_
#define _TAG_REPORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>	/* strftime */
#include <unistd.h>	/* getopt */
#include "symbol.h"
#include "splitstr.h"

int test_result( char *, char *, char *, char *, SYM );
int cuts_report( SYM, SYM, char *, char * );
int tag_report( SYM, SYM, SYM );
int print_header( SYM );
int cuts_testcase( SYM, SYM );

#endif
