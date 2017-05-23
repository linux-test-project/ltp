/*
 * bitmask user library implementation.
 *
 * Copyright (c) 2004-2006 Silicon Graphics, Inc. All rights reserved.
 *
 * Paul Jackson <pj@sgi.com>
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "bitmask.h"
#include "tst_minmax.h"

struct bitmask {
	unsigned int size;
	unsigned long *maskp;
};

/* How many bits in an unsigned long */
#define bitsperlong (8 * sizeof(unsigned long))

/* howmany(a,b) : how many elements of size b needed to hold all of a */
#define howmany(x,y) (((x)+((y)-1))/(y))

/* How many longs in mask of n bits */
#define longsperbits(n) howmany(n, bitsperlong)

/*
 * The routines _getbit() and _setbit() are the only
 * routines that actually understand the layout of bmp->maskp[].
 *
 * On little endian architectures, this could simply be an array of
 * bytes.  But the kernel layout of bitmasks _is_ visible to userspace
 * via the sched_(set/get)affinity calls in Linux 2.6, and on big
 * endian architectures, it is painfully obvious that this is an
 * array of unsigned longs.
 */

/* Return the value (0 or 1) of bit n in bitmask bmp */
static unsigned int _getbit(const struct bitmask *bmp, unsigned int n)
{
	if (n < bmp->size)
		return (bmp->maskp[n / bitsperlong] >> (n % bitsperlong)) & 1;
	else
		return 0;
}

/* Set bit n in bitmask bmp to value v (0 or 1) */
static void _setbit(struct bitmask *bmp, unsigned int n, unsigned int v)
{
	if (n < bmp->size) {
		if (v)
			bmp->maskp[n / bitsperlong] |= 1UL << (n % bitsperlong);
		else
			bmp->maskp[n / bitsperlong] &=
			    ~(1UL << (n % bitsperlong));
	}
}

/*
 * Allocate and free `struct bitmask *`
 */

/* Allocate a new `struct bitmask` with a size of n bits */
struct bitmask *bitmask_alloc(unsigned int n)
{
	struct bitmask *bmp;

	bmp = malloc(sizeof(*bmp));
	if (bmp == 0)
		return 0;
	bmp->size = n;
	bmp->maskp = calloc(longsperbits(n), sizeof(unsigned long));
	if (bmp->maskp == 0) {
		free(bmp);
		return 0;
	}
	return bmp;
}

/* Free `struct bitmask` */
void bitmask_free(struct bitmask *bmp)
{
	if (bmp == 0)
		return;
	free(bmp->maskp);
	bmp->maskp = (unsigned long *)0xdeadcdef;	/* double free tripwire */
	free(bmp);
}

/*
 * Display and parse ascii string representations.
 */

#define HEXCHUNKSZ 32		/* hex binary format shows 32 bits per chunk */
#define HEXCHARSZ 8		/* hex ascii format has up to 8 chars per chunk */

/*
 * Write hex word representation of bmp to buf, 32 bits per
 * comma-separated, zero-filled hex word.  Do not write more
 * than buflen chars to buf.
 *
 * Return number of chars that would have been written
 * if buf were large enough.
 */

int bitmask_displayhex(char *buf, int buflen, const struct bitmask *bmp)
{
	int chunk;
	int cnt = 0;
	const char *sep = "";

	if (buflen < 1)
		return 0;
	buf[0] = 0;

	for (chunk = howmany(bmp->size, HEXCHUNKSZ) - 1; chunk >= 0; chunk--) {
		uint32_t val = 0;
		int bit;

		for (bit = HEXCHUNKSZ - 1; bit >= 0; bit--)
			val = val << 1 | _getbit(bmp, chunk * HEXCHUNKSZ + bit);
		cnt += snprintf(buf + cnt, MAX(buflen - cnt, 0), "%s%0*x",
				sep, HEXCHARSZ, val);
		sep = ",";
	}
	return cnt;
}

/*
 * emit(buf, buflen, rbot, rtop, len)
 *
 * Helper routine for bitmask_displaylist().  Write decimal number
 * or range to buf+len, suppressing output past buf+buflen, with optional
 * comma-prefix.  Return len of what would be written to buf, if it
 * all fit.
 */

static inline int emit(char *buf, int buflen, int rbot, int rtop, int len)
{
	if (len > 0)
		len += snprintf(buf + len, MAX(buflen - len, 0), ",");
	if (rbot == rtop)
		len += snprintf(buf + len, MAX(buflen - len, 0), "%d", rbot);
	else
		len +=
		    snprintf(buf + len, MAX(buflen - len, 0), "%d-%d", rbot,
			     rtop);
	return len;
}

