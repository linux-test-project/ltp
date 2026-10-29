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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tso_random_range.h"

/*
 * Internal format of the range array set up by parse_range()
 */

struct range {
	int min;
	int max;
	int mult;
};

/*
 * parse_ranges() is a function to parse a comma-separated list of range
 * tokens each having the following form:
 *
 *		num
 *	or
 *		min:max[:mult]
 *
 * any of the values may be blank (ie. min::mult, :max, etc.) and default
 * values for missing arguments may be supplied by the caller.
 *
 * The special first form is short hand for 'num:num'.
 *
 * After parsing the string, the ranges are put into an array of integers,
 * which is malloc'd by the routine.  The min, max, and mult entries of each
 * range can be extracted from the array using the range_min(), range_max(),
 * and range_mult() functions.
 *
 * It is the responsibility of the caller to free the space allocated by
 * parse_ranges() - a single call to free() will free the space.
 *
 *	str		The string to parse - assumed to be a comma-separated
 *			list of tokens having the above format.
 *	defmin		default value to plug in for min, if it is missing
 *	defmax		default value to plug in for max, if it is missing
 *	defmult		default value to plug in for mult, if missing
 *	parse_func	A user-supplied function pointer, which parse_ranges()
 *			can call to parse the min, max, and mult strings.  This
 *			allows for customized number formats.  The function
 *			MUST have the following prototype:
 *				parse_func(char *str, int *val)
 *			The function should return -1 if str cannot be parsed
 *			into an integer, or >= 0 if it was successfully
 *			parsed.  The resulting integer will be stored in
 *			*val.  If parse_func is NULL, parse_ranges will parse
 *			the tokens in a manner consistent with the sscanf
 *			%i format.
 *	range_ptr	A user-supplied char **, which will be set to point
 *			at malloc'd space which holds the parsed range
 *			values.   If range_ptr is NULL, parse_ranges() just
 *			parses the string.  The data returned in range_ptr
 *			should not be processed directly - use the functions
 *			range_min(), range_max(), and range_mult() to access
 *			data for a given range.
 *	errptr		user-supplied char ** which can be set to point to a
 *			static error string.  If errptr is NULL, it is ignored.
 *
 * parse_range() returns -1 on error, or the number of ranges parsed.
 */

static int str_to_int();
static long long divider(long long, long long, long long, long long);

int parse_ranges(char *str, int defmin, int defmax, int defmult,
		int (*parse_func)(), char **rangeptr, char **errptr)
{
	int ncommas;
	char *tmpstr, *cp, *tok, *n1str, *n2str, *multstr;
	struct range *rp, *ranges;
	static char errmsg[256];

	if (errptr != NULL) {
		*errptr = errmsg;
	}

	for (ncommas = 0, cp = str; *cp != '\0'; cp++) {
		if (*cp == ',') {
			ncommas++;
		}
	}

	if (parse_func == NULL) {
		parse_func = str_to_int;
	}

	tmpstr = strdup(str);
	ranges = malloc((ncommas + 1) * sizeof(struct range));
	rp = ranges;

	tok = strtok(tmpstr, ",");
	while (tok != NULL) {
		n1str = tok;
		n2str = NULL;
		multstr = NULL;

		rp->min = defmin;
		rp->max = defmax;
		rp->mult = defmult;

		if ((cp = strchr(n1str, ':')) != NULL) {
			*cp = '\0';
			n2str = cp + 1;

			if ((cp = strchr(n2str, ':')) != NULL) {
				*cp = '\0';
				multstr = cp + 1;
			}
		}

		/*
		 * Parse the 'min' field - if it is zero length (:n2[:mult]
		 * format), retain the default value, otherwise, pass the
		 * string to the parse function.
		 */

		if ((int)strlen(n1str) > 0) {
			if ((*parse_func) (n1str, &rp->min) < 0) {
				sprintf(errmsg,
					"error parsing string %s into an integer",
					n1str);
				free(tmpstr);
				free(ranges);
				return -1;
			}
		}

		/*
		 * Process the 'max' field - if one was not present (n1 format)
		 * set max equal to min.  If the field was present, but
		 * zero length (n1: format), retain the default.  Otherwise
		 * pass the string to the parse function.
		 */

		if (n2str == NULL) {
			rp->max = rp->min;
		} else if ((int)strlen(n2str) > 0) {
			if ((*parse_func) (n2str, &rp->max) < 0) {
				sprintf(errmsg,
					"error parsing string %s into an integer",
					n2str);
				free(tmpstr);
				free(ranges);
				return -1;
			}
		}

		/*
		 * Process the 'mult' field - if one was not present
		 * (n1:n2 format), or the field was zero length (n1:n2: format)
		 * then set the mult field to defmult - otherwise pass then
		 * mult field to the parse function.
		 */

		if (multstr != NULL && (int)strlen(multstr) > 0) {
			if ((*parse_func) (multstr, &rp->mult) < 0) {
				sprintf(errmsg,
					"error parsing string %s into an integer",
					multstr);
				free(tmpstr);
				free(ranges);
				return -1;
			}
		}

		rp++;
		tok = strtok(NULL, ",");
	}

	free(tmpstr);

	if (rangeptr != NULL) {
		*rangeptr = (char *)ranges;
	} else {
		free(ranges);	/* just running in parse mode */
	}

	return (rp - ranges);
}

