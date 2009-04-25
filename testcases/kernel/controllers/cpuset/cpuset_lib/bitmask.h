/*
 * bitmask header file
 *
 * Copyright (c) 2004 Silicon Graphics, Inc. All rights reserved.
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

/*
 * bitmasks - dynamically sized multi-word bit masks, with many operators
 *
 *   link with -lbitmask
 *
 * ==== Allocate and free `struct bitmask *`
 *
 * bitmask_alloc(n): Allocate a new struct bitmask with a size of n bits
 * bitmask_free(bmp): Free struct bitmask
 *
 * ==== Display and parse ascii string representations
 *
 * bitmask_displayhex(buf, len, bmp): Write hex word bmp to buf
 * bitmask_displaylist(buf, len, bmp): Write decimal list bmp to buf
 * bitmask_parsehex(buf, bmp): Parse hex words in buf to bmp
 * bitmask_parselist(buf, bmp): Parse decimal list in buf to bmp
 *
 * ==== Basic initialization operations
 *
 * bitmask_copy(bmp1, bmp2): Copy bmp2 to bmp1
 * bitmask_setall(bmp): Set all bits in bitmask: bmp = ~0
 * bitmask_clearall(bmp): Clear all bits in bitmask: bmp = 0
 *
 * ==== Interface to kernel sched_{set,get}affinity system calls
 *
 * bitmask_nbytes(bmp): Length in bytes of mask - use as second argument
 * bitmask_mask(bmp): Direct pointer to bit mask - use as third argument
 *
 * ==== Unary numeric queries
 *
 * bitmask_nbits(bmp): Size in bits of entire bitmask
 * bitmask_weight(bmp): Hamming Weight: number of set bits
 *
 * ==== Unary Boolean queries
 *
 * bitmask_isbitset(bmp, i): True if specified bit i is set
 * bitmask_isbitclear(bmp, i): True if specified bit i is clear
 * bitmask_isallset(bmp): True if all bits are set
 * bitmask_isallclear(bmp): True if all bits are clear
 *
 * ==== Single bit operations
 *
 * bitmask_setbit(bmp, i): Set a single bit i in bitmask
 * bitmask_clearbit(bmp, i): Clear a single bit i in bitmask
 *
 * ==== Binary Boolean operations: bmp1 op? bmp2
 *
 * bitmask_equal(bmp1, bmp2): True if two bitmasks are equal
 * bitmask_subset(bmp1, bmp2): True if first bitmask is subset of second
 * bitmask_disjoint(bmp1, bmp2): True if two bitmasks don't overlap
 * bitmask_intersects(bmp1, bmp2): True if two bitmasks do overlap
 *
 * ==== Range operations
 *
 * bitmask_setrange(bmp, i, j): Set bits of bitmask in specified range [i, j)
 * bitmask_clearrange(bmp, i, j): Clear bits of bitmask in specified range
 * bitmask_keeprange(bmp, i, j): Clear all but specified range
 *
 * ==== Unary operations
 *
 * bitmask_complement(bmp1, bmp2): Complement: bmp1 = ~bmp2
 * bitmask_shiftright(bmp1, bmp2, n): Right shift: bmp1 = bmp2 >> n
 * bitmask_shiftleft(bmp1, bmp2, n): Left shift: bmp1 = bmp2 << n
 *
 * ==== Binary operations
 *
 * bitmask_and(bmp1, bmp2, bmp3): Logical `and`: bmp1 = bmp2 & bmp3
 * bitmask_andnot(bmp1, bmp2, bmp3): Logical `andnot`: bmp1 = bmp2 & ~bmp3
 * bitmask_or(bmp1, bmp2, bmp3): Logical `or`: bmp1 = bmp2 | bmp3
 * bitmask_eor(bmp1, bmp2, bmp3): Logical `eor`: bmp1 = bmp2 ^ bmp3
 *
 * ==== Iteration operators
 *
 * bitmask_first(bmp): Number of lowest set bit (min)
 * bitmask_next(bmp, i): Number of next set bit at or above given bit i
 * bitmask_rel_to_abs_pos(bmp, n): Absolute position of nth set bit
 * bitmask_abs_to_rel_pos(bmp, n): Relative position amongst set bits of bit n
 * bitmask_last(bmp): Number of highest set bit (max)
 *
 * ==== Example:
 *
 *    == allocate some bitmasks ==
 *    struct bitmask *a = bitmask_alloc(10);
 *    struct bitmask *b = bitmask_alloc(10);
 *    struct bitmask *c = bitmask_alloc(10);
 *    struct bitmask *d = bitmask_alloc(10);
 *    struct bitmask *e = bitmask_alloc(10);
 *    struct bitmask *f = bitmask_alloc(10);
 *    struct bitmask *g = bitmask_alloc(10);
 *
 *    == stuff some data in first two ==
 *    bitmask_setrange(a, 2, 6);    # set bits 2,3,4,5
 *    bitmask_shiftleft(b, a, 2);   # set bits 4,5,6,7
 *
 *    == d is complement of union ==
 *    bitmask_complement(d, bitmask_or(c, a, b));
 *
 *    == g is intersection of complements ==
 *    bitmask_and(g, bitmask_complement(e, a), bitmask_complement(f, b));
 *
 *    == d should equal g ==
 *    if (bitmask_equal(d, g))
 *        puts("DeMorgan's Law works!");
 *
 *    == free up bitmasks ==
 *    bitmask_free(a);
 *    bitmask_free(b);
 *    bitmask_free(c);
 *    bitmask_free(d);
 *    bitmask_free(e);
 *    bitmask_free(f);
 *    bitmask_free(g);
 */

