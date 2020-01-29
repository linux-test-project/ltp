#define TST_NO_DEFAULT_MAIN

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem.h"
#include "numa_helper.h"

/* OOM */

long overcommit = -1;

static int alloc_mem(long int length, int testcase)
{
	char *s;
	long i, pagesz = getpagesize();
	int loop = 10;

	tst_res(TINFO, "thread (%lx), allocating %ld bytes.",
		(unsigned long) pthread_self(), length);

	s = mmap(NULL, length, PROT_READ | PROT_WRITE,
		 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (s == MAP_FAILED)
		return errno;

	if (testcase == MLOCK) {
		while (mlock(s, length) == -1 && loop > 0) {
			if (EAGAIN != errno)
				return errno;
			usleep(300000);
			loop--;
		}
	}

#ifdef HAVE_DECL_MADV_MERGEABLE
	if (testcase == KSM && madvise(s, length, MADV_MERGEABLE) == -1)
		return errno;
#endif
	for (i = 0; i < length; i += pagesz)
		s[i] = '\a';

	return 0;
}

static void *child_alloc_thread(void *args)
{
	int ret = 0;

	/* keep allocating until there's an error */
	while (!ret)
		ret = alloc_mem(LENGTH, (long)args);
	exit(ret);
}

static void child_alloc(int testcase, int lite, int threads)
{
	int i;
	pthread_t *th;

	if (lite) {
		int ret = alloc_mem(TESTMEM + MB, testcase);
		exit(ret);
	}

	th = malloc(sizeof(pthread_t) * threads);
	if (!th) {
		tst_res(TINFO | TERRNO, "malloc");
		goto out;
	}

	for (i = 0; i < threads; i++) {
		TEST(pthread_create(&th[i], NULL, child_alloc_thread,
			(void *)((long)testcase)));
		if (TST_RET) {
			tst_res(TINFO | TRERRNO, "pthread_create");
			/*
			 * Keep going if thread other than first fails to
			 * spawn due to lack of resources.
			 */
			if (i == 0 || TST_RET != EAGAIN)
				goto out;
		}
	}

	/* wait for one of threads to exit whole process */
	while (1)
		sleep(1);
out:
	exit(1);
}

/*
 * oom - allocates memory according to specified testcase and checks
 *       desired outcome (e.g. child killed, operation failed with ENOMEM)
 * @testcase: selects how child allocates memory
 *            valid choices are: NORMAL, MLOCK and KSM
 * @lite: if non-zero, child makes only single TESTMEM+MB allocation
 *        if zero, child keeps allocating memory until it gets killed
 *        or some operation fails
 * @retcode: expected return code of child process
 *           if matches child ret code, this function reports PASS,
 *           otherwise it reports FAIL
 * @allow_sigkill: if zero and child is killed, this function reports FAIL
 *                 if non-zero, then if child is killed by SIGKILL
 *                 it is considered as PASS
 */
void oom(int testcase, int lite, int retcode, int allow_sigkill)
{
	pid_t pid;
	int status, threads;

	switch (pid = SAFE_FORK()) {
	case 0:
		threads = MAX(1, tst_ncpus() - 1);
		child_alloc(testcase, lite, threads);
	default:
		break;
	}

	tst_res(TINFO, "expected victim is %d.", pid);
	SAFE_WAITPID(-1, &status, 0);

	if (WIFSIGNALED(status)) {
		if (allow_sigkill && WTERMSIG(status) == SIGKILL) {
			tst_res(TPASS, "victim signalled: (%d) %s",
				SIGKILL,
				tst_strsig(SIGKILL));
		} else {
			tst_res(TFAIL, "victim signalled: (%d) %s",
				WTERMSIG(status),
				tst_strsig(WTERMSIG(status)));
		}
	} else if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == retcode) {
			tst_res(TPASS, "victim retcode: (%d) %s",
				retcode, strerror(retcode));
		} else {
			tst_res(TFAIL, "victim unexpectedly ended with "
				"retcode: %d, expected: %d",
				WEXITSTATUS(status), retcode);
		}
	} else {
		tst_res(TFAIL, "victim unexpectedly ended");
	}
}

