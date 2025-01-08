// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) International Business Machines Corp., 2001
 *
 * This program is designed to stress the NFS implimentation. Many bugs were
 * uncovered in the AIX operating system implimentation of NFS when AIX kernel
 * was built over NFS. Source directory on a remote machine (one server many
 * clients) NFS-mounted on to a directory on a local machine from which the
 * kernel build was initiated. Apparently many defects/bugs were uncovered when
 * multiple users tried to build the kernel by NFS mounting the kernel source
 * from a remote machine and tried to build the kernel on a local machine.
 *
 * The test's aimed to stress NFS client/server and recreates such a senario.
 * Spawn N number of threads. Each thread does the following:
 *   * create a directory tree;
 *   * populate it with ".c" files and makefiles;
 *     hostname.1234
 *             | - 1234.0.0.c
 *             | - ..........
 *             | - makefile
 *             |_ 1234.0
 *                    |
 *                    | - 1234.1.0.c
 *                    | - ..........
 *                    | - makefile
 *                    |_ 1234.1
 *                           |....
 *
 *   * initate a build, executable will print hello world;
 *   * clean up all the executables that were created;
 *   * recurssively remove each subdir and its contents.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mount.h>
#include <linux/limits.h>
#include <errno.h>
#include <linux/unistd.h>

#include "tst_safe_pthread.h"
#include "tst_safe_stdio.h"
#include "tst_test.h"

#define gettid() syscall(__NR_gettid)

static int thrd_num = 8;
static int dirs_num = 100;
static int file_num = 100;

static char *t_arg, *d_arg, *f_arg;

static struct tst_option opts[] = {
	{"t:", &t_arg, "Number of threads to generate, default: 8"},
	{"d:", &d_arg, "Number of subdirs to generate, default: 100"},
	{"f:", &f_arg, "Number of c files in each dir, default: 100"},
	{}
};

static void run_targets(const char *dirname, char *cfile, pid_t tid)
{
	int i, k, fd;
	char subdir[PATH_MAX] = {0};
	char *output_file;
	char buf[11];
	const char *const cmd_run[] = {cfile, NULL};

	SAFE_ASPRINTF(&output_file, "%s/cmd.out", dirname);

	/* run each binary */
	for (i = 0; i < dirs_num; ++i) {
		for (k = 0; k < file_num; ++k) {
			snprintf(cfile, PATH_MAX, "%s%s/%d.%d.%d",
				 dirname, subdir, tid, i, k);

			tst_cmd(cmd_run, output_file, NULL, 0);

			fd = SAFE_OPEN(output_file, O_RDONLY);
			SAFE_READ(1, fd, buf, 11);
			if (strncmp(buf, "hello world", 11))
				tst_brk(TFAIL, "command printed wrong message");
			SAFE_CLOSE(fd);
		}
		strcat(subdir, "/dir");
	}

	free(output_file);
}

static void *thread_fn(LTP_ATTRIBUTE_UNUSED void *args)
{
	const char prog_buf[] = "#include <stdio.h>\n"
				"int main(void)\n{\n"
				"\tprintf(\"hello world\");\n"
				"\treturn 0;\n}\n";

	const char make_buf_n[] = "CFLAGS := -O -w -g\n"
				  "SRCS=$(sort $(wildcard *.c))\n"
				  "TARGETS=$(SRCS:.c=)\n"
				  "all: $(TARGETS)\n"
				  "$(TARGETS): %: %.c\n"
				  "\t$(CC) -o $@ $<\n"
				  "clean:\n\trm -f $(TARGETS)\n"
				  ".PHONY: all clean\n";

	const char make_buf[] = "CFLAGS := -O -w -g\n"
				"SUBDIR = dir\n"
				"SRCS=$(sort $(wildcard *.c))\n"
				"TARGETS=$(SRCS:.c=)\n"
				"all: $(SUBDIR) $(TARGETS)\n"
				"$(TARGETS): %: %.c\n"
				"\t$(CC) -o $@ $<\n"
				"$(SUBDIR):\n\t$(MAKE) -C $@\n"
				"clean:\n"
				"\trm -f $(TARGETS)\n"
				"\t$(MAKE) -C $(SUBDIR) clean\n"
				".PHONY: all $(SUBDIR) clean\n";

	int i, k, fd, dirfd, ret;
	char *dirname;
	char cfile[PATH_MAX];
	char hostname[256];
	pid_t tid = gettid();

	SAFE_GETHOSTNAME(hostname, 256);
	SAFE_ASPRINTF(&dirname, "%s.%ld", hostname, tid);

	SAFE_MKDIR(dirname, 0755);
	dirfd = SAFE_OPEN(dirname, O_DIRECTORY);

	for (i = 0; i < dirs_num; ++i) {

		fd = openat(dirfd, "makefile", O_CREAT | O_RDWR,
			    S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0)
			tst_brk(TFAIL | TERRNO, "openat(makefile) failed");

		if (i == dirs_num - 1)
			SAFE_WRITE(SAFE_WRITE_ALL, fd, make_buf_n, sizeof(make_buf_n) - 1);
		else
			SAFE_WRITE(SAFE_WRITE_ALL, fd, make_buf, sizeof(make_buf) - 1);

		SAFE_CLOSE(fd);

		for (k = 0; k < file_num; ++k) {
			snprintf(cfile, PATH_MAX, "%d.%d.%d.c", tid, i, k);
			fd = openat(dirfd, cfile, O_CREAT | O_RDWR,
				    S_IRWXU | S_IRWXG | S_IRWXO);
			if (fd < 0) {
				tst_brk(TFAIL | TERRNO,
					"openat(%s) failed", cfile);
			}

			SAFE_WRITE(SAFE_WRITE_ALL, fd, prog_buf, sizeof(prog_buf) - 1);
			SAFE_CLOSE(fd);
		}

		if (i == dirs_num - 1)
			break;

		ret = mkdirat(dirfd, "dir", 0755);
		if (ret < 0)
			tst_brk(TFAIL | TERRNO, "mkdirat('dir') failed");
		dirfd = openat(dirfd, "dir", O_DIRECTORY);
		if (dirfd < 0)
			tst_brk(TFAIL | TERRNO, "openat('dir') failed");
	}

	const char *const cmd_make[] = {"make", "-s", "-C", dirname, NULL};
	const char *const cmd_make_clean[] = {
		"make", "-C", dirname, "-s", "clean", NULL};

	tst_cmd(cmd_make, NULL, NULL, 0);

	run_targets(dirname, cfile, tid);

	tst_cmd(cmd_make_clean, NULL, NULL, 0);

	free(dirname);

	return NULL;
}

static void setup(void)
{
	thrd_num = atoi(t_arg);
	dirs_num = atoi(d_arg);
	file_num = atoi(f_arg);
}

static void do_test(void)
{
	int i;
	pthread_t id[thrd_num];

	for (i = 0; i < thrd_num; ++i)
		SAFE_PTHREAD_CREATE(id + i, NULL, thread_fn, NULL);

	for (i = 0; i < thrd_num; ++i)
		SAFE_PTHREAD_JOIN(id[i], NULL);

	tst_res(TPASS, "'make' successfully build and clean all targets");
}

static struct tst_test test = {
	.options = opts,
	.test_all = do_test,
	.setup = setup,
	.timeout = 300,
};