/*
 * The default integer-parsing function
 */

static int str_to_int(char *str, int *ip)
{
	char c;

	if (sscanf(str, "%i%c", ip, &c) != 1) {
		return -1;
	} else {
		return 0;
	}
}

/*
 * Three simple functions to return the min, max, and mult values for a given
 * range.  It is assumed that rbuf is a range buffer set up by parse_ranges(),
 * and that r is a valid range within that buffer.
 */

int range_min(char *rbuf, int r)
{
	return ((struct range *)rbuf)[r].min;
}

int range_max(char *rbuf, int r)
{
	return ((struct range *)rbuf)[r].max;
}

int range_mult(char *rbuf, int r)
{
	return ((struct range *)rbuf)[r].mult;
}

/*****************************************************************************
 * random_range(int start, int end, int mult, char **errp)
 *
 * Returns a psuedo-random number which is >= 'start', <= 'end', and a multiple
 * of 'mult'.  Start and end may be any valid integer, but mult must be an
 * integer > 0.  errp is a char ** which will be set to point to a static
 * error message buffer if it is not NULL, and an error occurs.
 *
 * The errp is the only way to check if the routine fails - currently the only
 * failure conditions are:
 *
 *		mult < 1
 *		no numbers in the start-end range that are a multiple of 'mult'
 *
 * If random_range_fails, and errp is a valid pointer, it will point to an
 * internal error buffer.  If errp is a vaild pointer, and random_range
 * is successful, errp will be set to NULL.
 *
 * Note - if mult is 1 (the most common case), there are error conditions
 * possible, and errp need not be used.
 *
 * Note:    Uses lrand48(), assuming that set_random_seed() uses srand48() when
 *          setting the seed.
 *****************************************************************************/

long random_range(int min, int max, int mult, char **errp)
{
	int r, nmults, orig_min, orig_max, orig_mult, tmp;
	extern long lrand48();
	static char errbuf[128];

	/*
	 * Sanity check
	 */

	if (mult < 1) {
		if (errp != NULL) {
			sprintf(errbuf, "mult arg must be greater than 0");
			*errp = errbuf;
		}
		return -1;
	}

	/*
	 * Save original parameter values for use in error message
	 */

	orig_min = min;
	orig_max = max;
	orig_mult = mult;

	/*
	 * switch min/max if max < min
	 */

	if (max < min) {
		tmp = max;
		max = min;
		min = tmp;
	}

	/*
	 * select the random number
	 */

	if ((r = min % mult))	/* bump to the next higher 'mult' multiple */
		min += mult - r;

	if ((r = max % mult))	/* reduce to the next lower 'mult' multiple */
		max -= r;

	if (min > max) {	/* no 'mult' multiples between min & max */
		if (errp != NULL) {
			sprintf(errbuf,
				"no numbers in the range %d:%d that are a multiple of %d",
				orig_min, orig_max, orig_mult);
			*errp = errbuf;
		}
		return -1;
	}

	if (errp != NULL) {
		*errp = NULL;
	}

	nmults = ((max - min) / mult) + 1;
#if CRAY
	/*
	 * If max is less than 2gb, then the value can fit in 32 bits
	 * and the standard lrand48() routine can be used.
	 */
	if (max <= (long)2147483647) {
		return (long)(min + (((long)lrand48() % nmults) * mult));
	} else {
		/*
		 * max is greater than 2gb - meeds more than 32 bits.
		 * Since lrand48 only will get a number up to 32bits.
		 */
		long randnum;
		randnum = divider(min, max, 0, -1);
		return (long)(min + ((randnum % nmults) * mult));
	}

#else
	return (min + ((lrand48() % nmults) * mult));
#endif

}