/*
 * Write decimal list representation of bmp to buf.
 *
 * Output format is a comma-separated list of decimal numbers and
 * ranges.  Consecutively set bits are shown as two hyphen-separated
 * decimal numbers, the smallest and largest bit numbers set in
 * the range.  Output format is compatible with the format
 * accepted as input by bitmap_parselist().
 *
 * The return value is the number of characters which would be
 * generated for the given input, excluding the trailing '\0', as
 * per ISO C99.
 */

int bitmask_displaylist(char *buf, int buflen, const struct bitmask *bmp)
{
	int len = 0;
	/* current bit is 'cur', most recently seen range is [rbot, rtop] */
	unsigned int cur, rbot, rtop;

	if (buflen > 0)
		*buf = 0;
	rbot = cur = bitmask_first(bmp);
	while (cur < bmp->size) {
		rtop = cur;
		cur = bitmask_next(bmp, cur + 1);
		if (cur >= bmp->size || cur > rtop + 1) {
			len = emit(buf, buflen, rbot, rtop, len);
			rbot = cur;
		}
	}
	return len;
}

static const char *nexttoken(const char *q, int sep)
{
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

/*
 * Parse hex word representation in buf to bmp.
 *
 * Returns -1 on error, leaving unspecified results in bmp.
 */

int bitmask_parsehex(const char *buf, struct bitmask *bmp)
{
	const char *p, *q;
	int nchunks = 0, chunk;
	unsigned int size = 0;

	bitmask_clearall(bmp);
	size = longsperbits(bmp->size) * 8 * sizeof(unsigned long);

	q = buf;
	while (p = q, q = nexttoken(q, ','), p)
		nchunks++;

	chunk = nchunks - 1;
	q = buf;
	while (p = q, q = nexttoken(q, ','), p) {
		uint32_t val;
		int bit;
		char *endptr;
		int nchars_read, nchars_unread;

		val = strtoul(p, &endptr, 16);

		/* We should have consumed 1 to 8 (HEXCHARSZ) chars */
		nchars_read = endptr - p;
		if (nchars_read < 1 || nchars_read > HEXCHARSZ)
			goto err;

		/* We should have consumed up to next comma */
		nchars_unread = q - endptr;
		if (q != NULL && nchars_unread != 1)
			goto err;

		for (bit = HEXCHUNKSZ - 1; bit >= 0; bit--) {
			unsigned int n = chunk * HEXCHUNKSZ + bit;
			if (n >= size)
				goto err;
			_setbit(bmp, n, (val >> bit) & 1);
		}
		chunk--;
	}
	return 0;
err:
	bitmask_clearall(bmp);
	return -1;
}

/*
 * When parsing bitmask lists, only allow numbers, separated by one
 * of the allowed next characters.
 *
 * The parameter 'sret' is the return from a sscanf "%u%c".  It is
 * -1 if the sscanf input string was empty.  It is 0 if the first
 * character in the sscanf input string was not a decimal number.
 * It is 1 if the unsigned number matching the "%u" was the end of the
 * input string.  It is 2 if one or more additional characters followed
 * the matched unsigned number.  If it is 2, then 'nextc' is the first
 * character following the number.  The parameter 'ok_next_chars'
 * is the nul-terminated list of allowed next characters.
 *
 * The mask term just scanned was ok if and only if either the numbers
 * matching the %u were all of the input or if the next character in
 * the input past the numbers was one of the allowed next characters.
 */
static int scan_was_ok(int sret, char nextc, const char *ok_next_chars)
{
	return sret == 1 || (sret == 2 && strchr(ok_next_chars, nextc) != NULL);
}

/*
 * Parses a comma-separated list of numbers and ranges of numbers,
 * with optional ':%u' strides modifying ranges, into provided bitmask.
 * Some examples of input lists and their equivalent simple list:
 *	Input		Equivalent to
 *	0-3		0,1,2,3
 *	0-7:2		0,2,4,6
 *	1,3,5-7		1,3,5,6,7
 *	0-3:2,8-15:4	0,2,8,12
 */
int bitmask_parselist(const char *buf, struct bitmask *bmp)
{
	const char *p, *q;

	bitmask_clearall(bmp);

	q = buf;
	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* begin of range */
		unsigned int b;	/* end of range */
		unsigned int s;	/* stride */
		const char *c1, *c2;	/* next tokens after '-' or ',' */
		char nextc;	/* char after sscanf %u match */
		int sret;	/* sscanf return (number of matches) */

		sret = sscanf(p, "%u%c", &a, &nextc);
		if (!scan_was_ok(sret, nextc, ",-"))
			goto err;
		b = a;
		s = 1;
		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');
		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			sret = sscanf(c1, "%u%c", &b, &nextc);
			if (!scan_was_ok(sret, nextc, ",:"))
				goto err;
			c1 = nexttoken(c1, ':');
			if (c1 != NULL && (c2 == NULL || c1 < c2)) {
				sret = sscanf(c1, "%u%c", &s, &nextc);
				if (!scan_was_ok(sret, nextc, ","))
					goto err;
			}
		}
		if (!(a <= b))
			goto err;
		if (b >= bmp->size)
			goto err;
		while (a <= b) {
			_setbit(bmp, a, 1);
			a += s;
		}
	}
	return 0;
