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

/* 01/02/2003	Port to LTP	avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	atof1 -- ascii to floating point test
 *
 * CALLS
 *	atof(3), sprintf(3), ( doprnt.s )
 *
 * ALGORITHM
 *	Do some checks of floating point to ascii and back, arbitrate
 *	with a 3rd algorithm written in C.
 *
 * RESTRICTIONS
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

/**	LTP Port	**/
#include "test.h"

#define FAILED 0
#define PASSED 1

/*****		*****/
#define ERR		0.0000001

static double pi;

/*char progname[]= "atof1()"; */
/**	LTP Port	**/
static const char *TCID = "atof01";		/* Test program identifier */

static int local_flag = PASSED;
static int block_number;
static FILE *temp;
static int TST_TOTAL = 1;

static void setup(void);
static void blenter(void);
static void blexit(void);
static int numin(char *, double *);
static int checkbuf(char *, int, int);

/*--------------------------------------------------------------*/
int main(void)
{
	register int i, j;
	double r1, r2, x;
	char buf[100];

	setup();		/* temp file is now open        */
	pi = 4.0 * atan(1.0);

/*--------------------------------------------------------------*/
	blenter();

	for (i = 0; i < 30; i++)
		for (j = 0; j < 30; j++) {
			sprintf(buf, "%*.*f", i, j, pi);
			if (checkbuf(buf, i, j)) {
				fprintf(temp, "output conversion incorrect.");
				fprintf(temp, "%*.*f = '%s'", i, j, pi, buf);
				local_flag = FAILED;
			}
			r1 = atof(buf);
			if (numin(buf, &r2)) {
				fprintf(temp, "\tnumin('%s') failed\n", buf);
				local_flag = FAILED;
			}
			x = fabs(r1 - r2);
			if (x > ERR) {
				fprintf(temp, "\tcompare fails, %f vs %f\n",
					r1, r2);
				fprintf(temp, "\terr value is %f\n", x);
				local_flag = FAILED;
			}
			if (local_flag == FAILED)
				break;
		}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	x = 1.0 - exp(-100.0);	/* 1.0 - very small number */
	sprintf(buf, "%f", x);
	r1 = atof(buf);
	if (r1 != 1.0) {
		fprintf(temp, "\tsprintf small # failed\n");
		fprintf(temp, "\t printed '%s', expected 1.0\n", buf);
		local_flag = FAILED;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	for (i = 1; i < 200; i++) {
		x = 100.0 / (double)i;
		sprintf(buf, "%f", x);
		r1 = atof(buf);
		if (numin(buf, &r2)) {
			fprintf(temp, "\tnumin('%s') failed\n", buf);
			local_flag = FAILED;
		}
		/*
		 * Order subtraction  to produce a positive number.
		 * Then subtrace "fudge" factor which should give us
		 * a negative number, as the result fo subtraction should
		 * always be smaller than the fudge factor.
		 */
		if (r1 > r2)
			x = r1 - r2 - 1e-10;
		else
			x = r2 - r1 - 1e-10;
		if (x > 0.0) {
			fprintf(temp, "\tx = %.15f = %e\n", x, x);
			fprintf(temp, "\titeration %d\n", i);
			fprintf(temp, "\tcompare fails, %f vs %f\n", r1, r2);
			fprintf(temp, "\tcompare fails, %.15f vs %.15f\n",
				r1, r2);
			fprintf(temp, "\tbuf = '%s'\n", buf);
			x = r1 - r2;
			if (x == 0.0)
				fprintf(temp, "\tx == 0.0\n");
			else
				fprintf(temp, "\tx != 0.0\n");
			fprintf(temp, "\tx = %.15f = %e\n", x, x);
			local_flag = FAILED;
		}
		if (local_flag == FAILED)
			break;
	}

	blexit();
/*--------------------------------------------------------------*/
	blenter();

	for (i = -1; i > -200; i--) {
		x = 100.0 / (double)i;
		sprintf(buf, "%f", x);
		r1 = atof(buf);
		if (numin(buf, &r2)) {
			fprintf(temp, "\tnumin('%s') failed\n", buf);
			local_flag = FAILED;
		}
		/*
		 * Same ordering of subtraction as above.
		 */
		if (r1 > r2)
			x = r1 - r2 - 1e-10;
		else
			x = r2 - r1 - 1e-10;
		if (x > 0.0) {
			fprintf(temp, "\tcompare fails, %f vs %f\n", r1, r2);
			fprintf(temp, "\tcompare fails, %.15f vs %.15f\n",
				r1, r2);
			x = r1 - r2;
			if (x == 0.0)
				fprintf(temp, "\tx == 0.0)\n");
			else
				fprintf(temp, "\tx != 0.0\n");
			fprintf(temp, "\tx = %.15f = %e\n", x, x);
			local_flag = FAILED;
		}
		if (local_flag == FAILED)
			break;
	}

	blexit();
/*--------------------------------------------------------------*/
	tst_exit();
}

/* FUNCTIONS GO HERE */

static int numin(char *str, double *rval)
{
	register int i, v3, e_flag;
	register char c;
	double val, v1, v2, k;
	int neg_flag = 0;

	val = v1 = v2 = 0.0;
	v3 = 0;
	k = 0.1;

	while (*str == ' ')	/* scan past white space */
		str++;

	if (*str == '-') {	/* negitive value test */
		neg_flag++;
		str++;
	}

	for (;;) {
		c = *str;
		if (!isdigit(c))
			break;
		v1 *= 10.0;
		v1 += (double)(c - '0');
		str++;
	}

	val = v1;

#ifdef DEBUG
	printf("First conversion, val = %f = %e\n", val, val);
#endif

	if (*str == '.') {
		str++;
		for (;;) {
			c = *str;
			if (!isdigit(c))
				break;
			v2 += k * (double)(c - '0');
			k /= 10.0;
			str++;
		}
		val += v2;
	}
#ifdef DEBUG
	printf("Second conversion, val = %f = %e\n", val, val);
#endif

	if (*str == 'e') {
		str++;
		switch (*str) {
		case '+':
			e_flag = 1;
			break;
		case '-':
			e_flag = -1;
			break;
		default:
			fprintf(temp, "\tbad char '%c' after 'e'\n", *str);
			printf("bad char '%c' after 'e'\n", *str);
			return (-1);
		}
		str++;
		if (!isdigit(*str)) {
			fprintf(temp, "\tbad exponent field\n");
			printf("bad exponent field\n");
			return (-1);
		}
		v3 = 10 * (int)(*str - '0');
		str++;
		if (!isdigit(*str)) {
			fprintf(temp, "\tbad exponent field\n");
			printf("bad exponent field\n");
			return (-1);
		}
		v3 += (int)(*str - '0');
		str++;
		for (i = 0; i < v3; i++) {
			if (e_flag > 0)
				val *= 10.0;
			else
				val *= 0.1;
		}
	}

	if (neg_flag)
		val *= -1.0;

#ifdef DEBUG
	printf("Third conversion, val = %f = %e\n", val, val);
	printf("v1 = %f, v2 = %f, v3 = %d\n", v1, v2, v3);
#endif

	switch (*str) {
	case '\0':
	case ' ':
	case '\t':
	case '\n':
		break;
	default:
		printf("unexpected char '%c'\n", *str);
		return (-1);
	}

	*rval = val;
	return (0);
}

static int checkbuf(char *str, int n1, int n2)
{
	register int bd;	/* before decimal point */
	register int ad;	/* after decimal point */
	register int tw;	/* total width */
	register int dp;	/* decimal point */
	char *buf;

	bd = ad = dp = 0;
	buf = str;

	while (*str && *str != '.') {
		bd++;
		str++;
	}
	if (*str == '.') {
		dp++;
		str++;
		if (*str) {
			while (*str) {
				ad++;
				str++;
			}
		}
	}

	tw = bd + dp + ad;
	if (!n1)
		n1++;
	if (tw < n1) {
		fprintf(temp, "\tWidth too small.\n");
		fprintf(temp, "\tn1 = %d, n2 = %d, buf= '%s'\n", n1, n2, buf);
		return (-1);
	}

	if (ad != n2) {
		fprintf(temp, "\tNumber after decimal wrong.\n");
		fprintf(temp, "\tn1 = %d, n2 = %d, buf= '%s'\n", n1, n2, buf);
		return (-1);
	}

	if (n2 && !dp) {
		fprintf(temp, "\tMissed decimal point.\n");
		fprintf(temp, "\tn1 = %d, n2 = %d, buf= '%s'\n", n1, n2, buf);
		return (-1);
	}

	return (0);
}

/**	LTP Port	**/
static void setup(void)
{
	temp = stderr;
}

static void blenter(void)
{
	local_flag = PASSED;
}

static void blexit(void)
{
	if (local_flag == PASSED)
		tst_resm(TPASS, "Test passed");
	else
		tst_resm(TFAIL, "Test failed");
}
