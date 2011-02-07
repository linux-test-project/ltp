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
#include "oom.h"
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

void testoom(int mempolicy, int lite, int numa)
{
	int fd;
	char buf[BUFSIZ] = "";
	char cpus[BUFSIZ] = "";

	if (numa && !mempolicy) {
		gather_cpus(cpus);
		tst_resm(TINFO, "CPU list for 2nd node is %s.", cpus);

		fd = open(CPATH_NEW "/mems", O_WRONLY);
		if (fd == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
		if (write(fd, "1", 1) != 1)
			tst_brkm(TBROK|TERRNO, cleanup, "write %s", buf);
		close(fd);

		fd = open(CPATH_NEW "/cpus", O_WRONLY);
		if (fd == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "open %s", buf);
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
