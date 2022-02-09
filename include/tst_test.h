// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2016-2019
 */

#ifndef TST_TEST_H__
#define TST_TEST_H__

#ifdef __TEST_H__
# error Oldlib test.h already included
#endif /* __TEST_H__ */

#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include "tst_common.h"
#include "tst_res_flags.h"
#include "tst_test_macros.h"
#include "tst_checkpoint.h"
#include "tst_device.h"
#include "tst_mkfs.h"
#include "tst_fs.h"
#include "tst_pid.h"
#include "tst_cmd.h"
#include "tst_cpu.h"
#include "tst_process_state.h"
#include "tst_atomic.h"
#include "tst_kvercmp.h"
#include "tst_kernel.h"
#include "tst_minmax.h"
#include "tst_get_bad_addr.h"
#include "tst_path_has_mnt_flags.h"
#include "tst_sys_conf.h"
#include "tst_coredump.h"
#include "tst_buffers.h"
#include "tst_capability.h"
#include "tst_hugepage.h"
#include "tst_assert.h"
#include "tst_lockdown.h"
#include "tst_fips.h"
#include "tst_taint.h"
#include "tst_memutils.h"
#include "tst_arch.h"

/*
 * Reports testcase result.
 */
void tst_res_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
              __attribute__ ((format (printf, 4, 5)));

#define tst_res(ttype, arg_fmt, ...) \
	({									\
		TST_RES_SUPPORTS_TCONF_TFAIL_TINFO_TPASS_TWARN(!((TTYPE_RESULT(ttype) ?: TCONF) & \
			(TCONF | TFAIL | TINFO | TPASS | TWARN))); 				\
		tst_res_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__);\
	})

void tst_resm_hexd_(const char *file, const int lineno, int ttype,
	const void *buf, size_t size, const char *arg_fmt, ...)
	__attribute__ ((format (printf, 6, 7)));

#define tst_res_hexd(ttype, buf, size, arg_fmt, ...) \
	tst_resm_hexd_(__FILE__, __LINE__, (ttype), (buf), (size), \
			(arg_fmt), ##__VA_ARGS__)

/*
 * Reports result and exits a test.
 */
void tst_brk_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
              __attribute__ ((format (printf, 4, 5)));

#define tst_brk(ttype, arg_fmt, ...)						\
	({									\
		TST_BRK_SUPPORTS_ONLY_TCONF_TBROK(!((ttype) &			\
			(TBROK | TCONF | TFAIL))); 				\
		tst_brk_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__);\
	})

void tst_printf(const char *const fmt, ...)
		__attribute__((nonnull(1), format (printf, 1, 2)));

/* flush stderr and stdout */
void tst_flush(void);

pid_t safe_fork(const char *filename, unsigned int lineno);
#define SAFE_FORK() \
	safe_fork(__FILE__, __LINE__)

