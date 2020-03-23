/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) Linux Test Project, 2014
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
