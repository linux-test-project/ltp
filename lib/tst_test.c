// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2025 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2016-2025
 */

#define _GNU_SOURCE

#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <math.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_device.h"
#include "lapi/abisize.h"
#include "lapi/futex.h"
#include "lapi/syscalls.h"
#include "tst_ansi_color.h"
#include "tst_safe_stdio.h"
#include "tst_timer_test.h"
#include "tst_clocks.h"
#include "tst_timer.h"
#include "tst_wallclock.h"
#include "tst_sys_conf.h"
#include "tst_kconfig.h"
#include "tst_private.h"
#include "tso_resource.h"
#include "tso_device.h"
#include "tso_tmpdir.h"
#include "ltp-version.h"
#include "tst_hugepage.h"

/*
 * Hack to get TCID defined in newlib tests
 */
const char *TCID __attribute__((weak));

/* update also doc/conf.py */
#define LINUX_GIT_URL "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id="
#define LINUX_STABLE_GIT_URL "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/commit/?id="
#define GLIBC_GIT_URL "https://sourceware.org/git/?p=glibc.git;a=commit;h="
#define MUSL_GIT_URL "https://git.musl-libc.org/cgit/musl/commit/src/linux/clone.c?id="
#define CVE_DB_URL "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-"

#define DEFAULT_TIMEOUT 30

/* Magic number is "LTPM" */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define LTP_MAGIC 0x4C54504D
#else
# define LTP_MAGIC 0x4D50544C
#endif

struct tst_test *tst_test;

static const char *tcid;
static int iterations = 1;
static float duration = -1;
static float timeout_mul = -1;
static int reproducible_output;
static int quiet_output;

struct context {
	int32_t lib_pid;
	int32_t main_pid;
	struct timespec start_time;
	int32_t runtime;
	int32_t overall_time;
	/*
	 * This is set by a call to tst_brk() with TBROK parameter and means
	 * that the test should exit immediately.
	 */
	tst_atomic_t abort_flag;
	uint32_t mntpoint_mounted:1;
	uint32_t ovl_mounted:1;
	uint32_t tdebug:1;
};

struct results {
	tst_atomic_t passed;
	tst_atomic_t skipped;
	tst_atomic_t failed;
	tst_atomic_t warnings;
	tst_atomic_t broken;
};

struct ipc_region {
	int32_t magic;
	struct context context;
	struct results results;
	futex_t futexes[];
};

static struct ipc_region *ipc;
static struct context *context;
static struct results *results;

extern volatile void *tst_futexes;
extern unsigned int tst_max_futexes;

static int ipc_fd;
static char ipc_path[1064];

static char shm_path[1024];

int TST_ERR;
int TST_PASS;
long TST_RET;

static void do_cleanup(void);
static void do_exit(int ret) __attribute__ ((noreturn));

static void setup_ipc(void)
{
	size_t size = getpagesize();

	if (access("/dev/shm", F_OK) == 0) {
		snprintf(shm_path, sizeof(shm_path), "/dev/shm/ltp_%s_%d",
			 tcid, getpid());
	} else {
		char *tmpdir;

		if (!tst_tmpdir_created())
			tst_tmpdir();

		tmpdir = tst_get_tmpdir();
		snprintf(shm_path, sizeof(shm_path), "%s/ltp_%s_%d",
			 tmpdir, tcid, getpid());
		free(tmpdir);
	}

	ipc_fd = open(shm_path, O_CREAT | O_EXCL | O_RDWR, 0600);
	if (ipc_fd < 0)
		tst_brk(TBROK | TERRNO, "open(%s)", shm_path);
	SAFE_CHMOD(shm_path, 0666);

	SAFE_FTRUNCATE(ipc_fd, size);

	ipc = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, ipc_fd, 0);

	SAFE_CLOSE(ipc_fd);

	memset(ipc, 0, size);

	ipc->magic = LTP_MAGIC;
	context = &ipc->context;
	results = &ipc->results;
	context->lib_pid = getpid();

	if (tst_test->needs_checkpoints) {
		tst_futexes = ipc->futexes;
		tst_max_futexes = (size - offsetof(struct ipc_region, futexes)) / sizeof(futex_t);
	}

	/* Set environment variable for exec()'d children */
	if (tst_test->needs_checkpoints || tst_test->child_needs_reinit) {
		snprintf(ipc_path, sizeof(ipc_path), IPC_ENV_VAR "=%s", shm_path);
		putenv(ipc_path);
	} else {
		SAFE_UNLINK(shm_path);
	}
}

static void cleanup_ipc(void)
{
	size_t size = getpagesize();

	if (ipc_fd > 0 && close(ipc_fd))
		tst_res(TWARN | TERRNO, "close(ipc_fd) failed");

	if (shm_path[0] && !access(shm_path, F_OK) && unlink(shm_path))
		tst_res(TWARN | TERRNO, "unlink(%s) failed", shm_path);

	if (ipc) {
		msync((void *)ipc, size, MS_SYNC);
		munmap((void *)ipc, size);
		ipc = NULL;
		context = NULL;
		results = NULL;
	}
}

void tst_reinit(void)
{
	const char *path = getenv(IPC_ENV_VAR);
	size_t size = getpagesize();
	int fd;

	if (!path)
		tst_brk(TBROK, IPC_ENV_VAR" is not defined");

	if (access(path, F_OK))
		tst_brk(TBROK, "File %s does not exist!", path);

	fd = SAFE_OPEN(path, O_RDWR);
	ipc = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	SAFE_CLOSE(fd);

	if (ipc->magic != LTP_MAGIC)
		tst_brk(TBROK, "Invalid shared memory region (bad magic)");

	/* Restore the parent context from IPC region */
	context = &ipc->context;
	results = &ipc->results;

	tst_futexes = ipc->futexes;
	tst_max_futexes = (size - offsetof(struct ipc_region, futexes)) / sizeof(futex_t);

	if (context->tdebug)
		tst_res(TINFO, "Restored metadata for PID %d", getpid());
}

extern char **environ;

static unsigned int params_array_len(char *const array[])
{
	unsigned int ret = 0;

	if (!array)
		return 0;

	while (*(array++))
		ret++;

	return ret;
}

int tst_run_script(const char *script_name, char *const params[])
{
	int pid;
	unsigned int i, params_len = params_array_len(params);
	char *argv[params_len + 2];

	if (!tst_test->runs_script)
		tst_brk(TBROK, "runs_script flag must be set!");

	argv[0] = (char *)script_name;

	if (params) {
		for (i = 0; i < params_len; i++)
			argv[i+1] = params[i];
	}

	argv[params_len+1] = NULL;

	pid = SAFE_FORK();
	if (pid)
		return pid;

	execvpe(script_name, argv, environ);

	tst_brk(TBROK | TERRNO, "execvpe(%s, ...) failed!", script_name);

	return -1;
}

static void update_results(int ttype)
{
	if (!results)
		return;

	switch (ttype) {
	case TCONF:
		tst_atomic_inc(&results->skipped);
	break;
	case TPASS:
		tst_atomic_inc(&results->passed);
	break;
	case TWARN:
		tst_atomic_inc(&results->warnings);
	break;
	case TFAIL:
		tst_atomic_inc(&results->failed);
	break;
	case TBROK:
		tst_atomic_inc(&results->broken);
	break;
	}
}

