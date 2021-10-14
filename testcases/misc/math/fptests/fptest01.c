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

/* Group Bull & IBM Corporation */
/* 11/20/2002	Port to LTP	robbiew@us.ibm.com */
/*                                               jacky.malcles@bull.net */
/* IBM Corporation */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * fptest01.c -- Floating point test.
 *
 * It is taken from a benchmark called "barsim".
 *
 * If the computation arrives at the expected values this routine
 * prints a "passed" message and exits 0.  If an incorrect value is
 * computed a "failed" message is printed and the routine exits 1.
 */

#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAGIC1	1632.796126
#define DIFF1	0.001
#define MAGIC2	0.777807
#define DIFF2	0.001
#define EVENTMX	256
#define BIG 1.e50
#define FALSE 0
#define TRUE  1
#define TRYCRIT   1
#define ENTERCRIT 2
#define LEAVECRIT 3
#define ATBARRIER 4
#define ENTERWORK 5
#define LEAVEWORK 6
#define NULLEVENT 999

/** LTP Port **/
#include "test.h"

static const char *TCID = "fptest01";	/* Test program identifier.    */
static int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

struct event {
	int proc;
	int type;
	double time;
};

static int init(void);
static int doevent(struct event *);
static int term(void);
static int addevent(int, int, double);

static void gaussinit(double, double);
static double gauss(void);

static struct event eventtab[EVENTMX];
static struct event rtrevent;
static int waiting[EVENTMX];		/* array of waiting processors */
static int nwaiting;			/* number of waiting processors */
static double global_time;		/* global clock */
static double lsttime;			/* time used for editing */
static double dtc, dts, alpha;		/* timing parameters */
static int nproc;			/* number of processors */
static int barcnt;			/* number of processors ATBARRIER */
static int ncycle;			/* number of cycles completed */
static int ncycmax;			/* number of cycles to run */
static int critfree;			/* TRUE if critical section not occupied */
static int gcount;			/* # calls to gauss */

static struct event *nextevent(void);

int main(void)
{
	struct event *ev;

	nproc = 128;
	ncycmax = 10;
	dtc = 0.01;
	dts = 0.0;
	alpha = 0.1;

	init();

	while ((ev = nextevent()) != NULL) {
		doevent(ev);
	}

	term();
	tst_resm(TPASS, "PASS");
	tst_exit();
}

/*
	initialize all processes to "entering work section"
*/
static int init(void)
{
	int p;
	double dtw, dtwsig;

	ncycle = 0;
	global_time = 0;
	lsttime = 0;
	barcnt = 0;
	nwaiting = 0;
	critfree = TRUE;

	dtw = 1. / nproc;	/* mean process work time */
	dtwsig = dtw * alpha;	/* std deviation of work time */
	gaussinit(dtw, dtwsig);

	for (p = 1; p <= nproc; p++) {
		eventtab[p].type = NULLEVENT;
	}

	for (p = 1; p <= nproc; p++) {
		addevent(ENTERWORK, p, global_time);
	}

	return (0);
}

/*
	print edit quantities
*/
static int term(void)
{
	double avgspd;
	double t_total = 0.0;
	double v;
	int i;

	for (i = 0; i < nproc; i++)
		t_total += eventtab[i].time;

	avgspd = ncycle / global_time;

	v = t_total - MAGIC1;
	if (v < 0.0)
		v *= -1.0;

	if (v > DIFF1) {
		tst_resm(TFAIL, "FAIL");
		v = t_total - MAGIC1;
		tst_resm(TINFO, "t_total = %.15f\n", t_total);
		tst_resm(TINFO, "expected  %.15f\n", MAGIC1);
		tst_resm(TINFO, "diff = %.15f\n", v);
		tst_exit();
	}

	v = avgspd - MAGIC2;
	if (v < 0.0)
		v *= -1.0;

	if (v > DIFF2) {
		tst_resm(TFAIL, "FAIL");
		v = avgspd - MAGIC2;
		tst_resm(TINFO, "avgspd  = %.15f\n", avgspd);
		tst_resm(TINFO, "expected  %.15f\n", MAGIC2);
		tst_resm(TINFO, "diff = %.15f\n", v);
		tst_exit();
	}
	return (0);
}

