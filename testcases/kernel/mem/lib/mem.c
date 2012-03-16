#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "_private.h"
#include "mem.h"

/* OOM */

static int _alloc_mem(long int length, int testcase)
{
	void *s;

	tst_resm(TINFO, "allocating %ld bytes.", length);
	s = mmap(NULL, length, PROT_READ|PROT_WRITE,
		MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (s == MAP_FAILED) {
		if (testcase == OVERCOMMIT && errno == ENOMEM)
			return 1;
		else
			tst_brkm(TBROK|TERRNO, cleanup, "mmap");
	}
	if (testcase == MLOCK && mlock(s, length) == -1)
		tst_brkm(TINFO|TERRNO, cleanup, "mlock");
#ifdef HAVE_MADV_MERGEABLE
	if (testcase == KSM
		&& madvise(s, length, MADV_MERGEABLE) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "madvise");
#endif
	memset(s, '\a', length);

	return 0;
}

static void _test_alloc(int testcase, int lite)
{
	if (lite)
		_alloc_mem(TESTMEM + MB, testcase);
	else
		while (1)
			if (_alloc_mem(LENGTH, testcase))
				return;
}

void oom(int testcase, int mempolicy, int lite)
{
	pid_t pid;
	int status;
#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
	unsigned long nmask = 2;
#endif

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, cleanup, "fork");
	case 0:
#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
		if (mempolicy)
			if (set_mempolicy(MPOL_BIND, &nmask, MAXNODES) == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
					"set_mempolicy");
#endif
		_test_alloc(testcase, lite);
		exit(0);
	default:
		break;
	}
	tst_resm(TINFO, "expected victim is %d.", pid);
	if (waitpid(-1, &status, 0) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "waitpid");

	if (testcase == OVERCOMMIT) {
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "the victim unexpectedly failed: %d",
				status);
	} else {
		if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL)
			tst_resm(TFAIL, "the victim unexpectedly failed: %d",
				status);
	}
}

void testoom(int mempolicy, int lite, int numa)
{
	long nodes[MAXNODES];

	if (numa && !mempolicy) {
		if (count_numa(nodes) <= 1)
			tst_brkm(TCONF, cleanup, "required a NUMA system.");
		/* write cpusets to 2nd node */
		write_cpusets(nodes[1]);
	}

	tst_resm(TINFO, "start normal OOM testing.");
	oom(NORMAL, mempolicy, lite);

	tst_resm(TINFO, "start OOM testing for mlocked pages.");
	oom(MLOCK, mempolicy, lite);

	if (access(PATH_KSM, F_OK) == -1)
		tst_brkm(TCONF, NULL, "KSM configuration is not enabled");

	tst_resm(TINFO, "start OOM testing for KSM pages.");
	oom(KSM, mempolicy, lite);
}

/* KSM */

static void _check(char *path, long int value)
{
	FILE *fp;
	char buf[BUFSIZ];

	snprintf(buf, BUFSIZ, "%s%s", PATH_KSM, path);
	fp = fopen(buf, "r");
	if (fp == NULL)
		tst_brkm(TBROK|TERRNO, tst_exit, "fopen");
	if (fgets(buf, BUFSIZ, fp) == NULL)
		tst_brkm(TBROK|TERRNO, tst_exit, "fgets");
	fclose(fp);

	tst_resm(TINFO, "%s is %ld.", path, atol(buf));
	if (atol(buf) != value)
		tst_resm(TFAIL, "%s is not %ld.", path, value);
}

static void _group_check(int run, int pages_shared, int pages_sharing,
		int pages_volatile, int pages_unshared,
		int sleep_millisecs, int pages_to_scan)
{
	char buf[BUFSIZ];
	long old_num, new_num;

	/* 1 seconds for ksm to scan pages. */
	while (sleep(1) == 1)
		continue;

	read_file(PATH_KSM "full_scans", buf);
	/* wait 3 increments of full_scans */
	old_num = SAFE_STRTOL(cleanup, buf, 0, LONG_MAX);
	new_num = old_num;
	while (new_num < old_num * 3) {
		sleep(1);
		read_file(PATH_KSM "full_scans", buf);
		new_num = SAFE_STRTOL(cleanup, buf, 0, LONG_MAX);
	}

	tst_resm(TINFO, "check!");
	_check("run", run);
	_check("pages_shared", pages_shared);
	_check("pages_sharing", pages_sharing);
	_check("pages_volatile", pages_volatile);
	_check("pages_unshared", pages_unshared);
	_check("sleep_millisecs", sleep_millisecs);
	_check("pages_to_scan", pages_to_scan);
}

