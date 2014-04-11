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
/************

64 bits in a Cray word

				12345678901234567890123456789012
1234567890123456789012345678901234567890123456789012345678901234
________________________________________________________________
<    pid       >< word-offset in file (same #) ><    pid       >

1234567890123456789012345678901234567890123456789012345678901234
________________________________________________________________
<    pid       >< offset in file of this word  ><    pid       >

8 bits to a bytes == character
 NBPW            8
************/

#include <stdio.h>
#include <sys/param.h>
#ifdef UNIT_TEST
#include <unistd.h>
#include <stdlib.h>
#endif

static char Errmsg[80];

#define LOWER16BITS(X)	(X & 0177777)
#define LOWER32BITS(X)	(X & 0xffffffff)

/***
#define HIGHBITS(WRD, bits) ( (-1 << (64-bits)) & WRD)
#define LOWBITS(WRD, bits) ( (-1 >> (64-bits)) & WRD)
****/

#define NBPBYTE		8	/* number bits per byte */

#ifndef DEBUG
#define DEBUG	0
#endif

/***********************************************************************
 *
 *
 * 1   2   3   4   5   6   7   8   9   10  11  12  13  14  14  15	bytes
 * 1234567890123456789012345678901234567890123456789012345678901234	bits
 * ________________________________________________________________	1 word
 * <    pid       >< offset in file of this word  ><    pid       >
 *
 * the words are put together where offset zero is the start.
 * thus, offset 16 is the start of  the second full word
 * Thus, offset 8 is in middle of word 1
 ***********************************************************************/
int datapidgen(int pid, char *buffer, int bsize, int offset)
{
#if CRAY

	int cnt;
	int tmp;
	char *chr;
	long *wptr;
	long word;
	int woff;		/* file offset for the word */
	int boff;		/* buffer offset or index */
	int num_full_words;

	num_full_words = bsize / NBPW;
	boff = 0;

	if (cnt = (offset % NBPW)) {	/* partial word */

		woff = offset - cnt;
#if DEBUG
		printf("partial at beginning, cnt = %d, woff = %d\n", cnt,
		       woff);
#endif

		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;

		for (tmp = 0; tmp < cnt; tmp++) {	/* skip unused bytes */
			chr++;
		}

		for (; boff < (NBPW - cnt) && boff < bsize; boff++, chr++) {
			buffer[boff] = *chr;
		}
	}

	/*
	 * full words
	 */

	num_full_words = (bsize - boff) / NBPW;

	woff = offset + boff;

	for (cnt = 0; cnt < num_full_words; woff += NBPW, cnt++) {

		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;
		for (tmp = 0; tmp < NBPW; tmp++, chr++) {
			buffer[boff++] = *chr;
		}
/****** Only if wptr is a word ellined
	wptr = (long *)&buffer[boff];
	*wptr = word;
	boff += NBPW;
*****/

	}

	/*
	 * partial word at end of buffer
	 */

	if (cnt = ((bsize - boff) % NBPW)) {
#if DEBUG
		printf("partial at end\n");
#endif
		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;

		for (tmp = 0; tmp < cnt && boff < bsize; tmp++, chr++) {
			buffer[boff++] = *chr;
		}
	}

	return bsize;

#else
	return -1;		/* not support on non-64 bits word machines  */

#endif

}

/***********************************************************************
 *
 *
 ***********************************************************************/
