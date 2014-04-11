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
/* $Id: tag_report.c,v 1.2 2006/12/13 22:55:22 vapier Exp $ */
#include "tag_report.h"
#include "debug.h"
#include "reporter.h"
#include "splitstr.h"

static char *worst_case(char *, char *);

/************************************************************************
 *			Report Generation				*
 ************************************************************************/

/*
 * printf format statement for standard reports
 * 5 fields with max/min widths
 */
#define FORMAT "%-20.20s %-15.15s %10.10s %-20.20s %s\n"

/*
 *  This is the central results reporting function.  All standard report
 *  format results are printed thru test_result.
 */
int test_result(char *tag, char *tcid, char *tc, char *result, SYM tags)
{
	char *expert, expkey[KEYSIZE];
	register char *c;
	char **cont;
	const char **cont_save;

	if (tcid == NULL)
		tcid = "-";
	if (tc == NULL)
		tc = "-";
	if (tag == NULL)
		tag = "test_result: no tag";
	if (result == NULL)
		result = "(RESULT IS NULL)";

	strcpy(expkey, "contacts");
	/* note: the sym_get here does _not_ change the "cursor" */
	if ((expert = (char *)sym_get(tags, expkey)) == NULL) {
		expert = "UNKNOWN";
	}

	/* ' tr " " "_" ' */
	for (c = result; *c; c++) {
		if (*c == ' ') {
			*c = '_';
		}
	}
	if (*result == '\0')
		result = "?";

	/* split contacts on "," and print out a line for each */
	cont_save = splitstr(expert, ",", NULL);
	for (cont = (char **)cont_save; *cont != NULL; cont++) {
		printf(FORMAT, tag, tcid, tc, result, *cont);
	}
	splitstr_free(cont_save);

	return 0;
}

/*
 * CUTS test reporting.
 *
 *  (1) make a list (2d char array) of all TCIDs (see above for why)
 *  (2) look thru the list:
 *	(a) keep track of the "worst case" in this *TAG*
 *	(b) report each testcase's results
 *	(c) if the testcase number is != 0, count it
 *  (3) report tag's results
 *  (4) check the number of expected results with the actual results,
 *	report an error if they don't match.
 */

int cuts_report(SYM tags, SYM keys, char *at, char *tag)
{
	DBT Key, Data;

	/* analysis type: count of CUTS test cases */
	const char **ant;
	char *dat;		/* strdup(at) */
	int tccount;		/* expected count of testcases */
	int tcnum;		/* seen count of testcases */

	/* a list of tcids */
	char **taglist, **tl;
	int ntags, tagcount;

	char key_get[255];

	char *result = "", *worst_case();	/* overall result */

	/* parse analysis type: cuts:tc-count */
	ant = splitstr((dat = strdup(at)), ":", NULL);
	if (ant[1] != NULL)
		tccount = atoi(ant[1]);
	else
		tccount = 0;
	free(dat);
	splitstr_free(ant);

	/* extract tcids */
	ntags = NTCID_START;
	taglist = (char **)malloc(sizeof(char *) * ntags);
	tagcount = 0;

	tl = taglist;
	sym_seq(tags, &Key, &Data, R_FIRST);
	do {
		if (tagcount == ntags) {
			/* exceeded tag array size -- realloc */
			ntags += NTCID_START;
			taglist =
			    (char **)realloc(taglist, sizeof(char *) * ntags);
			tl = taglist + tagcount;
		}

		if (strcmp((char *)Key.data, "_keys") == 0)
			continue;
		DEBUG(D_REPORT, 10)
		    printf("cuts_report: tcid %s\n", (char *)Key.data);
		*tl++ = Key.data;
		tagcount++;
	} while (sym_seq(tags, &Key, &Data, R_NEXT) == 0);

	if (tagcount == ntags) {
		/* exceeded tag array size -- realloc */
		ntags++;	/* need just one more */
		taglist = (char **)realloc(taglist, sizeof(char *) * ntags);
		tl = taglist + tagcount;
	}

	*tl++ = NULL;

	ntags = tagcount;

	/* dump all found records */
	tcnum = 0;
	for (tl = taglist; *tl != NULL; tl++) {

		strcpy(key_get, *tl);
		Key.data = (void *)key_get;

		/*sym_dump_s(sym_get(tags, key_get), 0); */

		sym_seq(tags, &Key, &Data, R_CURSOR);
		do {
			DEBUG(D_REPORT, 10)
			    printf("cuts_report: tc %s = %s\n",
				   (char *)Key.data, (char *)Data.data);
			result = worst_case(result, (char *)Data.data);
			test_result(tag, *tl, (char *)Key.data,
				    (char *)Data.data, keys);
			if (atoi((char *)Key.data))
				tcnum++;
		} while (sym_seq(tags, &Key, &Data, R_NEXT) == 0);
	}

	test_result(tag, "*", "*", result, keys);

	if (tccount != 0 && tccount != tcnum)
		test_result(tag, "-", "-", "TC count wrong", keys);

	free(taglist);

	return 0;
}

