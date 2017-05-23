/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2009-2013 Cyril Hrubis chrubis@suse.cz
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
#include "tst_timer.h"
#include "tst_kvercmp.h"
#include "tst_fs.h"
#include "tst_pid.h"
#include "tst_cmd.h"
#include "tst_cpu.h"
#include "tst_clone.h"
#include "old_device.h"
#include "old_tmpdir.h"
#include "tst_minmax.h"

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
 * fork() can't be used on uClinux systems, so use FORK_OR_VFORK instead,
 * which will run vfork() on uClinux.
 * mmap() doesn't support MAP_PRIVATE on uClinux systems, so use
 * MAP_PRIVATE_EXCEPT_UCLINUX instead, which will skip the option on uClinux.
 * If MAP_PRIVATE really is required, the test can not be run on uClinux.
 */
#ifdef UCLINUX
# define FORK_OR_VFORK			tst_vfork
# define MAP_PRIVATE_EXCEPT_UCLINUX	0
/* tst_flush() + vfork() */
pid_t tst_vfork(void);
#else
# define FORK_OR_VFORK			tst_fork
# define MAP_PRIVATE_EXCEPT_UCLINUX	MAP_PRIVATE
#endif

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

void tst_brkm_(const char *file, const int lineno, int ttype,
	void (*func)(void), const char *arg_fmt, ...)
	__attribute__ ((format (printf, 5, 6))) LTP_ATTRIBUTE_NORETURN;

#ifdef LTPLIB
# include "ltp_priv.h"
# define tst_brkm(flags, cleanup, fmt, ...) do { \
	if (tst_test) \
		tst_brk_(__FILE__, __LINE__, flags, fmt, ##__VA_ARGS__); \
	else \
		tst_brkm_(__FILE__, __LINE__, flags, cleanup, fmt, ##__VA_ARGS__); \
	} while (0)
#else
# define tst_brkm(flags, cleanup, fmt, ...) do { \
		tst_brkm_(__FILE__, __LINE__, flags, cleanup, fmt, ##__VA_ARGS__); \
	} while (0)
#endif

void tst_require_root(void);
void tst_exit(void) LTP_ATTRIBUTE_NORETURN;
void tst_flush(void);

/*
 * tst_flush() + fork
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

/* lib/get_high_address.c */
char *get_high_address(void);

/* lib/self_exec.c */
void maybe_run_child(void (*child)(), const char *fmt, ...);
int self_exec(const char *argv0, const char *fmt, ...);

/* lib/tst_mkfs.c
 *
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: NULL or NULL terminated array of mkfs options
 * @extra_opt: extra mkfs option which is passed after the device name
 */
#define tst_mkfs(cleanup, dev, fs_type, fs_opts, extra_opt) \
	tst_mkfs_(__FILE__, __LINE__, cleanup, dev, fs_type, \
		  fs_opts, extra_opt)
void tst_mkfs_(const char *file, const int lineno, void (cleanup_fn)(void),
	       const char *dev, const char *fs_type,
	       const char *const fs_opts[], const char *extra_opt);

/* lib/tst_net.c
 *
 * Return unused port
 */
unsigned short tst_get_unused_port(void (cleanup_fn)(void),
	unsigned short family, int type);

/* lib/tst_res.c
 * tst_strsig converts signal's value to corresponding string.
 * tst_strerrno converts errno to corresponding string.
 */
const char *tst_strsig(int sig);
const char *tst_strerrno(int err);

/* lib/tst_path_has_mnt_flags.c
 *
 * Check whether a path is on a filesystem that is mounted with
 * specified flags
 * @path: path to file, if path is NULL tst_tmpdir is used.
 * @flags: NULL or NULL terminated array of mount flags
 *
 * Return: 0..n - number of flags matched
 */
int tst_path_has_mnt_flags(void (cleanup_fn)(void),
		const char *path, const char *flags[]);

#ifdef TST_USE_COMPAT16_SYSCALL
#define TCID_BIT_SUFFIX "_16"
#elif  TST_USE_NEWER64_SYSCALL
#define TCID_BIT_SUFFIX "_64"
#else
#define TCID_BIT_SUFFIX ""
#endif
#define TCID_DEFINE(ID) char *TCID = (#ID TCID_BIT_SUFFIX)

#endif	/* __TEST_H__ */
