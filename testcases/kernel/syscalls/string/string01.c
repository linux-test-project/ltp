/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	string01.c -  check string functions.
 *
 * CALLS
 *	strchr, strrchr, strcat, strcmp, strcpy, strlen,
 *	  strncat, strncmp, strncpy
 *
 * ALGORITHM
 *	Test functionality of the string functions:
 *		(strchr, strrchr, strcat, strcmp, strcpy, strlen,
 *		 strncat, strncmp, strncpy )
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include <errno.h>
#include <stdlib.h>
#include "test.h"

#define FAILED 0
#define PASSED 1

char *TCID = "string01";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;
/*****	**	** *****/

#define LONGSTR	(96*1024-1)
/* #define LONGSTR	(1024-1)		*/

/*
 *	Miscellaneous data strings for testing.
 */

char tiat[] = "This is a test of the string functions.  ";
char yat[] = "This is yet another test.";
char tiatyat[] =
    "This is a test of the string functions.  This is yet another test.";

char longstr[LONGSTR + 1];	/* a very long string */
char dst0[LONGSTR + 1];		/* place holders for various tests */
char dst1[LONGSTR + 1];
char dst2[LONGSTR + 1];


/*	Strlen	(strlen( s ) == e_res)		*/
struct t_strlen {
	char *s;
	int e_res;
} t_len[] = {
	{
	"", 0}, {
	"12345", 5}, {
	tiat, 41}, {
	longstr, LONGSTR}, {
	NULL, 0}
};

/*	Index	(index( s, c ) == e_res)		*/
struct t_index {
	char *s;
	char c;
	char *e_res;
} t_index[] = {
	{
	"", 'z', NULL}, {
	tiat, 'a', tiat + 8}, {
	tiat, 's', tiat + 3}, {
	tiat, 'o', tiat + 15}, {
	tiat, 'z', NULL}, {
	NULL, 0, NULL}
};

/*	Rindex	(rindex( s, c ) == e_res)		*/
struct t_rindex {
	char *s;
	char c;
	char *e_res;
} t_rindex[] = {
	{
	"", 'z', NULL}, {
	tiat, 'a', tiat + 8}, {
	tiat, 's', tiat + 37}, {
	tiat, 'o', tiat + 35}, {
	tiat, 'z', NULL}, {
	NULL, 0, NULL}
};

/*	Strcmp	(strcmp( s1, s2 ) == e_res)		*/
struct t_strcmp {
	char *s1;
	char *s2;
	int e_res;
} t_cmp[] = {
	{
	"", "", 0}, {
	"", tiat, -((int)'T')}, {
	tiat, "", 'T'}, {
	tiat, tiat, 0}, {
	yat, tiat, 'y' - 'a'}, {
	NULL, NULL, 0}
};

/*	Strcat	(strcmp( strcat(s1, s2),  s1s2 ) == e_res)		*/
/*	ASSUMES strcmp is working -- it is tested prior to strcat	*/
struct t_strcat {
	char *s1;
	char *s2;
	char *s1s2;
	int e_res;
} t_cat[] = {
	{
	dst0, "", "", 0}, {
	dst0, tiat, tiat, 0}, {
	dst0, "", tiat, 0}, {
	dst0, yat, tiatyat, 0}, {
	dst1, longstr, longstr, 0}, {
	NULL, NULL, NULL, 0}
};

/*	Strcpy	(strcmp( strcpy(s1, s2),  s1s2 ) == e_res)		*/
/*	ASSUMES strcmp is working -- it is tested prior to strcpy	*/
/*	No overlapping copies are tested				*/
struct t_strcpy {
	char *s1;
	char *s2;
	int e_res;
} t_cpy[] = {
	{
	dst0, "", 0}, {
	dst0, tiat, 0}, {
	dst0, longstr, 0}, {
	NULL, NULL, 0}
};

