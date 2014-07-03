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
#include <sys/param.h>
#include <string.h>		/* memset */
#include <stdlib.h>		/* rand */
#include "databin.h"

#if UNIT_TEST
#include <stdlib.h>
#endif

static char Errmsg[80];

void databingen(int mode, char *buffer, int bsize, int offset)
{
	int ind;

	switch (mode) {
	default:
	case 'a':		/* alternating bit pattern */
		memset(buffer, 0x55, bsize);
		break;

	case 'c':		/* checkerboard pattern */
		memset(buffer, 0xf0, bsize);
		break;

	case 'C':		/* */
		for (ind = 0; ind < bsize; ind++)
			buffer[ind] = ((offset + ind) % 8 & 0177);

		break;

	case 'o':
		memset(buffer, 0xff, bsize);
		break;

	case 'z':
		memset(buffer, 0x0, bsize);
		break;

	case 'r':		/* random */
		for (ind = 0; ind < bsize; ind++)
			buffer[ind] = (rand() & 0177) | 0100;
	}
}

/*
 * return values:
 *      >= 0 : error at byte offset into the file, offset+buffer[0-(bsize-1)]
 *      < 0  : no error
 */
int databinchk(int mode, char *buffer, int bsize, int offset, char **errmsg)
{
	int cnt;
	unsigned char *chr;
	long expbits;
	long actbits;

	chr = (unsigned char *)buffer;

	if (errmsg != NULL)
		*errmsg = Errmsg;

	switch (mode) {
	default:
	case 'a':		/* alternating bit pattern */
		expbits = 0x55;
		break;

	case 'c':		/* checkerboard pattern */
		expbits = 0xf0;
		break;

	case 'C':		/* counting pattern */
		for (cnt = 0; cnt < bsize; cnt++) {
			expbits = ((offset + cnt) % 8 & 0177);

			if (buffer[cnt] != expbits) {
				sprintf(Errmsg,
					"data mismatch at offset %d, exp:%#lo, act:%#o",
					offset + cnt, expbits, buffer[cnt]);
				return offset + cnt;
			}
		}
		sprintf(Errmsg, "all %d bytes match desired pattern", bsize);
		return -1;

	case 'o':
		expbits = 0xff;
		break;

	case 'z':
		expbits = 0;
		break;

	case 'r':
		return -1;	/* no check can be done for random */
	}

	for (cnt = 0; cnt < bsize; chr++, cnt++) {
		actbits = (long)*chr;

		if (actbits != expbits) {
			sprintf(Errmsg,
				"data mismatch at offset %d, exp:%#lo, act:%#lo",
				offset + cnt, expbits, actbits);
			return offset + cnt;
		}
	}

	sprintf(Errmsg, "all %d bytes match desired pattern", bsize);
	return -1;
}

#if UNIT_TEST

int main(int ac, char **ag)
{
	int size = 1023;
	int offset;
	int number;
	unsigned char *buffer;
	int ret;
	char *errmsg;

	buffer = malloc(size);
	if (buffer == NULL) {
		perror("malloc");
		exit(2);
	}

	printf("***** for a ****************************\n");
	databingen('a', buffer, size, 0);
	printf("databingen('a', buffer, %d, 0)\n", size);

	ret = databinchk('a', buffer, size, 0, &errmsg);
	printf("databinchk('a', buffer, %d, 0, &errmsg) returned %d: %s\n",
	       size, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	offset = 232400;
	ret = databinchk('a', &buffer[1], size - 1, offset, &errmsg);
	printf("databinchk('a', &buffer[1], %d, %d, &errmsg) returned %d: %s\n",
	       size, offset, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	buffer[15] = 0x0;
	printf("changing char 15 (offset (%d+15) = %d) to 0x0\n", offset,
	       offset + 15);
	number = offset + 15;

	ret = databinchk('a', &buffer[1], size - 1, offset + 1, &errmsg);
	printf("databinchk('a', &buffer[1], %d, %d, &errmsg) returned %d: %s\n",
	       size - 1, offset + 1, ret, errmsg);
	if (ret == number)
		printf("\tPASS return value of %d as expected\n", number);
	else
		printf("\tFAIL return value %d, expected %d\n", ret, number);

	printf("***** for c ****************************\n");
	databingen('c', buffer, size, 0);
	printf("databingen('c', buffer, %d, 0)\n", size);

	ret = databinchk('c', buffer, size, 0, &errmsg);
	printf("databinchk('c', buffer, %d, 0, &errmsg) returned %d: %s\n",
	       size, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	offset = 232400;
	ret = databinchk('c', &buffer[1], size - 1, offset, &errmsg);
	printf("databinchk('c', &buffer[1], %d, %d, &errmsg) returned %d: %s\n",
	       size, offset, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	buffer[15] = 0x0;
	printf("changing char 15 (offset (%d+15) = %d) to 0x0\n", offset,
	       offset + 15);
	number = offset + 15;

	ret = databinchk('c', &buffer[1], size - 1, offset + 1, &errmsg);
	printf("databinchk('c', &buffer[1], %d, %d, &errmsg) returned %d: %s\n",
	       size - 1, offset + 1, ret, errmsg);
	if (ret == number)
		printf("\tPASS return value of %d as expected\n", number);
	else
		printf("\tFAIL return value %d, expected %d\n", ret, number);

	printf("***** for C ****************************\n");

	databingen('C', buffer, size, 0);
	printf("databingen('C', buffer, %d, 0)\n", size);

	ret = databinchk('C', buffer, size, 0, &errmsg);
	printf("databinchk('C', buffer, %d, 0, &errmsg) returned %d: %s\n",
	       size, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	offset = 18;
	ret = databinchk('C', &buffer[18], size - 18, 18, &errmsg);
	printf
	    ("databinchk('C', &buffer[18], %d, 18, &errmsg) returned %d: %s\n",
	     size - 18, ret, errmsg);
	if (ret == -1)
		printf("\tPASS return value of -1 as expected\n");
	else
		printf("\tFAIL return value %d, expected -1\n", ret);

	buffer[20] = 0x0;
	buffer[21] = 0x0;
	printf("changing char 20 and 21 to 0x0 (offset %d and %d)\n", 20, 21);

	ret = databinchk('C', &buffer[18], size - 18, 18, &errmsg);
	printf("databinchk('C', &buffer[18], %d, 18, &errmsg) returned %d: %s\n",
		size - 18, ret, errmsg);

	if (ret == 20 || ret == 21)
		printf("\tPASS return value of %d or %d as expected\n", 20, 21);
	else
		printf("\tFAIL return value %d, expected %d or %d\n", ret,
		       20, 21);

	exit(0);

}

#endif
