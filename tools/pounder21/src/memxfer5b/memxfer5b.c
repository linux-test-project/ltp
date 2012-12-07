/* Memory streaming benchmark */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#define __int64 long long
#include <sys/time.h>
#define SLASHC		'/'
#define SLASHSTR	"/"
#include <sys/types.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define equal !strcmp

size_t atoik(char *);
void *Malloc(size_t sz);
void tstart(void);
void tend(void);
double tval(void);

char *methods[] = {
	"\"memcpy\"",
	"\"char *\"",
	"\"short *\"",
	"\"int *\"",
	"\"long *\"",
	"\"__int64 *\"",
	"\"double *\"",
};

int nmethods = sizeof(methods) / sizeof(methods[0]);

int fflag = 0;			// if 0, then just Malloc once; else malloc/free each time
int wflag = 0;			// if 1, call SetProcessWorkingSetSize() (WINDOWS ONLY)
int sflag = 0;			// if 1, only print averages.
int pflag = 0;
int csvflag = 0;		// Print Comma separated list for spreadsheet input.
char *progname;

double tottim = 0.0;

int main(int ac, char *av[])
{
	size_t size;
	int i;
	unsigned ui;
	size_t j;
	unsigned cnt;
	int method = 0;
	char *p1, *p2;
	char *p, *q;
	short *sp, *sq;
	int *ip, *iq;
	long *lp, *lq;
	__int64 *llp, *llq;
	double *dp, *dq;
	double t;

	progname = av[0];
	if (strrchr(progname, SLASHC))
		progname = strrchr(progname, SLASHC) + 1;

	while (ac > 1) {
		if (equal(av[1], "-f")) {
			ac--;
			fflag = 1;
			av++;
		} else if (equal(av[1], "-w")) {
			ac--;
			wflag = 1;
			av++;
		} else if (equal(av[1], "-s")) {
			ac--;
			sflag = 1;
			av++;
		} else if (equal(av[1], "-p")) {
			ac--;
			pflag = 1;
			av++;
		} else if (equal(av[1], "-csv")) {
			ac--;
			csvflag++;
			av++;
		} else
			break;
	}
	if (ac < 3) {
		(void)
		    printf("Usage: %s [-f] [-w] [-s] [-p] size cnt [method]\n",
			   progname);
		(void)
		    printf
		    ("\t-f flag says to malloc and free of the \"cnt\" times.\n");
		(void)
		    printf
		    ("\t-w = set process min and max working set size to \"size\"\n");
		(void)printf("\t-s = silent; only print averages\n");
		(void)
		    printf
		    ("\t-p = prep; \"freshen\" cache before; -w disables\n");
		(void)printf("\t-csv = print output in CSV format\n");

		(void)printf("\tmethods:\n");
		for (i = 0; i < nmethods; i++)
			printf("\t%2d:\t%s\n", i, methods[i]);
		return 0;
	}

	size = atoik(av[1]);

	//
	// Round size up to 4*sizeof(double) bytes.
	//
	if (size != ((size / (4 * sizeof(double))) * (4 * sizeof(double)))) {
		size += (4 * sizeof(double));
		size /= (4 * sizeof(double));
		size *= (4 * sizeof(double));
	}
	cnt = (unsigned)atoik(av[2]);

	if (fflag == 0) {
		p1 = (char *)Malloc(size);
		p2 = (char *)Malloc(size);
		if (pflag)
			memcpy(p1, p2, size);
	}

	printf("%s ", progname);
	if (fflag)
		printf("-f ");
	if (wflag)
		printf("-w ");
	if (sflag)
		printf("-s ");
	if (pflag)
		printf("-p ");
	if (csvflag)
		printf("-csv ");
	printf("%u %u ", size, cnt);
	if (csvflag) {
		printf("Linux");
	}
	printf("\n");

	if (ac == 3) {
		ac = 4;
		av[3] = "0";
	}

	for (; ac > 3; ac--, av++) {
		if (isdigit(*av[3]))
			method = *av[3] - '0';
		if (method < 0 || method >= nmethods)
			method = 0;
		if (sflag)
			tstart();
		for (ui = 0; ui < cnt; ui++) {
			if (!sflag) {
				(void)printf("%s %d %d %-18.18s\t",
					     progname, size, cnt,
					     methods[method]);
				tstart();
			}
			if (fflag == 1) {
				p1 = (char *)Malloc(size);
				p2 = (char *)Malloc(size);
			}
			switch (method) {
			case 0:
				(void)memcpy(p1, p2, size);
				break;
			case 1:
				p = p1;
				q = p2;
				for (j = 0; j < size; j++)
					*p++ = *q++;
				break;
			case 2:
				sp = (short *)p1;
				sq = (short *)p2;
				for (j = 0; j < size; j += sizeof(short))
					*sp++ = *sq++;
				break;
			case 3:
				ip = (int *)p1;
				iq = (int *)p2;
				for (j = 0; j < size; j += sizeof(int))
					*ip++ = *iq++;
				break;
			case 4:
				lp = (long *)p1;
				lq = (long *)p2;
				for (j = 0; j < size; j += sizeof(long))
					*lp++ = *lq++;
				break;
			case 5:
				llp = (__int64 *) p1;
				llq = (__int64 *) p2;
				for (j = 0; j < size; j += sizeof(__int64))
					*llp++ = *llq++;
				break;
			case 6:
				dp = (double *)p1;
				dq = (double *)p2;
				for (j = 0; j < size; j += 4 * sizeof(double)) {
					*dp++ = *dq++;
					*dp++ = *dq++;
					*dp++ = *dq++;
					*dp++ = *dq++;
				}
				break;

			}
			if (fflag == 1) {
				free(p1);
				free(p2);
			}
			if (!sflag) {
				tend();
				t = tval();
				tottim += t;
				if (t == 0.0)
					t = .0001;
				printf(" %8.6f seconds %8.3f MB/s\n",
				       t, (double)size / t / 1000000.);
			}
		}
		if (sflag) {
			tend();
			tottim = tval();
		}
		if (csvflag) {
			printf("%s,%u,%u,%8.3f,%8.3f\n",
			       methods[method], size, size * cnt, tottim,
			       (double)size / (tottim / cnt) / 1000000.);
		} else {
			(void)printf("\tAVG: %d %-18.18s\t", size,
				     methods[method]);
			(void)printf(" %8.3f MB/s\n",
				     (double)size / (tottim / cnt) / 1000000.);
		}
		tottim = 0.0;
	}
	return 0;
}