/*	Strncmp	(strncmp( s1, s2 ) == e_res)		*/
struct t_strncmp {
	char *s1;
	char *s2;
	int n;
	int e_res;
	int a_res;		/* Allowable results, some platforms only return 1 or -1 */
} t_ncmp[] = {
	{
	"", "", 0, 0, 0}, {
	"", "", 80, 0, 0}, {
	tiat, "", 0, 0, 0}, {
	"", tiat, 80, -((int)'T'), -1}, {
	tiat, "", 80, 'T', 1}, {
	tiat, tiat, 80, 0, 0}, {
	yat, tiat, 80, 'y' - 'a', 1}, {
	yat, tiat, 8, 0, 1}, {
	yat, tiat, 9, 'y' - 'a', 1}, {
	NULL, NULL, 0, 0, 0}

};

/*	Strncat	(strcmp( strncat(s1, s2, n),  s1ns2 ) == e_res)	*/
/*	ASSUMES strcmp is working -- it is tested prior to strncat	*/
/*	dest is guaranteed to be all '\0' s at start of test		*/
struct t_strncat {
	char *s1;
	char *s2;
	int n;
	char *s1ns2;
	int e_res;
} t_ncat[] = {
	/*      Regular strcat stuff -- i.e., n is large enough         */
	{
	dst0, "", LONGSTR, "", 0}, {
	dst0, tiat, LONGSTR, tiat, 0}, {
	dst0, "", LONGSTR, tiat, 0}, {
	dst0, yat, LONGSTR, tiatyat, 0}, {
	dst1, longstr, LONGSTR, longstr, 0},
	    /*      Restricted strcat stuff                                 */
	{
	dst2, longstr, 0, "", 0}, {
	dst2, longstr, 1, "t", 0}, {
	dst2, longstr, LONGSTR - 1, longstr, 0}, {
	NULL, NULL, 0, NULL, 0}

};

/*	Strncpy	(strcmp( strncpy(s1, s2),  s1n ) == e_res)		*/
/*	ASSUMES strcmp is working -- it is tested prior to strncpy	*/
/*	No overlapping copies are tested				*/
struct t_strncpy {
	char *s1;
	char *s2;
	int n;
	char *s1n;
	int e_res;
} t_ncpy[] = {
	/*      Regular strcpy stuff -- i.e., n is large enough         */
	{
	dst0, "", LONGSTR, "", 0}, {
	dst0, tiat, LONGSTR, tiat, 0}, {
	dst0, longstr, LONGSTR, longstr, 0},
	    /*      Restricted strcpy stuff                                 */
	{
	dst1, tiat, 0, "", 0}, {
	dst1, longstr, 5, "ttttt", 0}, {
	NULL, NULL, 0, NULL, 0}
};

void setup();
int blenter();
int blexit();
int anyfail();

void setup(void)
{
	temp = stderr;
}

int blenter(void)
{
	local_flag = PASSED;
	return 0;
}

int blexit(void)
{
	(local_flag == PASSED) ? tst_resm(TPASS,
					  "Test passed") : tst_resm(TFAIL,
								    "Test failed");
	return 0;
}

int anyfail(void)
{
	tst_exit();
}

