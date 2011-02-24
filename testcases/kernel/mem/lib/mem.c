#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "test.h"
#include "usctest.h"
#include "../include/mem.h"
#include "config.h"
#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
#include <numaif.h>
#endif

void oom(int testcase, int mempolicy, int lite)
{
	pid_t pid;
	int status;
#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
	unsigned long nmask = 2;
#endif

	switch(pid = fork()) {
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
		test_alloc(testcase, lite);
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

void write_memcg(void)
{
	int fd;
	char buf[BUFSIZ], mem[BUFSIZ];

	fd = open(MEMCG_PATH_NEW "/memory.limit_in_bytes", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
	sprintf(mem, "%ld", TESTMEM);
	if (write(fd, mem, strlen(mem)) != strlen(mem))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
	close(fd);

	fd = open(MEMCG_PATH_NEW "/tasks", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
	snprintf(buf, BUFSIZ, "%d", getpid());
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
	close(fd);	
}

void write_cpusets(void)
{
	char cpus[BUFSIZ] = "";
	char buf[BUFSIZ] = "";
	int fd;

	gather_cpus(cpus);
	tst_resm(TINFO, "CPU list for 2nd node is %s.", cpus);

	/* try either '/dev/cpuset/mems' or '/dev/cpuset/cpuset.mems'
	 * please see Documentation/cgroups/cpusets.txt of kernel src for detail */
	fd = open(CPATH_NEW "/mems", O_WRONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			fd = open(CPATH_NEW "/cpuset.mems", O_WRONLY);
			if (fd == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
		} else
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
	}
	if (write(fd, "1", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
	close(fd);

	/* try either '/dev/cpuset/cpus' or '/dev/cpuset/cpuset.cpus'
	 * please see Documentation/cgroups/cpusets.txt of kernel src for detail */
	fd = open(CPATH_NEW "/cpus", O_WRONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			fd = open(CPATH_NEW "/cpuset.cpus", O_WRONLY);
			if (fd == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
		} else
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
	}
	if (write(fd, cpus, strlen(cpus)) != strlen(cpus))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
	close(fd);

	fd = open(CPATH_NEW "/tasks", O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
	snprintf(buf, BUFSIZ, "%d", getpid());
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
	close(fd);
}

void testoom(int mempolicy, int lite, int numa)
{
	if (numa && !mempolicy)
		write_cpusets();

	tst_resm(TINFO, "start normal OOM testing.");
	oom(NORMAL, mempolicy, lite);

	tst_resm(TINFO, "start OOM testing for mlocked pages.");
	oom(MLOCK, mempolicy, lite);

	tst_resm(TINFO, "start OOM testing for KSM pages.");
	oom(KSM, mempolicy, lite);
}

long count_numa(void)
{
	int nnodes = 0;

	while(path_exist(PATH_SYS_SYSTEM "/node/node%d", nnodes))
		nnodes++;

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

void gather_cpus(char *cpus)
{
	int ncpus = 0;
	int i;
	char buf[BUFSIZ];

	while(path_exist(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	for (i = 0; i < ncpus; i++)
		if (path_exist(PATH_SYS_SYSTEM "/node/node1/cpu%d", i)) {
			sprintf(buf, "%d,", i);
			strcat(cpus, buf);
		}
	/* Remove the trailing comma. */
	cpus[strlen(cpus) - 1] = '\0';
}

int alloc_mem(long int length, int testcase)
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

void test_alloc(int testcase, int lite)
{
	if (lite)
		alloc_mem(TESTMEM + MB, testcase);
	else
		while(1)
			if (alloc_mem(LENGTH, testcase))
				return;
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
	if (mount(name, path, fs, 0, options) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mount %s", path);
	if (mkdir(path_new, 0777) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir %s", path_new);
}

void ksm_usage(void)
{
	printf("  -n      Number of processes\n");
	printf("  -s      Memory allocation size in MB\n");
	printf("  -u      Memory allocation unit in MB\n");
}

void check(char *path, long int value)
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

void verify(char value, int proc, int start, int end, int start2, int end2)
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

void group_check(int run, int pages_shared, int pages_sharing,
		int pages_volatile, int pages_unshared,
		int sleep_millisecs, int pages_to_scan)
{
	int fd;
	char buf[BUFSIZ];
	int old_num, new_num;

	/* 1 seconds for ksm to scan pages. */
	while (sleep(1) == 1)
	    continue;

	fd = open("/sys/kernel/mm/ksm/full_scans", O_RDONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");

	/* wait 3 increments of full_scans */
	if (read(fd, buf, BUFSIZ) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "read");
	old_num = new_num = atoi(buf);
	if (lseek(fd, 0, SEEK_SET) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "lseek");
	while (new_num < old_num * 3) {
		sleep(1);
		if (read(fd, buf, BUFSIZ) < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "read");
		new_num = atoi(buf);
		if (lseek(fd, 0, SEEK_SET) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "lseek");
	}
	close(fd);

	tst_resm(TINFO, "check!");
	check("run", run);
	check("pages_shared", pages_shared);
	check("pages_sharing", pages_sharing);
	check("pages_volatile", pages_volatile);
	check("pages_unshared", pages_unshared);
	check("sleep_millisecs", sleep_millisecs);
	check("pages_to_scan", pages_to_scan);
}

void create_same_memory(int size, int num, int unit)
{
	char buf[BUFSIZ], buf2[BUFSIZ];
	int i, j, k;
	int status, fd;
	int *child;

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
		verify('c', 0, 0, size / unit, 0, unit * MB);
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
		verify('d', 0, 0, size / unit, 0, unit * MB);
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
		verify('a', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes memory content to 'b'.");
		for (j = 0; j < size / unit; j++) {
			for (i = 0; i < unit * MB; i++)
				memory[1][j][i] = 'b';
		}
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		verify('b', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes memory content to 'd'");
		for (j = 0; j < size / unit; j++) {
			for (i = 0; i < unit * MB; i++)
				memory[1][j][i] = 'd';
		}
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

		tst_resm(TINFO, "child 1 continues...");
		verify('d', 1, 0, size / unit, 0, unit * MB);
		tst_resm(TINFO, "child 1 changes one page to 'e'.");
		memory[1][size / unit - 1][unit * MB - 1] = 'e';

		/* Unmerge. */
		tst_resm(TINFO, "child 1 stops.");
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");
		tst_resm(TINFO, "child 1 continues...");
		verify('e', 1, size / unit - 1, size / unit,
			unit * MB - 1, unit * MB);
		verify('d', 1, 0, size / unit - 1, 0, unit * MB - 1);

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
	snprintf(buf, BUFSIZ, "%s%s", PATH_KSM, "run");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (write(fd, "1", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	close(fd);
	snprintf(buf, BUFSIZ, "%s%s", PATH_KSM, "pages_to_scan");
	snprintf(buf2, BUFSIZ, "%d", size * 256 * num);
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (write(fd, buf2, strlen(buf2)) != strlen(buf2))
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	close(fd);

	snprintf(buf, BUFSIZ, "%s%s", PATH_KSM, "sleep_millisecs");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (write(fd, "0", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	close(fd);

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
	group_check(1, 2, size * num * 256 - 2, 0, 0, 0, size * 256 * num);

	tst_resm(TINFO, "wait for child 1 to stop.");
	if (waitpid(child[1], &status, WUNTRACED) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
	if (!WIFSTOPPED(status))
		tst_brkm(TBROK, cleanup, "child 1 was not stopped.");

	/* Child 1 changes all pages to 'b'. */
	tst_resm(TINFO, "resume child 1.");
	if (kill(child[1], SIGCONT) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "kill");
	group_check(1, 3, size * num * 256 - 3, 0, 0, 0, size * 256 * num);

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
	group_check(1, 1, size * num * 256 - 1, 0, 0, 0, size * 256 * num);

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
	group_check(1, 1, size * num * 256 - 2, 0, 1, 0, size * 256 * num);

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
	snprintf(buf, BUFSIZ, "%s%s", PATH_KSM, "run");
	fd = open(buf, O_WRONLY);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open");
	if (write(fd, "2", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	group_check(2, 0, 0, 0, 0, 0, size * 256 * num);

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
	if (lseek(fd, 0, SEEK_SET) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "lseek");
	if (write(fd, "0", 1) != 1)
		tst_brkm(TBROK|TERRNO, cleanup, "write");
	close(fd);
	group_check(0, 0, 0, 0, 0, 0, size * 256 * num);
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