#define TST_TRACE(expr)	                                            \
	({int ret = expr;                                           \
	  ret != 0 ? tst_res(TINFO, #expr " failed"), ret : ret; }) \

#include "tst_safe_macros.h"
#include "tst_safe_file_ops.h"
#include "tst_safe_net.h"
#include "tst_clone.h"
#include "tst_cgroup.h"

/*
 * Wait for all children and exit with TBROK if
 * any of them returned a non-zero exit status.
 */
void tst_reap_children(void);

struct tst_option {
	char *optstr;
	char **arg;
	char *help;
};

/*
 * Options parsing helpers.
 *
 * If str is NULL these are No-op.
 *
 * On failure non-zero (errno) is returned.
 */
int tst_parse_int(const char *str, int *val, int min, int max);
int tst_parse_long(const char *str, long *val, long min, long max);
int tst_parse_float(const char *str, float *val, float min, float max);
int tst_parse_filesize(const char *str, long long *val, long long min, long long max);

struct tst_tag {
	const char *name;
	const char *value;
};

extern unsigned int tst_variant;

#define TST_NO_HUGEPAGES ((unsigned long)-1)

struct tst_test {
	/* number of tests available in test() function */
	unsigned int tcnt;

	struct tst_option *options;

	const char *min_kver;

	/*
	 * The supported_archs is a NULL terminated list of archs the test
	 * does support.
	 */
	const char *const *supported_archs;

	/* If set the test is compiled out */
	const char *tconf_msg;

	int needs_tmpdir:1;
	int needs_root:1;
	int forks_child:1;
	int needs_device:1;
	int needs_checkpoints:1;
	int needs_overlay:1;
	int format_device:1;
	int mount_device:1;
	int needs_rofs:1;
	int child_needs_reinit:1;
	int needs_devfs:1;
	int restore_wallclock:1;
	/*
	 * If set the test function will be executed for all available
	 * filesystems and the current filesytem type would be set in the
	 * tst_device->fs_type.
	 *
	 * The test setup and cleanup are executed before/after __EACH__ call
	 * to the test function.
	 */
	int all_filesystems:1;
	int skip_in_lockdown:1;
	int skip_in_compat:1;

	/*
	 * The skip_filesystem is a NULL terminated list of filesystems the
	 * test does not support. It can also be used to disable whole class of
	 * filesystems with a special keyworks such as "fuse".
	 */
	const char *const *skip_filesystems;

	/* Minimum number of online CPU required by the test */
	unsigned long min_cpus;

	/* Minimum size(MB) of MemAvailable required by the test */
	unsigned long min_mem_avail;

	/*
	 * If set non-zero number of request_hugepages, test will try to reserve the
	 * expected number of hugepage for testing in setup phase. If system does not
	 * have enough hpage for using, it will try the best to reserve 80% available
	 * number of hpages. With success test stores the reserved hugepage number in
	 * 'tst_hugepages. For the system without hugetlb supporting, variable
	 * 'tst_hugepages' will be set to 0. If the hugepage number needs to be set to
	 * 0 on supported hugetlb system, please use '.request_hugepages = TST_NO_HUGEPAGES'.
	 *
	 * Also, we do cleanup and restore work for the hpages resetting automatically.
	 */
	unsigned long request_hugepages;

	/*
	 * If set to non-zero, call tst_taint_init(taint_check) during setup
	 * and check kernel taint at the end of the test. If all_filesystems
	 * is non-zero, taint check will be performed after each FS test and
	 * testing will be terminated by TBROK if taint is detected.
	 */
	unsigned int taint_check;

	/*
	 * If set non-zero denotes number of test variant, the test is executed
	 * variants times each time with tst_variant set to different number.
	 *
	 * This allows us to run the same test for different settings. The
	 * intended use is to test different syscall wrappers/variants but the
	 * API is generic and does not limit the usage in any way.
	 */
	unsigned int test_variants;

	/* Minimal device size in megabytes */
	unsigned int dev_min_size;

	/* Device filesystem type override NULL == default */
	const char *dev_fs_type;

	/* Options passed to SAFE_MKFS() when format_device is set */
	const char *const *dev_fs_opts;
	const char *const *dev_extra_opts;

	/* Device mount options, used if mount_device is set */
	const char *mntpoint;
	unsigned int mnt_flags;
	void *mnt_data;

	/* override default timeout per test run, disabled == -1 */
	int timeout;

	void (*setup)(void);
	void (*cleanup)(void);

	void (*test)(unsigned int test_nr);
	void (*test_all)(void);

	/* Syscall name used by the timer measurement library */
	const char *scall;

	/* Sampling function for timer measurement testcases */
	int (*sample)(int clk_id, long long usec);

	/* NULL terminated array of resource file names */
	const char *const *resource_files;

	/* NULL terminated array of needed kernel drivers */
	const char * const *needs_drivers;

	/*
	 * NULL terminated array of (/proc, /sys) files to save
	 * before setup and restore after cleanup
	 */
	const char * const *save_restore;

	/*
	 * NULL terminated array of kernel config options required for the
	 * test.
	 */
	const char *const *needs_kconfigs;

	/*
	 * NULL-terminated array to be allocated buffers.
	 */
	struct tst_buffers *bufs;

	/*
	 * NULL-terminated array of capability settings
	 */
	struct tst_cap *caps;

	/*
	 * {NULL, NULL} terminated array of tags.
	 */
	const struct tst_tag *tags;

	/* NULL terminated array of required commands */
	const char *const *needs_cmds;

	/* Requires a particular CGroup API version. */
	const enum tst_cgroup_ver needs_cgroup_ver;

	/* {} terminated array of required CGroup controllers */
	const char *const *needs_cgroup_ctrls;
};

/*
 * Runs tests.
 */
void tst_run_tcases(int argc, char *argv[], struct tst_test *self)
                    __attribute__ ((noreturn));

#define IPC_ENV_VAR "LTP_IPC_PATH"

/*
 * Does library initialization for child processes started by exec()
 *
 * The LTP_IPC_PATH variable must be passed to the program environment.
 */
void tst_reinit(void);

/*
 * Functions to convert ERRNO to its name and SIGNAL to its name.
 */
const char *tst_strerrno(int err);
const char *tst_strsig(int sig);
/*
 * Returns string describing status as returned by wait().
 *
 * BEWARE: Not thread safe.
 */
const char *tst_strstatus(int status);

unsigned int tst_timeout_remaining(void);
unsigned int tst_multiply_timeout(unsigned int timeout);
void tst_set_timeout(int timeout);


/*
 * Returns path to the test temporary directory in a newly allocated buffer.
 */
char *tst_get_tmpdir(void);

#ifndef TST_NO_DEFAULT_MAIN

static struct tst_test test;

int main(int argc, char *argv[])
{
	tst_run_tcases(argc, argv, &test);
}

#endif /* TST_NO_DEFAULT_MAIN */

#define TST_TEST_TCONF(message)                                 \
        static struct tst_test test = { .tconf_msg = message  } \

#endif	/* TST_TEST_H__ */