static void print_result(const char *file, const int lineno, int ttype,
			 const char *fmt, va_list va)
{
	char buf[1024];
	char *str = buf;
	int ret, size = sizeof(buf), ssize, int_errno, buflen;
	const char *str_errno = NULL;
	const char *res;

	switch (TTYPE_RESULT(ttype)) {
	case TPASS:
		res = "TPASS";
	break;
	case TFAIL:
		res = "TFAIL";
	break;
	case TBROK:
		res = "TBROK";
	break;
	case TCONF:
		if (quiet_output)
			return;
		res = "TCONF";
	break;
	case TWARN:
		res = "TWARN";
	break;
	case TINFO:
		if (quiet_output)
			return;
		res = "TINFO";
	break;
	case TDEBUG:
		if (quiet_output)
			return;
		res = "TDEBUG";
	break;
	default:
		tst_brk(TBROK, "Invalid ttype value %i", ttype);
		abort();
	}

	if (ttype & TERRNO) {
		str_errno = tst_strerrno(errno);
		int_errno = errno;
	}

	if (ttype & TTERRNO) {
		str_errno = tst_strerrno(TST_ERR);
		int_errno = TST_ERR;
	}

	if (ttype & TRERRNO) {
		int_errno = TST_RET < 0 ? -(int)TST_RET : (int)TST_RET;
		str_errno = tst_strerrno(int_errno);
	}

	ret = snprintf(str, size, "%s:%i: ", file, lineno);
	str += ret;
	size -= ret;

	if (tst_color_enabled(STDERR_FILENO))
		ret = snprintf(str, size, "%s%s: %s", tst_ttype2color(ttype),
			       res, ANSI_COLOR_RESET);
	else
		ret = snprintf(str, size, "%s: ", res);
	str += ret;
	size -= ret;

	if (reproducible_output)
		goto print;

	ssize = size - 2;
	ret = vsnprintf(str, size, fmt, va);
	str += MIN(ret, ssize);
	size -= MIN(ret, ssize);
	if (ret >= ssize) {
		tst_res_(file, lineno, TWARN,
				"Next message is too long and truncated:");
	} else if (str_errno) {
		ssize = size - 2;
		ret = snprintf(str, size, ": %s (%d)", str_errno, int_errno);
		str += MIN(ret, ssize);
		size -= MIN(ret, ssize);
		if (ret >= ssize)
			tst_res_(file, lineno, TWARN,
				"Next message is too long and truncated:");
	}

print:
	snprintf(str, size, "\n");

	/* we might be called from signal handler, so use write() */
	buflen = str - buf + 1;
	str = buf;
	while (buflen) {
		ret = write(STDERR_FILENO, str, buflen);
		if (ret <= 0)
			break;

		str += ret;
		buflen -= ret;
	}
}

void tst_vres_(const char *file, const int lineno, int ttype, const char *fmt,
	       va_list va)
{
	print_result(file, lineno, ttype, fmt, va);

	update_results(TTYPE_RESULT(ttype));
}

void tst_vbrk_(const char *file, const int lineno, int ttype, const char *fmt,
	       va_list va);

static void (*tst_brk_handler)(const char *file, const int lineno, int ttype,
			       const char *fmt, va_list va) = tst_vbrk_;

static void tst_cvres(const char *file, const int lineno, int ttype,
		      const char *fmt, va_list va)
{
	if (TTYPE_RESULT(ttype) == TBROK) {
		ttype &= ~TTYPE_MASK;
		ttype |= TWARN;
	}

	print_result(file, lineno, ttype, fmt, va);
	update_results(TTYPE_RESULT(ttype));
}

static void do_test_cleanup(void)
{
	tst_brk_handler = tst_cvres;

	if (tst_test->cleanup)
		tst_test->cleanup();

	tst_free_all();

	tst_brk_handler = tst_vbrk_;
}

void tst_vbrk_(const char *file, const int lineno, int ttype, const char *fmt,
	       va_list va)
{
	print_result(file, lineno, ttype, fmt, va);

	/*
	 * If tst_brk() is called from some of the C helpers even before the
	 * library was initialized, just exit.
	 */
	if (!results || !context->lib_pid)
		exit(TTYPE_RESULT(ttype));

	update_results(TTYPE_RESULT(ttype));

	/*
	 * The getpid implementation in some C library versions may cause cloned
	 * test threads to show the same pid as their parent when CLONE_VM is
	 * specified but CLONE_THREAD is not. Use direct syscall to avoid
	 * cleanup running in the child.
	 */
	if (tst_getpid() == context->main_pid)
		do_test_cleanup();

	/*
	 * The test library process reports result statistics and exits.
	 */
	if (getpid() == context->lib_pid)
		do_exit(TTYPE_RESULT(ttype));

	/*
	 * If we get here we are in a child process, either the main child
	 * running the test or its children. If any of them called tst_brk()
	 * with TBROK we need to exit the test. Otherwise we just exit the
	 * current process.
	 */
	if (TTYPE_RESULT(ttype) == TBROK) {
		if (results)
			tst_atomic_inc(&context->abort_flag);

		/*
		 * If TBROK was called from one of the child processes we kill
		 * the main test process. That in turn triggers the code that
		 * kills leftover children once the main test process did exit.
		 */
		if (context->main_pid && tst_getpid() != context->main_pid) {
			tst_res(TINFO, "Child process reported TBROK killing the test");
			kill(context->main_pid, SIGKILL);
		}
	}

	exit(0);
}

void tst_res_(const char *file, const int lineno, int ttype,
	      const char *fmt, ...)
{
	va_list va;

	/*
	 * Suppress TDEBUG output in these cases:
	 * 1. No context available (e.g., called before IPC initialization)
	 * 2. Called from the library process, unless explicitly enabled
	 * 3. Debug output is not enabled (context->tdebug == 0)
	 */
	if (ttype == TDEBUG) {
		if (!context)
			return;

		if (context->lib_pid == getpid())
			return;

		if (!context->tdebug)
			return;
	}

	va_start(va, fmt);
	tst_vres_(file, lineno, ttype, fmt, va);
	va_end(va);
}

void tst_brk_(const char *file, const int lineno, int ttype,
	      const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	tst_brk_handler(file, lineno, ttype, fmt, va);
	va_end(va);
}

void tst_printf(const char *const fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vdprintf(STDERR_FILENO, fmt, va);
	va_end(va);
}

static void check_child_status(pid_t pid, int status)
{
	if (WIFSIGNALED(status)) {
		tst_brk(TBROK, "Child (%i) killed by signal %s", pid,
			tst_strsig(WTERMSIG(status)));
	}

	if (!(WIFEXITED(status)))
		tst_brk(TBROK, "Child (%i) exited abnormally", pid);

	if (WEXITSTATUS(status))
		tst_brk(TBROK, "Invalid child (%i) exit value %i", pid, WEXITSTATUS(status));
}

void tst_reap_children(void)
{
	int status;
	pid_t pid;

	for (;;) {
		pid = wait(&status);

		if (pid > 0) {
			check_child_status(pid, status);
			continue;
		}

		if (errno == ECHILD)
			break;

		if (errno == EINTR)
			continue;

		tst_brk(TBROK | TERRNO, "wait() failed");
	}
}


