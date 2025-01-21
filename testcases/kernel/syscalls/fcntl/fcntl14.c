// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *     07/2001 Ported by Wayne Boyer
 * Copyright (C) 2024 Andrea Cervesato andrea.cervesato@suse.com
 */

/*\
 * [Description]
 *
 * This test is checking fcntl() syscall locking mechanism between two
 * processes.
 * The test sets a random starting position on file using lseek(), it randomly
 * generates fcntl() parameters for parent and child and it verifies that
 * fcntl() will raise a blocking error on child when it's supposed to.
 */

#include <fcntl.h>
#include <stdlib.h>
#include "tst_test.h"

#ifndef S_ENFMT
# define S_ENFMT S_ISGID
#endif

#define CHECK_FAILURE(VAL_A, VAL_B) do { \
	TST_EXP_EQ_LI_SILENT(VAL_A, VAL_B); \
	if (!TST_PASS) \
		results->last_failed = 1; \
} while (0)

struct file_conf {
	short type;
	short whence;
	long start;
	long len;
};

struct testcase {
	struct flock flock;
	struct file_conf parent;           /* parent parameters for fcntl() */
	struct file_conf child;            /* child parameters for fcntl() */
	short blocking;                    /* blocking/non-blocking flag */
	long pos;                          /* starting file position */
};

struct tc_results {
	int num_pass;
	int last_failed;
};

static const char filepath[] = "unlocked.txt";
static const char filedata[] = "Here some bytes!";
static char *str_op_nums;
static int op_nums = 5000;
static int file_mode = 0777;
static struct tc_results *results;

static void dochild(struct testcase *tc, const int fd, const pid_t parent_pid)
{
	struct flock flock = tc->flock;

	results->last_failed = 0;

	flock.l_type = tc->child.type;
	flock.l_whence = tc->child.whence;
	flock.l_start = tc->child.start;
	flock.l_len = tc->child.len;
	flock.l_pid = 0;

	SAFE_FCNTL(fd, F_GETLK, &flock);

	if (tc->blocking) {
		tst_res(TDEBUG, "Child: expecting blocked file by parent");

		CHECK_FAILURE(flock.l_pid, parent_pid);
		CHECK_FAILURE(flock.l_type, tc->parent.type);

		flock.l_type = tc->child.type;
		flock.l_whence = tc->child.whence;
		flock.l_start = tc->child.start;
		flock.l_len = tc->child.len;
		flock.l_pid = 0;

		TST_EXP_FAIL_SILENT(fcntl(fd, F_SETLK, &flock), EWOULDBLOCK);
	} else {
		tst_res(TDEBUG, "Child: expecting no blocking errors");

		CHECK_FAILURE(flock.l_type, F_UNLCK);
		CHECK_FAILURE(flock.l_whence, tc->child.whence);
		CHECK_FAILURE(flock.l_start, tc->child.start);
		CHECK_FAILURE(flock.l_len, tc->child.len);
		CHECK_FAILURE(flock.l_pid, 0);

		TST_EXP_PASS_SILENT(fcntl(fd, F_SETLK, &flock));
	}
}

static void run_testcase(struct testcase *tc, const int file_mode)
{
	struct flock flock = tc->flock;
	pid_t parent_pid;
	pid_t child_pid;
	int fd;

	tst_res(TDEBUG, "Parent: locking file");

	/* open file and move cursor according with the test */
	fd = SAFE_OPEN(filepath, O_RDWR, file_mode);
	SAFE_LSEEK(fd, tc->pos, 0);

	/* set the initial parent lock on the file */
	flock.l_type = tc->parent.type;
	flock.l_whence = tc->parent.whence;
	flock.l_start = tc->parent.start;
	flock.l_len = tc->parent.len;
	flock.l_pid = 0;

	SAFE_FCNTL(fd, F_SETLK, &flock);

	/* set the child lock on the file */
	parent_pid = getpid();
	child_pid = SAFE_FORK();

	if (!child_pid) {
		dochild(tc, fd, parent_pid);
		exit(0);
	}

	tst_reap_children();

	flock.l_type = F_UNLCK;
	flock.l_whence = 0;
	flock.l_start = 0;
	flock.l_len = 0;
	flock.l_pid = 0;

	SAFE_CLOSE(fd);
}

