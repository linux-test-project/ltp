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
#include <string.h>
#include "pattern.h"

/*
 * The routines in this module are used to fill/check a data buffer
 * with/against a known pattern.
 */

int pattern_check(char *buf, int buflen, char *pat, int patlen, int patshift)
{
	int nb, ncmp, nleft;
	char *cp;

	if (patlen)
		patshift = patshift % patlen;

	cp = buf;
	nleft = buflen;

	/*
	 * The following 2 blocks of code are to compare the first patlen
	 * bytes of buf.  We need 2 checks if patshift is > 0 since we
	 * must check the last (patlen - patshift) bytes, and then the
	 * first (patshift) bytes.
	 */

	nb = patlen - patshift;
	if (nleft < nb) {
		return (memcmp(cp, pat + patshift, nleft) ? -1 : 0);
	} else {
		if (memcmp(cp, pat + patshift, nb))
			return -1;

		nleft -= nb;
		cp += nb;
	}

	if (patshift > 0) {
		nb = patshift;
		if (nleft < nb) {
			return (memcmp(cp, pat, nleft) ? -1 : 0);
		} else {
			if (memcmp(cp, pat, nb))
				return -1;

			nleft -= nb;
			cp += nb;
		}
	}

	/*
	 * Now, verify the rest of the buffer using the algorithm described
	 * in the function header.
	 */

	ncmp = cp - buf;
	while (ncmp < buflen) {
		nb = (ncmp < nleft) ? ncmp : nleft;
		if (memcmp(buf, cp, nb))
			return -1;

		cp += nb;
		ncmp += nb;
		nleft -= nb;
	}

	return 0;
}

int pattern_fill(char *buf, int buflen, char *pat, int patlen, int patshift)
{
	int trans, ncopied, nleft;
	char *cp;

	if (patlen)
		patshift = patshift % patlen;

	cp = buf;
	nleft = buflen;

	/*
	 * The following 2 blocks of code are to fill the first patlen
	 * bytes of buf.  We need 2 sections if patshift is > 0 since we
	 * must first copy the last (patlen - patshift) bytes into buf[0]...,
	 * and then the first (patshift) bytes of pattern following them.
	 */

	trans = patlen - patshift;
	if (nleft < trans) {
		memcpy(cp, pat + patshift, nleft);
		return 0;
	} else {
		memcpy(cp, pat + patshift, trans);
		nleft -= trans;
		cp += trans;
	}

	if (patshift > 0) {
		trans = patshift;
		if (nleft < trans) {
			memcpy(cp, pat, nleft);
			return 0;
		} else {
			memcpy(cp, pat, trans);
			nleft -= trans;
			cp += trans;
		}
	}

	/*
	 * Now, fill the rest of the buffer using the algorithm described
	 * in the function header comment.
	 */

	ncopied = cp - buf;
	while (ncopied < buflen) {
		trans = (ncopied < nleft) ? ncopied : nleft;
		memcpy(cp, buf, trans);
		cp += trans;
		ncopied += trans;
		nleft -= trans;
	}

	return (0);
}