pid_t safe_fork(const char *filename, unsigned int lineno)
{
	pid_t pid;

	if (!tst_test->forks_child)
		tst_brk(TBROK, "test.forks_child must be set!");

	tst_flush();

	pid = fork();
	if (pid < 0)
		tst_brk_(filename, lineno, TBROK | TERRNO, "fork() failed");

	if (!pid)
		atexit(tst_free_all);

	return pid;
}

/* too fast creating namespaces => retrying */
#define TST_CHECK_ENOSPC(x) ((x) >= 0 || !(errno == ENOSPC))

pid_t safe_clone(const char *file, const int lineno,
		 const struct tst_clone_args *args)
{
	pid_t pid;

	if (!tst_test->forks_child)
		tst_brk(TBROK, "test.forks_child must be set!");

	pid = TST_RETRY_FUNC(tst_clone(args), TST_CHECK_ENOSPC);

	switch (pid) {
	case -1:
		tst_brk_(file, lineno, TBROK | TERRNO, "clone3 failed");
		break;
	case -2:
		tst_brk_(file, lineno, TBROK | TERRNO, "clone failed");
		return -1;
	}

	if (!pid)
		atexit(tst_free_all);

	return pid;
}

static void parse_mul(float *mul, const char *env_name, float min, float max)
{
	char *str_mul;
	int ret;

	if (*mul > 0)
		return;

	str_mul = getenv(env_name);

	if (!str_mul) {
		*mul = 1;
		return;
	}

	ret = tst_parse_float(str_mul, mul, min, max);
	if (ret) {
		tst_brk(TBROK, "Failed to parse %s: %s",
			env_name, tst_strerrno(ret));
	}
}

static int multiply_runtime(unsigned int runtime)
{
	static float runtime_mul = -1;
	int min_runtime = 1;

	if (tst_test->min_runtime > 0)
		min_runtime = tst_test->min_runtime;

	parse_mul(&runtime_mul, "LTP_RUNTIME_MUL", 0.0099, 100);

	return MAX(runtime * runtime_mul, min_runtime);
}

static struct option {
	char *optstr;
	char *help;
} options[] = {
	{"h",  "-h       Prints this help"},
	{"i:", "-i n     Execute test n times"},
	{"I:", "-I x     Execute test for n seconds"},
	{"D",  "-D       Prints debug information"},
	{"V",  "-V       Prints LTP version"},
};

static void print_help(void)
{
	unsigned int i;
	int timeout, runtime;

	/* see doc/users/setup_tests.rst, which lists also shell API variables */
	fprintf(stderr, "Environment Variables\n");
	fprintf(stderr, "---------------------\n");
	fprintf(stderr, "KCONFIG_PATH             Specify kernel config file\n");
	fprintf(stderr, "KCONFIG_SKIP_CHECK       Skip kernel config check if variable set (not set by default)\n");
	fprintf(stderr, "LTPROOT                  Prefix for installed LTP (default: /opt/ltp)\n");
	fprintf(stderr, "LTP_COLORIZE_OUTPUT      Force colorized output behaviour (y/1 always, n/0: never)\n");
	fprintf(stderr, "LTP_DEV                  Path to the block device to be used (for .needs_device)\n");
	fprintf(stderr, "LTP_DEV_FS_TYPE          Filesystem used for testing (default: %s)\n", DEFAULT_FS_TYPE);
	fprintf(stderr, "LTP_ENABLE_DEBUG         Print debug messages (set 1 or y)\n");
	fprintf(stderr, "LTP_REPRODUCIBLE_OUTPUT  Values 1 or y discard the actual content of the messages printed by the test\n");
	fprintf(stderr, "LTP_QUIET                Values 1 or y will suppress printing TCONF, TWARN, TINFO, and TDEBUG messages\n");
	fprintf(stderr, "LTP_SINGLE_FS_TYPE       Specifies filesystem instead all supported (for .all_filesystems)\n");
	fprintf(stderr, "LTP_FORCE_SINGLE_FS_TYPE Testing only. The same as LTP_SINGLE_FS_TYPE but ignores test skiplist.\n");
	fprintf(stderr, "LTP_TIMEOUT_MUL          Timeout multiplier (must be a number >=1)\n");
	fprintf(stderr, "LTP_RUNTIME_MUL          Runtime multiplier (must be a number >0)\n");
	fprintf(stderr, "LTP_VIRT_OVERRIDE        Overrides virtual machine detection (values: \"\"|kvm|microsoft|xen|zvm)\n");
	fprintf(stderr, "TMPDIR                   Base directory for template directory (for .needs_tmpdir, default: %s)\n", TEMPDIR);
	fprintf(stderr, "\n");

	fprintf(stderr, "Timeout and runtime\n");
	fprintf(stderr, "-------------------\n");

	if (tst_test->timeout == TST_UNLIMITED_TIMEOUT) {
		fprintf(stderr, "Test timeout is not limited\n");
	} else {
		timeout = tst_multiply_timeout(DEFAULT_TIMEOUT + tst_test->timeout);

		fprintf(stderr, "Test timeout (not including runtime) %ih %im %is\n",
				timeout/3600, (timeout%3600)/60, timeout % 60);
	}

	if (tst_test->runtime) {
		runtime = multiply_runtime(tst_test->runtime);

		fprintf(stderr, "Test iteration runtime cap %ih %im %is\n",
				runtime/3600, (runtime%3600)/60, runtime % 60);
	}

	fprintf(stderr, "\n");

	fprintf(stderr, "Options\n");
	fprintf(stderr, "-------\n");

	for (i = 0; i < ARRAY_SIZE(options); i++)
		fprintf(stderr, "%s\n", options[i].help);

	if (!tst_test->options)
		return;

	for (i = 0; tst_test->options[i].optstr; i++) {
		fprintf(stderr, "-%c\t %s\n",
			tst_test->options[i].optstr[0],
			tst_test->options[i].help);
	}
}

static void print_test_tags(void)
{
	unsigned int i;
	const struct tst_tag *tags = tst_test->tags;

	if (!tags)
		return;

	fprintf(stderr, "\nTags\n");
	fprintf(stderr, "----\n");

	for (i = 0; tags[i].name; i++) {
		if (!strcmp(tags[i].name, "CVE"))
			fprintf(stderr, CVE_DB_URL "%s\n", tags[i].value);
		else if (!strcmp(tags[i].name, "linux-git"))
			fprintf(stderr, LINUX_GIT_URL "%s\n", tags[i].value);
		else if (!strcmp(tags[i].name, "linux-stable-git"))
			fprintf(stderr, LINUX_STABLE_GIT_URL "%s\n", tags[i].value);
		else if (!strcmp(tags[i].name, "glibc-git"))
			fprintf(stderr, GLIBC_GIT_URL "%s\n", tags[i].value);
		else if (!strcmp(tags[i].name, "musl-git"))
			fprintf(stderr, MUSL_GIT_URL "%s\n", tags[i].value);
		else
			fprintf(stderr, "%s: %s\n", tags[i].name, tags[i].value);
	}

	fprintf(stderr, "\n");
}

static void check_option_collision(void)
{
	unsigned int i, j;
	struct tst_option *toptions = tst_test->options;

	if (!toptions)
		return;

	for (i = 0; toptions[i].optstr; i++) {
		for (j = 0; j < ARRAY_SIZE(options); j++) {
			if (toptions[i].optstr[0] == options[j].optstr[0]) {
				tst_brk(TBROK, "Option collision '%s'",
					options[j].help);
			}
		}
	}
}