static void _verify(char value, int proc, int start, int end,
		int start2, int end2)
{
	int i, j;
	void *s = NULL;

	s = malloc((end - start) * (end2 - start2));
	if (s == NULL)
		tst_brkm(TBROK|TERRNO, tst_exit, "malloc");

	tst_resm(TINFO, "child %d verifies memory content.", proc);
	memset(s, value, (end - start) * (end2 - start2));
	if (memcmp(memory[proc][start], s, (end - start) * (end2 - start2))
		!= 0)
		for (j = start; j < end; j++)
			for (i = start2; i < end2; i++)
				if (memory[proc][j][i] != value)
					tst_resm(TFAIL, "child %d has %c at "
						"%d,%d,%d.",
						proc, memory[proc][j][i], proc,
						j, i);
	free(s);
}

void write_memcg(void)
{
	char buf[BUFSIZ], mem[BUFSIZ];

	snprintf(mem, BUFSIZ, "%ld", TESTMEM);
	write_file(MEMCG_PATH_NEW "/memory.limit_in_bytes", mem);

	snprintf(buf, BUFSIZ, "%d", getpid());
	write_file(MEMCG_PATH_NEW "/tasks", buf);
}

void create_same_memory(int size, int num, int unit)
{
	char buf[BUFSIZ];
	int i, j, k;
	int status;
	int *child;
	long ps, pages;

	ps = sysconf(_SC_PAGE_SIZE);
	pages = 1024 * 1024 / ps;

	child = malloc(num);
	if (child == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "malloc");

	memory = malloc(num * sizeof(**memory));
	if (memory == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "malloc");

	/* Don't call cleanup in those children. Instead, do a cleanup from the
	   parent after fetched children's status.*/
	switch (child[0] = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, cleanup, "fork");
	case 0:
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

		tst_resm(TINFO, "child 0 continues...");
		tst_resm(TINFO, "child 0 allocates %d MB filled with 'c'.",
			size);
		memory[0] = malloc(size / unit * sizeof(*memory));
		if (memory[0] == NULL)
			tst_brkm(TBROK|TERRNO, tst_exit, "malloc");
		for (j = 0; j * unit < size; j++) {
			memory[0][j] = mmap(NULL, unit * MB,
					PROT_READ|PROT_WRITE,
					MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
			if (memory[0][j] == MAP_FAILED)
				tst_brkm(TBROK|TERRNO, tst_exit, "mmap");

#ifdef HAVE_MADV_MERGEABLE
			if (madvise(memory[0][j], unit * MB, MADV_MERGEABLE)
				== -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "madvise");
#endif
			for (i = 0; i < unit * MB; i++)
				memory[0][j][i] = 'c';
		}
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

		tst_resm(TINFO, "child 0 continues...");
		_verify('c', 0, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 0 changes memory content to 'd'.");
		for (j = 0; j < size / unit; j++) {
			for (i = 0; i < unit * MB; i++)
				memory[0][j][i] = 'd';
		}
		/* Unmerge. */
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

		tst_resm(TINFO, "child 0 continues...");
		_verify('d', 0, 0, size / unit, 0, unit * MB);
		/* Stop. */
		tst_resm(TINFO, "child 0 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 0 continues...");
		exit(0);
	}
	switch (child[1] = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, cleanup, "fork");
	case 0:
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		tst_resm(TINFO, "child 1 allocates %d MB filled with 'a'.",
			size);
		memory[1] = malloc(size / unit * sizeof(*memory));
		if (memory[1] == NULL)
			tst_brkm(TBROK|TERRNO, tst_exit, "malloc");
		for (j = 0; j < size / unit; j++) {
			memory[1][j] = mmap(NULL, unit * MB,
					PROT_READ|PROT_WRITE,
					MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
			if (memory[1][j] == MAP_FAILED)
				tst_brkm(TBROK|TERRNO, tst_exit, "mmap");
#ifdef HAVE_MADV_MERGEABLE
			if (madvise(memory[1][j], unit * MB, MADV_MERGEABLE)
				== -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "madvise");
#endif
			for (i = 0; i < unit * MB; i++)
				memory[1][j][i] = 'a';
		}
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		_verify('a', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes memory content to 'b'.");
		for (j = 0; j < size / unit; j++) {
			for (i = 0; i < unit * MB; i++)
				memory[1][j][i] = 'b';
		}
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		_verify('b', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes memory content to 'd'");
		for (j = 0; j < size / unit; j++) {
			for (i = 0; i < unit * MB; i++)
				memory[1][j][i] = 'd';
		}
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

		tst_resm(TINFO, "child 1 continues...");
		_verify('d', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes one page to 'e'.");
		memory[1][size / unit - 1][unit * MB - 1] = 'e';

		/* Unmerge. */
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		_verify('e', 1, size / unit - 1, size / unit,
			unit * MB - 1, unit * MB);
		_verify('d', 1, 0, size / unit - 1, 0, unit * MB - 1);

		/* Stop. */
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		exit(0);
	}
	for (k = 2; k < num; k++) {
		switch (child[k] = fork()) {
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "fork");
		case 0:
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			tst_resm(TINFO, "child %d allocates %d "
				"MB filled with 'a'.", k, size);
			memory[k] = malloc(size / unit * sizeof(*memory));
			if (memory[k] == NULL)
				tst_brkm(TBROK|TERRNO, tst_exit, "malloc");
			for (j = 0; j < size / unit; j++) {
				memory[k][j] = mmap(NULL, unit * MB,
						PROT_READ|PROT_WRITE,
						MAP_ANONYMOUS
						|MAP_PRIVATE, -1, 0);
				if (memory[k][j] == MAP_FAILED)
					tst_brkm(TBROK|TERRNO, cleanup,
						"mmap");
#ifdef HAVE_MADV_MERGEABLE
				if (madvise(memory[k][j], unit * MB,
						MADV_MERGEABLE) == -1)
					tst_brkm(TBROK|TERRNO, cleanup,
						"madvise");
#endif
				for (i = 0; i < unit * MB; i++)
					memory[k][j][i] = 'a';
			}
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			tst_resm(TINFO, "child %d changes memory content to "
				"'d'", k);
			for (j = 0; j < size / unit; j++) {
				for (i = 0; i < unit * MB; i++)
					memory[k][j][i] = 'd';
			}
			/* Unmerge. */
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "kill");
			tst_resm(TINFO, "child %d continues...", k);

			/* Stop. */
			tst_resm(TINFO, "child %d stops.", k);
			if (raise(SIGSTOP) == -1)
				tst_brkm(TBROK|TERRNO, tst_exit, "kill");
			tst_resm(TINFO, "child %d continues...", k);
			exit(0);
		}
	}
	tst_resm(TINFO, "KSM merging...");
	write_file(PATH_KSM "run", "1");
	snprintf(buf, BUFSIZ, "%ld", size * pages * num);
	write_file(PATH_KSM "pages_to_scan", buf);
	write_file(PATH_KSM "sleep_millisecs", "0");

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup, "child %d was not stopped.",
				k);
	}
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "kill child[%d]", k);
	}
	_group_check(1, 2, size * num * pages - 2, 0, 0, 0, size * pages * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, cleanup, "child 1 was not stopped.");

	/* Child 1 changes all pages to 'b'. */
	tst_resm(TINFO, "resume child 1.");
	if (kill(child[1], SIGCONT) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "kill");
	_group_check(1, 3, size * num * pages - 3, 0, 0, 0, size * pages * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, cleanup, "child 1 was not stopped.");

	/* All children change pages to 'd'. */
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "kill child[%d]", k);
	}
	_group_check(1, 1, size * num * pages - 1, 0, 0, 0, size * pages * num);

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup, "child %d was not stopped.",
				k);
	}
	/* Child 1 changes pages to 'e'. */
	tst_resm(TINFO, "resume child 1.");
	if (kill(child[1], SIGCONT) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "kill");
	_group_check(1, 1, size * num * pages - 2, 0, 1, 0, size * pages * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, cleanup, "child 1 was not stopped.");

	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "kill child[%d]", k);
	}
	tst_resm(TINFO, "KSM unmerging...");
	write_file(PATH_KSM "run", "2");
	_group_check(2, 0, 0, 0, 0, 0, size * pages * num);

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup, "child %d was not stopped.",
				k);
	}
	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "kill child[%d]", k);
	}
	tst_resm(TINFO, "stop KSM.");
	write_file(PATH_KSM "run", "0");
	_group_check(0, 0, 0, 0, 0, 0, size * pages * num);
	while (waitpid(-1, &status, WUNTRACED | WCONTINUED) > 0)
		if (WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child exit status is %d",
				WEXITSTATUS(status));
}