/*
 * Do the report generation.
 *
 * A problem: I really need multiple cursors.  I'd rather not look into
 * the depths of the current symbol table implimentation (there are the
 * cursors there that I could use) so that a different (faster!) symbol
 * table can be used in the future.
 *
 * I could get a key (tag), get it's sub-keys (TCIDs), then get the key
 * again to reset to the top level, _then_ get the next key.  That would
 * be very inefficient.
 *
 * The solution I chose is to extract all tags into a list (char array),
 * then go thru that list with the cursor free for other levels to use.
 *
 *  (1) make a list (2d char array) of all Tags
 *  (2) search for the first tag that has a "stime" record, and use that as
 *      the date (MMDDYY) that the tests were run.
 *  (3) print the report header
 *  (4) go thru all tags and report each as described at the beginning of
 *      this file
 */
int tag_report(SYM alltags, SYM ctag, SYM keys)
{

	extern int extended;

	char key_get[KEYSIZE];
	char *info;

	/* retrieved _keys values: initation status, start time, duration,
	 * termination type, termination id, start line, end line.          */
	char *tag, *contact, *is, *mystime, *duration, *tt, *ti, *sl, *el;

	/* Check all driver-level status first */
	strcpy(key_get, "tag");
	if ((tag = (char *)sym_get(keys, key_get)) == NULL) {
		return -1;
	}

	/* Check all driver-level status first */
	strcpy(key_get, "initiation_status");
	if ((is = (char *)sym_get(keys, key_get)) == NULL) {
		test_result(tag, NULL, NULL, "no init status", keys);
		return -1;
	}

	if (strcmp(is, "ok")) {
		test_result(tag, NULL, NULL, is, keys);
	} else {

		strcpy(key_get, "corefile");
		if ((info = (char *)sym_get(keys, key_get)) != NULL)
			if (strcmp(info, "no") != 0) {
				test_result(tag, NULL, NULL, "coredump", keys);
			}

		strcpy(key_get, "termination_type");
		if ((tt = (char *)sym_get(keys, key_get)) == NULL) {
			test_result(tag, NULL, NULL, "no Term Type", keys);
			return -1;
		}

		if (strcmp(tt, "exited")) {
			test_result(tag, NULL, NULL, tt, keys);
		}

		strcpy(key_get, "analysis");
		if ((info = (char *)sym_get(keys, key_get)) == NULL) {
			test_result(tag, NULL, NULL, "no Analysis Type", keys);
			return -1;
		}

		/* Getting here indicates that there were no fatal driver-level
		 * errors.  Do the kind of reporting requested by the test.
		 */

		if (strncmp(info, "none", 4) == 0) {
			/*
			 * If analysis is 'none', alway report the test as
			 * a pass regardless of output or exit status.
			 */
			test_result(tag, NULL, NULL, "pass", keys);

		} else if (strncmp(info, "cuts", 4)) {

			/*
			 * If analysis is not cuts, assume it is 'exit', thus
			 * the termination_id is used to determine pass/fail result.
			 */
			if (strcmp(tt, "timeout")) {
				strcpy(key_get, "termination_id");
				if ((info =
				     (char *)sym_get(keys, key_get)) == NULL) {
					test_result(tag, NULL, NULL,
						    "no_Term_Id", keys);
				} else {
					if (strcmp(info, "0")) {
						test_result(tag, NULL, NULL,
							    "fail", keys);
					} else {
						test_result(tag, NULL, NULL,
							    "pass", keys);
					}
				}
			}
		} else {
			cuts_report(ctag, keys, info, tag);
		}
	}

	/*
	 * Extended Format:
	 *  - tcid+tc = "!"
	 *  - tab separated fields
	 *  - no field widths
	 *  - fields 6 - ~ are:
	 *  start-time (time_t)
	 *  duration
	 *  termination_id
	 *  termination_type
	 *  Start Line (of test results in output file)
	 *  End Line
	 */

	if (extended) {

		strcpy(key_get, "termination_id");
		if ((ti = (char *)sym_get(keys, key_get)) == NULL) {
			ti = "No_Termination_ID";
		}

		strcpy(key_get, "termination_type");
		if ((tt = (char *)sym_get(keys, key_get)) == NULL) {
			tt = "No_Termination_Type";
		}

		strcpy(key_get, "duration");
		if ((duration = (char *)sym_get(keys, key_get)) == NULL) {
			duration = "No_Duration";
		}

		strcpy(key_get, "_Start_line");
		if ((sl = (char *)sym_get(keys, key_get)) == NULL) {
			sl = "No_Start_line";
		}

		strcpy(key_get, "_End_line");
		if ((el = (char *)sym_get(keys, key_get)) == NULL) {
			el = "No_End_line";
		}

		strcpy(key_get, "contacts");
		if ((contact = (char *)sym_get(keys, key_get)) == NULL) {
			contact = "No_Contacts";
		}

		strcpy(key_get, "stime");
		if ((mystime = (char *)sym_get(keys, key_get)) == NULL) {
			mystime = "No_stime";
		}

		printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n",
		       tag, "!", "!", is, contact, mystime, duration,
		       ti, tt, sl, el);
	}

	return 0;
}

