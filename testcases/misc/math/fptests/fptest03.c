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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Group Bull & IBM Corporation */
/* 11/20/2002            Port to LTP             robbiew@us.ibm.com */
/*                                               jacky.malcles@bull.net */
/* IBM Corporation */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * fptest03.c  -- Floating point test.
 *
 * This causes the floating point unit to get some exercise.  Also
 * uses doprint.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>

#if _BYTE_ORDER == BIG_ENDIAN
union flt {
	struct ff { float vv; } ff;
	struct fb { unsigned ss:1, ee:8, ff:23; } fb;
};
#endif 
#if _BYTE_ORDER == LITLE_ENDIAN
union flt {
	struct ff { float vv; } ff;
	struct fb { unsigned ff:23, ee:8, ss:1; } fb;
};
#endif
#define	s	fb.ss
#define	e	fb.ee
#define	f	fb.ff
#define	v	ff.vv


#if _BYTE_ORDER == BIG_ENDIAN
union dbl {
	struct dd { double d; } dd;
	struct db { unsigned ss:1, ee:11, ffh:20, ffl:32; } db;
};
#endif 
#if _BYTE_ORDER == LITLE_ENDIAN
union dbl {
	struct dd { double d; } dd;
	struct db { unsigned ffl:32, ffh:20, ee:11, ss:1; } db;
};
#endif
#define	S	db.ss
#define	E	db.ee
#define	FL	db.ffl
#define	FH	db.ffh
#define	V	dd.d

#define N_ITER	2000		/* times to loop */

/** LTP Port **/
#include "test.h"
#include "usctest.h"

char *TCID="fptest03";          /* Test program identifier.    */
int TST_TOTAL=1;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
/**************/


double atof();
int tflt();
int dflt();
void fpe(int signo);

int ddif[32];
int seed;

int
main(argc,argv)
int argc; char **argv;
{
	int j;

	setbuf(stdout, NULL);
        /*
         * Capture signal SIGFPE if any
        */
        signal(SIGFPE, &fpe);

	seed = getpid();

	srand48(seed);
	
	for(j=0; j < N_ITER; j++) {
		tflt(j);
		dflt(j);
	}

		tst_resm(TPASS,"PASS");
		tst_exit();

                 /**NOT REACHED**/
                 return(0);
}

int
tflt(n)
{
	char buf[100];
	volatile union flt in, out;

	in.s = lrand48();
	in.e = lrand48()>>(31-8);
	if( in.e > 254 )
		in.e = 254;
	else if( in.e < 1 )
		in.e = 1;
	in.f = lrand48();
	sprintf(buf, "%20.10e\n", in.v);
	buf[20] = 0;
	out.v = atof(buf);

	if( out.v != in.v ) {
		tst_resm(TINFO,"\n%4d: %s (%d %d %x)\n%4s  %20.10e (%d %d %x)\n",
			n, buf, in.s, (int)in.e-127, in.f,
			" -> ", out.v, out.s, (int)out.e-127, out.f);
		tst_resm(TFAIL,"FAILED in tflt(), seed was %d\n", seed);
		tst_exit();
	}
	return(0);
}

int
dflt(num)
{
	char bufi[100];
	union dbl in, out;
	int n;

	in.S = lrand48();
	in.E = lrand48()>>(31-11);
	if( in.E > 2046 )
		in.E = 2046;
	else if( in.E < 1 )
		in.E = 1;
	in.FH = lrand48();
	in.FL = lrand48();

	sprintf(bufi, "%25.15e", in.V);
	out.V = atof(bufi);

	n = out.FL - in.FL;
	if( n < 0 )
		n = -n;
	if( n > 0x1f )
		n = 0x20;
	ddif[n]++;
	if( (out.S!=in.S) || (out.E!=in.E) || ( out.FH!=in.FH) || (n > 0x1f) ){
		tst_resm(TINFO,"\n<<%d>>\n", n);
		tst_resm(TINFO,"%4d: %25.15e (%d %d %x %x)\n%6s%25.15e (%d %d %x %x)\n",
			num, in.V, in.S, (int)in.E-1023, in.FH, in.FL, 
			" ->   ",out.V,out.S,(int)out.E-1023,out.FH,out.FL);
		sprintf(bufi, "%25.15e\n", out.V);
		bufi[25] = 0;
		out.V = atof(bufi);
		tst_resm(TINFO,"%4s  %25.15e (%d %d %x %x)\n",
			" -> ", out.V, out.S, (int)out.E-1023, out.FH, out.FL);
		tst_resm(TFAIL,"FAILED in dflt(), seed was %d\n", seed);
		tst_exit();
	}
	return(0);
}

void fpe(int signo)
{
        if (signo == SIGFPE) {
		tst_resm(TFAIL,"fpu: trap type SIGFPE, signal number = %d",
		signo);
        } else {
		tst_resm(TFAIL,"fpu: this should not have been reached");
        }
	tst_exit();
}