static unsigned int count_options(void)
{
	unsigned int i;

	if (!tst_test->options)
		return 0;

	for (i = 0; tst_test->options[i].optstr; i++)
		;

	return i;
}

static void parse_topt(unsigned int topts_len, int opt, char *optarg)
{
	unsigned int i;
	struct tst_option *toptions = tst_test->options;

	for (i = 0; i < topts_len; i++) {
		if (toptions[i].optstr[0] == opt)
			break;
	}

	if (i >= topts_len)
		tst_brk(TBROK, "Invalid option '%c' (should not happen)", opt);

	if (*toptions[i].arg)
		tst_res(TWARN, "Option -%c passed multiple times", opt);

	*(toptions[i].arg) = optarg ? optarg : "";
}

static void parse_opts(int argc, char *argv[])
{
	unsigned int i, topts_len = count_options();
	char optstr[2 * ARRAY_SIZE(options) + 2 * topts_len];
	int opt;

	check_option_collision();

	optstr[0] = 0;

	for (i = 0; i < ARRAY_SIZE(options); i++)
		strcat(optstr, options[i].optstr);

	for (i = 0; i < topts_len; i++)
		strcat(optstr, tst_test->options[i].optstr);

	while ((opt = getopt(argc, argv, optstr)) > 0) {
		switch (opt) {
		case '?':
			print_help();
			tst_brk(TBROK, "Invalid option");
		break;
		case 'D':
			tst_res(TINFO, "Enabling debug info");
			context->tdebug = 1;
		break;
		case 'h':
			print_help();
			print_test_tags();
			exit(0);
		case 'i':
			iterations = SAFE_STRTOL(optarg, 0, INT_MAX);
		break;
		case 'I':
			if (tst_test->runtime > 0)
				tst_test->runtime = SAFE_STRTOL(optarg, 1, INT_MAX);
			else
				duration = SAFE_STRTOF(optarg, 0.1, HUGE_VALF);
		break;
		case 'V':
			fprintf(stderr, "LTP version: " LTP_VERSION "\n");
			exit(0);
		break;
		default:
			parse_topt(topts_len, opt, optarg);
		}
	}

	if (optind < argc)
		tst_brk(TBROK, "Unexpected argument(s) '%s'...", argv[optind]);
}

int tst_parse_int(const char *str, int *val, int min, int max)
{
	long rval;

	if (!str)
		return 0;

	int ret = tst_parse_long(str, &rval, min, max);

	if (ret)
		return ret;

	*val = (int)rval;
	return 0;
}

int tst_parse_long(const char *str, long *val, long min, long max)
{
	long rval;
	char *end;

	if (!str)
		return 0;

	errno = 0;
	rval = strtol(str, &end, 10);

	if (str == end || *end != '\0')
		return EINVAL;

	if (errno)
		return errno;

	if (rval > max || rval < min)
		return ERANGE;

	*val = rval;
	return 0;
}

int tst_parse_float(const char *str, float *val, float min, float max)
{
	double rval;
	char *end;

	if (!str)
		return 0;

	errno = 0;
	rval = strtod(str, &end);

	if (str == end || *end != '\0')
		return EINVAL;

	if (errno)
		return errno;

	if (rval > (double)max || rval < (double)min)
		return ERANGE;

	*val = (float)rval;
	return 0;
}

int tst_parse_filesize(const char *str, long long *val, long long min, long long max)
{
	long long rval;
	char *end;

	if (!str)
		return 0;

	errno = 0;
	rval = strtoll(str, &end, 0);

	if (str == end || (end[0] && end[1]))
		return EINVAL;

	if (errno)
		return errno;

	switch (*end) {
	case 'g':
	case 'G':
		rval *= (1024 * 1024 * 1024);
		break;
	case 'm':
	case 'M':
		rval *= (1024 * 1024);
		break;
	case 'k':
	case 'K':
		rval *= 1024;
		break;
	default:
		break;
	}

	if (rval > max || rval < min)
		return ERANGE;

	*val = rval;
	return 0;
}

static void print_colored(const char *str)
{
	if (tst_color_enabled(STDOUT_FILENO))
		fprintf(stderr, "%s%s%s", ANSI_COLOR_YELLOW, str, ANSI_COLOR_RESET);
	else
		fprintf(stderr, "%s", str);
}

static void print_failure_hint(const char *tag, const char *hint,
			       const char *url)
{
	const struct tst_tag *tags = tst_test->tags;

	if (!tags)
		return;

	unsigned int i;
	int hint_printed = 0;

	for (i = 0; tags[i].name; i++) {
		if (!strcmp(tags[i].name, tag)) {
			if (!hint_printed) {
				hint_printed = 1;
				fprintf(stderr, "\n");
				print_colored("HINT: ");
				fprintf(stderr, "You _MAY_ be %s:\n\n", hint);
			}

			if (url)
				fprintf(stderr, "%s%s\n", url, tags[i].value);
			else
				fprintf(stderr, "%s\n", tags[i].value);
		}
	}
}

static int show_failure_hints;

/* update also doc/conf.py */
static void print_failure_hints(void)
{
	print_failure_hint("linux-git", "missing kernel fixes", LINUX_GIT_URL);
	print_failure_hint("linux-stable-git", "missing stable kernel fixes",
					   LINUX_STABLE_GIT_URL);
	print_failure_hint("glibc-git", "missing glibc fixes", GLIBC_GIT_URL);
	print_failure_hint("musl-git", "missing musl fixes", MUSL_GIT_URL);
	print_failure_hint("CVE", "vulnerable to CVE(s)", CVE_DB_URL);
	print_failure_hint("known-fail", "hit by known kernel failures", NULL);

	show_failure_hints = 0;
}

/*
 * Prints results, cleans up after the test library and exits the test library
 * process. The ret parameter is used to pass the result flags in a case of a
 * failure before we managed to set up the shared memory where we store the
 * results. This allows us to use SAFE_MACROS() in the initialization of the
 * shared memory. The ret parameter is not used (passed 0) when called
 * explicitly from the rest of the library.
 */
static void do_exit(int ret)
{
	if (results) {
		if (results->passed && ret == TCONF)
			ret = 0;

		if (results->failed) {
			ret |= TFAIL;
			if (show_failure_hints)
				print_failure_hints();
		}

		if (results->skipped && !results->passed)
			ret |= TCONF;

		if (results->warnings)
			ret |= TWARN;

		if (results->broken) {
			ret |= TBROK;
			if (show_failure_hints)
				print_failure_hints();
		}

		fprintf(stderr, "\nSummary:\n");
		fprintf(stderr, "passed   %d\n", results->passed);
		fprintf(stderr, "failed   %d\n", results->failed);
		fprintf(stderr, "broken   %d\n", results->broken);
		fprintf(stderr, "skipped  %d\n", results->skipped);
		fprintf(stderr, "warnings %d\n", results->warnings);
	}

	do_cleanup();

	exit(ret);
}

/*
 * Check for the required kernel version.
 *
 * return: true if the kernel version is high enough, false otherwise.
 */
static bool check_kver(const char *min_kver, const int brk_nosupp)
{
	char *msg;
	int v1, v2, v3;

	if (tst_parse_kver(min_kver, &v1, &v2, &v3)) {
		tst_res(TWARN,
			"Invalid kernel version %s, expected %%d.%%d.%%d",
			min_kver);
	}

	if (tst_kvercmp(v1, v2, v3) < 0) {
		msg = "The test requires kernel %s or newer";

		if (brk_nosupp)
			tst_brk(TCONF, msg, min_kver);
		else
			tst_res(TCONF, msg, min_kver);

		return false;
	}

	return true;
}

