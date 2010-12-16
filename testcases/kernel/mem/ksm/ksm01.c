/*
 * functional testing for Kernel Samepage Merging (KSM)
 *
 * Basic tests were to start several programs with same and different
 * memory contents and ensure only to merge the ones with the same
 * contents. When changed the content of one of merged pages in a
 * process and to the mode "unmerging", it should discard all merged
 * pages there. Also tested it is possible to disable KSM. There are
 * also command-line options to specify the memory allocation size, and
 * number of processes have same memory contents so it is possible to
 * test more advanced things like KSM + OOM etc.
 *
 * Prerequisites:
 *
 * 1) ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 *    distrub the testing as they also change some ksm tunables depends
 *    on current workloads.
 *
 * The test steps are:
 * - Check ksm feature and backup current run setting.
 * - Change run setting to 1 - merging.
 * - 3 memory allocation programs have the memory contents that 2 of
 *   them are all 'a' and one is all 'b'.
 * - Check ksm statistics and verify the content.
 * - 1 program changes the memory content from all 'a' to all 'b'.
 * - Check ksm statistics and verify the content.
 * - All programs change the memory content to all 'd'.
 * - Check ksm statistics and verify the content.
 * - Change one page of a process.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 2 - unmerging.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 0 - stop.
 *
 * Copyright (C) 2010  Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

#define _PATH_KSM	"/sys/kernel/mm/ksm/"
#define MB		(1024 * 1024)

char *TCID = "ksm01";
int TST_TOTAL = 1;
extern int Tst_count;
static int opt_num, opt_size;
static char *opt_numstr, *opt_sizestr;
/* memory pointer to identify per process, MB, and byte like
   memory[process No.][MB No.][byte No.]. */
static char ***memory;
static option_t options[] = {
	{ "n:", &opt_num,	&opt_numstr},
	{ "s:", &opt_size,	&opt_sizestr},
	{ NULL, NULL,		NULL}
};
static void setup(void);
static void ksmtest(void);
static void usage(void);
static void check(char *path, char *path2, long int value);
static void verify(char value, int proc, int start, int end, int start2,
		int end2);
static void group_check(int run, int pages_shared, int pages_sharing,
			int pages_volatile, int pages_unshared,
			int sleep_millisecs, int pages_to_scan);

int main(int argc, char *argv[])
{
	int lc;
	char *msg;

	msg = parse_opts(argc, argv, options, usage);
	if (msg != (char *)NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	setup();
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		ksmtest();
	}
	tst_exit();
}