size_t atoik(char *s)
{
	size_t ret = 0;
	size_t base;

	if (*s == '0') {
		base = 8;
		if (*++s == 'x' || *s == 'X') {
			base = 16;
			s++;
		}
	} else
		base = 10;

	for (; isxdigit(*s); s++) {
		if (base == 16)
			if (isalpha(*s))
				ret = base * ret + (toupper(*s) - 'A');
			else
				ret = base * ret + (*s - '0');
		else if (isdigit(*s))
			ret = base * ret + (*s - '0');
		else
			break;
	}
	for (; isalpha(*s); s++) {
		switch (toupper(*s)) {
		case 'K':
			ret *= 1024;
			break;
		case 'M':
			ret *= 1024 * 1024;
			break;
		default:
			return ret;
		}
	}
	return ret;
}

void *Malloc(size_t sz)
{
	char *p;

	p = (char *)malloc(sz);
	if (p == NULL) {
		(void)printf("malloc(%d) failed\n", sz);
		exit(1);
	}
	return (void *)p;
}

static struct timeval _tstart, _tend;

void tstart(void)
{
	gettimeofday(&_tstart, NULL);
}

void tend(void)
{
	gettimeofday(&_tend, NULL);
}

double tval()
{
	double t1, t2;

	t1 = (double)_tstart.tv_sec + (double)_tstart.tv_usec / (1000 * 1000);
	t2 = (double)_tend.tv_sec + (double)_tend.tv_usec / (1000 * 1000);
	return t2 - t1;
}