#ifdef HAVE_NUMA_V2
static void set_global_mempolicy(int mempolicy)
{
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	int num_nodes, *nodes;
	int ret;

	if (mempolicy) {
		ret = get_allowed_nodes_arr(NH_MEMS|NH_CPUS, &num_nodes, &nodes);
		if (ret != 0)
			tst_brk(TBROK|TERRNO, "get_allowed_nodes_arr");
		if (num_nodes < 2) {
			tst_res(TINFO, "mempolicy need NUMA system support");
			free(nodes);
			return;
		}
		switch(mempolicy) {
		case MPOL_BIND:
			/* bind the second node */
			set_node(nmask, nodes[1]);
			break;
		case MPOL_INTERLEAVE:
		case MPOL_PREFERRED:
			if (num_nodes == 2) {
				tst_res(TINFO, "The mempolicy need "
					 "more than 2 numa nodes");
				free(nodes);
				return;
			} else {
				/* Using the 2nd,3rd node */
				set_node(nmask, nodes[1]);
				set_node(nmask, nodes[2]);
			}
			break;
		default:
			tst_brk(TBROK|TERRNO, "Bad mempolicy mode");
		}
		if (set_mempolicy(mempolicy, nmask, MAXNODES) == -1)
			tst_brk(TBROK|TERRNO, "set_mempolicy");
	}
}
#else
static void set_global_mempolicy(int mempolicy LTP_ATTRIBUTE_UNUSED) { }
#endif

void testoom(int mempolicy, int lite, int retcode, int allow_sigkill)
{
	int ksm_run_orig;

	set_global_mempolicy(mempolicy);

	tst_res(TINFO, "start normal OOM testing.");
	oom(NORMAL, lite, retcode, allow_sigkill);

	tst_res(TINFO, "start OOM testing for mlocked pages.");
	oom(MLOCK, lite, retcode, allow_sigkill);

	/*
	 * Skip oom(KSM) if lite == 1, since limit_in_bytes may vary from
	 * run to run, which isn't reliable for oom03 cgroup test.
	 */
	if (access(PATH_KSM, F_OK) == -1 || lite == 1) {
		tst_res(TINFO, "KSM is not configed or lite == 1, "
			 "skip OOM test for KSM pags");
	} else {
		tst_res(TINFO, "start OOM testing for KSM pages.");
		SAFE_FILE_SCANF(PATH_KSM "run", "%d", &ksm_run_orig);
		SAFE_FILE_PRINTF(PATH_KSM "run", "1");
		oom(KSM, lite, retcode, allow_sigkill);
		SAFE_FILE_PRINTF(PATH_KSM "run", "%d", ksm_run_orig);
	}
}

/* KSM */

static void check(char *path, long int value)
{
	char fullpath[BUFSIZ];
	long actual_val;

	snprintf(fullpath, BUFSIZ, PATH_KSM "%s", path);
	SAFE_FILE_SCANF(fullpath, "%ld", &actual_val);

	if (actual_val != value)
		tst_res(TFAIL, "%s is not %ld but %ld.", path, value,
			actual_val);
	else
		tst_res(TPASS, "%s is %ld.", path, actual_val);
}

static void final_group_check(int run, int pages_shared, int pages_sharing,
			  int pages_volatile, int pages_unshared,
			  int sleep_millisecs, int pages_to_scan)
{
	tst_res(TINFO, "check!");
	check("run", run);
	check("pages_shared", pages_shared);
	check("pages_sharing", pages_sharing);
	check("pages_volatile", pages_volatile);
	check("pages_unshared", pages_unshared);
	check("sleep_millisecs", sleep_millisecs);
	check("pages_to_scan", pages_to_scan);
}

static void group_check(int run, int pages_shared, int pages_sharing,
			int pages_volatile, int pages_unshared,
			int sleep_millisecs, int pages_to_scan)
{
	if (run != 1) {
		tst_res(TFAIL, "group_check run is not 1, %d.", run);
	} else {
		/* wait for ksm daemon to scan all mergeable pages. */
		wait_ksmd_full_scan();
	}

	final_group_check(run, pages_shared, pages_sharing,
			  pages_volatile, pages_unshared,
			  sleep_millisecs, pages_to_scan);
}