/*
	add an event to the event queue
*/
static int addevent(int type, int proc, double t)
{
	int i;
	int ok = FALSE;

	for (i = 1; i <= nproc; i++) {
		if (eventtab[i].type == NULLEVENT) {
			eventtab[i].type = type;
			eventtab[i].proc = proc;
			eventtab[i].time = t;
			ok = TRUE;
			break;
		}
	}
	if (ok)
		return (0);
	else
		tst_brkm(TBROK, NULL, "No room for event");
	return (0);
}

/*
	get earliest event in event queue
*/
static struct event *nextevent(void)
{
	double mintime = BIG;
	int imin = 0;
	int i;

	for (i = 1; i <= nproc; i++) {
		if (eventtab[i].type != NULLEVENT && eventtab[i].time < mintime) {
			imin = i;
			mintime = eventtab[i].time;
		}
	}

	if (imin) {
		rtrevent.type = eventtab[imin].type;
		rtrevent.proc = eventtab[imin].proc;
		rtrevent.time = eventtab[imin].time;
		eventtab[imin].type = NULLEVENT;
		return (&rtrevent);
	} else
		return (NULL);
}

/*
	add a processor to the waiting queue
*/
static int addwaiting(int p)
{
	waiting[++nwaiting] = p;
	return (0);
}

/*
	remove the next processor from the waiting queue
*/
static int getwaiting(void)
{
	if (nwaiting)
		return (waiting[nwaiting--]);
	else
		return (0);
}

static double dtcrit(void)
{
	return (dtc);
}

static double dtspinoff(void)
{
	return (dts);
}

static double dtwork(void)
{
	return (gauss());
}

/*
	take the action prescribed by 'ev', update the clock, and
	generate any subsequent events
*/
static int doevent(struct event *ev)
{
	double nxttime;
	int i, p, proc;

	global_time = ev->time;
	proc = ev->proc;

	switch (ev->type) {
	case TRYCRIT:
		if (critfree == TRUE)
			addevent(ENTERCRIT, proc, global_time);
		else
			addwaiting(proc);
		break;
	case ENTERCRIT:
		critfree = FALSE;
		nxttime = global_time + dtcrit();
		addevent(LEAVECRIT, proc, nxttime);
		break;
	case LEAVECRIT:
		critfree = TRUE;
		addevent(ATBARRIER, proc, global_time);
		if ((p = getwaiting()) != 0) {
			nxttime = global_time;
			addevent(ENTERCRIT, p, nxttime);
		}
		break;
	case ATBARRIER:
		barcnt++;
		if (barcnt == nproc) {
			nxttime = global_time;
			for (i = 1; i <= nproc; i++) {
				nxttime += dtspinoff();
				addevent(ENTERWORK, i, nxttime);
			}
			barcnt = 0;
			ncycle++;
		}
		break;
	case ENTERWORK:
		nxttime = global_time + dtwork();
		if (ncycle < ncycmax)
			addevent(LEAVEWORK, proc, nxttime);
		break;
	case LEAVEWORK:
		addevent(TRYCRIT, proc, global_time);
		break;
	default:
		tst_brkm(TBROK, NULL, "Illegal event");
		break;
	}
	return (0);
}

static int alternator = 1;
static double mean;
static double stdev;
static double u1, u2;
static double twopi;

static void gaussinit(double m, double s)
{
	mean = m;
	stdev = s;
	twopi = 2. * acos((double)-1.0);
	u1 = twopi / 400.0;
	u2 = twopi / 500.0;
}

static double gauss(void)
{
	double x1, x2;

	gcount++;

	u1 += u2;
	if (u1 > 0.99)
		u1 = twopi / 500.0;
	u2 += u1;
	if (u2 > 0.99)
		u2 = twopi / 400.0;

	if (alternator == 1) {
		alternator = -1;
		x1 = sqrt(-2.0 * log(u1)) * cos(twopi * u2);
		return (mean + stdev * x1);
	} else {
		alternator = 1;
		x2 = sqrt(-2.0 * log(u1)) * sin(twopi * u2);
		return (mean + stdev * x2);
	}
}