/*
 * Checks if the struct results values are equal.
 *
 * return: true if results are equal, false otherwise.
 */
static bool results_equal(struct results *a, struct results *b)
{
	if (a->passed != b->passed)
		return false;

	if (a->failed != b->failed)
		return false;

	if (a->skipped != b->skipped)
		return false;

	if (a->broken != b->broken)
		return false;

	return true;
}

static bool needs_tmpdir(void)
{
	return tst_test->needs_tmpdir ||
	       tst_test->needs_device ||
	       tst_test->mntpoint ||
	       tst_test->resource_files ||
	       tst_test->needs_checkpoints;
}

static void copy_resources(void)
{
	unsigned int i;

	for (i = 0; tst_test->resource_files[i]; i++)
		TST_RESOURCE_COPY(NULL, tst_test->resource_files[i], NULL);
}

static const char *get_tcid(char *argv[])
{
	char *p;

	if (!argv[0] || !argv[0][0]) {
		tst_res(TINFO, "argv[0] is empty!");
		return "ltp_empty_argv";
	}

	p = strrchr(argv[0], '/');
	if (p)
		return p+1;

	return argv[0];
}

static struct tst_device tdev;
struct tst_device *tst_device;

static void assert_test_fn(void)
{
	int cnt = 0;

	if (tst_test->test)
		cnt++;

	if (tst_test->test_all)
		cnt++;

	if (tst_test->sample)
		cnt++;

	if (!cnt)
		tst_brk(TBROK, "No test function specified");

	if (cnt != 1)
		tst_brk(TBROK, "You can define only one test function");

	if (tst_test->test && !tst_test->tcnt)
		tst_brk(TBROK, "Number of tests (tcnt) must be > 0");

	if (!tst_test->test && tst_test->tcnt)
		tst_brk(TBROK, "You can define tcnt only for test()");
}

static int prepare_and_mount_ro_fs(const char *dev, const char *mntpoint,
				   const char *fs_type)
{
	char buf[PATH_MAX];

	if (mount(dev, mntpoint, fs_type, 0, NULL)) {
		tst_res(TINFO | TERRNO, "Can't mount %s at %s (%s)",
			dev, mntpoint, fs_type);
		return 1;
	}

	context->mntpoint_mounted = 1;

	snprintf(buf, sizeof(buf), "%s/dir/", mntpoint);
	SAFE_MKDIR(buf, 0777);

	snprintf(buf, sizeof(buf), "%s/file", mntpoint);
	SAFE_FILE_PRINTF(buf, "file content");
	SAFE_CHMOD(buf, 0777);

	SAFE_MOUNT(dev, mntpoint, fs_type, MS_REMOUNT | MS_RDONLY, NULL);

	return 0;
}

static void prepare_and_mount_dev_fs(const char *mntpoint)
{
	const char *flags[] = {"nodev", NULL};
	int mounted_nodev;

	mounted_nodev = tst_path_has_mnt_flags(NULL, flags);
	if (mounted_nodev) {
		tst_res(TINFO, "tmpdir isn't suitable for creating devices, "
			"mounting tmpfs without nodev on %s", mntpoint);
		SAFE_MOUNT(NULL, mntpoint, "tmpfs", 0, NULL);
		context->mntpoint_mounted = 1;
	}
}

static void prepare_and_mount_hugetlb_fs(void)
{
	if (access(PATH_HUGEPAGES, F_OK))
		tst_brk(TCONF, "hugetlbfs is not supported");

	SAFE_MOUNT("none", tst_test->mntpoint, "hugetlbfs", 0, NULL);
	context->mntpoint_mounted = 1;
}

int tst_creat_unlinked(const char *path, int flags, mode_t mode)
{
	char template[PATH_MAX];
	int len, c, range;
	int fd;
	int start[3] = {'0', 'a', 'A'};

	snprintf(template, PATH_MAX, "%s/ltp_%.3sXXXXXX",
			path, tcid);

	len = strlen(template) - 1;
	while (template[len] == 'X') {
		c = rand() % 3;
		range = start[c] == '0' ? 10 : 26;
		c = start[c] + (rand() % range);
		template[len--] = (char)c;
	}

	flags |= O_CREAT|O_EXCL|O_RDWR;
	fd = SAFE_OPEN(template, flags, mode);
	SAFE_UNLINK(template);
	return fd;
}

static const char *limit_tmpfs_mount_size(const char *mnt_data,
		char *buf, size_t buf_size, const char *fs_type)
{
	unsigned int tmpfs_size;

	if (strcmp(fs_type, "tmpfs"))
		return mnt_data;

	if (!tst_test->dev_min_size)
		tmpfs_size = 32;
	else
		tmpfs_size = tdev.size;

	if ((tst_available_mem() / 1024) < (tmpfs_size * 2))
		tst_brk(TCONF, "No enough memory for tmpfs use");

	if (mnt_data)
		snprintf(buf, buf_size, "%s,size=%uM", mnt_data, tmpfs_size);
	else
		snprintf(buf, buf_size, "size=%uM", tmpfs_size);

	tst_res(TINFO, "Limiting tmpfs size to %uMB", tmpfs_size);

	return buf;
}

static const char *get_device_name(const char *fs_type)
{
	if (!strcmp(fs_type, "tmpfs"))
		return "ltp-tmpfs";
	else
		return tdev.dev;
}

static void prepare_device(struct tst_fs *fs)
{
	const char *mnt_data;
	char buf[1024];
	struct tst_fs dummy = {};

	fs = fs ?: &dummy;

	const char *const extra[] = {fs->mkfs_size_opt, NULL};

	if (tst_test->format_device)
		SAFE_MKFS(tdev.dev, tdev.fs_type, fs->mkfs_opts, extra);

	if (tst_test->needs_rofs) {
		prepare_and_mount_ro_fs(tdev.dev, tst_test->mntpoint,
					tdev.fs_type);
		return;
	}

	if (tst_test->mount_device) {
		mnt_data = limit_tmpfs_mount_size(fs->mnt_data,
				buf, sizeof(buf), tdev.fs_type);

		SAFE_MOUNT2(get_device_name(tdev.fs_type), tst_test->mntpoint,
				tdev.fs_type, fs->mnt_flags, mnt_data, &tdev.is_fuse);
		context->mntpoint_mounted = 1;
	}
}

static void do_cgroup_requires(void)
{
	const struct tst_cg_opts cg_opts = {
		.needs_ver = tst_test->needs_cgroup_ver,
		.needs_nsdelegate = tst_test->needs_cgroup_nsdelegate,
	};
	const char *const *ctrl_names = tst_test->needs_cgroup_ctrls;

	for (; *ctrl_names; ctrl_names++)
		tst_cg_require(*ctrl_names, &cg_opts);

	tst_cg_init();
}

#define tst_set_ulimit(conf) \
	set_ulimit_(__FILE__, __LINE__, (conf))

/*
 * Set resource limits.
 */
