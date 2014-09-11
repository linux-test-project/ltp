/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef TST_RES_FLAGS_H
#define TST_RES_FLAGS_H

/* Use low 6 bits to encode test type */
#define TTYPE_MASK	0x3f
#define TPASS	0	/* Test passed flag */
#define TFAIL	1	/* Test failed flag */
#define TBROK	2	/* Test broken flag */
#define TWARN	4	/* Test warning flag */
#define TINFO	16	/* Test information flag */
#define TCONF	32	/* Test not appropriate for configuration flag */
#define TTYPE_RESULT(ttype)	((ttype) & TTYPE_MASK)

#define TERRNO	0x100	/* Append errno information to output */
#define TTERRNO	0x200	/* Append TEST_ERRNO information to output */
#define TRERRNO	0x400	/* Capture errno information from TEST_RETURN to
			   output; useful for pthread-like APIs :). */

#endif /* TST_RES_FLAGS_H */
