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

#include <unistd.h>

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

/*
 * Reports testcase result.
 */
void tst_res_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
              __attribute__ ((format (printf, 4, 5)));

#define tst_res(ttype, arg_fmt, ...) \
	tst_res_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__)

/*
 * Reports result and exits a test.
 */
void tst_brk_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
              __attribute__ ((format (printf, 4, 5)))
              __attribute__ ((noreturn));

#define tst_brk(ttype, arg_fmt, ...) \
	tst_brk_(__FILE__, __LINE__, (ttype), (arg_fmt), ##__VA_ARGS__)

pid_t safe_fork(const char *filename, unsigned int lineno);
#define SAFE_FORK() \
	safe_fork(__FILE__, __LINE__)

#include "tst_safe_macros.h"
#include "tst_safe_file_ops.h"
#include "tst_safe_net.h"
#include "tst_safe_pthread.h"

struct tst_option {
	char *optstr;
	char **arg;
	char *help;
};

struct tst_test {
	/* test id usually the same as test filename without file suffix */
	const char *tid;
	/* number of tests available in test() function */
	unsigned int tcnt;

	struct tst_option *options;

	const char *min_kver;

	int needs_tmpdir:1;
	int needs_root:1;
	int forks_child:1;
	int needs_device:1;
	int needs_checkpoints:1;

	/* override default timeout per test run */
	unsigned int timeout;

	void (*setup)(void);
	void (*cleanup)(void);

	void (*test)(unsigned int test_nr);
	void (*test_all)(void);

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

#ifndef TST_NO_DEFAULT_MAIN

static struct tst_test test;

int main(int argc, char *argv[])
{
	tst_run_tcases(argc, argv, &test);
}

#endif /* TST_NO_DEFAULT_MAIN */

#define TST_TEST_TCONF(message)                                           \
	static void tst_do_test(void) { tst_brk(TCONF, "%s", message); }; \
        static struct tst_test test = { .test_all = tst_do_test }         \

/*
 * This is a hack to make the testcases link without defining TCID
 */
const char *TCID;

#endif	/* TST_TEST_H__ */