static void verify(char **memory, char value, int proc,
		    int start, int end, int start2, int end2)
{
	int i, j;
	void *s = NULL;

	s = SAFE_MALLOC((end - start) * (end2 - start2));

	tst_res(TINFO, "child %d verifies memory content.", proc);
	memset(s, value, (end - start) * (end2 - start2));
	if (memcmp(memory[start], s, (end - start) * (end2 - start2))
	    != 0)
		for (j = start; j < end; j++)
			for (i = start2; i < end2; i++)
				if (memory[j][i] != value)
					tst_res(TFAIL, "child %d has %c at "
						 "%d,%d,%d.",
						 proc, memory[j][i], proc,
						 j, i);
	free(s);
}

void check_hugepage(void)
{
	if (access(PATH_HUGEPAGES, F_OK))
		tst_brk(TCONF, "Huge page is not supported.");
}

void write_memcg(void)
{
	SAFE_FILE_PRINTF(MEMCG_LIMIT, "%ld", TESTMEM);

	SAFE_FILE_PRINTF(MEMCG_PATH_NEW "/tasks", "%d", getpid());
}

struct ksm_merge_data {
	char data;
	unsigned int mergeable_size;
};

static void ksm_child_memset(int child_num, int size, int total_unit,
		 struct ksm_merge_data ksm_merge_data, char **memory)
{
	int i = 0, j;
	int unit = size / total_unit;

	tst_res(TINFO, "child %d continues...", child_num);

	if (ksm_merge_data.mergeable_size == size * MB) {
		tst_res(TINFO, "child %d allocates %d MB filled with '%c'",
			child_num, size, ksm_merge_data.data);

	} else {
		tst_res(TINFO, "child %d allocates %d MB filled with '%c'"
				" except one page with 'e'",
				child_num, size, ksm_merge_data.data);
	}

	for (j = 0; j < total_unit; j++) {
		for (i = 0; (unsigned int)i < unit * MB; i++)
			memory[j][i] = ksm_merge_data.data;
	}

	/* if it contains unshared page, then set 'e' char
	 * at the end of the last page
	 */
	if (ksm_merge_data.mergeable_size < size * MB)
		memory[j-1][i-1] = 'e';
}

static void create_ksm_child(int child_num, int size, int unit,
		       struct ksm_merge_data *ksm_merge_data)
{
	int j, total_unit;
	char **memory;

	/* The total units in all */
	total_unit = size / unit;