/*
 * Just like random_range, but all values are longs.
 */
long random_rangel(long min, long max, long mult, char **errp)
{
	long r, nmults, orig_min, orig_max, orig_mult, tmp;
	extern long lrand48();
	static char errbuf[128];

	/*
	 * Sanity check
	 */

	if (mult < 1) {
		if (errp != NULL) {
			sprintf(errbuf, "mult arg must be greater than 0");
			*errp = errbuf;
		}
		return -1;
	}

	/*
	 * Save original parameter values for use in error message
	 */

	orig_min = min;
	orig_max = max;
	orig_mult = mult;

	/*
	 * switch min/max if max < min
	 */

	if (max < min) {
		tmp = max;
		max = min;
		min = tmp;
	}

	/*
	 * select the random number
	 */

	if ((r = min % mult))	/* bump to the next higher 'mult' multiple */
		min += mult - r;

	if ((r = max % mult))	/* reduce to the next lower 'mult' multiple */
		max -= r;

	if (min > max) {	/* no 'mult' multiples between min & max */
		if (errp != NULL) {
			sprintf(errbuf,
				"no numbers in the range %ld:%ld that are a multiple of %ld",
				orig_min, orig_max, orig_mult);
			*errp = errbuf;
		}
		return -1;
	}

	if (errp != NULL) {
		*errp = NULL;
	}

	nmults = ((max - min) / mult) + 1;
#if CRAY || (_MIPS_SZLONG == 64)
	/*
	 * If max is less than 2gb, then the value can fit in 32 bits
	 * and the standard lrand48() routine can be used.
	 */
	if (max <= (long)2147483647) {
		return (long)(min + (((long)lrand48() % nmults) * mult));
	} else {
		/*
		 * max is greater than 2gb - meeds more than 32 bits.
		 * Since lrand48 only will get a number up to 32bits.
		 */
		long randnum;
		randnum = divider(min, max, 0, -1);
		return (long)(min + ((randnum % nmults) * mult));
	}

#else
	return (min + ((lrand48() % nmults) * mult));
#endif
}

/*
 *  Attempts to be just like random_range, but everything is long long (64 bit)
 */
