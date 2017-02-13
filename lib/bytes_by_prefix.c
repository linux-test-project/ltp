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

#include "bytes_by_prefix.h"

/****************************************************************************
 * bytes_by_prefix(s)
 *
 * Computes the number of bytes described by string s.  s is assumed to be
 * a base 10 positive (ie. >= 0) number followed by an optional single
 * character multiplier.  The following multipliers are supported:
 *
 *              char    mult
 *              -----------------
 *              b       BSIZE  or BBSIZE
 *              k       1024 bytes
 *              K       1024 * sizeof(long)
 *              m       2^20 (1048576)
 *              M       2^20 (1048576 * sizeof(long)
 *              g       2^30 (1073741824)
 *              G       2^30 (1073741824) * sizeof(long)
 *
 * for instance, "1k" and "1024" would both cause bytes_by_prefix to return 1024
 *
 * Returns -1 if mult is an invalid character, or if the integer portion of
 * s is not a positive integer.
 *
 ****************************************************************************/

#ifdef DEV_BSIZE
#define B_MULT	DEV_BSIZE	/* block size */
#else
#warning DEV_BSIZE is not defined, defaulting to 512
#define B_MULT	512
#endif

#define K_MULT	1024		/* Kilo or 2^10 */
#define M_MULT	1048576		/* Mega or 2^20 */
#define G_MULT	1073741824	/* Giga or 2^30 */
#define T_MULT	1099511627776	/* tera or 2^40 */

int bytes_by_prefix(char *s)
{
	char mult, junk;
	int nconv;
	float num;
	int result;

	nconv = sscanf(s, "%f%c%c", &num, &mult, &junk);
	if (nconv == 0 || nconv == 3)
		return -1;

	if (nconv == 1) {
		result = num;
		return result < 0 ? -1 : result;
	}

	switch (mult) {
	case 'b':
		result = (int)(num * (float)B_MULT);
		break;
	case 'k':
		result = (int)(num * (float)K_MULT);
		break;
	case 'K':
		result = (int)((num * (float)K_MULT) * sizeof(long));
		break;
	case 'm':
		result = (int)(num * (float)M_MULT);
		break;
	case 'M':
		result = (int)((num * (float)M_MULT) * sizeof(long));
		break;
	case 'g':
		result = (int)(num * (float)G_MULT);
		break;
	case 'G':
		result = (int)((num * (float)G_MULT) * sizeof(long));
		break;
	default:
		return -1;
	}

	if (result < 0)
		return -1;

	return result;
}

long lbytes_by_prefix(char *s)
{
	char mult, junk;
	int nconv;
	float num;
	long result;

	nconv = sscanf(s, "%f%c%c", &num, &mult, &junk);
	if (nconv == 0 || nconv == 3)
		return -1;

	if (nconv == 1) {
		result = (long)num;
		return result < 0 ? -1 : result;
	}

	switch (mult) {
	case 'b':
		result = (long)(num * (float)B_MULT);
		break;
	case 'k':
		result = (long)(num * (float)K_MULT);
		break;
	case 'K':
		result = (long)((num * (float)K_MULT) * sizeof(long));
		break;
	case 'm':
		result = (long)(num * (float)M_MULT);
		break;
	case 'M':
		result = (long)((num * (float)M_MULT) * sizeof(long));
		break;
	case 'g':
		result = (long)(num * (float)G_MULT);
		break;
	case 'G':
		result = (long)((num * (float)G_MULT) * sizeof(long));
		break;
	default:
		return -1;
	}

	if (result < 0)
		return -1;

	return result;
}

/*
 * Force 64 bits number when compiled as 32 IRIX binary.
 * This allows for a number bigger than 2G.
 */
long long llbytes_by_prefix(char *s)
{
	char mult, junk;
	int nconv;
	double num;
	long long result;

	nconv = sscanf(s, "%lf%c%c", &num, &mult, &junk);
	if (nconv == 0 || nconv == 3)
		return -1;
	if (nconv == 1) {
		result = (long long)num;
		return result < 0 ? -1 : result;
	}

	switch (mult) {
	case 'b':
		result = (long long)(num * (float)B_MULT);
		break;
	case 'k':
		result = (long long)(num * (float)K_MULT);
		break;
	case 'K':
		result = (long long)((num * (float)K_MULT) * sizeof(long long));
		break;
	case 'm':
		result = (long long)(num * (float)M_MULT);
		break;
	case 'M':
		result = (long long)((num * (float)M_MULT) * sizeof(long long));
		break;
	case 'g':
		result = (long long)(num * (float)G_MULT);
		break;
	case 'G':
		result = (long long)((num * (float)G_MULT) * sizeof(long long));
		break;
	default:
		return -1;
	}

	if (result < 0)
		return -1;

	return result;
}
