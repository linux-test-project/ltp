// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2013 Cyril Hrubis chrubis@suse.cz
 */

#ifndef __TEST_H__
#define __TEST_H__

#ifdef TST_TEST_H__
# error Newlib tst_test.h already included
#endif /* TST_TEST_H__ */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "usctest.h"

#include "tst_common.h"
#include "old_safe_file_ops.h"
#include "old_checkpoint.h"
#include "tst_process_state.h"
#include "old_resource.h"
#include "tst_res_flags.h"
#include "tst_kvercmp.h"
#include "tst_fs.h"
#include "tst_pid.h"
#include "tst_cmd.h"
#include "tst_cpu.h"
#include "tst_clone.h"
#include "old_device.h"
#include "old_tmpdir.h"
#include "tst_minmax.h"
#include "tst_get_bad_addr.h"
#include "tst_path_has_mnt_flags.h"

/*
 * Ensure that NUMSIGS is defined.
 * It should be defined in signal.h or sys/signal.h on
 * UNICOS/mk and IRIX systems.   On UNICOS systems,
 * it is not defined, thus it is being set to UNICOS's NSIG.
 * Note:  IRIX's NSIG (signals are 1-(NSIG-1))
 *      is not same meaning as UNICOS/UMK's NSIG  (signals 1-NSIG)
 */
#ifndef NUMSIGS
#define NUMSIGS NSIG
#endif


/* defines for unexpected signal setup routine (set_usig.c) */
#define FORK    1		/* SIGCHLD is to be ignored */
#define NOFORK  0		/* SIGCHLD is to be caught */
#define DEF_HANDLER SIG_ERR	/* tells set_usig() to use default signal handler */

/*
 * The following defines are used to control tst_res and t_result reporting.
 */

#define TOUTPUT	   "TOUTPUT"		/* The name of the environment variable */
					/* that can be set to one of the following */
					/* strings to control tst_res output */
					/* If not set, TOUT_VERBOSE_S is assumed */

/*
 * Macro to use for making functions called only once in
 * multi-threaded tests such as init or cleanup function.
 * The first call to @name_fn function by any thread shall
 * call the @exec_fn. Subsequent calls shall not call @exec_fn.
 * *_fn functions must not take any arguments.
 */
#define TST_DECLARE_ONCE_FN(name_fn, exec_fn)				\
	void name_fn(void)						\
	{								\
		static pthread_once_t ltp_once = PTHREAD_ONCE_INIT;	\
		pthread_once(&ltp_once, exec_fn);			\
	}

/*
 * lib/forker.c
 */
extern int Forker_pids[];
extern int Forker_npids;

typedef struct {
	char *option;	/* Valid option string (one option only) like "a:"  */
	int  *flag;	/* Pointer to location to set true if option given  */
	char **arg;	/* Pointer to location to place argument, if needed */
} option_t;

/* lib/tst_parse_opts.c */
void tst_parse_opts(int argc, char *argv[], const option_t *user_optarg,
                    void (*user_help)(void));

/* lib/tst_res.c */
const char *strttype(int ttype);

void tst_resm_(const char *file, const int lineno, int ttype,
	const char *arg_fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
#define tst_resm(ttype, arg_fmt, ...) \
	tst_resm_(__FILE__, __LINE__, (ttype), \
		  (arg_fmt), ##__VA_ARGS__)

void tst_resm_hexd_(const char *file, const int lineno, int ttype,
	const void *buf, size_t size, const char *arg_fmt, ...)
	__attribute__ ((format (printf, 6, 7)));
#define tst_resm_hexd(ttype, buf, size, arg_fmt, ...) \
	tst_resm_hexd_(__FILE__, __LINE__, (ttype), (buf), (size), \
		       (arg_fmt), ##__VA_ARGS__)

void tst_brkm__(const char *file, const int lineno, int ttype,
	void (*func)(void), const char *arg_fmt, ...)
	__attribute__ ((format (printf, 5, 6))) LTP_ATTRIBUTE_NORETURN;

#ifdef LTPLIB
# include "ltp_priv.h"
# define tst_brkm(flags, cleanup, fmt, ...) do { \
	if (tst_test) \
		tst_brk_(__FILE__, __LINE__, flags, fmt, ##__VA_ARGS__); \
	else \
		tst_brkm__(__FILE__, __LINE__, flags, cleanup, fmt, ##__VA_ARGS__); \
	} while (0)

#define tst_brkm_(file, lineno, flags, cleanup, fmt, ...) do { \
	if (tst_test) \
		tst_brk_(file, lineno, flags, fmt, ##__VA_ARGS__); \
	else \
		tst_brkm__(file, lineno, flags, cleanup, fmt, ##__VA_ARGS__); \
	} while (0)
#else
# define tst_brkm(flags, cleanup, fmt, ...) do { \
		tst_brkm__(__FILE__, __LINE__, flags, cleanup, fmt, ##__VA_ARGS__); \
	} while (0)
#endif

void tst_require_root(void);
void tst_exit(void) LTP_ATTRIBUTE_NORETURN;
void tst_old_flush(void);

/*
 * tst_old_flush() + fork
 * NOTE: tst_fork() will reset T_exitval to 0 for child process.
 */
pid_t tst_fork(void);

/* lib/tst_res.c */
/*
 * In case we need do real test work in child process parent process can use
 * tst_record_childstatus() to make child process's test results propagated to
 * parent process correctly.
 *
 * The child can use tst_resm(), tst_brkm() followed by the tst_exit() or
 * plain old exit() (with TPASS, TFAIL and TBROK).
 *
 * WARNING: Be wary that the child cleanup function passed to tst_brkm()
 *          must clean only resources the child has allocated. E.g. the
 *          child cleanup is different function from the parent cleanup.
 */
void tst_record_childstatus(void (*cleanup)(void), pid_t child);

extern int tst_count;

/* lib/tst_sig.c */
void tst_sig(int fork_flag, void (*handler)(), void (*cleanup)());

/* lib/tst_mkfs.c
 *
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: NULL or NULL terminated array of mkfs options
 * @extra_opts: NULL or NULL terminated array of extra mkfs options which are
 * passed after the device name.
 */
#define tst_mkfs(cleanup, dev, fs_type, fs_opts, extra_opts) \
	tst_mkfs_(__FILE__, __LINE__, cleanup, dev, fs_type, \
		  fs_opts, extra_opts)
void tst_mkfs_(const char *file, const int lineno, void (cleanup_fn)(void),
	       const char *dev, const char *fs_type,
	       const char *const fs_opts[], const char *const extra_opts[]);

/* lib/tst_res.c
 * tst_strsig converts signal's value to corresponding string.
 * tst_strerrno converts errno to corresponding string.
 */
const char *tst_strsig(int sig);
const char *tst_strerrno(int err);

#ifdef TST_USE_COMPAT16_SYSCALL
#define TCID_BIT_SUFFIX "_16"
#elif  TST_USE_NEWER64_SYSCALL
#define TCID_BIT_SUFFIX "_64"
#else
#define TCID_BIT_SUFFIX ""
#endif
#define TCID_DEFINE(ID) char *TCID = (#ID TCID_BIT_SUFFIX)

#endif	/* __TEST_H__ */