void ksmtest(void)
{
	char buf[BUFSIZ], buf2[BUFSIZ];
	int i, j, status, k, fd, *child;
	int size = 128, num = 3;

	if (opt_size) {
		size = atoi(opt_sizestr);
		if (size < 1)
			tst_brkm(TBROK, NULL,
				"size cannot be less than 1.");
	}
	if (opt_num) {
		num = atoi(opt_numstr);
		if (num < 3)
			tst_brkm(TBROK, NULL,
				"process number cannot be less 3.");
	}
	child = malloc(num);
	if (child == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "malloc");

	memory = malloc(num * sizeof(**memory));
	if (memory == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "malloc");

	switch (child[0] = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, NULL, "fork");
	case 0:
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");

		tst_resm(TINFO, "child 0 continues...");

		/* Avoid TransparentHugePage allocation which can't ksm at the
		   moment. */
		tst_resm(TINFO, "child 0 allocates %d MB filled with 'c'.",
			size);
		memory[0] = malloc(size * sizeof(*memory));
		if (memory[0] == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "malloc");
		for (j = 0; j < size; j++) {
			memory[0][j] = mmap(NULL, MB, PROT_READ|PROT_WRITE,
					MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
			if (memory[0][j] == MAP_FAILED)
				tst_brkm(TBROK|TERRNO, NULL, "mmap");
			if (madvise(memory[0][j], MB, MADV_MERGEABLE) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "madvise");
			for (i = 0; i < MB; i++)
				memory[0][j][i] = 'c';
		}
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");

		tst_resm(TINFO, "child 0 continues...");
		verify('c', 0, 0, size, 0, MB);
		tst_resm(TINFO, "child 0 changes memory content to 'd'.");
		for (j = 0; j < size; j++) {
			for (i = 0; i < MB; i++)
				memory[0][j][i] = 'd';
		}
		/* Unmerge. */
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");

		tst_resm(TINFO, "child 0 continues...");
		verify('d', 0, 0, size, 0, MB);
		/* Stop. */
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 0 continues...");
		exit(0);
	}
	switch (child[1] = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, NULL, "fork");
	case 0:
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 1 continues...");
		tst_resm(TINFO, "child 1 allocates %d MB filled with 'a'.",
			size);
		memory[1] = malloc(size * sizeof(*memory));
		if (memory[1] == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "malloc");
		for (j = 0; j < size; j++) {
			memory[1][j] = mmap(NULL, MB, PROT_READ|PROT_WRITE,
					MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
			if (memory[1][j] == MAP_FAILED)
				tst_brkm(TBROK|TERRNO, NULL, "mmap");
			if (madvise(memory[1][j], MB, MADV_MERGEABLE) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "madvise");
			for (i = 0; i < MB; i++)
				memory[1][j][i] = 'a';
		}

		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 1 continues...");
		verify('a', 1, 0, size, 0, MB);
		tst_resm(TINFO, "child 1 changes memory content to 'b'.");
		for (j = 0; j < size; j++) {
			for (i = 0; i < MB; i++)
				memory[1][j][i] = 'b';
		}

		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 1 continues...");
		verify('b', 1, 0, size, 0, MB);
		tst_resm(TINFO, "child 1 changes memory content to 'd'");
		for (j = 0; j < size; j++) {
			for (i = 0; i < MB; i++)
				memory[1][j][i] = 'd';
		}
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");

		tst_resm(TINFO, "child 1 continues...");
		verify('d', 1, 0, size, 0, MB);
		tst_resm(TINFO, "child 1 changes one page to 'e'.");
		memory[1][size - 1][MB - 1] = 'e';

		/* Unmerge. */
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 1 continues...");
		verify('e', 1, size - 1, size, MB - 1, MB);
		verify('d', 1, 0, size - 1, 0, MB - 1);

		/* Stop. */
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill");
		tst_resm(TINFO, "child 1 continues...");
		exit(0);
	}
	for (k = 2; k < num; k++) {
		switch (child[k] = fork()) {
		case -1:
			tst_brkm(TBROK|TERRNO, NULL, "fork");
		case 0:
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			tst_resm(TINFO, "child %d allocates %d "
				"MB filled with 'a'.", k, size);
			memory[k] = malloc(size * sizeof(*memory));
			if (memory[k] == NULL)
				tst_brkm(TBROK|TERRNO, NULL, "malloc");
			for (j = 0; j < size; j++) {
				memory[k][j] = mmap(NULL, MB,
						PROT_READ|PROT_WRITE,
						MAP_ANONYMOUS
						|MAP_PRIVATE, -1, 0);
				if (memory[k][j] == MAP_FAILED)
					tst_brkm(TBROK|TERRNO, NULL,
						"mmap");
				if (madvise(memory[k][j], MB,
						MADV_MERGEABLE) == -1)
					tst_brkm(TBROK|TERRNO, NULL,
						"madvise");
				for (i = 0; i < MB; i++)
					memory[k][j][i] = 'a';
			}
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			tst_resm(TINFO, "child %d changes memory content to "
				"'d'", k);
			for (j = 0; j < size; j++) {
				for (i = 0; i < MB; i++)
					memory[k][j][i] = 'd';
		        }
			/* Unmerge. */
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "kill");
			tst_resm(TINFO, "child %d continues...", k);

			/* Stop. */
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, NULL, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			exit(0);
		}
	}
	tst_resm(TINFO, "KSM merging...");
	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, "run");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, NULL, "open");
	if (write(fd, "1", 1) != 1)
		tst_brkm(TBROK|TERRNO, NULL, "write");
	close(fd);
	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, "pages_to_scan");
	snprintf(buf2, BUFSIZ, "%d", size * 256 * num);
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, NULL, "open");
	if (write(fd, buf2, strlen(buf2)) != strlen(buf2))
		tst_brkm(TBROK|TERRNO, NULL, "write");
	close(fd);

	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, "sleep_millisecs");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, NULL, "open");
	if (write(fd, "0", 1) != 1)
		tst_brkm(TBROK|TERRNO, NULL, "write");
	close(fd);

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, NULL, "child %d was not stopped.",
				k);
	}
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill child[%d]", k);
	}
	group_check(1, 2, size * num * 256 - 2, 0, 0, 0, size * 256 * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, NULL, "child 1 was not stopped.");

	/* Child 1 changes all pages to 'b'. */
	tst_resm(TINFO, "resume child 1.");
	if (kill(child[1], SIGCONT) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "kill");
	group_check(1, 3, size * num * 256 - 3, 0, 0, 0, size * 256 * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, NULL, "child 1 was not stopped.");

	/* All children change pages to 'd'. */
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill child[%d]", k);
	}
	group_check(1, 1, size * num * 256 - 1, 0, 0, 0, size * 256 * num);

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, NULL, "child %d was not stopped.",
				k);
	}
	/* Child 1 changes pages to 'e'. */
	tst_resm(TINFO, "resume child 1.");
	if (kill(child[1], SIGCONT) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "kill");
	group_check(1, 1, size * num * 256 - 2, 0, 1, 0, size * 256 * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, NULL, "child 1 was not stopped.");

	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill child[%d]", k);
	}
	tst_resm(TINFO, "KSM unmerging...");
	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, "run");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, NULL, "open");
	if (write(fd, "2", 1) != 1)
		tst_brkm(TBROK|TERRNO, NULL, "write");
	group_check(2, 0, 0, 0, 0, 0, size * 256 * num);

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, NULL, "child %d was not stopped.",
				k);
	}
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, NULL, "kill child[%d]", k);
	}
	tst_resm(TINFO, "stop KSM.");
	if (lseek(fd, 0, SEEK_SET) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "lseek");
	if (write(fd, "0", 1) != 1)
		tst_brkm(TBROK|TERRNO, NULL, "write");
	close(fd);
	group_check(0, 0, 0, 0, 0, 0, size * 256 * num);
	while (waitpid(-1, &status, WUNTRACED | WCONTINUED) > 0)
		if (WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child exit status is %d",
				WEXITSTATUS(status));
}

