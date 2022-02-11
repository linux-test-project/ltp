// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2016-2021
 */

#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>

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
#include "old_resource.h"
#include "old_device.h"
#include "old_tmpdir.h"

/*
 * Hack to get TCID defined in newlib tests
 */
const char *TCID __attribute__((weak));

/* update also docparse/testinfo.pl */
#define LINUX_GIT_URL "https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id="
#define LINUX_STABLE_GIT_URL "https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/commit/?id="
#define GLIBC_GIT_URL "https://sourceware.org/git/?p=glibc.git;a=commit;h="
#define CVE_DB_URL "https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-"

struct tst_test *tst_test;

static const char *tid;
static int iterations = 1;
static float duration = -1;
static float timeout_mul = -1;
static pid_t main_pid, lib_pid;
static int mntpoint_mounted;
static int ovl_mounted;
static struct timespec tst_start_time; /* valid only for test pid */

struct results {
	int passed;
	int skipped;
	int failed;
	int warnings;
	int broken;
	unsigned int timeout;
};

static struct results *results;

static int ipc_fd;

extern void *tst_futexes;
extern unsigned int tst_max_futexes;

static char ipc_path[1064];
const char *tst_ipc_path = ipc_path;

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
		         tid, getpid());
	} else {
		char *tmpdir;

		if (!tst_tmpdir_created())
			tst_tmpdir();

		tmpdir = tst_get_tmpdir();
		snprintf(shm_path, sizeof(shm_path), "%s/ltp_%s_%d",
		         tmpdir, tid, getpid());
		free(tmpdir);
	}

	ipc_fd = open(shm_path, O_CREAT | O_EXCL | O_RDWR, 0600);
	if (ipc_fd < 0)
		tst_brk(TBROK | TERRNO, "open(%s)", shm_path);
	SAFE_CHMOD(shm_path, 0666);

	SAFE_FTRUNCATE(ipc_fd, size);

	results = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, ipc_fd, 0);

	/* Checkpoints needs to be accessible from processes started by exec() */
	if (tst_test->needs_checkpoints || tst_test->child_needs_reinit) {
		sprintf(ipc_path, IPC_ENV_VAR "=%s", shm_path);
		putenv(ipc_path);
	} else {
		SAFE_UNLINK(shm_path);
	}

	SAFE_CLOSE(ipc_fd);

	if (tst_test->needs_checkpoints) {
		tst_futexes = (char*)results + sizeof(struct results);
		tst_max_futexes = (size - sizeof(struct results))/sizeof(futex_t);
	}
}

static void cleanup_ipc(void)
{
	size_t size = getpagesize();

	if (ipc_fd > 0 && close(ipc_fd))
		tst_res(TWARN | TERRNO, "close(ipc_fd) failed");

	if (shm_path[0] && !access(shm_path, F_OK) && unlink(shm_path))
		tst_res(TWARN | TERRNO, "unlink(%s) failed", shm_path);

	if (results) {
		msync((void*)results, size, MS_SYNC);
		munmap((void*)results, size);
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

	results = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	tst_futexes = (char*)results + sizeof(struct results);
	tst_max_futexes = (size - sizeof(struct results))/sizeof(futex_t);

	SAFE_CLOSE(fd);
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
		res = "TCONF";
	break;
	case TWARN:
		res = "TWARN";
	break;
	case TINFO:
		res = "TINFO";
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

void tst_vres_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va)
{
	print_result(file, lineno, ttype, fmt, va);

	update_results(TTYPE_RESULT(ttype));
}

void tst_vbrk_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va);

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

void tst_vbrk_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va)
{
	print_result(file, lineno, ttype, fmt, va);
	update_results(TTYPE_RESULT(ttype));

	/*
	 * The getpid implementation in some C library versions may cause cloned
	 * test threads to show the same pid as their parent when CLONE_VM is
	 * specified but CLONE_THREAD is not. Use direct syscall to avoid
	 * cleanup running in the child.
	 */
	if (syscall(SYS_getpid) == main_pid)
		do_test_cleanup();

	if (getpid() == lib_pid)
		do_exit(TTYPE_RESULT(ttype));

	exit(TTYPE_RESULT(ttype));
}

