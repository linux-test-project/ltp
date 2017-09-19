/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TST_TEST_H__
#define TST_TEST_H__

#ifdef __TEST_H__
# error Oldlib test.h already included
#endif /* __TEST_H__ */

#include <unistd.h>
#include <limits.h>

#include "tst_common.h"
#include "tst_res_flags.h"
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
#include "tst_clone.h"
#include "tst_kernel.h"
#include "tst_minmax.h"

/*
 * Reports testcase result.
 */
void tst_res_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
              __attribute__ ((format (printf, 4, 5)));

#define tst_res(ttype, arg_fmt, ...) \
	tst_res_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__)

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

#define tst_brk(ttype, arg_fmt, ...) \
	tst_brk_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__)

pid_t safe_fork(const char *filename, unsigned int lineno);
#define SAFE_FORK() \
	safe_fork(__FILE__, __LINE__)

#define TST_TRACE(expr)	                                            \
	({int ret = expr;                                           \
	  ret != 0 ? tst_res(TINFO, #expr " failed"), ret : ret; }) \

#include "tst_safe_macros.h"
#include "tst_safe_file_ops.h"
#include "tst_safe_net.h"

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

struct tst_test {
	/* test id usually the same as test filename without file suffix */
	const char *tid;
	/* number of tests available in test() function */
	unsigned int tcnt;

	struct tst_option *options;

	const char *min_kver;

	/* If set the test is compiled out */
	const char *tconf_msg;

	int needs_tmpdir:1;
	int needs_root:1;
	int forks_child:1;
	int needs_device:1;
	int needs_checkpoints:1;
	int format_device:1;
	int mount_device:1;
	int needs_rofs:1;

	/* Minimal device size in megabytes */
	unsigned int dev_min_size;

	/* Device filesystem type override NULL == default */
	const char *dev_fs_type;

	/* Options passed to SAFE_MKFS() when format_device is set */
	const char *const *dev_fs_opts;
	const char *dev_extra_opt;

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

	/* Sampling function for timer measurement testcases */
	int (*sample)(int clk_id, long long usec);

	/* NULL terminated array of resource file names */
	const char *const *resource_files;
};

/*
 * Runs tests.
 */
void tst_run_tcases(int argc, char *argv[], struct tst_test *self)
                    __attribute__ ((noreturn));

/*
 * Does library initialization for child processes started by exec()
 *
 * The LTP_IPC_PATH variable must be passed to the program environment.
 */
void tst_reinit(void);

//TODO Clean?
#define TEST(SCALL) \
	do { \
		errno = 0; \
		TEST_RETURN = SCALL; \
		TEST_ERRNO = errno; \
	} while (0)

extern long TEST_RETURN;
extern int TEST_ERRNO;

/*
 * Functions to convert ERRNO to its name and SIGNAL to its name.
 */
const char *tst_strerrno(int err);
const char *tst_strsig(int sig);

void tst_set_timeout(int timeout);

#ifndef TST_NO_DEFAULT_MAIN

static struct tst_test test;

int main(int argc, char *argv[])
{
	tst_run_tcases(argc, argv, &test);
}

#endif /* TST_NO_DEFAULT_MAIN */

#define TST_TEST_TCONF(message)                                 \
        static struct tst_test test = { .tconf_msg = message  } \
/*
 * This is a hack to make the testcases link without defining TCID
 */
const char *TCID;

#endif	/* TST_TEST_H__ */