#ifndef _BITMASK_H
#define _BITMASK_H

#ifdef __cplusplus
extern "C" {
#endif

struct bitmask;

struct bitmask *bitmask_alloc(unsigned int n);
void bitmask_free(struct bitmask *bmp);

int bitmask_displayhex(char *buf, int len, const struct bitmask *bmp);
int bitmask_displaylist(char *buf, int len, const struct bitmask *bmp);
int bitmask_parsehex(const char *buf, struct bitmask *bmp);
int bitmask_parselist(const char *buf, struct bitmask *bmp);

struct bitmask *bitmask_copy(struct bitmask *bmp1, const struct bitmask *bmp2);
struct bitmask *bitmask_setall(struct bitmask *bmp);
struct bitmask *bitmask_clearall(struct bitmask *bmp);

unsigned int bitmask_nbytes(struct bitmask *bmp);
unsigned long *bitmask_mask(struct bitmask *bmp);

unsigned int bitmask_nbits(const struct bitmask *bmp);
unsigned int bitmask_weight(const struct bitmask *bmp);

int bitmask_isbitset(const struct bitmask *bmp, unsigned int i);
int bitmask_isbitclear(const struct bitmask *bmp, unsigned int i);
int bitmask_isallset(const struct bitmask *bmp);
int bitmask_isallclear(const struct bitmask *bmp);

struct bitmask *bitmask_setbit(struct bitmask *bmp, unsigned int i);
struct bitmask *bitmask_clearbit(struct bitmask *bmp, unsigned int i);

int bitmask_equal(const struct bitmask *bmp1, const struct bitmask *bmp2);
int bitmask_subset(const struct bitmask *bmp1, const struct bitmask *bmp2);
int bitmask_disjoint(const struct bitmask *bmp1, const struct bitmask *bmp2);
int bitmask_intersects(const struct bitmask *bmp1, const struct bitmask *bmp2);

struct bitmask *bitmask_setrange(struct bitmask *bmp,
				unsigned int i, unsigned int j);
struct bitmask *bitmask_clearrange(struct bitmask *bmp,
				unsigned int i, unsigned int j);
struct bitmask *bitmask_keeprange(struct bitmask *bmp,
				unsigned int i, unsigned int j);

struct bitmask *bitmask_complement(struct bitmask *bmp1,
				const struct bitmask *bmp2);
struct bitmask *bitmask_shiftright(struct bitmask *bmp1,
				const struct bitmask *bmp2, unsigned int n);
struct bitmask *bitmask_shiftleft(struct bitmask *bmp1,
				const struct bitmask *bmp2, unsigned int n);

struct bitmask *bitmask_and(struct bitmask *bmp1,const struct bitmask *bmp2,
				const struct bitmask *bmp3);
struct bitmask *bitmask_andnot(struct bitmask *bmp1, const struct bitmask *bmp2,
				const struct bitmask *bmp3);
struct bitmask *bitmask_or(struct bitmask *bmp1, const struct bitmask *bmp2,
				const struct bitmask *bmp3);
struct bitmask *bitmask_eor(struct bitmask *bmp1, const struct bitmask *bmp2,
				const struct bitmask *bmp3);

unsigned int bitmask_first(const struct bitmask *bmp);
unsigned int bitmask_next(const struct bitmask *bmp, unsigned int i);
unsigned int bitmask_rel_to_abs_pos(const struct bitmask *bmp, unsigned int n);
unsigned int bitmask_abs_to_rel_pos(const struct bitmask *bmp, unsigned int n);
unsigned int bitmask_last(const struct bitmask *bmp);

#ifdef __cplusplus
}
#endif

#endif