long long random_rangell(long long min, long long max,
			long long mult, char **errp)
{
	long long r, nmults, orig_min, orig_max, orig_mult, tmp;
	long long randnum;
	extern long lrand48();
	static char errbuf[128];

	/*
	 * Sanity check
	 */

	if (mult < 1) {
		if (errp != NULL) {
			sprintf(errbuf, "mult arg must be greater than 0");
			*errp = errbuf;
		}
		return -1;
	}

	/*
	 * Save original parameter values for use in error message
	 */

	orig_min = min;
	orig_max = max;
	orig_mult = mult;

	/*
	 * switch min/max if max < min
	 */

	if (max < min) {
		tmp = max;
		max = min;
		min = tmp;
	}

	/*
	 * select the random number
	 */

	if ((r = min % mult))	/* bump to the next higher 'mult' multiple */
		min += mult - r;

	if ((r = max % mult))	/* reduce to the next lower 'mult' multiple */
		max -= r;

	if (min > max) {	/* no 'mult' multiples between min & max */
		if (errp != NULL) {
			sprintf(errbuf,
				"no numbers in the range %lld:%lld that are a multiple of %lld",
				orig_min, orig_max, orig_mult);
			*errp = errbuf;
		}
		return -1;
	}

	if (errp != NULL) {
		*errp = NULL;
	}

	nmults = ((max - min) / mult) + 1;
	/*
	 * If max is less than 2gb, then the value can fit in 32 bits
	 * and the standard lrand48() routine can be used.
	 */
	if (max <= (long)2147483647) {
		return (long long)(min +
				   (((long long)lrand48() % nmults) * mult));
	} else {
		/*
		 * max is greater than 2gb - meeds more than 32 bits.
		 * Since lrand48 only will get a number up to 32bits.
		 */
		randnum = divider(min, max, 0, -1);
		return (long long)(min + ((randnum % nmults) * mult));
	}

}

/*
 * This functional will recusively call itself to return a random
 * number min and max.   It was designed to work the 64bit numbers
 * even when compiled as 32 bit process.
 * algorithm:  to use the official lrand48() routine - limited to 32 bits.
 *   find the difference between min and max (max-min).
 *   if the difference is 2g or less, use the random number gotton from lrand48().
 *   Determine the midway point between min and max.
 *   if the midway point is less than 2g from min or max,
 *      randomly add the random number gotton from lrand48() to
 *      either min or the midpoint.
 *   Otherwise, call outself with min and max being min and midway value or
 *   midway value and max.  This will reduce the range in half.
 */
static long long
divider(long long min, long long max, long long cnt, long long rand)
{
	long long med, half, diff;

	/*
	 * prevent run away code.  We are dividing by two each count.
	 * if we get to a count of more than 32, we should have gotten
	 * to 2gb.
	 */
	if (cnt > 32)
		return -1;

	/*
	 * Only get a random number the first time.
	 */
	if (cnt == 0 || rand < -1) {
		rand = (long long)lrand48();	/* 32 bit random number */
	}

	diff = max - min;

	if (diff <= 2147483647)
		return min + rand;

	half = diff / (long long)2;	/* half the distance between min and max */
	med = min + half;	/* med way point between min and max */

#if DEBUG
	printf("divider: min=%lld, max=%lld, cnt=%lld, rand=%lld\n", min, max,
	       cnt, rand);
	printf("   diff = %lld, half = %lld,   med = %lld\n", diff, half, med);
#endif

	if (half <= 2147483647) {
		/*
		 * If half is smaller than 2gb, we can use the random number
		 * to pick the number within the min to med or med to max
		 * if the cnt bit of rand is zero or one, respectively.
		 */
		if (rand & (1 << cnt))
			return med + rand;
		else
			return min + rand;
	} else {
		/*
		 * recursively call ourself to reduce the value to the bottom half
		 * or top half (bit cnt is set).
		 */
		if (rand & (1 << cnt)) {
			return divider(med, max, cnt + 1, rand);
		} else {
			return divider(min, med, cnt + 1, rand);
		}

	}

}

/*****************************************************************************
 * random_range_seed(s)
 *
 * Sets the random seed to s.  Uses srand48(), assuming that lrand48() will
 * be used in random_range().
 *****************************************************************************/

void random_range_seed(long s)
{
	extern void srand48();

	srand48(s);
}

/****************************************************************************
 * random_bit(mask)
 *
 * This function randomly returns a single bit from the bits
 * set in mask.  If mask is zero, zero is returned.
 *
 ****************************************************************************/