	/* Apply for the space for memory */
	memory = SAFE_MALLOC(total_unit * sizeof(char *));
	for (j = 0; j < total_unit; j++) {
		memory[j] = SAFE_MMAP(NULL, unit * MB, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
		if (madvise(memory[j], unit * MB, MADV_MERGEABLE) == -1)
			tst_brk(TBROK|TERRNO, "madvise");
#endif
	}

	tst_res(TINFO, "child %d stops.", child_num);
	if (raise(SIGSTOP) == -1)
		tst_brk(TBROK|TERRNO, "kill");
	fflush(stdout);

	for (j = 0; j < 4; j++) {

		ksm_child_memset(child_num, size, total_unit,
				  ksm_merge_data[j], memory);

		fflush(stdout);

		tst_res(TINFO, "child %d stops.", child_num);
		if (raise(SIGSTOP) == -1)
			tst_brk(TBROK|TERRNO, "kill");

		if (ksm_merge_data[j].mergeable_size < size * MB) {
			verify(memory, 'e', child_num, total_unit - 1,
				total_unit, unit * MB - 1, unit * MB);
			verify(memory, ksm_merge_data[j].data, child_num,
				0, total_unit, 0, unit * MB - 1);
		} else {
			verify(memory, ksm_merge_data[j].data, child_num,
				0, total_unit, 0, unit * MB);
		}
	}

	tst_res(TINFO, "child %d finished.", child_num);
}

static void stop_ksm_children(int *child, int num)
{
	int k, status;

	tst_res(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		SAFE_WAITPID(child[k], &status, WUNTRACED);
		if (!WIFSTOPPED(status))
			tst_brk(TBROK, "child %d was not stopped", k);
	}
}

static void resume_ksm_children(int *child, int num)
{
	int k;

	tst_res(TINFO, "resume all children.");
	for (k = 0; k < num; k++)
		SAFE_KILL(child[k], SIGCONT);

	fflush(stdout);
}

void create_same_memory(int size, int num, int unit)
{
	int i, j, status, *child;
	unsigned long ps, pages;
	struct ksm_merge_data **ksm_data;

	struct ksm_merge_data ksm_data0[] = {
	       {'c', size*MB}, {'c', size*MB}, {'d', size*MB}, {'d', size*MB},
	};
	struct ksm_merge_data ksm_data1[] = {
	       {'a', size*MB}, {'b', size*MB}, {'d', size*MB}, {'d', size*MB-1},
	};
	struct ksm_merge_data ksm_data2[] = {
	       {'a', size*MB}, {'a', size*MB}, {'d', size*MB}, {'d', size*MB},
	};

	ps = sysconf(_SC_PAGE_SIZE);
	pages = MB / ps;

	ksm_data = malloc((num - 3) * sizeof(struct ksm_merge_data *));
	/* Since from third child, the data is same with the first child's */
	for (i = 0; i < num - 3; i++) {
		ksm_data[i] = malloc(4 * sizeof(struct ksm_merge_data));
		for (j = 0; j < 4; j++) {
			ksm_data[i][j].data = ksm_data0[j].data;
			ksm_data[i][j].mergeable_size =
				ksm_data0[j].mergeable_size;
		}
	}

	child = SAFE_MALLOC(num * sizeof(int));

	for (i = 0; i < num; i++) {
		fflush(stdout);
		switch (child[i] = SAFE_FORK()) {
		case 0:
			if (i == 0) {
				create_ksm_child(i, size, unit, ksm_data0);
				exit(0);
			} else if (i == 1) {
				create_ksm_child(i, size, unit, ksm_data1);
				exit(0);
			} else if (i == 2) {
				create_ksm_child(i, size, unit, ksm_data2);
				exit(0);
			} else {
				create_ksm_child(i, size, unit, ksm_data[i-3]);
				exit(0);
			}
		}
	}

	stop_ksm_children(child, num);

	tst_res(TINFO, "KSM merging...");
	if (access(PATH_KSM "max_page_sharing", F_OK) == 0) {
		SAFE_FILE_PRINTF(PATH_KSM "run", "2");
		SAFE_FILE_PRINTF(PATH_KSM "max_page_sharing", "%ld", size * pages * num);
	}

	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	SAFE_FILE_PRINTF(PATH_KSM "pages_to_scan", "%ld", size * pages * num);
	SAFE_FILE_PRINTF(PATH_KSM "sleep_millisecs", "0");

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	group_check(1, 2, size * num * pages - 2, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	group_check(1, 3, size * num * pages - 3, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	group_check(1, 1, size * num * pages - 1, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	group_check(1, 1, size * num * pages - 2, 0, 1, 0, size * pages * num);

	tst_res(TINFO, "KSM unmerging...");
	SAFE_FILE_PRINTF(PATH_KSM "run", "2");

	resume_ksm_children(child, num);
	final_group_check(2, 0, 0, 0, 0, 0, size * pages * num);

	tst_res(TINFO, "stop KSM.");
	SAFE_FILE_PRINTF(PATH_KSM "run", "0");
	final_group_check(0, 0, 0, 0, 0, 0, size * pages * num);

	while (waitpid(-1, &status, 0) > 0)
		if (WEXITSTATUS(status) != 0)
			tst_res(TFAIL, "child exit status is %d",
				 WEXITSTATUS(status));
}

void test_ksm_merge_across_nodes(unsigned long nr_pages)
{
	char **memory;
	int i, ret;
	int num_nodes, *nodes;
	unsigned long length;
	unsigned long pagesize;

#ifdef HAVE_NUMA_V2
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
#endif

	ret = get_allowed_nodes_arr(NH_MEMS, &num_nodes, &nodes);
	if (ret != 0)
		tst_brk(TBROK|TERRNO, "get_allowed_nodes_arr");
	if (num_nodes < 2) {
		tst_res(TINFO, "need NUMA system support");
		free(nodes);
		return;
	}

	pagesize = sysconf(_SC_PAGE_SIZE);
	length = nr_pages * pagesize;

	memory = SAFE_MALLOC(num_nodes * sizeof(char *));
	for (i = 0; i < num_nodes; i++) {
		memory[i] = SAFE_MMAP(NULL, length, PROT_READ|PROT_WRITE,
			    MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
		if (madvise(memory[i], length, MADV_MERGEABLE) == -1)
			tst_brk(TBROK|TERRNO, "madvise");
#endif

#ifdef HAVE_NUMA_V2
		clean_node(nmask);
		set_node(nmask, nodes[i]);
		/*
		 * Use mbind() to make sure each node contains
		 * length size memory.
		 */
		ret = mbind(memory[i], length, MPOL_BIND, nmask, MAXNODES, 0);
		if (ret == -1)
			tst_brk(TBROK|TERRNO, "mbind");
#endif

		memset(memory[i], 10, length);
	}

	SAFE_FILE_PRINTF(PATH_KSM "sleep_millisecs", "0");
	SAFE_FILE_PRINTF(PATH_KSM "pages_to_scan", "%ld",
			 nr_pages * num_nodes);
	/*
	 * merge_across_nodes and max_page_sharing setting can be changed
	 * only when there are no ksm shared pages in system, so set run 2
	 * to unmerge pages first, then to 1 after changing merge_across_nodes,
	 * to remerge according to the new setting.
	 */
	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	if (access(PATH_KSM "max_page_sharing", F_OK) == 0)
		SAFE_FILE_PRINTF(PATH_KSM "max_page_sharing",
			"%ld", nr_pages * num_nodes);
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=1");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "1");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	group_check(1, 1, nr_pages * num_nodes - 1, 0, 0, 0,
		    nr_pages * num_nodes);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
	tst_res(TINFO, "Start to test KSM with merge_across_nodes=0");
	SAFE_FILE_PRINTF(PATH_KSM "merge_across_nodes", "0");
	SAFE_FILE_PRINTF(PATH_KSM "run", "1");
	group_check(1, num_nodes, nr_pages * num_nodes - num_nodes,
		    0, 0, 0, nr_pages * num_nodes);

	SAFE_FILE_PRINTF(PATH_KSM "run", "2");
}

/* THP */

/* cpuset/memcg */

static void gather_node_cpus(char *cpus, long nd)
{
	int ncpus = 0;
	int i;
	long online;
	char buf[BUFSIZ];
	char path[BUFSIZ], path1[BUFSIZ];

	while (path_exist(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	for (i = 0; i < ncpus; i++) {
		snprintf(path, BUFSIZ,
			 PATH_SYS_SYSTEM "/node/node%ld/cpu%d", nd, i);
		if (path_exist(path)) {
			snprintf(path1, BUFSIZ, "%s/online", path);
			/*
			 * if there is no online knob, then the cpu cannot
			 * be taken offline
			 */
			if (path_exist(path1)) {
				SAFE_FILE_SCANF(path1, "%ld", &online);
				if (online == 0)
					continue;
			}
			sprintf(buf, "%d,", i);
			strcat(cpus, buf);
		}
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
			fd = SAFE_OPEN(path, O_RDONLY);
		} else
			tst_brk(TBROK | TERRNO, "open %s", path);
	}
	if (read(fd, retbuf, BUFSIZ) < 0)
		tst_brk(TBROK | TERRNO, "read %s", path);
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
			fd = SAFE_OPEN(path, O_WRONLY);
		} else
			tst_brk(TBROK | TERRNO, "open %s", path);
	}
	SAFE_WRITE(1, fd, buf, strlen(buf));
	close(fd);
}

void write_cpusets(long nd)
{
	char buf[BUFSIZ];
	char cpus[BUFSIZ] = "";

	snprintf(buf, BUFSIZ, "%ld", nd);
	write_cpuset_files(CPATH_NEW, "mems", buf);

	gather_node_cpus(cpus, nd);
	/*
	 * If the 'nd' node doesn't contain any CPUs,
	 * the first ID of CPU '0' will be used as
	 * the value of cpuset.cpus.
	 */
	if (strlen(cpus) != 0) {
		write_cpuset_files(CPATH_NEW, "cpus", cpus);
	} else {
		tst_res(TINFO, "No CPUs in the node%ld; "
				"using only CPU0", nd);
		write_cpuset_files(CPATH_NEW, "cpus", "0");
	}

	SAFE_FILE_PRINTF(CPATH_NEW "/tasks", "%d", getpid());
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
		tst_res(TWARN | TERRNO, "open %s", s);

	snprintf(s_new, BUFSIZ, "%s/tasks", path_new);
	fp = fopen(s_new, "r");
	if (fp == NULL)
		tst_res(TWARN | TERRNO, "fopen %s", s_new);
	if ((fd != -1) && (fp != NULL)) {
		while (fgets(value, BUFSIZ, fp) != NULL)
			if (write(fd, value, strlen(value) - 1)
			    != (ssize_t)strlen(value) - 1)
				tst_res(TWARN | TERRNO, "write %s", s);
	}
	if (fd != -1)
		close(fd);
	if (fp != NULL)
		fclose(fp);
	if (rmdir(path_new) == -1)
		tst_res(TWARN | TERRNO, "rmdir %s", path_new);
	if (umount(path) == -1)
		tst_res(TWARN | TERRNO, "umount %s", path);
	if (rmdir(path) == -1)
		tst_res(TWARN | TERRNO, "rmdir %s", path);
}

void mount_mem(char *name, char *fs, char *options, char *path, char *path_new)
{
	SAFE_MKDIR(path, 0777);
	if (mount(name, path, fs, 0, options) == -1) {
		if (errno == ENODEV) {
			if (rmdir(path) == -1)
				tst_res(TWARN | TERRNO, "rmdir %s failed",
					 path);
			tst_brk(TCONF,
				 "file system %s is not configured in kernel",
				 fs);
		}
		tst_brk(TBROK | TERRNO, "mount %s", path);
	}
	SAFE_MKDIR(path_new, 0777);
}

/* shared */

/* Warning: *DO NOT* use this function in child */
unsigned int get_a_numa_node(void)
{
	unsigned int nd1, nd2;
	int ret;

	ret = get_allowed_nodes(0, 2, &nd1, &nd2);
	switch (ret) {
	case 0:
		break;
	case -3:
		tst_brk(TCONF, "requires a NUMA system.");
	default:
		tst_brk(TBROK | TERRNO, "1st get_allowed_nodes");
	}

	ret = get_allowed_nodes(NH_MEMS | NH_CPUS, 1, &nd1);
	switch (ret) {
	case 0:
		tst_res(TINFO, "get node%u.", nd1);
		return nd1;
	case -3:
		tst_brk(TCONF, "requires a NUMA system that has "
			 "at least one node with both memory and CPU "
			 "available.");
	default:
		tst_brk(TBROK | TERRNO, "2nd get_allowed_nodes");
	}

	/* not reached */
	abort();
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

void set_sys_tune(char *sys_file, long tune, int check)
{
	long val;
	char path[BUFSIZ];

	tst_res(TINFO, "set %s to %ld", sys_file, tune);

	snprintf(path, BUFSIZ, PATH_SYSVM "%s", sys_file);
	SAFE_FILE_PRINTF(path, "%ld", tune);

	if (check) {
		val = get_sys_tune(sys_file);
		if (val != tune)
			tst_brk(TBROK, "%s = %ld, but expect %ld",
				 sys_file, val, tune);
	}
}

long get_sys_tune(char *sys_file)
{
	char path[BUFSIZ];
	long tune;

	snprintf(path, BUFSIZ, PATH_SYSVM "%s", sys_file);
	SAFE_FILE_SCANF(path, "%ld", &tune);

	return tune;
}

void update_shm_size(size_t * shm_size)
{
	size_t shmmax;

	SAFE_FILE_SCANF(PATH_SHMMAX, "%zu", &shmmax);
	if (*shm_size > shmmax) {
		tst_res(TINFO, "Set shm_size to shmmax: %zu", shmmax);
		*shm_size = shmmax;
	}
}

int range_is_mapped(unsigned long low, unsigned long high)
{
	FILE *fp;

	fp = fopen("/proc/self/maps", "r");
	if (fp == NULL)
		tst_brk(TBROK | TERRNO, "Failed to open /proc/self/maps.");

	while (!feof(fp)) {
		unsigned long start, end;
		int ret;

		ret = fscanf(fp, "%lx-%lx %*[^\n]\n", &start, &end);
		if (ret != 2) {
			fclose(fp);
			tst_brk(TBROK | TERRNO, "Couldn't parse /proc/self/maps line.");
		}

		if ((start >= low) && (start < high)) {
			fclose(fp);
			return 1;
		}
		if ((end >= low) && (end < high)) {
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}