void check_ksm_options(int *size, int *num, int *unit)
{
	if (opt_size) {
		*size = atoi(opt_sizestr);
		if (*size < 1)
			tst_brkm(TBROK, cleanup,
				"size cannot be less than 1.");
	}
	if (opt_unit) {
		*unit = atoi(opt_unitstr);
		if (*unit > *size)
			tst_brkm(TBROK, cleanup,
				"unit cannot be greater than size.");
		if (*size % *unit != 0)
			tst_brkm(TBROK, cleanup,
				"the remainder of division of size by unit is "
				"not zero.");
	}
	if (opt_num) {
		*num = atoi(opt_numstr);
		if (*num < 3)
			tst_brkm(TBROK, cleanup,
				"process number cannot be less 3.");
	}
}

void ksm_usage(void)
{
	printf("  -n      Number of processes\n");
	printf("  -s      Memory allocation size in MB\n");
	printf("  -u      Memory allocation unit in MB\n");
}

/* cpuset/memcg */

static void _gather_cpus(char *cpus, long nd)
{
	int ncpus = 0;
	int i;
	char buf[BUFSIZ];

	while (path_exist(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	for (i = 0; i < ncpus; i++)
		if (path_exist(PATH_SYS_SYSTEM "/node/node%ld/cpu%d", nd, i)) {
			sprintf(buf, "%d,", i);
			strcat(cpus, buf);
		}
	/* Remove the trailing comma. */
	cpus[strlen(cpus) - 1] = '\0';
}

void read_cpuset_files(char *prefix, char *filename, char *retbuf)
{
	int fd;
	char path[BUFSIZ];

	/*
	 * try either '/dev/cpuset/XXXX' or '/dev/cpuset/cpuset.XXXX'
	 * please see Documentation/cgroups/cpusets.txt from kernel src
	 * for details
	 */
	snprintf(path, BUFSIZ, "%s/%s", prefix, filename);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			snprintf(path, BUFSIZ, "%s/cpuset.%s",
					prefix, filename);
			fd = open(path, O_RDONLY);
			if (fd == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
					    "open %s", path);
		} else
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", path);
	}
	if (read(fd, retbuf, BUFSIZ) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "read %s", path);
	close(fd);
}