/*
 *  Print a header made up of the RTS keywords
 *  In "extended" mode, print the header to stderr.
 */
int print_header(SYM tags)
{
	DBT Key, Data;
	char key_get[255];

	FILE *out;

	extern int extended;

	if (extended)
		out = stderr;
	else
		out = stdout;

	fprintf(out, "System Configuration:\n");
	/* build header out of RTS keywords */
	sprintf(key_get, "_RTS");
	Key.data = (void *)key_get;
	if (sym_seq(tags, &Key, &Data, R_CURSOR) == 0) {
		do {
			if (strcmp((char *)Key.data, "PATH") == 0)
				continue;
			fprintf(out, "%-20.20s %s\n", (char *)Key.data,
				(char *)Data.data);
		} while (sym_seq(tags, &Key, &Data, R_NEXT) == 0);
	}

	fprintf(out, "\n");
	fprintf(out, FORMAT, "tag", "tcid", "testcase", "status", "contact");
	fprintf(out,
		"-------------------------------------------------------------------------------\n");

	return 0;
}

/*
 * CUTS testcase record
 *
 * This is passed s SYM for the current tag and the initiation keys.
 * The text seen by lex is in yytext (global).
 */
int cuts_testcase(SYM tag, SYM keys)
{
	char *cuts_info[6];
	char key[KEYSIZE];
	char *oldresult, *newresult, *worst_case();
	int tok_num = 0;
	extern char yytext[];

	cuts_info[tok_num] = strtok(yytext, "\t ");
	while (tok_num < 5 &&
	       (cuts_info[++tok_num] = strtok(NULL, "\t ")) != NULL) ;

	strcpy(key, cuts_info[0]);
	strcat(key, ",");
	strcat(key, cuts_info[1]);

#ifdef DEBUGGING
	DEBUG(D_SCAN_CUTS, 1) {
		printf("cuts_testcase: TCID=%s TC=%s Result=%s\n", cuts_info[0],
		       cuts_info[1], cuts_info[2]);
		printf("cuts_testcase: %d %s\n", tok_num, key);
	}
#endif

	if ((oldresult = (char *)sym_get(tag, key)) != NULL) {
		/* Duplicate -- assume mulitple runs */
		/* keep "worst case" */
		newresult = worst_case(oldresult, cuts_info[2]);
		sym_put(tag, key, strdup(newresult), PUT_REPLACE);
		free(oldresult);	/* remove the "data" portion of the key */
	} else {
		sym_put(tag, key, strdup(cuts_info[2]), 0);
	}
	return 0;
}

/*
 * Determine a "worst case" status from two given statuses.
 */
static char *worst_case(char *t1, char *t2)
{
	/* NULL-terminated table, ordered from worst-case to best-case */
	static char *worst[] = {
		"FAIL", "BROK", "PASS", "CONF",
		"WARN", "INFO", NULL,
	};

	char **w1, **w2;

	/* Search the table for each status, then use the index to determine
	   which has a lower precedence */
	for (w1 = worst; *w1 != NULL && strcmp(t1, *w1); w1++) ;

	for (w2 = worst; *w2 != NULL && strcmp(t2, *w2); w2++) ;

	if (w1 < w2)
		return (t1);
	else
		return (t2);

}
