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
/* $Id: ltp-scanner.c,v 1.1 2009/05/19 09:39:11 subrata_modak Exp $ */
/*
 * An RTS/pan driver output processing program.
 *
 * This program reads an RTS/pan driver output format file, parses it using lex
 * and saves the information into an in-memory hierarchical keyword table.
 *
 * The reporting segment of the program reads that keyword table to produce
 * it's reports.
 *
 * Synopsis:
 * 	ltp-scanner [ -e ] [ -D area:level ] [ -h ]
 *
 * Description:
 *   Scanner is part of the RTS 2.0 reporting mechanism or pan.
 *   It processes RTS/pan driver format output and produces a single simple report
 *   of each test tag executed, the TCIDs it executed, and their testcases.
 *
 * Options:
 *   -e
 *	use an "extended" output format
 *
 *   -D
 *	enable debug statements.  Areas are listed in report2.h and levels
 *	are in the code.  Must be compiled with "-DDEBUGGING"
 *
 *   -h
 *	print out a command usage statement and exit.
 *
 * INPUT
 *   The input must conform to the RTS/pan driver format.
 *
 * Report Format
 *   A single report style is used.  It consists of a header made of all
 *   keywords in the rts_keywords fields of the driver output, and the test
 *   information.
 *	interpretation of CUTS "number of testcases" field when there are
 *	multiple TCIDs.  It must be the sum of all TCIDs' testcases.
 *
 * System Configuration:
 * ARCHITECTURE         IOS_MODEL_E CRAY_YMP YMP7XX
 * CONFIG               JOBCNTL AVL BMD EMA HPM SECURE TFM_UDB_6 SDS SSD
 * RELEASE              82
 * UNAME                sn1703c cool 8.2.0ae d82.25
 * date                 03/24/94
 *
 * tag		tcid		testcase	status		contact
 * ------------------------------------------------------------------------
 *
 *   When a report is made for only a tag, the TCID and Testcase fields
 *   contain a dash ( "-" ).  The intention is that the output be usable
 *   by other Unix programs.
 *
 *   When a report is made for all TCIDs and Testcases, a star ( "*" ) is used.
 *
 *   When in extended mode, an additional output line is produced for each
 *   tag.
 *
 *	This line is identified with a "!" in the TCID and Testcase fields.
 *
 *	It has no minimum and maximum field widths, so the output does not
 *	line up in columns
 *
 *	the "status" field contains the initiation status
 *
 *	the "contact" field does not expand multiple comma-separated contacts
 *
 *	fields:
 *		tag, tcid, testcase, status, contact,
 *		start time, duration, termination type, termination id,
 *		output starting line, output ending line
 *
 * RELATED DOCUMENTS
 *	Regression Test System Phase 2 Test Result Reporting System
 *
 * AUTHOR
 *   Glen Overby wrote the code.
 *
 * Internal Data Format
 *   All data is maintained in a hierarchical key database.  While there are
 *   many available databases, this impliments a simple ASCII comma-separated
 *   keyed database.
 *
 *   Key Naming
 *	- The top-level keys are named after the RTS or pan test tags.
 *	- The top-level key named "_RTS" contains the RTS Keywords
 *	- Each tag has a "_keys" tag that contains the key fields from
 *	  the TEST_START and EXECUTION_STATUS fields.
 */

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scan.h"
#include "debug.h"
#include "reporter.h"
#include "symbol.h"

char *cnf;			/* current filename */
int extended = 0;		/* -e option        */

int main(int argc, char *argv[])
{
	SYM tags;		/* tag data */
	int c;

	while ((c = getopt(argc, argv, "D:ehi")) != -1) {
		switch (c) {
		case 'i':
			set_iscanner();
			break;
		case 'D':
			set_debug(optarg);
			break;
		case 'e':
			extended++;
			break;
		case 'h':
			fprintf(stderr,
				"%s [-e] [-i] [ -D area, level ] input-filenames\n",
				argv[0]);
			exit(0);
		default:
			fprintf(stderr, "invalid argument, %c\n", c);
			exit(1);
		}
	}

	lex_files(&argv[optind]);	/* I hope that argv[argc+1] == NULL */
	tags = sym_open(0, 0, 0);

	scanner(tags);
#ifdef DEBUGGING
	DEBUG(D_INIT, 1)
	    sym_dump_s(tags, 0);
#endif
	reporter(tags);

	exit(0);
}