static void genconf(struct file_conf *conf, const int size, const long pos)
{
	conf->type = rand() % 2 ? F_RDLCK : F_WRLCK;
	conf->whence = SEEK_CUR;
	conf->start = rand() % (size - 1);
	conf->len = rand() % (size - conf->start - 1) + 1;
	conf->start -= pos;
}

static short fcntl_overlap(
	struct file_conf *parent,
	struct file_conf *child)
{
	short overlap;

	if (child->start < parent->start)
		overlap = parent->start < (child->start + child->len);
	else
		overlap = child->start < (parent->start + parent->len);

	if (overlap)
		tst_res(TDEBUG, "child/parent fcntl() configurations overlap");

	return overlap;
}

static void gentestcase(struct testcase *tc)
{
	struct file_conf *parent = &tc->parent;
	struct file_conf *child = &tc->child;
	int size;

	memset(&tc->flock, 0, sizeof(struct flock));

	size = strlen(filedata);
	tc->pos = rand() % size;

	genconf(parent, size, tc->pos);
	genconf(child, size, tc->pos);

	tc->blocking = fcntl_overlap(parent, child);

	if (parent->type == F_RDLCK && child->type == F_RDLCK)
		tc->blocking = 0;
}

static void print_testcase(struct testcase *tc)
{
	tst_res(TDEBUG, "Starting read/write position: %ld", tc->pos);

	tst_res(TDEBUG,
		"Parent: type=%s whence=%s start=%ld len=%ld",
		tc->parent.type == F_RDLCK ? "F_RDLCK" : "F_WRLCK",
		tc->parent.whence == SEEK_SET ? "SEEK_SET" : "SEEK_CUR",
		tc->parent.start,
		tc->parent.len);

	tst_res(TDEBUG,
		"Child: type=%s whence=%s start=%ld len=%ld",
		tc->child.type == F_RDLCK ? "F_RDLCK" : "F_WRLCK",
		tc->child.whence == SEEK_SET ? "SEEK_SET" : "SEEK_CUR",
		tc->child.start,
		tc->child.len);

	tst_res(TDEBUG, "Expencted %s test",
		tc->blocking ? "blocking" : "non-blocking");
}

static void run(void)
{
	struct testcase tc;

	results->num_pass = 0;

	for (int i = 0; i < op_nums; i++) {
		gentestcase(&tc);
		print_testcase(&tc);

		tst_res(TDEBUG, "Running test #%u", i);
		run_testcase(&tc, file_mode);

		if (results->last_failed)
			return;

		results->num_pass++;
	}

	if (results->num_pass == op_nums)
		tst_res(TPASS, "All %d test file operations passed", op_nums);
}

static void setup(void)
{
	int fd;

	if (tst_parse_int(str_op_nums, &op_nums, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of operations '%s'", str_op_nums);

	if (tst_variant == 1) {
		tst_res(TINFO, "Requested mandatory locking");

		file_mode = S_ENFMT | 0600;
	}

	fd = SAFE_OPEN(filepath, O_CREAT | O_RDWR | O_TRUNC, 0777);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, filedata, strlen(filedata));
	SAFE_CLOSE(fd);

	srand(time(0));

	results = SAFE_MMAP(
		NULL,
		sizeof(struct tc_results),
	    PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_SHARED,
		-1, 0);
}

static void cleanup(void)
{
	if (results)
		SAFE_MUNMAP(results, sizeof(struct tc_results));
}

static struct tst_test test = {
	.timeout = 8,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = 2,
	.forks_child = 1,
	.needs_tmpdir = 1,
	.options = (struct tst_option[]) {
		{ "n:", &str_op_nums, "Total # operations to do (default 5000)" },
		{},
	},
	.skip_filesystems = (const char *const []) {
		"nfs",
		NULL
	},
};