err:
	bitmask_clearall(bmp);
	return -1;
}

/*
 * Basic assignment operations
 */

/* Copy bmp2 to bmp1 */
struct bitmask *bitmask_copy(struct bitmask *bmp1, const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i));
	return bmp1;
}

/* Set all bits in bitmask: bmp = ~0 */
struct bitmask *bitmask_setall(struct bitmask *bmp)
{
	unsigned int i;
	for (i = 0; i < bmp->size; i++)
		_setbit(bmp, i, 1);
	return bmp;
}

/* Clear all bits in bitmask: bmp = 0 */
struct bitmask *bitmask_clearall(struct bitmask *bmp)
{
	unsigned int i;
	for (i = 0; i < bmp->size; i++)
		_setbit(bmp, i, 0);
	return bmp;
}

/*
 * Interface to kernel sched_setaffinity system call
 */

/* Length in bytes of mask - use as second argument to sched_setaffinity */
unsigned int bitmask_nbytes(struct bitmask *bmp)
{
	return longsperbits(bmp->size) * sizeof(unsigned long);
}

/* Direct pointer to bit mask - use as third argument to sched_setaffinty */
unsigned long *bitmask_mask(struct bitmask *bmp)
{
	return bmp->maskp;
}

/*
 * Unary numeric queries
 */

/* Size in bits of entire bitmask */
unsigned int bitmask_nbits(const struct bitmask *bmp)
{
	return bmp->size;
}

/* Hamming Weight: number of set bits */
unsigned int bitmask_weight(const struct bitmask *bmp)
{
	unsigned int i;
	unsigned int w = 0;
	for (i = 0; i < bmp->size; i++)
		if (_getbit(bmp, i))
			w++;
	return w;
}

/*
 * Unary Boolean queries
 */

/* True if specified bit i is set */
int bitmask_isbitset(const struct bitmask *bmp, unsigned int i)
{
	return _getbit(bmp, i);
}

/* True if specified bit i is clear */
int bitmask_isbitclear(const struct bitmask *bmp, unsigned int i)
{
	return !_getbit(bmp, i);
}

/* True if all bits are set */
int bitmask_isallset(const struct bitmask *bmp)
{
	unsigned int i;
	for (i = 0; i < bmp->size; i++)
		if (!_getbit(bmp, i))
			return 0;
	return 1;
}

/* True if all bits are clear */
int bitmask_isallclear(const struct bitmask *bmp)
{
	unsigned int i;
	for (i = 0; i < bmp->size; i++)
		if (_getbit(bmp, i))
			return 0;
	return 1;
}

/*
 * Single bit operations
 */

/* Set a single bit i in bitmask */
struct bitmask *bitmask_setbit(struct bitmask *bmp, unsigned int i)
{
	_setbit(bmp, i, 1);
	return bmp;
}

/* Clear a single bit i in bitmask */
struct bitmask *bitmask_clearbit(struct bitmask *bmp, unsigned int i)
{
	_setbit(bmp, i, 0);
	return bmp;
}

/*
 * Binary Boolean operations: bmp1 op? bmp2
 */

/* True if two bitmasks are equal */
int bitmask_equal(const struct bitmask *bmp1, const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size || i < bmp2->size; i++)
		if (_getbit(bmp1, i) != _getbit(bmp2, i))
			return 0;
	return 1;
}

/* True if first bitmask is subset of second */
int bitmask_subset(const struct bitmask *bmp1, const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		if (_getbit(bmp1, i) > _getbit(bmp2, i))
			return 0;
	return 1;
}

/* True if two bitmasks don't overlap */
int bitmask_disjoint(const struct bitmask *bmp1, const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		if (_getbit(bmp1, i) & _getbit(bmp2, i))
			return 0;
	return 1;
}

/* True if two bitmasks do overlap */
int bitmask_intersects(const struct bitmask *bmp1, const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		if (_getbit(bmp1, i) & _getbit(bmp2, i))
			return 1;
	return 0;
}

/*
 * Range operations
 */

