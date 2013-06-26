/*
 * Copyright (c) 2013 Cyril Hrubis chrubis@suse.cz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 */

#ifndef __LTP_PRIV_H__
#define __LTP_PRIV_H__

/* declared in tst_tmpdir.c */
const char *tst_get_startwd(void);

/*
 * This is the default temporary directory used by tst_tmpdir().
 *
 * This is used when TMPDIR env variable is not set.
 */
#define TEMPDIR	"/tmp"

/* environment variables for controlling  tst_res verbosity */
#define TOUT_VERBOSE_S  "VERBOSE"	/* All test cases reported */
#define TOUT_NOPASS_S   "NOPASS"	/* No pass test cases are reported */
#define TOUT_DISCARD_S  "DISCARD"	/* No output is reported */

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify the number of iterations.
 * It is supported in parse_opts.c and USC_setup.c.
 */
#define USC_ITERATION_ENV       "USC_ITERATIONS"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify to iteration until desired time
 * in floating point seconds has gone by.
 * Supported in USC_setup.c.
 */
#define USC_LOOP_WALLTIME	"USC_LOOP_WALLTIME"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify that no functional checks are wanted.
 * It is supported in parse_opts.c and USC_setup.c.
 */
#define USC_NO_FUNC_CHECK	"USC_NO_FUNC_CHECK"

/*
 * The following define contains the name of an environmental variable
 * that can be used to specify the delay between each loop iteration.
 * The value is in seconds (fractional numbers are allowed).
 * It is supported in parse_opts.c.
 */
#define USC_LOOP_DELAY		"USC_LOOP_DELAY"


#endif /* __LTP_PRIV_H__ */
