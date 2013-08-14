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

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "compiler.h"

#include "safe_file_ops.h"
#include "tst_checkpoint.h"
#include "tst_process_state.h"
#include "tst_resource.h"

/* Use low 6 bits to encode test type */
#define TTYPE_MASK 0x3f
#define TPASS      0    /* Test passed flag */
#define TFAIL      1    /* Test failed flag */
#define TBROK      2    /* Test broken flag */
#define TWARN      4    /* Test warning flag */
#define TRETR      8    /* Test retire flag */
#define TINFO      16   /* Test information flag */
#define TCONF      32   /* Test not appropriate for configuration flag */
#define TTYPE_RESULT(ttype) ((ttype) & TTYPE_MASK)

#define TERRNO     0x100   /* Append errno information to output */
#define TTERRNO    0x200   /* Append TEST_ERRNO information to output */
#define TRERRNO    0x300   /* Capture errno information from TEST_RETURN to
			      output; useful for pthread-like APIs :). */

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
#define FORK    1		/* SIGCLD is to be ignored */
#define NOFORK  0		/* SIGCLD is to be caught */
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
#define FORK_OR_VFORK			vfork
#define MAP_PRIVATE_EXCEPT_UCLINUX	0
#else
#define FORK_OR_VFORK			fork
#define MAP_PRIVATE_EXCEPT_UCLINUX	MAP_PRIVATE
#endif

/*
 * lib/forker.c
 */
extern int Forker_pids[];
extern int Forker_npids;

/* lib/tst_res.c */
const char *strttype(int ttype);
void tst_res(int ttype, char *fname, char *arg_fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
void tst_resm(int ttype, char *arg_fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
void tst_resm_hexd(int ttype, const void *buf, size_t size, char *arg_fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
void tst_brk(int ttype, char *fname, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 4, 5)));
void tst_brkm(int ttype, void (*func)(void), char *arg_fmt, ...)
	__attribute__ ((format (printf, 3, 4))) LTP_ATTRIBUTE_NORETURN;
void tst_require_root(void (*func)(void));
int  tst_environ(void);
void tst_exit(void) LTP_ATTRIBUTE_NORETURN;
void tst_flush(void);

extern int tst_count;

/* lib/tst_sig.c */
void tst_sig(int fork_flag, void (*handler)(), void (*cleanup)());

/* lib/tst_tmpdir.c */

/* tst_tmpdir()
 *
 * Create a unique temporary directory and chdir() to it. It expects the caller
 * to have defined/initialized the TCID/TST_TOTAL global variables.
 * The TESTDIR global variable will be set to the directory that gets used
 * as the testing directory.
 *
 * NOTE: This function must be called BEFORE any activity that would require
 * CLEANUP.  If tst_tmpdir() fails, it cleans up afer itself and calls
 * tst_exit() (i.e. does not return).
 */
void tst_tmpdir(void);
/* tst_rmdir()
 *
 * Recursively remove the temporary directory created by tst_tmpdir().
 * This function is intended ONLY as a companion to tst_tmpdir().
 */
void tst_rmdir(void);
/* get_tst_tmpdir()
 *
 * Return a copy of the test temp directory as seen by LTP. This is for
 * path-oriented tests like chroot, etc, that may munge the path a bit.
 *
 * FREE VARIABLE AFTER USE IF IT IS REUSED!
 */
char *get_tst_tmpdir(void);
/*
 * Returns 1 if temp directory was created.
 */
int tst_tmpdir_created(void);

/* lib/get_high_address.c */
char *get_high_address(void);

/* lib/tst_kvercmp.c */
void tst_getkver(int *k1, int *k2, int *k3);
int tst_kvercmp(int r1, int r2, int r3);

struct tst_kern_exv {
	char *dist_name;
	char *extra_ver;
};

int tst_kvercmp2(int r1, int r2, int r3, struct tst_kern_exv *vers);

/* lib/tst_is_cwd.c */
int tst_is_cwd_nfs(void);
int tst_is_cwd_v9fs(void);
int tst_is_cwd_tmpfs(void);
int tst_is_cwd_ramfs(void);

/* lib/tst_cwd_has_free.c */
int tst_cwd_has_free(int required_kib);

/* lib/self_exec.c */
void maybe_run_child(void (*child)(), char *fmt, ...);
int self_exec(char *argv0, char *fmt, ...);

/* Functions from lib/cloner.c */
int ltp_clone(unsigned long clone_flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack);
int ltp_clone_malloc(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg, size_t stacksize);
int ltp_clone_quick(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg);
#define clone(...) use_the_ltp_clone_functions,do_not_use_clone

/* Functions from lib/mount_utils.c */
char *get_block_device(const char *path);
char *get_mountpoint(const char *path);

/* Function from lib/get_path.c */
int tst_get_path(const char *prog_name, char *buf, size_t buf_len);

/* lib/tst_cpu.c */
long tst_ncpus(void);
long tst_ncpus_max(void);

/* lib/tst_run_cmd.c
 *
 * vfork() + execvp() specified program.
 * @argv: a list of two (at least program name + NULL) or more pointers that
 * represent the argument list to the new program. The array of pointers
 * must be terminated by a NULL pointer.
 * @stdout_fd: file descriptor where to redirect stdout. Set -1 if
 * redirection is not needed.
 * @stderr_fd: file descriptor where to redirect stderr. Set -1 if
 * redirection is not needed.
 */
void tst_run_cmd_fds(void (cleanup_fn)(void),
			const char *const argv[],
			int stdout_fd,
			int stderr_fd);

/* Executes tst_run_cmd_fds() and redirects its output to a file
 * @stdout_path: path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @stderr_path: path where to redirect stderr. Set NULL if redirection is
 * not needed.
 */
void tst_run_cmd(void (cleanup_fn)(void),
		const char *const argv[],
		const char *stdout_path,
		const char *stderr_path);

/* lib/tst_mkfs.c
 *
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: extra mkfs options
 */
void tst_mkfs(void (cleanup_fn)(void), const char *dev,
              const char *fs_type, const char *fs_opts);

/* lib/tst_fill_file.c
 *
 * Creates/ovewrites a file with specified pattern
 * @path: path to file
 * @pattern: pattern
 * @bs: block size
 * @bcount: blocks amount
 */
int tst_fill_file(const char *path, char pattern, size_t bs, size_t bcount);

#ifdef TST_USE_COMPAT16_SYSCALL
#define TCID_BIT_SUFFIX "_16"
#elif  TST_USE_NEWER64_SYSCALL
#define TCID_BIT_SUFFIX "_64"
#else
#define TCID_BIT_SUFFIX ""
#endif
#define TCID_DEFINE(ID) char *TCID = (#ID TCID_BIT_SUFFIX)

#endif	/* __TEST_H__ */
