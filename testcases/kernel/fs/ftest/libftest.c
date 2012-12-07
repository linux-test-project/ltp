/*
 *   Copyright (c) Cyril Hrubis chrubis@suse.cz 2009
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

#include <sys/uio.h>
#include <inttypes.h>
#include <limits.h>
#include <assert.h>
#include "test.h"
#include "libftest.h"

/*
 * Dump content of iov structure.
 */
void ft_dumpiov(struct iovec *iov)
{
	char *buf, val;
	int idx, nout, i;

	tst_resm(TINFO, "\tBuf:");

	nout = 0;
	idx = 0;
	buf = (char *)iov->iov_base;
	val = *((char *)buf);

	for (i = 0; (unsigned int)i < iov->iov_len; i++) {

		if (buf[i] != val) {
			if (i == idx + 1)
				tst_resm(TINFO, "\t%" PRIx32 "x,",
					 buf[idx] & 0xff);
			else
				tst_resm(TINFO, "\t%d*%" PRIx32 "x, ", i - idx,
					 buf[idx] & 0xff);
			idx = i;
			++nout;
		}

		if (nout > 10) {
			tst_resm(TINFO, "\t ... more");
			return;
		}
	}

	if (i == idx + 1)
		tst_resm(TINFO, "\t%" PRIx32 "x", buf[idx] & 0xff);
	else
		tst_resm(TINFO, "\t%d*%" PRIx32 "x", i - idx, buf[idx]);
}

/*
 * Dump bits string.
 */
void ft_dumpbits(void *bits, size_t size)
{
	void *buf;

	tst_resm(TINFO, "\tBits array:");

	for (buf = bits; size > 0; --size, ++buf) {
		tst_resm(TINFO, "\t%td:\t", 8 * (buf - bits));
		if ((buf - bits) % 16 == 0) {
			assert(0 < (buf - bits));
			tst_resm(TINFO, "\t%td:\t", 8 * (buf - bits));
		}
		tst_resm(TINFO, "\t%02" PRIx32 "x ", *((char *)buf) & 0xff);
	}

	tst_resm(TINFO, "\t");
}

/*
 * Do logical or of hold and bits (of size)
 * fields and store result into hold field.
 */
void ft_orbits(char *hold, char *bits, int size)
{
	while (size-- > 0)
		*hold++ |= *bits++;
}

/*
 * Dumps buffer in hexadecimal format.
 */
void ft_dumpbuf(char *buf, int csize)
{
	char val;
	int idx, nout, i;

	tst_resm(TINFO, "\tBuf:");
	nout = 0;
	idx = 0;
	val = buf[0];

	for (i = 0; i < csize; i++) {
		if (buf[i] != val) {
			if (i == idx + 1)
				tst_resm(TINFO, "\t%x, ", buf[idx] & 0xff);
			else
				tst_resm(TINFO, "\t%d*%x, ", i - idx,
					 buf[idx] & 0xff);
			idx = i;
			++nout;
		}
		if (nout > 10) {
			tst_resm(TINFO, "\t ... more");
			return;
		}
	}

	if (i == idx + 1)
		tst_resm(TINFO, "\t%x", buf[idx] & 0xff);
	else
		tst_resm(TINFO, "\t%d*%x", i - idx, buf[idx]);
}

/*
 * Creates filename from path and numbers.
 *
 * TODO: name is big enough?
 */
void ft_mkname(char *name, char *dirname, int me, int idx)
{
	char a, b;

	a = 'A' + (me % 26);
	b = 'a' + (idx % 26);

	if (dirname[0] != '\0')
		snprintf(name, PATH_MAX, "%s/%c%c", dirname, a, b);
	else
		snprintf(name, PATH_MAX, "%c%c", a, b);
}
