// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_DEFAULTS_H_
#define TST_DEFAULTS_H_

/*
 * This is the default temporary directory used by tst_tmpdir().
 *
 * This is used when TMPDIR env variable is not set.
 */
#define TEMPDIR	"/tmp"

/*
 * Default filesystem to be used for tests.
 */
#define DEFAULT_FS_TYPE "ext2"

#endif /* TST_DEFAULTS_H_ */