int main(int argc, char *argv[])
{
	register int n, i;
	char *s, *pr;

	tst_parse_opts(argc, argv, NULL, NULL);

	/*
	 * Init longstr
	 */

	s = longstr;
	n = LONGSTR;
	while (n--)
		*s++ = 't';
	*s = '\0';

	setup();
/*--------------------------------------------------------------*/

	/*
	 * Index
	 */
	//fprintf(temp, "\tStrchr\n" );
	i = 0;
	while (t_index[i].s) {
		if ((pr =
		     strchr(t_index[i].s, t_index[i].c)) != t_index[i].e_res) {
			fprintf(temp, "(Strchr) test %d", i);
			local_flag = FAILED;
		}
		i++;
	}
	/*
	 * Strrchr
	 */
	//fprintf(temp, "\tStrrchr\n" );
	i = 0;
	while (t_rindex[i].s) {
		if ((pr = strrchr(t_rindex[i].s, t_rindex[i].c))
		    != t_rindex[i].e_res) {
			fprintf(temp, "(Strrchr) test %d", i);
			local_flag = FAILED;
		}
		i++;
	}
	/*
	 * Strlen
	 */
	//fprintf(temp, "\tStrlen\n" );
	i = 0;
	while (t_len[i].s) {
		if ((n = strlen(t_len[i].s)) != t_len[i].e_res) {
			fprintf(temp, "(Strlen) test %d: expected %d, got %d",
				i, t_len[i].e_res, n);
			local_flag = FAILED;
		}
		i++;
	}

	/*
	 * Strcmp
	 */
	//fprintf(temp, "\tStrcmp\n" );
	i = 0;
#define sign(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
	while (t_cmp[i].s1) {
		n = strcmp(t_cmp[i].s1, t_cmp[i].s2);
		if (sign(n) != sign(t_cmp[i].e_res)) {
			fprintf(temp, "(Strcmp) test %d: expected %d, got %d",
				i, sign(t_cmp[i].e_res), sign(n));
			local_flag = FAILED;
		}
		i++;
	}

	/*
	 * Strcat
	 */
	//fprintf(temp, "\tStrcat\n" );
	memset(dst0, 0, LONGSTR + 1);	/* clean slate */
	memset(dst1, 0, LONGSTR + 1);	/* clean slate */
	i = 0;
	while (t_cat[i].s1) {
		if ((n =
		     strcmp(strcat(t_cat[i].s1, t_cat[i].s2), t_cat[i].s1s2))
		    != t_cat[i].e_res) {
			fprintf(temp, "(Strcat) test %d: expected %d, got %d",
				i, t_cat[i].e_res, n);
			local_flag = FAILED;
		}
		i++;
	}

	/*
	 * Strcpy
	 */
	//fprintf(temp, "\tStrcpy\n" );
	i = 0;
	while (t_cpy[i].s1) {
		if ((n = strcmp(strcpy(t_cpy[i].s1, t_cpy[i].s2), t_cpy[i].s2))
		    != t_cpy[i].e_res) {
			fprintf(temp, "(Strcpy) test %d: expected %d, got %d",
				i, t_cpy[i].e_res, n);
			local_flag = FAILED;
		}
		i++;
	}

	/*
	 * Strncat
	 */
	//fprintf(temp, "\tStrncat\n" );
	memset(dst0, 0, LONGSTR + 1);	/* clean slate */
	memset(dst1, 0, LONGSTR + 1);	/* clean slate */
	memset(dst2, 0, LONGSTR + 1);	/* clean slate */
	i = 0;
	while (t_ncat[i].s1) {
		if ((n =
		     strcmp(strncat(t_ncat[i].s1, t_ncat[i].s2, t_ncat[i].n),
			    t_ncat[i].s1ns2)) != t_ncat[i].e_res) {
			fprintf(temp, "(Strncat) test %d: expected %d, got %d",
				i, t_ncat[i].e_res, n);
			local_flag = FAILED;
		}
		i++;
	}

	/*
	 * Strncmp
	 */
	//fprintf(temp, "\tStrncmp\n" );
	i = 0;
	while (t_ncmp[i].s1) {
		if ((n = strncmp(t_ncmp[i].s1, t_ncmp[i].s2, t_ncmp[i].n))
		    != t_ncmp[i].e_res) {
			if ((t_ncmp[i].a_res < 0 && n > t_ncmp[i].a_res)
			    || (t_ncmp[i].a_res > 0 && n < t_ncmp[i].a_res)) {
				fprintf(temp,
					"(Strncmp) test %d: expected %d, got %d",
					i, t_ncmp[i].e_res, n);
				local_flag = FAILED;
			}
		}
		i++;
	}

	/*
	 * Strncpy
	 */
	//fprintf(temp, "\tStrncpy\n" );
	i = 0;
	memset(dst0, 0, LONGSTR + 1);	/* clean slate */
	memset(dst1, 0, LONGSTR + 1);	/* clean slate */
	while (t_ncpy[i].s1) {
		if ((n =
		     strcmp(strncpy(t_ncpy[i].s1, t_ncpy[i].s2, t_ncpy[i].n),
			    t_ncpy[i].s1n)) != t_ncpy[i].e_res) {
			fprintf(temp, "(Strncpy) test %d: expected %d, got %d",
				i, t_ncpy[i].e_res, n);
			local_flag = FAILED;
		}
		i++;
	}

	blexit();
	anyfail();
	tst_exit();
}