long random_bit(long mask)
{
	int nbits = 0;		/* number of set bits in mask */
	long bit;		/* used to count bits and num of set bits choosen */
	int nshift;		/* used to count bit shifts */

	if (mask == 0)
		return 0;

	/*
	 * get the number of bits set in mask
	 */
#ifndef CRAY

	bit = 1L;
	for (nshift = 0; (unsigned int)nshift < sizeof(long) * 8; nshift++) {
		if (mask & bit)
			nbits++;
		bit = bit << 1;
	}

#else
	nbits = _popcnt(mask);
#endif /* if CRAY */

	/*
	 * randomly choose a bit.
	 */
	bit = random_range(1, nbits, 1, NULL);

	/*
	 * shift bits until you determine which bit was randomly choosen.
	 * nshift will hold the number of shifts to make.
	 */

	nshift = 0;
	while (bit) {
		/* check if the current one's bit is set */
		if (mask & 1L) {
			bit--;
		}
		mask = mask >> 1;
		nshift++;
	}

	return 01L << (nshift - 1);

}

#if RANDOM_BIT_UNITTEST
/*
 *  The following is a unit test main function for random_bit().
 */
main(argc, argv)
int argc;
char **argv;
{
	int ind;
	int cnt, iter;
	long mask, ret;

	printf("test for first and last bit set\n");
	mask = 1L;
	ret = random_bit(mask);
	printf("random_bit(%#o) returned %#o\n", mask, ret);

	mask = 1L << (sizeof(long) * 8 - 1);
	ret = random_bit(mask);
	printf("random_bit(%#o) returned %#o\n", mask, ret);

	if (argc >= 3) {
		iter = atoi(argv[1]);
		for (ind = 2; ind < argc; ind++) {
			printf("Calling random_bit %d times for mask %#o\n",
			       iter, mask);
			sscanf(argv[ind], "%i", &mask);
			for (cnt = 0; cnt < iter; cnt++) {
				ret = random_bit(mask);
				printf("random_bit(%#o) returned %#o\n", mask,
				       ret);
			}
		}
	}
	exit(0);
}

#endif /* end if RANDOM_BIT_UNITTEST */

#if UNIT_TEST
/*
 *  The following is a unit test main function for random_range*().
 */

