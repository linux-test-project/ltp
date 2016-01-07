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
#include <stdint.h>

#include "compiler.h"

#include "usctest.h"

#include "safe_file_ops.h"
#include "tst_checkpoint.h"
#include "tst_process_state.h"
#include "tst_resource.h"
#include "tst_res_flags.h"
#include "tst_timer.h"

/* virt types for tst_is_virt() */
#define VIRT_XEN	1	/* xen dom0/domU */
#define VIRT_KVM	2	/* only default virtual CPU */

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
#define tst_brkm(ttype, func, arg_fmt, ...) \
	tst_brkm_(__FILE__, __LINE__, (ttype), (func), \
		  (arg_fmt), ##__VA_ARGS__)

void tst_cat_file(const char *filename);

void tst_require_root(void);
int  tst_environ(void);
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
/* tst_get_tmpdir()
 *
 * Return a copy of the test temp directory as seen by LTP. This is for
 * path-oriented tests like chroot, etc, that may munge the path a bit.
 *
 * FREE VARIABLE AFTER USE IF IT IS REUSED!
 */
char *tst_get_tmpdir(void);
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

enum {
	TST_BYTES = 1,
	TST_KB = 1024,
	TST_MB = 1048576,
	TST_GB = 1073741824,
};

/* lib/tst_fs_has_free.c
 *
 * @path: path is the pathname of any file within the mounted file system
 * @mult: mult should be TST_KB, TST_MB or TST_GB
 * the required free space is calculated by @size * @mult
 */
int tst_fs_has_free(void (*cleanup)(void), const char *path,
		    unsigned int size, unsigned int mult);

int tst_is_virt(int virt_type);

/* lib/self_exec.c */
void maybe_run_child(void (*child)(), const char *fmt, ...);
int self_exec(const char *argv0, const char *fmt, ...);

/* Functions from lib/cloner.c */
int ltp_clone(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack);
int ltp_clone7(unsigned long flags, int (*fn)(void *arg), void *arg,
		size_t stack_size, void *stack, ...);
int ltp_clone_malloc(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg, size_t stacksize);
int ltp_clone_quick(unsigned long clone_flags, int (*fn)(void *arg),
		void *arg);
#define clone(...) use_the_ltp_clone_functions,do_not_use_clone

/* Function from lib/get_path.c */
int tst_get_path(const char *prog_name, char *buf, size_t buf_len);

/* lib/tst_cpu.c */
long tst_ncpus(void);
long tst_ncpus_conf(void);
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
 * @pass_exit_val: if it's non-zero, this function will return the program
 * exit code, otherwise it will call cleanup_fn() if the program
 * exit code is not zero.
 */
int tst_run_cmd_fds(void (cleanup_fn)(void),
			const char *const argv[],
			int stdout_fd,
			int stderr_fd,
			int pass_exit_val);

/* Executes tst_run_cmd_fds() and redirects its output to a file
 * @stdout_path: path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @stderr_path: path where to redirect stderr. Set NULL if redirection is
 * not needed.
 * @pass_exit_val: if it's non-zero, this function will return the program
 * exit code, otherwise it will call cleanup_fn() if the program
 * exit code is not zero.
 */
int tst_run_cmd(void (cleanup_fn)(void),
		const char *const argv[],
		const char *stdout_path,
		const char *stderr_path,
		int pass_exit_val);

/* Wrapper function for system(3), ignorcing SIGCLD signal.
 * @command: the command to be run.
 */
int tst_system(const char *command);

/* lib/tst_mkfs.c
 *
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: NULL or NULL terminated array of extra mkfs options
 */
void tst_mkfs(void (cleanup_fn)(void), const char *dev,
	      const char *fs_type, const char *const fs_opts[]);

/*
 * Returns filesystem type to be used for the testing. Unless your test is
 * designed for specific filesystem you should use this function to the tested
 * filesytem.
 *
 * If TST_DEV_FS_TYPE is set the function returns it's content,
 * otherwise default fs type hardcoded in the library is returned.
 */
const char *tst_dev_fs_type(void);

/* lib/tst_device.c
 *
 * Acquires test device.
 *
 * Can be used only once, i.e. you cannot get two different devices.
 *
 * Looks for LTP_DEV env variable first (which may be passed by the test
 * driver or by a user) and returns just it's value if found.
 *
 * Otherwise creates a temp file and loop device.
 *
 * Note that you have to call tst_tmpdir() beforehand.
 *
 * Returns path to the device or NULL if it cannot be created.
 */
const char *tst_acquire_device(void (cleanup_fn)(void));

/* lib/tst_device.c
 * @dev: device path returned by the tst_acquire_device()
 */
void tst_release_device(void (cleanup_fn)(void), const char *dev);

/* lib/tst_device.c
 *
 * Just like umount() but retries several times on failure.
 * @path: Path to umount
 */
int tst_umount(const char *path);

/* lib/tst_fill_file.c
 *
 * Creates/ovewrites a file with specified pattern
 * @path: path to file
 * @pattern: pattern
 * @bs: block size
 * @bcount: blocks amount
 */
int tst_fill_file(const char *path, char pattern, size_t bs, size_t bcount);

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

/*
 * lib/tst_fs_link_count.c
 *
 * Try to get maximum number of hard links to a regular file inside the @dir.
 *
 * Note: This number depends on the filesystem @dir is on.
 *
 * The code uses link(2) to create hard links to a single file until it gets
 * EMLINK or creates 65535 links.
 *
 * If limit is hit maximal number of hardlinks is returned and the the @dir is
 * filled with hardlinks in format "testfile%i" where i belongs to [0, limit)
 * interval.
 *
 * If no limit is hit (succed to create 65535 without error) or if link()
 * failed with ENOSPC or EDQUOT zero is returned previously created files are
 * removed.
 */
int tst_fs_fill_hardlinks(void (*cleanup) (void), const char *dir);

/*
 * lib/tst_fs_link_count.c
 *
 * Try to get maximum number of subdirectories in directory.
 *
 * Note: This number depends on the filesystem @dir is on.
 *
 * The code uses mkdir(2) to create directories in @dir until it gets EMLINK
 * or creates 65535 directories.
 *
 * If limit is hit the maximal number of subdirectories is returned and the
 * @dir is filled with subdirectories in format "testdir%i" where i belongs to
 * [0, limit - 2) interval (because each newly created dir has two links
 * allready the '.' and link from parent dir).
 *
 * If no limit is hit or mkdir() failed with ENOSPC or EDQUOT zero is returned
 * previously created directories are removed.
 *
 */
int tst_fs_fill_subdirs(void (*cleanup) (void), const char *dir);

/*
 * lib/tst_pid.c
 *
 * Get a pid value not used by the OS
 */
pid_t tst_get_unused_pid(void (*cleanup_fn) (void));

/*
 * lib/tst_pid.c
 *
 * Returns number of free pids by substarction of the number of pids
 * currently used ('ps -eT') from max_pids
 */
int tst_get_free_pids(void (*cleanup_fn) (void));

#ifdef TST_USE_COMPAT16_SYSCALL
#define TCID_BIT_SUFFIX "_16"
#elif  TST_USE_NEWER64_SYSCALL
#define TCID_BIT_SUFFIX "_64"
#else
#define TCID_BIT_SUFFIX ""
#endif
#define TCID_DEFINE(ID) char *TCID = (#ID TCID_BIT_SUFFIX)

#endif	/* __TEST_H__ */