static void set_ulimit_(const char *file, const int lineno, const struct tst_ulimit_val *conf)
{
	struct rlimit rlim;

	safe_getrlimit(file, lineno, conf->resource, &rlim);

	rlim.rlim_cur = conf->rlim_cur;

	if (conf->rlim_cur > rlim.rlim_max)
		rlim.rlim_max = conf->rlim_cur;

	tst_res_(file, lineno, TINFO, "Set ulimit resource: %d rlim_cur: %llu rlim_max: %llu",
		conf->resource, (long long unsigned int)rlim.rlim_cur,
		(long long unsigned int)rlim.rlim_max);

	safe_setrlimit(file, lineno, conf->resource, &rlim);
}

static unsigned int count_fs_descs(void)
{
	unsigned int ret = 0;

	if (!tst_test->filesystems)
		return 0;

	/*
	 * First entry is special, if it has zero type it's the default entry
	 * and is either followed by a terminating entry or by filesystem
	 * description(s) plus terminating entry.
	 */
	if (!tst_test->filesystems[0].type)
		ret = 1;

	while (tst_test->filesystems[ret].type)
		ret++;

	return ret;
}

static const char *default_fs_type(void)
{
	if (!tst_test->filesystems)
		return tst_dev_fs_type();

	if (tst_test->filesystems[0].type)
		return tst_test->filesystems[0].type;

	return tst_dev_fs_type();
}

bool tst_cmd_present(const char *cmd)
{
	struct tst_cmd *pcmd = tst_test->needs_cmds;

	if (!cmd || cmd[0] == '\0')
		tst_brk(TBROK, "Invalid cmd");

	while (pcmd->cmd) {
		if (!strcmp(pcmd->cmd, cmd))
			return pcmd->present;

		pcmd++;
	}

	tst_brk(TBROK, "'%s' not checked", cmd);
	return false;
}

static void do_setup(int argc, char *argv[])
{
	char *tdebug_env = getenv("LTP_ENABLE_DEBUG");
	char *reproducible_env = getenv("LTP_REPRODUCIBLE_OUTPUT");
	char *quiet_env = getenv("LTP_QUIET");

	if (!tst_test)
		tst_brk(TBROK, "No tests to run");

	if (tst_test->timeout < -1) {
		tst_brk(TBROK, "Invalid timeout value %i",
			tst_test->timeout);
	}

	if (tst_test->runtime < 0)
		tst_brk(TBROK, "Invalid runtime value %i", tst_test->runtime);

	if (tst_test->tconf_msg)
		tst_brk(TCONF, "%s", tst_test->tconf_msg);

	if (tst_test->supported_archs && !tst_is_on_arch(tst_test->supported_archs))
		tst_brk(TCONF, "This arch '%s' is not supported for test!", tst_arch.name);

	if (tst_test->sample)
		tst_test = tst_timer_test_setup(tst_test);

	if (tst_test->runs_script) {
		tst_test->child_needs_reinit = 1;
		tst_test->forks_child = 1;
	}

	if (reproducible_env &&
	    (!strcmp(reproducible_env, "1") || !strcmp(reproducible_env, "y")))
		reproducible_output = 1;

	if (quiet_env &&
	    (!strcmp(quiet_env, "1") || !strcmp(quiet_env, "y")))
		quiet_output = 1;

	assert_test_fn();

	TCID = tcid = get_tcid(argv);

	setup_ipc();

	parse_opts(argc, argv);

	if (tdebug_env && (!strcmp(tdebug_env, "1") || !strcmp(tdebug_env, "y"))) {
		tst_res(TINFO, "Enabling debug info");
		context->tdebug = 1;
	}

	if (tst_test->needs_kconfigs && tst_kconfig_check(tst_test->needs_kconfigs))
		tst_brk(TCONF, "Aborting due to unsuitable kernel config, see above!");

	if (tst_test->needs_root && geteuid() != 0)
		tst_brk(TCONF, "Test needs to be run as root");

	if (tst_test->min_kver)
		check_kver(tst_test->min_kver, 1);

	if (tst_test->skip_in_lockdown && tst_lockdown_enabled() > 0)
		tst_brk(TCONF, "Kernel is locked down, skipping test");

	if (tst_test->skip_in_secureboot && tst_secureboot_enabled() > 0)
		tst_brk(TCONF, "SecureBoot enabled, skipping test");

	if (tst_test->skip_in_compat && tst_is_compat_mode())
		tst_brk(TCONF, "Not supported in 32-bit compat mode");

	if (tst_test->needs_abi_bits && !tst_abi_bits(tst_test->needs_abi_bits))
		tst_brk(TCONF, "%dbit ABI is not supported", tst_test->needs_abi_bits);

	if (tst_test->needs_cmds) {
		struct tst_cmd *pcmd = tst_test->needs_cmds;

		while (pcmd->cmd) {
			pcmd->present = tst_check_cmd(pcmd->cmd, !pcmd->optional) ? 1 : 0;
			pcmd++;
		}
	}

	if (tst_test->needs_drivers) {
		const char *name;
		int i;

		for (i = 0; (name = tst_test->needs_drivers[i]); ++i)
			if (tst_check_driver(name))
				tst_brk(TCONF, "%s driver not available", name);
	}

	if (tst_test->mount_device)
		tst_test->format_device = 1;

	if (tst_test->format_device)
		tst_test->needs_device = 1;

	if (tst_test->all_filesystems)
		tst_test->needs_device = 1;

	if (tst_test->min_cpus > (unsigned long)tst_ncpus())
		tst_brk(TCONF, "Test needs at least %lu CPUs online", tst_test->min_cpus);

	if (tst_test->min_mem_avail > (unsigned long)(tst_available_mem() / 1024))
		tst_brk(TCONF, "Test needs at least %luMB MemAvailable", tst_test->min_mem_avail);

	if (tst_test->min_swap_avail > (unsigned long)(tst_available_swap() / 1024))
		tst_brk(TCONF, "Test needs at least %luMB SwapFree", tst_test->min_swap_avail);

	if (tst_test->hugepages.number)
		tst_reserve_hugepages(&tst_test->hugepages);

	if (tst_test->bufs)
		tst_buffers_alloc(tst_test->bufs);

	if (needs_tmpdir() && !tst_tmpdir_created())
		tst_tmpdir();

	if (tst_test->save_restore) {
		const struct tst_path_val *pvl = tst_test->save_restore;

		while (pvl->path) {
			tst_sys_conf_save(pvl);
			pvl++;
		}
	}

	if (tst_test->ulimit) {
		const struct tst_ulimit_val *pvl = tst_test->ulimit;

		while (pvl->resource) {
			tst_set_ulimit(pvl);
			pvl++;
		}
	}

	if (tst_test->mntpoint)
		SAFE_MKDIR(tst_test->mntpoint, 0777);

	if ((tst_test->needs_devfs || tst_test->needs_rofs ||
	     tst_test->mount_device || tst_test->all_filesystems ||
		 tst_test->needs_hugetlbfs) &&
	     !tst_test->mntpoint) {
		tst_brk(TBROK, "tst_test->mntpoint must be set!");
	}

	if (!!tst_test->needs_rofs + !!tst_test->needs_devfs +
	    !!tst_test->needs_device + !!tst_test->needs_hugetlbfs > 1) {
		tst_brk(TBROK,
			"Two or more of needs_{rofs, devfs, device, hugetlbfs} are set");
	}

	if (tst_test->needs_devfs)
		prepare_and_mount_dev_fs(tst_test->mntpoint);

	if (tst_test->needs_rofs) {
		/* If we failed to mount read-only tmpfs. Fallback to
		 * using a device with read-only filesystem.
		 */
		if (prepare_and_mount_ro_fs(NULL, tst_test->mntpoint, "tmpfs")) {
			tst_res(TINFO, "Can't mount tmpfs read-only, "
				"falling back to block device...");
			tst_test->needs_device = 1;
			tst_test->format_device = 1;
		}
	}

	if (tst_test->needs_hugetlbfs)
		prepare_and_mount_hugetlb_fs();

	if (tst_test->needs_device && !context->mntpoint_mounted) {
		tdev.dev = tst_acquire_device_(NULL, tst_test->dev_min_size);

		if (!tdev.dev)
			tst_brk(TCONF, "Failed to acquire device");

		tdev.size = tst_get_device_size(tdev.dev);

		tst_device = &tdev;

		tdev.fs_type = default_fs_type();

		if (!tst_test->all_filesystems && count_fs_descs() <= 1) {

			if (!tst_fs_is_supported(tdev.fs_type))
				tst_brk(TCONF, "The %s filesystem is not supported", tdev.fs_type);

			if (tst_test->filesystems && tst_test->filesystems->mkfs_ver)
				tst_check_cmd(tst_test->filesystems->mkfs_ver, 1);

			if (tst_test->filesystems && tst_test->filesystems->min_kver)
				check_kver(tst_test->filesystems->min_kver, 1);

			prepare_device(tst_test->filesystems);
		}
	}

	if (tst_test->needs_overlay && !tst_test->mount_device)
		tst_brk(TBROK, "tst_test->mount_device must be set");

	if (tst_test->needs_overlay && !context->mntpoint_mounted)
		tst_brk(TBROK, "tst_test->mntpoint must be mounted");

	if (tst_test->needs_overlay && !context->ovl_mounted) {
		SAFE_MOUNT_OVERLAY();
		context->ovl_mounted = 1;
	}

	if (tst_test->resource_files)
		copy_resources();

	if (tst_test->restore_wallclock)
		tst_wallclock_save();

	if (tst_test->taint_check)
		tst_taint_init(tst_test->taint_check);

	if (tst_test->needs_cgroup_ctrls)
		do_cgroup_requires();
	else if (tst_test->needs_cgroup_ver)
		tst_brk(TBROK, "tst_test->needs_cgroup_ctrls must be set");
}