void setup(void)
{
	char buf[BUFSIZ];
	struct stat new;

	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, "run");
	if (stat(buf, &new) == -1) {
		if (errno == ENOENT)
			tst_brkm(TCONF, NULL, "no KSM.");
		else
			tst_brkm(TBROK, NULL, "stat");
	}
	/*
	 * setup a default signal hander and a
	 * temporary working directory.
	 */
	tst_sig(FORK, DEF_HANDLER, NULL);
	TEST_PAUSE;
}

void usage(void)
{
	printf("  -n      Number of processes\n");
	printf("  -s      Memory allocation size in MB\n");
}

/* There is currently a bug will cause the test failure - *
 http://marc.info/?l=linux-mm&m=128928530308526&w=2 .  Since it has
 still been discussed upstream, the interface here was added some
 flexiblity with path2, so it is possible to make changes here depends
 on the future implementation in kernel. For example, to verify
 (pages_sharing + pages_volatile) instead of a single item. */
void check(char *path, char *path2, long int value)
{
	FILE *fp;
	char buf[BUFSIZ], buf2[BUFSIZ];

	snprintf(buf, BUFSIZ, "%s%s", _PATH_KSM, path);
	fp = fopen(buf, "r");
	if (fp == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "fopen");
	if (fgets(buf, BUFSIZ, fp) == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "fgets");
	fclose(fp);

	tst_resm(TINFO, "%s is %ld.", path, atol(buf));
	if (path2 != NULL) {
		snprintf(buf2, BUFSIZ, "%s%s", _PATH_KSM, path2);
		fp = fopen(buf2, "r");
		if (fp == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "fopen");
		if (fgets(buf2, BUFSIZ, fp) == NULL)
			tst_brkm(TBROK|TERRNO, NULL, "fgets");
		fclose(fp);

		tst_resm(TINFO, "%s is %ld.", path2, atol(buf2));
		if (atol(buf) + atol(buf2) != value)
			tst_resm(TFAIL, "%s + %s is not %ld.", path, path2,
				value);
	} else
		if (atol(buf) != value)
			tst_resm(TFAIL, "%s is not %ld.", path, value);
}

void verify(char value, int proc, int start, int end, int start2, int end2)
{
	int i, j;
	void *s = NULL;

	s = malloc((end - start) * (end2 - start2));
	if (s == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "malloc");

	tst_resm(TINFO, "child %d verifies memory content.", proc);
	memset(s, value, (end - start) * (end2 - start2));
	if (memcmp(memory[proc][start], s, (end - start) * (end2 - start2)) != 0)
		for (j = start; j < end; j++)
			for (i = start2; i < end2; i++)
				if (memory[proc][j][i] != value)
					tst_resm(TFAIL, "child %d has %c at "
						"%d,%d,%d.",
						proc, memory[proc][j][i], proc,
						j, i);
	free(s);
}

void group_check(int run, int pages_shared, int pages_sharing,
		int pages_volatile, int pages_unshared,
		int sleep_millisecs, int pages_to_scan)
{
        /* 5 seconds for ksm to scan pages. */
	sleep(5);
	tst_resm(TINFO, "check!");
	check("run", NULL, run);
	check("pages_shared", NULL, pages_shared);
	check("pages_sharing", NULL, pages_sharing);
	check("pages_volatile", NULL, pages_volatile);
	check("pages_unshared", NULL, pages_unshared);
	check("sleep_millisecs", NULL, sleep_millisecs);
	check("pages_to_scan", NULL, pages_to_scan);
}