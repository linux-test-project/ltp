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
/* $Id: reporter.h,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
#ifndef _REPORT_H_
#define _REPORT_H_
#include "symbol.h"

void set_scanner(void);
void set_iscanner(void);

int reporter( SYM );
int test_end( SYM, SYM, SYM );

/*
 * how much TCID space to start with (table)
 */
#define NTCID_START 5

/*
 * how much tag space to start with (table)
 */
#define	NTAGS_START	500

/* Return Tokens (from lex) */
#define		KW_START	100
#define		KW_END		101
#define		TEST_START	102
#define		TEST_OUTPUT	103
#define		EXEC_STATUS	104
#define		TEST_END	105
#define		TEXT_LINE	106
#define		KEYWORD		107
#define		KEYWORD_QUOTED	108
#define		CUTS_RESULT	109
#define		CUTS_RESULT_R	110
#define		SPACE		999

/* Scan Modes (above and beyond what I use lex for) */
#define		SCAN_OUTSIDE	10	/* not in anything */
#define		SCAN_RTSKEY	20	/* keywords: rts_keyword */
#define		SCAN_TSTKEY	21	/* keywords: either test_start or
					   execution_status */
#define		SCAN_OUTPUT	30	/* test_output */

/*
 *	Configuration type things
 */
#define KEYSIZE	255	/* maximum key size */

#endif