/* Set bits of bitmask in specified range [i, j) */
struct bitmask *bitmask_setrange(struct bitmask *bmp,
				 unsigned int i, unsigned int j)
{
	unsigned int n;
	for (n = i; n < j; n++)
		_setbit(bmp, n, 1);
	return bmp;
}

/* Clear bits of bitmask in specified range */
struct bitmask *bitmask_clearrange(struct bitmask *bmp,
				   unsigned int i, unsigned int j)
{
	unsigned int n;
	for (n = i; n < j; n++)
		_setbit(bmp, n, 0);
	return bmp;
}

/* Clear all but specified range */
struct bitmask *bitmask_keeprange(struct bitmask *bmp,
				  unsigned int i, unsigned int j)
{
	bitmask_clearrange(bmp, 0, i);
	bitmask_clearrange(bmp, j, bmp->size);
	return bmp;
}

/*
 * Unary operations: bmp1 = op(struct bitmask *bmp2)
 */

/* Complement: bmp1 = ~bmp2 */
struct bitmask *bitmask_complement(struct bitmask *bmp1,
				   const struct bitmask *bmp2)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, !_getbit(bmp2, i));
	return bmp1;
}

/* Right shift: bmp1 = bmp2 >> n */
struct bitmask *bitmask_shiftright(struct bitmask *bmp1,
				   const struct bitmask *bmp2, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i + n));
	return bmp1;
}

/* Left shift: bmp1 = bmp2 << n */
struct bitmask *bitmask_shiftleft(struct bitmask *bmp1,
				  const struct bitmask *bmp2, unsigned int n)
{
	int i;
	for (i = bmp1->size - 1; i >= 0; i--)
		_setbit(bmp1, i, _getbit(bmp2, i - n));
	return bmp1;
}

/*
 * Binary operations: bmp1 = bmp2 op bmp3
 */

/* Logical `and` of two bitmasks: bmp1 = bmp2 & bmp3 */
struct bitmask *bitmask_and(struct bitmask *bmp1, const struct bitmask *bmp2,
			    const struct bitmask *bmp3)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i) & _getbit(bmp3, i));
	return bmp1;
}

/* Logical `andnot` of two bitmasks: bmp1 = bmp2 & ~bmp3 */
struct bitmask *bitmask_andnot(struct bitmask *bmp1, const struct bitmask *bmp2,
			       const struct bitmask *bmp3)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i) & ~_getbit(bmp3, i));
	return bmp1;
}

/* Logical `or` of two bitmasks: bmp1 = bmp2 | bmp3 */
struct bitmask *bitmask_or(struct bitmask *bmp1, const struct bitmask *bmp2,
			   const struct bitmask *bmp3)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i) | _getbit(bmp3, i));
	return bmp1;
}

/* Logical `eor` of two bitmasks: bmp1 = bmp2 ^ bmp3 */
struct bitmask *bitmask_eor(struct bitmask *bmp1, const struct bitmask *bmp2,
			    const struct bitmask *bmp3)
{
	unsigned int i;
	for (i = 0; i < bmp1->size; i++)
		_setbit(bmp1, i, _getbit(bmp2, i) ^ _getbit(bmp3, i));
	return bmp1;
}

/*
 * Iteration operators
 */

/* Number of lowest set bit (min) */
unsigned int bitmask_first(const struct bitmask *bmp)
{
	return bitmask_next(bmp, 0);
}

/* Number of next set bit at or above given bit i */
unsigned int bitmask_next(const struct bitmask *bmp, unsigned int i)
{
	unsigned int n;
	for (n = i; n < bmp->size; n++)
		if (_getbit(bmp, n))
			break;
	return n;
}

/* Absolute position of nth set bit */
unsigned int bitmask_rel_to_abs_pos(const struct bitmask *bmp, unsigned int n)
{
	unsigned int i;
	for (i = 0; i < bmp->size; i++)
		if (_getbit(bmp, i))
			if (n-- == 0)
				break;
	return i;
}

/* Relative position amongst set bits of bit n */
unsigned int bitmask_abs_to_rel_pos(const struct bitmask *bmp, unsigned int n)
{
	unsigned int i;
	unsigned int w = 0;

	if (!_getbit(bmp, n))
		return bmp->size;
	/* Add in number bits set before bit n */
	for (i = 0; i < n; i++)
		if (_getbit(bmp, i))
			w++;
	return w;
}

/* Number of highest set bit (max) */
unsigned int bitmask_last(const struct bitmask *bmp)
{
	unsigned int i;
	unsigned int m = bmp->size;
	for (i = 0; i < bmp->size; i++)
		if (_getbit(bmp, i))
			m = i;
	return m;
}