void tst_res_(const char *file, const int lineno, int ttype,
              const char *fmt, ...)
{
	va_list va;

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
	int ret;

	if (WIFSIGNALED(status)) {
		tst_brk(TBROK, "Child (%i) killed by signal %s",
		        pid, tst_strsig(WTERMSIG(status)));
	}

	if (!(WIFEXITED(status)))
		tst_brk(TBROK, "Child (%i) exited abnormally", pid);

	ret = WEXITSTATUS(status);
	switch (ret) {
	case TPASS:
	case TBROK:
	case TCONF:
	break;
	default:
		tst_brk(TBROK, "Invalid child (%i) exit value %i", pid, ret);
	}
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

pid_t safe_clone(const char *file, const int lineno,
		 const struct tst_clone_args *args)
{
	pid_t pid;

	if (!tst_test->forks_child)
		tst_brk(TBROK, "test.forks_child must be set!");

	pid = tst_clone(args);

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

static struct option {
	char *optstr;
	char *help;
} options[] = {
	{"h",  "-h       Prints this help"},
	{"i:", "-i n     Execute test n times"},
	{"I:", "-I x     Execute test for n seconds"},
	{"C:", "-C ARG   Run child process with ARG arguments (used internally)"},
};

static void print_help(void)
{
	unsigned int i;

	/* see doc/user-guide.txt, which lists also shell API variables */
	fprintf(stderr, "Environment Variables\n");
	fprintf(stderr, "---------------------\n");
	fprintf(stderr, "KCONFIG_PATH         Specify kernel config file\n");
	fprintf(stderr, "KCONFIG_SKIP_CHECK   Skip kernel config check if variable set (not set by default)\n");
	fprintf(stderr, "LTPROOT              Prefix for installed LTP (default: /opt/ltp)\n");
	fprintf(stderr, "LTP_COLORIZE_OUTPUT  Force colorized output behaviour (y/1 always, n/0: never)\n");
	fprintf(stderr, "LTP_DEV              Path to the block device to be used (for .needs_device)\n");
	fprintf(stderr, "LTP_DEV_FS_TYPE      Filesystem used for testing (default: %s)\n", DEFAULT_FS_TYPE);
	fprintf(stderr, "LTP_SINGLE_FS_TYPE   Testing only - specifies filesystem instead all supported (for .all_filesystems)\n");
	fprintf(stderr, "LTP_TIMEOUT_MUL      Timeout multiplier (must be a number >=1)\n");
	fprintf(stderr, "LTP_VIRT_OVERRIDE    Overrides virtual machine detection (values: \"\"|kvm|microsoft|xen|zvm)\n");
	fprintf(stderr, "TMPDIR               Base directory for template directory (for .needs_tmpdir, default: %s)\n", TEMPDIR);
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

	for (i = 0; tst_test->options[i].optstr; i++);

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

/* see self_exec.c */
#ifdef UCLINUX
extern char *child_args;
#endif

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
		case 'h':
			print_help();
			print_test_tags();
			exit(0);
		case 'i':
			iterations = atoi(optarg);
		break;
		case 'I':
			duration = atof(optarg);
		break;
		case 'C':
#ifdef UCLINUX
			child_args = optarg;
#endif
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

/* update also docparse/testinfo.pl */
static void print_failure_hints(void)
{
	print_failure_hint("linux-git", "missing kernel fixes", LINUX_GIT_URL);
	print_failure_hint("linux-stable-git", "missing stable kernel fixes",
					   LINUX_STABLE_GIT_URL);
	print_failure_hint("glibc-git", "missing glibc fixes", GLIBC_GIT_URL);
	print_failure_hint("CVE", "vulnerable to CVE(s)", CVE_DB_URL);
	print_failure_hint("known-fail", "hit by known kernel failures", NULL);
}

static void do_exit(int ret)
{
	if (results) {
		if (results->passed && ret == TCONF)
			ret = 0;

		if (results->failed) {
			ret |= TFAIL;
			print_failure_hints();
		}

		if (results->skipped && !results->passed)
			ret |= TCONF;

		if (results->warnings)
			ret |= TWARN;

		if (results->broken)
			ret |= TBROK;

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

void check_kver(void)
{
	int v1, v2, v3;

	if (tst_parse_kver(tst_test->min_kver, &v1, &v2, &v3)) {
		tst_res(TWARN,
		        "Invalid kernel version %s, expected %%d.%%d.%%d",
		        tst_test->min_kver);
	}

	if (tst_kvercmp(v1, v2, v3) < 0) {
		tst_brk(TCONF, "The test requires kernel %s or newer",
		        tst_test->min_kver);
	}
}

static int results_equal(struct results *a, struct results *b)
{
	if (a->passed != b->passed)
		return 0;

	if (a->failed != b->failed)
		return 0;

	if (a->skipped != b->skipped)
		return 0;

	if (a->broken != b->broken)
		return 0;

	return 1;
}

static int needs_tmpdir(void)
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

static const char *get_tid(char *argv[])
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

static int prepare_and_mount_ro_fs(const char *dev,
                                   const char *mntpoint,
                                   const char *fs_type)
{
	char buf[PATH_MAX];

	if (mount(dev, mntpoint, fs_type, 0, NULL)) {
		tst_res(TINFO | TERRNO, "Can't mount %s at %s (%s)",
			dev, mntpoint, fs_type);
		return 1;
	}

	mntpoint_mounted = 1;

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
		mntpoint_mounted = 1;
	}
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

static void prepare_device(void)
{
	const char *mnt_data;
	char buf[1024];

	if (tst_test->format_device) {
		SAFE_MKFS(tdev.dev, tdev.fs_type, tst_test->dev_fs_opts,
			  tst_test->dev_extra_opts);
	}

	if (tst_test->needs_rofs) {
		prepare_and_mount_ro_fs(tdev.dev, tst_test->mntpoint,
		                        tdev.fs_type);
		return;
	}

	if (tst_test->mount_device) {
		mnt_data = limit_tmpfs_mount_size(tst_test->mnt_data,
				buf, sizeof(buf), tdev.fs_type);

		SAFE_MOUNT(get_device_name(tdev.fs_type), tst_test->mntpoint,
				tdev.fs_type, tst_test->mnt_flags, mnt_data);
		mntpoint_mounted = 1;
	}
}

static void do_cgroup_requires(void)
{
	const struct tst_cgroup_opts cg_opts = {
		.needs_ver = tst_test->needs_cgroup_ver,
	};
	const char *const *ctrl_names = tst_test->needs_cgroup_ctrls;

	for (; *ctrl_names; ctrl_names++)
		tst_cgroup_require(*ctrl_names, &cg_opts);

	tst_cgroup_init();
}

static void do_setup(int argc, char *argv[])
{
	if (!tst_test)
		tst_brk(TBROK, "No tests to run");

	if (tst_test->tconf_msg)
		tst_brk(TCONF, "%s", tst_test->tconf_msg);

	assert_test_fn();

	TCID = tid = get_tid(argv);

	if (tst_test->sample)
		tst_test = tst_timer_test_setup(tst_test);

	parse_opts(argc, argv);

	if (tst_test->needs_kconfigs && tst_kconfig_check(tst_test->needs_kconfigs))
		tst_brk(TCONF, "Aborting due to unsuitable kernel config, see above!");

	if (tst_test->needs_root && geteuid() != 0)
		tst_brk(TCONF, "Test needs to be run as root");

	if (tst_test->min_kver)
		check_kver();

	if (tst_test->supported_archs && !tst_is_on_arch(tst_test->supported_archs))
		tst_brk(TCONF, "This arch '%s' is not supported for test!", tst_arch.name);

	if (tst_test->skip_in_lockdown && tst_lockdown_enabled())
		tst_brk(TCONF, "Kernel is locked down, skipping test");

	if (tst_test->skip_in_compat && TST_ABI != tst_kernel_bits())
		tst_brk(TCONF, "Not supported in 32-bit compat mode");

	if (tst_test->needs_cmds) {
		const char *cmd;
		int i;

		for (i = 0; (cmd = tst_test->needs_cmds[i]); ++i)
			tst_check_cmd(cmd);
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

	if (tst_test->request_hugepages)
		tst_request_hugepages(tst_test->request_hugepages);

	setup_ipc();

	if (tst_test->bufs)
		tst_buffers_alloc(tst_test->bufs);

	if (needs_tmpdir() && !tst_tmpdir_created())
		tst_tmpdir();

	if (tst_test->save_restore) {
		const char * const *name = tst_test->save_restore;

		while (*name) {
			tst_sys_conf_save(*name);
			name++;
		}
	}

	if (tst_test->mntpoint)
		SAFE_MKDIR(tst_test->mntpoint, 0777);

	if ((tst_test->needs_devfs || tst_test->needs_rofs ||
	     tst_test->mount_device || tst_test->all_filesystems) &&
	     !tst_test->mntpoint) {
		tst_brk(TBROK, "tst_test->mntpoint must be set!");
	}

	if (!!tst_test->needs_rofs + !!tst_test->needs_devfs +
	    !!tst_test->needs_device > 1) {
		tst_brk(TBROK,
			"Two or more of needs_{rofs, devfs, device} are set");
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

	if (tst_test->needs_device && !mntpoint_mounted) {
		tdev.dev = tst_acquire_device_(NULL, tst_test->dev_min_size);

		if (!tdev.dev)
			tst_brk(TCONF, "Failed to acquire device");

		tdev.size = tst_get_device_size(tdev.dev);

		tst_device = &tdev;

		if (tst_test->dev_fs_type)
			tdev.fs_type = tst_test->dev_fs_type;
		else
			tdev.fs_type = tst_dev_fs_type();

		if (!tst_test->all_filesystems)
			prepare_device();
	}

	if (tst_test->needs_overlay && !tst_test->mount_device) {
		tst_brk(TBROK, "tst_test->mount_device must be set");
	}
	if (tst_test->needs_overlay && !mntpoint_mounted) {
		tst_brk(TBROK, "tst_test->mntpoint must be mounted");
	}
	if (tst_test->needs_overlay && !ovl_mounted) {
		SAFE_MOUNT_OVERLAY();
		ovl_mounted = 1;
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
		tst_brk(TBROK, "needs_cgroup_ver only works with needs_cgroup_controllers");
}

static void do_test_setup(void)
{
	main_pid = getpid();

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

	if (main_pid != getpid())
		tst_brk(TBROK, "Runaway child in setup()!");

	if (tst_test->caps)
		tst_cap_setup(tst_test->caps, TST_CAP_DROP);
}

static void do_cleanup(void)
{
	if (tst_test->needs_cgroup_ctrls)
		tst_cgroup_cleanup();

	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);

	if (mntpoint_mounted)
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

static void run_tests(void)
{
	unsigned int i;
	struct results saved_results;

	if (!tst_test->test) {
		saved_results = *results;
		tst_test->test_all();

		if (getpid() != main_pid) {
			exit(0);
		}

		tst_reap_children();

		if (results_equal(&saved_results, results))
			tst_brk(TBROK, "Test haven't reported results!");
		return;
	}

	for (i = 0; i < tst_test->tcnt; i++) {
		saved_results = *results;
		tst_test->test(i);

		if (getpid() != main_pid) {
			exit(0);
		}

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

static void heartbeat(void)
{
	if (tst_clock_gettime(CLOCK_MONOTONIC, &tst_start_time))
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
	alarm(results->timeout);
	sigkill_retries = 0;
}

static void sigint_handler(int sig LTP_ATTRIBUTE_UNUSED)
{
	if (test_pid > 0) {
		WRITE_MSG("Sending SIGKILL to test process...\n");
		kill(-test_pid, SIGKILL);
	}
}

unsigned int tst_timeout_remaining(void)
{
	static struct timespec now;
	unsigned int elapsed;

	if (tst_clock_gettime(CLOCK_MONOTONIC, &now))
		tst_res(TWARN | TERRNO, "tst_clock_gettime() failed");

	elapsed = (tst_timespec_diff_ms(now, tst_start_time) + 500) / 1000;
	if (results->timeout > elapsed)
		return results->timeout - elapsed;

	return 0;
}

unsigned int tst_multiply_timeout(unsigned int timeout)
{
	char *mul;
	int ret;

	if (timeout_mul == -1) {
		mul = getenv("LTP_TIMEOUT_MUL");
		if (mul) {
			if ((ret = tst_parse_float(mul, &timeout_mul, 1, 10000))) {
				tst_brk(TBROK, "Failed to parse LTP_TIMEOUT_MUL: %s",
						tst_strerrno(ret));
			}
		} else {
			timeout_mul = 1;
		}
	}
	if (timeout_mul < 1)
		tst_brk(TBROK, "LTP_TIMEOUT_MUL must to be int >= 1! (%.2f)",
				timeout_mul);

	if (timeout < 1)
		tst_brk(TBROK, "timeout must to be >= 1! (%d)", timeout);

	return timeout * timeout_mul;
}

void tst_set_timeout(int timeout)
{
	if (timeout == -1) {
		tst_res(TINFO, "Timeout per run is disabled");
		return;
	}

	if (timeout < 1)
		tst_brk(TBROK, "timeout must to be >= 1! (%d)", timeout);

	results->timeout = tst_multiply_timeout(timeout);

	tst_res(TINFO, "Timeout per run is %uh %02um %02us",
		results->timeout/3600, (results->timeout%3600)/60,
		results->timeout % 60);

	if (getpid() == lib_pid)
		alarm(results->timeout);
	else
		heartbeat();
}

static int fork_testrun(void)
{
	int status;

	if (tst_test->timeout)
		tst_set_timeout(tst_test->timeout);
	else
		tst_set_timeout(300);

	SAFE_SIGNAL(SIGINT, sigint_handler);

	test_pid = fork();
	if (test_pid < 0)
		tst_brk(TBROK | TERRNO, "fork()");

	if (!test_pid) {
		tst_disable_oom_protection(0);
		SAFE_SIGNAL(SIGALRM, SIG_DFL);
		SAFE_SIGNAL(SIGUSR1, SIG_DFL);
		SAFE_SIGNAL(SIGINT, SIG_DFL);
		SAFE_SETPGID(0, 0);
		testrun();
	}

	SAFE_WAITPID(test_pid, &status, 0);
	alarm(0);
	SAFE_SIGNAL(SIGINT, SIG_DFL);

	if (tst_test->taint_check && tst_taint_check()) {
		tst_res(TFAIL, "Kernel is now tainted.");
		return TFAIL;
	}

	if (tst_test->forks_child && kill(-test_pid, SIGKILL) == 0)
		tst_res(TINFO, "Killed the leftover descendant processes");

	if (WIFEXITED(status) && WEXITSTATUS(status))
		return WEXITSTATUS(status);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(TINFO, "If you are running on slow machine, "
			       "try exporting LTP_TIMEOUT_MUL > 1");
		tst_brk(TBROK, "Test killed! (timeout?)");
	}

	if (WIFSIGNALED(status))
		tst_brk(TBROK, "Test killed by %s!", tst_strsig(WTERMSIG(status)));

	return 0;
}

static int run_tcases_per_fs(void)
{
	int ret = 0;
	unsigned int i;
	const char *const *filesystems = tst_get_supported_fs_types(tst_test->skip_filesystems);

	if (!filesystems[0])
		tst_brk(TCONF, "There are no supported filesystems");

	for (i = 0; filesystems[i]; i++) {

		tst_res(TINFO, "Testing on %s", filesystems[i]);
		tdev.fs_type = filesystems[i];

		prepare_device();

		ret = fork_testrun();

		if (mntpoint_mounted) {
			tst_umount(tst_test->mntpoint);
			mntpoint_mounted = 0;
		}

		if (ret == TCONF)
			continue;

		if (ret == 0)
			continue;

		do_exit(ret);
	}

	return ret;
}

unsigned int tst_variant;

void tst_run_tcases(int argc, char *argv[], struct tst_test *self)
{
	int ret = 0;
	unsigned int test_variants = 1;

	lib_pid = getpid();
	tst_test = self;

	do_setup(argc, argv);
	tst_enable_oom_protection(lib_pid);

	SAFE_SIGNAL(SIGALRM, alarm_handler);
	SAFE_SIGNAL(SIGUSR1, heartbeat_handler);

	if (tst_test->test_variants)
		test_variants = tst_test->test_variants;

	for (tst_variant = 0; tst_variant < test_variants; tst_variant++) {
		if (tst_test->all_filesystems)
			ret |= run_tcases_per_fs();
		else
			ret |= fork_testrun();

		if (ret & ~(TCONF))
			goto exit;
	}

exit:
	do_exit(ret);
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
