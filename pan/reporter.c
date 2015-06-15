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
/* $Id: reporter.c,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
/*
 * This is the report generator half of the scanner program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "reporter.h"
#include "symbol.h"
#include "tag_report.h"
#include "splitstr.h"

/************************************************************************
 *                      Report Generation                               *
 ************************************************************************/

static int scanner_reporter(SYM);
static int iscanner_reporter(SYM);
static int scanner_test_end(SYM, SYM, SYM);
static int iscanner_test_end(SYM, SYM, SYM);

static int (*reporter_func) (SYM) = scanner_reporter;
static int (*test_end_func) (SYM, SYM, SYM) = scanner_test_end;

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
static int scanner_reporter(SYM tags)
{
	DBT Key, Data;
	SYM Tag, Keys;

	time_t clock;
	struct tm *tm;

	/* a list of tags, a count of the number of tags allocated to the list,
	   and a pointer to go thru the list */
	char **taglist, **tl;
	int ntags;
	int tagcount;		/* how many tags used */

	char key_get[KEYSIZE];
	char *info;

	/*
	 * extract tag names from data
	 */
	ntags = NTAGS_START;
	taglist = malloc(sizeof(char *) * ntags);
	tagcount = 0;

	tl = taglist;
	sym_seq(tags, &Key, &Data, R_FIRST);
	do {
		if (tagcount == ntags) {
			/* exceeded tag array size -- realloc */
			ntags += NTAGS_START;
			taglist =
			    (char **)realloc(taglist, sizeof(char *) * ntags);
			tl = taglist + tagcount;
		}

		*tl++ = Key.data;
		tagcount++;
	} while (sym_seq(tags, &Key, &Data, R_NEXT) == 0);

	if (tagcount == ntags) {
		/* exceeded tag array size -- realloc */
		ntags += NTAGS_START;
		taglist = (char **)realloc(taglist, sizeof(char *) * ntags);
		tl = taglist + tagcount;
	}

	*tl++ = NULL;
	ntags = tagcount;
	/* Retrieve one "stime" to get the date. */
	for (tl = taglist; *tl != NULL; tl++) {
		strcpy(key_get, *tl);
		strcat(key_get, ",_keys,stime");
		if ((info = (char *)sym_get(tags, key_get)) != NULL) {
			clock = atoi(info);
			tm = gmtime(&clock);
			strftime(key_get, KEYSIZE, "%x", tm);
			sym_put(tags, strdup("_RTS,date"), strdup(key_get), 0);
			break;
		}
	}

	print_header(tags);

	/*
	 * The way that I am using 'Keys' and 'Tag' makes assumptions about the
	 * internals of the sym_* data structure.
	 */
	/* dump 'em all */
	for (tl = taglist; *tl != NULL; tl++) {
		if (!strcmp(*tl, "_RTS"))
			continue;

		strcpy(key_get, *tl);
		strcat(key_get, ",_keys");
		if ((Keys = sym_get(tags, key_get)) == NULL) {
			return 0;
		}

		strcpy(key_get, *tl);
		if ((Tag = sym_get(tags, key_get)) != NULL) {
			tag_report(NULL, Tag, Keys);
		}
	}
	free(taglist);

	return 0;
}

/*
 * End-Of-Test seen, insert this tag into the global tag data.
 * (1) Get the test's tag
 * (2) insert the keywords in the "_keys" tag
 * (3) insert it into the global data under this tag, replacing any existing
 *      data.
 *
 * a "feature" of the key implimentation: I can insert a key tree
 * under another key tree with almost zero brainwork because a SYM
 * is what the DATA area points to.
 */
static int scanner_test_end(SYM alltags, SYM ctag, SYM keys)
{
	static int notag = 0;	/* counter for records with no tag (error) */
	char tagname[KEYSIZE];	/* used when creating name (see above) */
	char *tag;		/* tag name to look things up in */
	char *status;		/* initiation status of old tag */
	SYM rm;			/* pointer to old tag -- to remove it */

	if (alltags == NULL || keys == NULL || ctag == NULL)
		return -1;	/* for really messed up test output */

	/* insert keys into tag */
	sym_put(ctag, "_keys", (void *)keys, 0);

	/* get the tag, or build a new one */
	if ((tag = (char *)sym_get(keys, "tag")) == NULL) {
		/* this is an "impossible" situation: test_output checks for this
		 * and creates a dummy tag. */
		sprintf(tagname, "no_tag_%d", notag++);
		fprintf(stderr, "No TAG key!  Using %s\n", tagname);
		sym_put(keys, "tag", strdup(tagname), 0);
		tag = strdup(tagname);
	}

	/*
	 * Special case: duplicate tag that has an initiation_status failure
	 * is thrown away.
	 */
	if ((rm = (SYM) sym_get(alltags, tag)) != NULL) {
		if ((status =
		     (char *)sym_get(keys, "initiation_status")) != NULL) {
			if (strcmp(status, "ok")) {
				/* do not take new data.  remove new data */
				sym_rm(ctag, RM_KEY | RM_DATA);
				return 1;
			} else {
				/* remove old data in alltags */
				sym_rm(rm, RM_KEY | RM_DATA);
			}
		} else {
			/* new data does not have an initiation_status -- throw it away */
			sym_rm(ctag, RM_KEY | RM_DATA);
			return 1;
		}
	}

	/* put new data.. replaces existing "tag" key if it exists
	 * (it's data should have been removed above) */
	sym_put(alltags, tag, ctag, PUT_REPLACE);

	return 0;
}

static int iscanner_reporter(SYM tags)
{
	return 0;
}

static int iscanner_test_end(SYM alltags, SYM ctag, SYM keys)
{
	if (alltags == NULL || keys == NULL || ctag == NULL)
		return -1;	/* for really messed up test output */

	/* insert keys into tag */
	sym_put(ctag, "_keys", (void *)keys, 0);

	return tag_report(alltags, ctag, keys);
}

int reporter(SYM s)
{
	return reporter_func(s);
}

int test_end(SYM a, SYM b, SYM c)
{
	return test_end_func(a, b, c);
}

void set_scanner(void)
{
	reporter_func = scanner_reporter;
	test_end_func = scanner_test_end;
}

void set_iscanner(void)
{
	reporter_func = iscanner_reporter;
	test_end_func = iscanner_test_end;
}
