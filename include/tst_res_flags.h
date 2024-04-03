/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) Linux Test Project, 2014
 */

#ifndef TST_RES_FLAGS_H
#define TST_RES_FLAGS_H

/**
 * enum tst_res_flags - Test result reporting flags.
 *
 * @TPASS: Reports a single success.
 * @TFAIL: Reports a single failure.
 * @TBROK: Reports a single breakage.
 * @TWARN: Reports a single warning. Warnings increment a warning counter and
 *         show up in test results.
 *
 * @TDEBUG: Prints additional debugging messages, it does not change the test result counters and
 *          the message is not displayed unless debugging is enabled with -D
 *          test command line parameter.
 *
 * @TINFO: Prints an additional information, it does not change the test result
 *         counters but unlike TDEBUG the message is always displayed.
 *
 * @TCONF: Reports unsupported configuration. When tests produce this result at
 *         least a subset of test was skipped, because it couldn't run. The
 *         usual reasons are, missing kernel modules or CONFIG options.
 *         Unsuitable CPU architecture, not enough memory, etc.
 *
 * @TERRNO: Combine bitwise with result flags to append errno to the output message.
 *
 * @TTERRNO: Combine bitwise with result flags to append error from TST_ERR to
 *           the message. The TST_TEST() macros store the errno into the
 *           TST_ERR global variable in order to make sure it's not change
 *           between the test is done and results are printed.
 *
 * @TRERRNO: Combine bitwise with result flags to errno from TST_RET variable
 *           to the message. The TST_TEST() macros store return value into the
 *           TST_RET global variable and quite a few, e.g. pthread functions,
 *           return the error value directly instead of storing it to the errno.
 *
 * A result flag with optional bitwise combination of errno flag are passed to
 * the tst_res() and tst_brk() functions. Each message counts as a single test
 * result and tests can produce arbitrary number of results, i.e. TPASS, TFAIL,
 * TBROK, TWARN and TCONF messages. Each such message increases a result
 * counter in a piece of shared memory, which means that reported results are
 * accounted immediately even from child processes and there is no need for
 * result propagation.
 */
enum tst_res_flags {
	TPASS = 0,
	TFAIL = 1,
	TBROK = 2,
	TWARN = 4,
	TDEBUG = 8,
	TINFO = 16,
	TCONF = 32,
	TERRNO = 0x100,
	TTERRNO = 0x200,
	TRERRNO	= 0x400,
};

#define TTYPE_RESULT(ttype)	((ttype) & TTYPE_MASK)
#define TTYPE_MASK 0x3f

#endif /* TST_RES_FLAGS_H */