void write_cpuset_files(char *prefix, char *filename, char *buf)
{
	int fd;
	char path[BUFSIZ];

	/*
	 * try either '/dev/cpuset/XXXX' or '/dev/cpuset/cpuset.XXXX'
	 * please see Documentation/cgroups/cpusets.txt from kernel src
	 * for details
	 */
	snprintf(path, BUFSIZ, "%s/%s", prefix, filename);
	fd = open(path, O_WRONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			snprintf(path, BUFSIZ, "%s/cpuset.%s",
					prefix, filename);
			fd = open(path, O_WRONLY);
			if (fd == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
					    "open %s", path);
		} else
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", path);
	}
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", path);
	close(fd);
}

void write_cpusets(long nd)
{
	char buf[BUFSIZ];
	char cpus[BUFSIZ] = "";

	snprintf(buf, BUFSIZ, "%ld", nd);
	write_cpuset_files(CPATH_NEW, "mems", buf);

	_gather_cpus(cpus, nd);
	write_cpuset_files(CPATH_NEW, "cpus", cpus);

	snprintf(buf, BUFSIZ, "%d", getpid());
	write_file(CPATH_NEW "/tasks", buf);
}

void umount_mem(char *path, char *path_new)
{
	FILE *fp;
	int fd;
	char s_new[BUFSIZ], s[BUFSIZ], value[BUFSIZ];

	/* Move all processes in task to its parent node. */
	sprintf(s, "%s/tasks", path);
	fd = open(s, O_WRONLY);
	if (fd == -1)
		tst_resm(TWARN|TERRNO, "open %s", s);

	snprintf(s_new, BUFSIZ, "%s/tasks", path_new);
	fp = fopen(s_new, "r");
	if (fp == NULL)
		tst_resm(TWARN|TERRNO, "fopen %s", s_new);
	if ((fd != -1) && (fp != NULL)) {
		while (fgets(value, BUFSIZ, fp) != NULL)
			if (write(fd, value, strlen(value) - 1)
				!= strlen(value) - 1)
				tst_resm(TWARN|TERRNO, "write %s", s);
	}
	if (fd != -1)
		close(fd);
	if (fp != NULL)
		fclose(fp);
	if (rmdir(path_new) == -1)
		tst_resm(TWARN|TERRNO, "rmdir %s", path_new);
	if (umount(path) == -1)
		tst_resm(TWARN|TERRNO, "umount %s", path);
	if (rmdir(path) == -1)
		tst_resm(TWARN|TERRNO, "rmdir %s", path);
}

