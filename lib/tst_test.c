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

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_device.h"
#include "lapi/futex.h"
#include "lapi/syscalls.h"
#include "tst_ansi_color.h"
#include "tst_timer_test.h"

#include "old_resource.h"
#include "old_device.h"
#include "old_tmpdir.h"

struct tst_test *tst_test;

static int iterations = 1;
static float duration = -1;
static pid_t main_pid, lib_pid;
static int mntpoint_mounted;

struct results {
	int passed;
	int skipped;
	int failed;
	int warnings;
	unsigned int timeout;
};

static struct results *results;

static int ipc_fd;

extern void *tst_futexes;
extern unsigned int tst_max_futexes;

#define IPC_ENV_VAR "LTP_IPC_PATH"

static char ipc_path[1024];
const char *tst_ipc_path = ipc_path;
char *const tst_ipc_envp[] = {ipc_path, NULL};

static char shm_path[1024];

static void do_cleanup(void);
static void do_exit(int ret) __attribute__ ((noreturn));

static void setup_ipc(void)
{
	size_t size = getpagesize();

	if (access("/dev/shm", F_OK) == 0) {
		snprintf(shm_path, sizeof(shm_path), "/dev/shm/ltp_%s_%d",
		         tst_test->tid, getpid());
	} else {
		char *tmpdir;

		if (!tst_tmpdir_created())
			tst_tmpdir();

		tmpdir = tst_get_tmpdir();
		snprintf(shm_path, sizeof(shm_path), "%s/ltp_%s_%d",
		         tmpdir, tst_test->tid, getpid());
		free(tmpdir);
	}

	ipc_fd = open(shm_path, O_CREAT | O_EXCL | O_RDWR, 0600);
	if (ipc_fd < 0)
		tst_brk(TBROK | TERRNO, "open(%s)", shm_path);

	SAFE_FTRUNCATE(ipc_fd, size);

	results = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, ipc_fd, 0);

	/* Checkpoints needs to be accessible from processes started by exec() */
	if (tst_test->needs_checkpoints)
		sprintf(ipc_path, IPC_ENV_VAR "=%s", shm_path);
	else
		SAFE_UNLINK(shm_path);

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
	}
}

void tst_reinit(void)
{
	const char *path = getenv("LTP_IPC_PATH");
	size_t size = getpagesize();
	int fd;
	void *ptr;

	if (!path)
		tst_brk(TBROK, "LTP_IPC_PATH is not defined");

	if (access(path, F_OK))
		tst_brk(TBROK, "File %s does not exist!", path);

	fd = SAFE_OPEN(path, O_RDWR);

	ptr = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	tst_futexes = (char*)ptr + sizeof(struct results);
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
	}
}

static void print_result(const char *file, const int lineno, int ttype,
                         const char *fmt, va_list va)
{
	char buf[1024];
	char *str = buf;
	int ret, size = sizeof(buf);
	const char *str_errno = NULL;
	const char *res;

	switch (TTYPE_RESULT(ttype)) {
	case TPASS:
		res = "PASS";
	break;
	case TFAIL:
		res = "FAIL";
	break;
	case TBROK:
		res = "BROK";
	break;
	case TCONF:
		res = "CONF";
	break;
	case TWARN:
		res = "WARN";
	break;
	case TINFO:
		res = "INFO";
	break;
	default:
		tst_brk(TBROK, "Invalid ttype value %i", ttype);
		abort();
	}

	if (ttype & TERRNO)
		str_errno = tst_strerrno(errno);

	if (ttype & TTERRNO)
		str_errno = tst_strerrno(TEST_ERRNO);

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

	ret = vsnprintf(str, size, fmt, va);
	str += ret;
	size -= ret;

	if (str_errno) {
		ret = snprintf(str, size, ": %s", str_errno);
		str += ret;
		size -= ret;
	}

	snprintf(str, size, "\n");

	fputs(buf, stderr);
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

	tst_brk_handler = tst_vbrk_;
}