static void do_test_setup(void)
{
	context->main_pid = getpid();

	if (!tst_test->all_filesystems && tst_test->skip_filesystems) {
		long fs_type = tst_fs_type(".");
		const char *fs_name = tst_fs_type_name(fs_type);

		if (tst_fs_in_skiplist(fs_name, tst_test->skip_filesystems)) {
			tst_brk(TCONF, "%s is not supported by the test",
				fs_name);
		}

		tst_res(TINFO, "%s is supported by the test", fs_name);
	}

	if (tst_test->caps)
		tst_cap_setup(tst_test->caps, TST_CAP_REQ);

	if (tst_test->setup)
		tst_test->setup();

	if (context->main_pid != tst_getpid())
		tst_brk(TBROK, "Runaway child in setup()!");

	if (tst_test->caps)
		tst_cap_setup(tst_test->caps, TST_CAP_DROP);
}

static void do_cleanup(void)
{
	if (tst_test->needs_cgroup_ctrls)
		tst_cg_cleanup();

	if (context->ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);

	if (context->mntpoint_mounted)
		tst_umount(tst_test->mntpoint);

	if (tst_test->needs_device && tdev.dev)
		tst_release_device(tdev.dev);

	if (tst_tmpdir_created()) {
		/* avoid munmap() on wrong pointer in tst_rmdir() */
		tst_futexes = NULL;
		tst_rmdir();
	}

	tst_sys_conf_restore(0);

	if (tst_test->restore_wallclock)
		tst_wallclock_restore();

	cleanup_ipc();
}

static void heartbeat(void)
{
	if (tst_clock_gettime(CLOCK_MONOTONIC, &context->start_time))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");

	if (getppid() == 1) {
		tst_res(TFAIL, "Main test process might have exit!");
		/*
		 * We need kill the task group immediately since the
		 * main process has exit.
		 */
		kill(0, SIGKILL);
		exit(TBROK);
	}

	kill(getppid(), SIGUSR1);
}

static void run_tests(void)
{
	unsigned int i;
	struct results saved_results;

	if (!tst_test->test) {
		saved_results = *results;
		heartbeat();
		tst_test->test_all();

		if (tst_getpid() != context->main_pid)
			exit(0);

		tst_reap_children();

		if (results_equal(&saved_results, results))
			tst_brk(TBROK, "Test haven't reported results!");

		return;
	}

	for (i = 0; i < tst_test->tcnt; i++) {
		saved_results = *results;
		heartbeat();
		tst_test->test(i);

		if (tst_getpid() != context->main_pid)
			exit(0);

		tst_reap_children();

		if (results_equal(&saved_results, results))
			tst_brk(TBROK, "Test %i haven't reported results!", i);
	}
}

static unsigned long long get_time_ms(void)
{
	struct timespec ts;

	if (tst_clock_gettime(CLOCK_MONOTONIC, &ts))
		tst_brk(TBROK | TERRNO, "tst_clock_gettime()");

	return tst_timespec_to_ms(ts);
}

static void add_paths(void)
{
	char *old_path = getenv("PATH");
	const char *start_dir;
	char *new_path;

	start_dir = tst_get_startwd();

	if (old_path)
		SAFE_ASPRINTF(&new_path, "%s::%s", old_path, start_dir);
	else
		SAFE_ASPRINTF(&new_path, "::%s", start_dir);

	SAFE_SETENV("PATH", new_path, 1);
	free(new_path);
}

static void testrun(void)
{
	unsigned int i = 0;
	unsigned long long stop_time = 0;
	int cont = 1;

	heartbeat();
	add_paths();
	do_test_setup();

	if (duration > 0)
		stop_time = get_time_ms() + (unsigned long long)(duration * 1000);

	for (;;) {
		cont = 0;

		if (i < (unsigned int)iterations) {
			i++;
			cont = 1;
		}

		if (stop_time && get_time_ms() < stop_time)
			cont = 1;

		if (!cont)
			break;

		run_tests();
		heartbeat();
	}

	do_test_cleanup();
	exit(0);
}

static pid_t test_pid;


static volatile sig_atomic_t sigkill_retries;

#define WRITE_MSG(msg) do { \
	if (write(2, msg, sizeof(msg) - 1)) { \
		/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425 */ \
	} \
} while (0)

static void alarm_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	WRITE_MSG("Test timeouted, sending SIGKILL!\n");

	kill(-test_pid, SIGKILL);
	alarm(5);

	if (++sigkill_retries > 10) {
		WRITE_MSG("Cannot kill test processes!\n");
		WRITE_MSG("Congratulation, likely test hit a kernel bug.\n");
		WRITE_MSG("Exiting uncleanly...\n");
		_exit(TFAIL);
	}
}

static void heartbeat_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	alarm(context->overall_time);
	sigkill_retries = 0;
}

static void sigint_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	if (test_pid > 0) {
		WRITE_MSG("Sending SIGKILL to test process...\n");
		kill(-test_pid, SIGKILL);
	}
}