#define PARTNUM	10		/* used to determine even distribution of random numbers */
#define MEG  1024*1024*1024
#define GIG 1073741824
int main(argc, argv)
int argc;
char **argv;
{
	int ind;
	int cnt, iter = 10;
	int imin = 0, imult = 1, itmin, itmax = 0;
#if CRAY
	int imax = 6 * GIG;	/* higher than 32 bits */
#else
	int imax = 1048576;
#endif

	long lret, lmin = 0, lmult = 1, ltmin, ltmax = 0;
#if CRAY || (_MIPS_SZLONG == 64)
	long lmax = 6 * (long)GIG;	/* higher than 32 bits */
#else
	long lmax = 1048576;
#endif
	long long llret, llmin = 0, llmult = 1, lltmin, lltmax = 0;
	long long llmax = (long long)80 * (long long)GIG;

	long part;
	long long lpart;
	long cntarr[PARTNUM];
	long valbound[PARTNUM];
	long long lvalbound[PARTNUM];

	for (ind = 0; ind < PARTNUM; ind++)
		cntarr[ind] = 0;

	if (argc < 2) {
		printf("Usage: %s func [iterations] \n", argv[0]);
		printf
		    ("func can be random_range, random_rangel, random_rangell\n");
		exit(1);
	}

	if (argc >= 3) {
		if (sscanf(argv[2], "%i", &iter) != 1) {
			printf("Usage: %s [func iterations] \n", argv[0]);
			printf("argv[2] is not a number\n");
			exit(1);
		}
	}

	/*
	 * random_rangel ()
	 */
	if (strcmp(argv[1], "random_rangel") == 0) {
		ltmin = lmax;
		part = lmax / PARTNUM;
		for (ind = 0; ind < PARTNUM; ind++) {
			valbound[ind] = part * ind;
		}

		for (cnt = 0; cnt < iter; cnt++) {
			lret = random_rangel(lmin, lmax, lmult, NULL);
			if (iter < 100)
				printf("%ld\n", lret);
			if (lret < ltmin)
				ltmin = lret;
			if (lret > ltmax)
				ltmax = lret;
			for (ind = 0; ind < PARTNUM - 1; ind++) {
				if (valbound[ind] < lret
				    && lret <= valbound[ind + 1]) {
					cntarr[ind]++;
					break;
				}
			}
			if (lret > valbound[PARTNUM - 1]) {
				cntarr[PARTNUM - 1]++;
			}
		}
		for (ind = 0; ind < PARTNUM - 1; ind++) {
			printf("%2d %-13ld to  %-13ld   %5ld %4.4f\n", ind + 1,
			       valbound[ind], valbound[ind + 1], cntarr[ind],
			       (float)(cntarr[ind] / (float)iter));
		}
		printf("%2d %-13ld to  %-13ld   %5ld %4.4f\n", PARTNUM,
		       valbound[PARTNUM - 1], lmax, cntarr[PARTNUM - 1],
		       (float)(cntarr[PARTNUM - 1] / (float)iter));
		printf("  min=%ld,  max=%ld\n", ltmin, ltmax);

	} else if (strcmp(argv[1], "random_rangell") == 0) {
		/*
		 * random_rangell() unit test
		 */
		lltmin = llmax;
		lpart = llmax / PARTNUM;
		for (ind = 0; ind < PARTNUM; ind++) {
			lvalbound[ind] = (long long)(lpart * ind);
		}

		for (cnt = 0; cnt < iter; cnt++) {
			llret = random_rangell(llmin, llmax, llmult, NULL);
			if (iter < 100)
				printf("random_rangell returned %lld\n", llret);
			if (llret < lltmin)
				lltmin = llret;
			if (llret > lltmax)
				lltmax = llret;

			for (ind = 0; ind < PARTNUM - 1; ind++) {
				if (lvalbound[ind] < llret
				    && llret <= lvalbound[ind + 1]) {
					cntarr[ind]++;
					break;
				}
			}
			if (llret > lvalbound[PARTNUM - 1]) {
				cntarr[PARTNUM - 1]++;
			}
		}
		for (ind = 0; ind < PARTNUM - 1; ind++) {
			printf("%2d %-13lld to  %-13lld   %5ld %4.4f\n",
			       ind + 1, lvalbound[ind], lvalbound[ind + 1],
			       cntarr[ind], (float)(cntarr[ind] / (float)iter));
		}
		printf("%2d %-13lld to  %-13lld   %5ld %4.4f\n", PARTNUM,
		       lvalbound[PARTNUM - 1], llmax, cntarr[PARTNUM - 1],
		       (float)(cntarr[PARTNUM - 1] / (float)iter));
		printf("  min=%lld,  max=%lld\n", lltmin, lltmax);

	} else {
		/*
		 * random_range() unit test
		 */
		itmin = imax;
		part = imax / PARTNUM;
		for (ind = 0; ind < PARTNUM; ind++) {
			valbound[ind] = part * ind;
		}

		for (cnt = 0; cnt < iter; cnt++) {
			lret = random_range(imin, imax, imult, NULL);
			if (iter < 100)
				printf("%ld\n", lret);
			if (lret < itmin)
				itmin = lret;
			if (lret > itmax)
				itmax = lret;

			for (ind = 0; ind < PARTNUM - 1; ind++) {
				if (valbound[ind] < lret
				    && lret <= valbound[ind + 1]) {
					cntarr[ind]++;
					break;
				}
			}
			if (lret > valbound[PARTNUM - 1]) {
				cntarr[PARTNUM - 1]++;
			}
		}
		for (ind = 0; ind < PARTNUM - 1; ind++) {
			printf("%2d %-13ld to  %-13ld   %5ld %4.4f\n", ind + 1,
			       valbound[ind], valbound[ind + 1], cntarr[ind],
			       (float)(cntarr[ind] / (float)iter));
		}
		printf("%2d %-13ld to  %-13ld   %5ld %4.4f\n", PARTNUM,
		       valbound[PARTNUM - 1], (long)imax, cntarr[PARTNUM - 1],
		       (float)(cntarr[PARTNUM - 1] / (float)iter));
		printf("  min=%d,  max=%d\n", itmin, itmax);

	}

	exit(0);
}

#endif