int datapidchk(int pid, char *buffer, int bsize, int offset, char **errmsg)
{
#if CRAY

	int cnt;
	int tmp;
	char *chr;
	long *wptr;
	long word;
	int woff;		/* file offset for the word */
	int boff;		/* buffer offset or index */
	int num_full_words;

	if (errmsg != NULL) {
		*errmsg = Errmsg;
	}

	num_full_words = bsize / NBPW;
	boff = 0;

	if (cnt = (offset % NBPW)) {	/* partial word */
		woff = offset - cnt;
		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;

		for (tmp = 0; tmp < cnt; tmp++) {	/* skip unused bytes */
			chr++;
		}

		for (; boff < (NBPW - cnt) && boff < bsize; boff++, chr++) {
			if (buffer[boff] != *chr) {
				sprintf(Errmsg,
					"Data mismatch at offset %d, exp:%#o, act:%#o",
					offset + boff, *chr, buffer[boff]);
				return offset + boff;
			}
		}
	}

	/*
	 * full words
	 */

	num_full_words = (bsize - boff) / NBPW;

	woff = offset + boff;

	for (cnt = 0; cnt < num_full_words; woff += NBPW, cnt++) {
		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;
		for (tmp = 0; tmp < NBPW; tmp++, boff++, chr++) {
			if (buffer[boff] != *chr) {
				sprintf(Errmsg,
					"Data mismatch at offset %d, exp:%#o, act:%#o",
					woff, *chr, buffer[boff]);
				return woff;
			}
		}

/****** only if a word elined
	wptr = (long *)&buffer[boff];
	if (*wptr != word) {
	    sprintf(Errmsg, "Data mismatch at offset %d, exp:%#o, act:%#o",
	        woff, word, *wptr);
	    return woff;
	}
	boff += NBPW;
******/
	}

	/*
	 * partial word at end of buffer
	 */

	if (cnt = ((bsize - boff) % NBPW)) {
#if DEBUG
		printf("partial at end\n");
#endif
		word =
		    ((LOWER16BITS(pid) << 48) | (LOWER32BITS(woff) << 16) |
		     LOWER16BITS(pid));

		chr = (char *)&word;

		for (tmp = 0; tmp < cnt && boff < bsize; boff++, tmp++, chr++) {
			if (buffer[boff] != *chr) {
				sprintf(Errmsg,
					"Data mismatch at offset %d, exp:%#o, act:%#o",
					offset + boff, *chr, buffer[boff]);
				return offset + boff;
			}
		}
	}

	sprintf(Errmsg, "all %d bytes match desired pattern", bsize);
	return -1;		/* buffer is ok */

#else

	if (errmsg != NULL) {
		*errmsg = Errmsg;
	}
	sprintf(Errmsg, "Not supported on this OS.");
	return 0;

#endif

}				/* end of datapidchk */

#if UNIT_TEST

/***********************************************************************
 * main for doing unit testing
 ***********************************************************************/
int main(ac, ag)
int ac;
char **ag;
{

	int size = 1234;
	char *buffer;
	int ret;
	char *errmsg;

	if ((buffer = (char *)malloc(size)) == NULL) {
		perror("malloc");
		exit(2);
	}

	datapidgen(-1, buffer, size, 3);

/***
fwrite(buffer, size, 1, stdout);
fwrite("\n", 1, 1, stdout);
****/

	printf("datapidgen(-1, buffer, size, 3)\n");

	ret = datapidchk(-1, buffer, size, 3, &errmsg);
	printf("datapidchk(-1, buffer, %d, 3, &errmsg) returned %d %s\n",
	       size, ret, errmsg);
	ret = datapidchk(-1, &buffer[1], size - 1, 4, &errmsg);
	printf("datapidchk(-1, &buffer[1], %d, 4, &errmsg) returned %d %s\n",
	       size - 1, ret, errmsg);

	buffer[25] = 0x0;
	buffer[26] = 0x0;
	buffer[27] = 0x0;
	buffer[28] = 0x0;
	printf("changing char 25-28\n");

	ret = datapidchk(-1, &buffer[1], size - 1, 4, &errmsg);
	printf("datapidchk(-1, &buffer[1], %d, 4, &errmsg) returned %d %s\n",
	       size - 1, ret, errmsg);

	printf("------------------------------------------\n");

	datapidgen(getpid(), buffer, size, 5);

/*******
fwrite(buffer, size, 1, stdout);
fwrite("\n", 1, 1, stdout);
******/

	printf("\ndatapidgen(getpid(), buffer, size, 5)\n");

	ret = datapidchk(getpid(), buffer, size, 5, &errmsg);
	printf("datapidchk(getpid(), buffer, %d, 5, &errmsg) returned %d %s\n",
	       size, ret, errmsg);

	ret = datapidchk(getpid(), &buffer[1], size - 1, 6, &errmsg);
	printf
	    ("datapidchk(getpid(), &buffer[1], %d, 6, &errmsg) returned %d %s\n",
	     size - 1, ret, errmsg);

	buffer[25] = 0x0;
	printf("changing char 25\n");

	ret = datapidchk(getpid(), &buffer[1], size - 1, 6, &errmsg);
	printf
	    ("datapidchk(getpid(), &buffer[1], %d, 6, &errmsg) returned %d %s\n",
	     size - 1, ret, errmsg);

	exit(0);
}

#endif