void tst_vbrk_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va)
{
	print_result(file, lineno, ttype, fmt, va);

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

static void check_child_status(pid_t pid, int status)
{
	int ret;

	if (WIFSIGNALED(status)) {
		tst_brk(TBROK, "Child (%i) killed by signal %s",
		        pid, tst_strsig(WTERMSIG(status)));
	}

	if (!(WIFEXITED(status)))
		tst_brk(TBROK, "Child (%i) exitted abnormaly", pid);

	ret = WEXITSTATUS(status);
	switch (ret) {
	case TPASS:
	break;
	case TBROK:
	case TCONF:
		tst_brk(ret, "Reported by child (%i)", pid);
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

	fflush(stdout);

	pid = fork();
	if (pid < 0)
		tst_brk_(filename, lineno, TBROK | TERRNO, "fork() failed");

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

	for (i = 0; i < ARRAY_SIZE(options); i++)
		fprintf(stderr, "%s\n", options[i].help);

	if (!tst_test->options)
		return;

	for (i = 0; tst_test->options[i].optstr; i++)
		fprintf(stderr, "%s\n", tst_test->options[i].help);
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
		case 'h':
			print_help();
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

static void do_exit(int ret)
{
	if (results) {
		printf("\nSummary:\n");
		printf("passed   %d\n", results->passed);
		printf("failed   %d\n", results->failed);
		printf("skipped  %d\n", results->skipped);
		printf("warnings %d\n", results->warnings);

		if (results->failed)
			ret |= TFAIL;

		if (results->skipped && !results->passed)
			ret |= TCONF;

		if (results->warnings)
			ret |= TWARN;
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

	return 1;
}

static int needs_tmpdir(void)
{
	return tst_test->needs_tmpdir ||
	       tst_test->needs_device ||
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
		tst_brk(TBROK, "No test function speficied");

	if (cnt != 1)
		tst_brk(TBROK, "You can define only one test function");

	if (tst_test->test && !tst_test->tcnt)
		tst_brk(TBROK, "Number of tests (tcnt) must not be > 0");

	if (!tst_test->test && tst_test->tcnt)
		tst_brk(TBROK, "You can define tcnt only for test()");
}

static void do_setup(int argc, char *argv[])
{
	if (!tst_test)
		tst_brk(TBROK, "No tests to run");

	if (tst_test->tconf_msg)
		tst_brk(TCONF, "%s", tst_test->tconf_msg);

	assert_test_fn();

	if (tst_test->sample)
		tst_test = tst_timer_test_setup(tst_test);

	if (!tst_test->tid)
		tst_test->tid = get_tid(argv);

	parse_opts(argc, argv);

	if (tst_test->needs_root && geteuid() != 0)
		tst_brk(TCONF, "Test needs to be run as root");

	if (tst_test->min_kver)
		check_kver();

	if (tst_test->format_device)
		tst_test->needs_device = 1;

	if (tst_test->mount_device) {
		tst_test->needs_device = 1;
		tst_test->format_device = 1;
	}

	setup_ipc();

	if (needs_tmpdir() && !tst_tmpdir_created())
		tst_tmpdir();

	if (tst_test->mntpoint)
		SAFE_MKDIR(tst_test->mntpoint, 0777);

	if ((tst_test->needs_rofs || tst_test->mount_device) &&
	    !tst_test->mntpoint) {
		tst_brk(TBROK, "tst_test->mntpoint must be set!");
	}

	if (tst_test->needs_rofs) {
		/* If we failed to mount read-only tmpfs. Fallback to
		 * using a device with empty read-only filesystem.
		 */
		if (mount(NULL, tst_test->mntpoint, "tmpfs", MS_RDONLY, NULL)) {
			tst_res(TINFO | TERRNO, "Can't mount tmpfs read-only"
				" at %s, setting up a device instead\n",
				tst_test->mntpoint);
			tst_test->mount_device = 1;
			tst_test->needs_device = 1;
			tst_test->format_device = 1;
			tst_test->mnt_flags = MS_RDONLY;
		} else {
			mntpoint_mounted = 1;
		}
	}

	if (tst_test->needs_device && !mntpoint_mounted) {
		tdev.dev = tst_acquire_device_(NULL, tst_test->dev_min_size);

		if (!tdev.dev)
			tst_brk(TCONF, "Failed to acquire device");

		tst_device = &tdev;

		if (tst_test->dev_fs_type)
			tdev.fs_type = tst_test->dev_fs_type;
		else
			tdev.fs_type = tst_dev_fs_type();

		if (tst_test->format_device) {
			SAFE_MKFS(tdev.dev, tdev.fs_type,
			          tst_test->dev_fs_opts,
				  tst_test->dev_extra_opt);
		}

		if (tst_test->mount_device) {
			SAFE_MOUNT(tdev.dev, tst_test->mntpoint, tdev.fs_type,
				   tst_test->mnt_flags, tst_test->mnt_data);
			mntpoint_mounted = 1;
		}
	}

	if (tst_test->resource_files)
		copy_resources();
}

static void do_test_setup(void)
{
	main_pid = getpid();

	if (tst_test->setup)
		tst_test->setup();

	if (main_pid != getpid())
		tst_brk(TBROK, "Runaway child in setup()!");
}

static void do_cleanup(void)
{
	if (mntpoint_mounted)
		tst_umount(tst_test->mntpoint);

	if (tst_test->needs_device && tdev.dev)
		tst_release_device(tdev.dev);

	if (tst_tmpdir_created()) {
		/* avoid munmap() on wrong pointer in tst_rmdir() */
		tst_futexes = NULL;
		tst_rmdir();
	}

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
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void testrun(void)
{
	unsigned int i = 0;
	unsigned long long stop_time = 0;
	int cont = 1;

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

		kill(getppid(), SIGUSR1);
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
		WRITE_MSG("Exitting uncleanly...\n");
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

void tst_set_timeout(int timeout)
{
	char *mul = getenv("LTP_TIMEOUT_MUL");

	if (timeout == -1) {
		tst_res(TINFO, "Timeout per run is disabled");
		return;
	}

	results->timeout = timeout;

	if (mul) {
		float m = atof(mul);

		if (m < 1)
			tst_brk(TBROK, "Invalid timeout multiplier '%s'", mul);

		results->timeout = results->timeout * m + 0.5;
	}

	tst_res(TINFO, "Timeout per run is %uh %02um %02us",
		results->timeout/3600, (results->timeout%3600)/60,
		results->timeout % 60);

	if (getpid() == lib_pid)
		alarm(results->timeout);
	else
		kill(getppid(), SIGUSR1);
}

void tst_run_tcases(int argc, char *argv[], struct tst_test *self)
{
	int status;

	lib_pid = getpid();
	tst_test = self;

	do_setup(argc, argv);

	TCID = tst_test->tid;

	SAFE_SIGNAL(SIGALRM, alarm_handler);
	SAFE_SIGNAL(SIGUSR1, heartbeat_handler);

	if (tst_test->timeout)
		tst_set_timeout(tst_test->timeout);
	else
		tst_set_timeout(300);

	SAFE_SIGNAL(SIGINT, sigint_handler);

	test_pid = fork();
	if (test_pid < 0)
		tst_brk(TBROK | TERRNO, "fork()");

	if (!test_pid) {
		SAFE_SIGNAL(SIGALRM, SIG_DFL);
		SAFE_SIGNAL(SIGUSR1, SIG_DFL);
		SAFE_SIGNAL(SIGINT, SIG_DFL);
		SAFE_SETPGID(0, 0);
		testrun();
	}

	SAFE_WAITPID(test_pid, &status, 0);
	alarm(0);
	SAFE_SIGNAL(SIGINT, SIG_DFL);

	if (WIFEXITED(status) && WEXITSTATUS(status))
		do_exit(WEXITSTATUS(status));

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		tst_res(TINFO, "If you are running on slow machine, "
			       "try exporting LTP_TIMEOUT_MUL > 1");
		tst_brk(TBROK, "Test killed! (timeout?)");
	}

	if (WIFSIGNALED(status))
		tst_brk(TBROK, "Test killed by %s!", tst_strsig(WTERMSIG(status)));

	do_exit(0);
}