void mount_mem(char *name, char *fs, char *options, char *path, char *path_new)
{
	if (mkdir(path, 0777) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir %s", path);
	if (mount(name, path, fs, 0, options) == -1) {
		if (errno == ENODEV) {
			if (rmdir(path) == -1)
				tst_resm(TWARN|TERRNO, "rmdir %s failed",
				    path);
			tst_brkm(TCONF, NULL,
			    "file system %s is not configured in kernel", fs);
		}
		tst_brkm(TBROK|TERRNO, cleanup, "mount %s", path);
	}
	if (mkdir(path_new, 0777) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir %s", path_new);
}

/* shared */

long count_numa(long nodes[])
{
	long nnodes, i;

	nnodes = 0;
	for (i = 0; i <= MAXNODES; i++)
		if(path_exist(PATH_SYS_SYSTEM "/node/node%d", i))
			nodes[nnodes++] = i;

	return nnodes;
}

int path_exist(const char *path, ...)
{
	va_list ap;
	char pathbuf[PATH_MAX];

	va_start(ap, path);
	vsnprintf(pathbuf, sizeof(pathbuf), path, ap);
	va_end(ap);

	return access(pathbuf, F_OK) == 0;
}

long read_meminfo(char *item)
{
	FILE *fp;
	char line[BUFSIZ], buf[BUFSIZ];
	long val;

	fp = fopen(PATH_MEMINFO, "r");
	if (fp == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "fopen %s", PATH_MEMINFO);

	while (fgets(line, BUFSIZ, fp) != NULL) {
		if (sscanf(line, "%64s %ld", buf, &val) == 2)
			if (strcmp(buf, item) == 0) {
				fclose(fp);
				return val;
			}
		continue;
	}
	fclose(fp);

	tst_brkm(TBROK, cleanup, "cannot find \"%s\" in %s",
			item, PATH_MEMINFO);
}

void set_sys_tune(char *sys_file, long tune, int check)
{
	long val;
	char buf[BUFSIZ], path[BUFSIZ];

	tst_resm(TINFO, "set %s to %ld", sys_file, tune);

	snprintf(path, BUFSIZ, "%s%s", PATH_SYSVM, sys_file);
	snprintf(buf, BUFSIZ, "%ld", tune);
	write_file(path, buf);

	if (check) {
		val = get_sys_tune(sys_file);
		if (val != tune)
			tst_brkm(TBROK, cleanup, "%s = %ld, but expect %ld",
					sys_file, val, tune);
	}
}

long get_sys_tune(char *sys_file)
{
	char buf[BUFSIZ], path[BUFSIZ];

	snprintf(path, BUFSIZ, "%s%s", PATH_SYSVM, sys_file);
	read_file(path, buf);
	return SAFE_STRTOL(cleanup, buf, LONG_MIN, LONG_MAX);
}

void write_file(char *filename, char *buf)
{
	int fd;

	fd = open(filename, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", filename);
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", filename);
	close(fd);
}

void read_file(char *filename, char *retbuf)
{
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", filename);
	if (read(fd, retbuf, BUFSIZ) < 0)
		tst_brkm(TBROK|TERRNO, cleanup, "read %s", filename);
	close(fd);
}