unsigned int tst_remaining_runtime(void)
{
	static struct timespec now;
	int elapsed;

	if (context->runtime == 0)
		tst_brk(TBROK, "Runtime not set!");

	if (tst_clock_gettime(CLOCK_MONOTONIC, &now))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");

	elapsed = tst_timespec_diff_ms(now, context->start_time) / 1000;
	if (context->runtime > elapsed)
		return context->runtime - elapsed;

	return 0;
}


unsigned int tst_multiply_timeout(unsigned int timeout)
{
	parse_mul(&timeout_mul, "LTP_TIMEOUT_MUL", 0.099, 10000);

	if (timeout < 1)
		tst_brk(TBROK, "timeout must to be >= 1! (%d)", timeout);

	if (tst_has_slow_kconfig())
		timeout *= 4;

	return timeout * timeout_mul;
}

static void set_overall_timeout(void)
{
	unsigned int timeout = DEFAULT_TIMEOUT + tst_test->timeout;

	if (tst_test->timeout == TST_UNLIMITED_TIMEOUT) {
		tst_res(TINFO, "Test timeout is not limited");
		return;
	}

	context->overall_time = tst_multiply_timeout(timeout) + context->runtime;

	tst_res(TINFO, "Overall timeout per run is %uh %02um %02us",
		context->overall_time/3600, (context->overall_time%3600)/60,
		context->overall_time % 60);
}

void tst_set_timeout(int timeout)
{
	int timeout_adj = DEFAULT_TIMEOUT + timeout;

	context->overall_time = tst_multiply_timeout(timeout_adj) + context->runtime;

	tst_res(TINFO, "Overall timeout per run is %uh %02um %02us",
		context->overall_time/3600, (context->overall_time%3600)/60,
		context->overall_time % 60);

	heartbeat();
}

void tst_set_runtime(int runtime)
{
	context->runtime = multiply_runtime(runtime);
	tst_res(TINFO, "Updating runtime to %uh %02um %02us",
		runtime/3600, (runtime%3600)/60, runtime % 60);
	set_overall_timeout();
	heartbeat();
}

static void fork_testrun(void)
{
	int status;

	SAFE_SIGNAL(SIGINT, sigint_handler);
	SAFE_SIGNAL(SIGTERM, sigint_handler);

	alarm(context->overall_time);

	show_failure_hints = 1;

	test_pid = fork();
	if (test_pid < 0)
		tst_brk(TBROK | TERRNO, "fork()");

	if (!test_pid) {
		tst_disable_oom_protection(0);
		SAFE_SIGNAL(SIGALRM, SIG_DFL);
		SAFE_SIGNAL(SIGUSR1, SIG_DFL);
		SAFE_SIGNAL(SIGTERM, SIG_DFL);
		SAFE_SIGNAL(SIGINT, SIG_DFL);
		SAFE_SETPGID(0, 0);
		testrun();
	}

	SAFE_WAITPID(test_pid, &status, 0);
	alarm(0);
	SAFE_SIGNAL(SIGTERM, SIG_DFL);
	SAFE_SIGNAL(SIGINT, SIG_DFL);

	if (tst_test->taint_check && tst_taint_check()) {
		tst_res(TFAIL, "Kernel is now tainted");
		return;
	}

	if (tst_test->forks_child && kill(-test_pid, SIGKILL) == 0)
		tst_res(TINFO, "Killed the leftover descendant processes");

	if (WIFEXITED(status) && WEXITSTATUS(status))
		tst_brk(TBROK, "Child returned with %i", WEXITSTATUS(status));

	if (context->abort_flag)
		return;

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(TINFO, "If you are running on slow machine, "
			       "try exporting LTP_TIMEOUT_MUL > 1");
		tst_brk(TBROK, "Test killed! (timeout?)");
	}

	if (WIFSIGNALED(status))
		tst_brk(TBROK, "Test killed by %s!", tst_strsig(WTERMSIG(status)));
}

static struct tst_fs *lookup_fs_desc(const char *fs_type, int all_filesystems)
{
	struct tst_fs *fs = tst_test->filesystems;
	static struct tst_fs empty;

	if (!fs)
		goto ret;

	for (; fs->type; fs++) {

		if (!fs->type)
			continue;

		if (!strcmp(fs_type, fs->type))
			return fs;
	}

ret:
	if (!all_filesystems)
		return NULL;

	if (!tst_test->filesystems || tst_test->filesystems[0].type)
		return &empty;

	return &tst_test->filesystems[0];
}

static void run_tcase_on_fs(struct tst_fs *fs, const char *fs_type)
{
	tst_res(TINFO, "=== Testing on %s ===", fs_type);
	tdev.fs_type = fs_type;

	if (fs->mkfs_ver && !tst_check_cmd(fs->mkfs_ver, 0))
		return;

	if (fs->min_kver && !check_kver(fs->min_kver, 0))
		return;

	prepare_device(fs);

	fork_testrun();

	if (context->mntpoint_mounted) {
		tst_umount(tst_test->mntpoint);
		context->mntpoint_mounted = 0;
	}

	return;
}

static void run_tcases_per_fs(void)
{
	unsigned int i;
	bool found_valid_fs = false;
	const char *const *filesystems = tst_get_supported_fs_types(tst_test->skip_filesystems);

	if (!filesystems[0])
		tst_brk(TCONF, "There are no supported filesystems");

	for (i = 0; filesystems[i]; i++) {
		struct tst_fs *fs = lookup_fs_desc(filesystems[i], tst_test->all_filesystems);

		if (!fs)
			continue;

		found_valid_fs = true;
		run_tcase_on_fs(fs, filesystems[i]);

		if (tst_atomic_load(&context->abort_flag))
			do_exit(0);
	}

	if (!found_valid_fs)
		tst_brk(TCONF, "No required filesystems are available");
}

unsigned int tst_variant;

void tst_run_tcases(int argc, char *argv[], struct tst_test *self)
{
	unsigned int test_variants = 1;
	struct utsname uval;

	tst_test = self;

	do_setup(argc, argv);
	tst_enable_oom_protection(context->lib_pid);

	SAFE_SIGNAL(SIGALRM, alarm_handler);
	SAFE_SIGNAL(SIGUSR1, heartbeat_handler);

	tst_res(TINFO, "LTP version: "LTP_VERSION);

	uname(&uval);
	tst_res(TINFO, "Tested kernel: %s %s %s", uval.release, uval.version, uval.machine);

	if (tst_test->min_runtime && !tst_test->runtime)
		tst_test->runtime = tst_test->min_runtime;

	if (tst_test->runtime)
		context->runtime = multiply_runtime(tst_test->runtime);

	set_overall_timeout();

	if (tst_test->test_variants)
		test_variants = tst_test->test_variants;

	for (tst_variant = 0; tst_variant < test_variants; tst_variant++) {
		if (tst_test->all_filesystems || count_fs_descs() > 1)
			run_tcases_per_fs();
		else
			fork_testrun();

		if (tst_atomic_load(&context->abort_flag))
			do_exit(0);
	}

	do_exit(0);
}

void tst_flush(void)
{
	int rval;

	rval = fflush(stderr);
	if (rval != 0)
		tst_brk(TBROK | TERRNO, "fflush(stderr) failed");

	rval = fflush(stdout);
	if (rval != 0)
		tst_brk(TBROK | TERRNO, "fflush(stdout) failed");

}
