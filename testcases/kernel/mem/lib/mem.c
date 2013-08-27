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
#include "safe_file_ops.h"
#include "_private.h"
#include "mem.h"
#include "numa_helper.h"

/* OOM */

static int alloc_mem(long int length, int testcase)
{
	void *s;

	tst_resm(TINFO, "allocating %ld bytes.", length);
	s = mmap(NULL, length, PROT_READ | PROT_WRITE,
		 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (s == MAP_FAILED) {
		if (testcase == OVERCOMMIT && errno == ENOMEM)
			return 1;
		else
			tst_brkm(TBROK | TERRNO, cleanup, "mmap");
	}
	if (testcase == MLOCK && mlock(s, length) == -1)
		tst_brkm(TINFO | TERRNO, cleanup, "mlock");
#ifdef HAVE_MADV_MERGEABLE
	if (testcase == KSM && madvise(s, length, MADV_MERGEABLE) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "madvise");
#endif
	memset(s, '\a', length);

	return 0;
}

static void test_alloc(int testcase, int lite)
{
	if (lite)
		alloc_mem(TESTMEM + MB, testcase);
	else
		while (1)
			if (alloc_mem(LENGTH, testcase))
				return;
}

void oom(int testcase, int lite)
{
	pid_t pid;
	int status;

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		test_alloc(testcase, lite);
		exit(0);
	default:
		break;
	}

	tst_resm(TINFO, "expected victim is %d.", pid);
	if (waitpid(-1, &status, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "waitpid");

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

static void set_global_mempolicy(int mempolicy)
{
#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	int num_nodes, *nodes;
	int ret;

	if (mempolicy) {
		ret = get_allowed_nodes_arr(NH_MEMS|NH_CPUS, &num_nodes, &nodes);
		if (ret != 0)
			tst_brkm(TBROK|TERRNO, cleanup,
				 "get_allowed_nodes_arr");
		if (num_nodes < 2) {
			tst_resm(TINFO, "mempolicy need NUMA system support");
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
				tst_resm(TINFO, "The mempolicy need "
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
			tst_brkm(TBROK|TERRNO, cleanup, "Bad mempolicy mode");
		}
		if (set_mempolicy(mempolicy, nmask, MAXNODES) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "set_mempolicy");
	}
#endif
}

void testoom(int mempolicy, int lite)
{
	set_global_mempolicy(mempolicy);

	tst_resm(TINFO, "start normal OOM testing.");
	oom(NORMAL, lite);

	tst_resm(TINFO, "start OOM testing for mlocked pages.");
	oom(MLOCK, lite);

	if (access(PATH_KSM, F_OK) == -1)
		tst_brkm(TCONF, NULL, "KSM configuration is not enabled");

	tst_resm(TINFO, "start OOM testing for KSM pages.");
	oom(KSM, lite);
}

/* KSM */

static void check(char *path, long int value)
{
	char fullpath[BUFSIZ];
	long actual_val;

	snprintf(fullpath, BUFSIZ, PATH_KSM "%s", path);
	SAFE_FILE_SCANF(cleanup, fullpath, "%ld", &actual_val);

	tst_resm(TINFO, "%s is %ld.", path, actual_val);
	if (actual_val != value)
		tst_resm(TFAIL, "%s is not %ld.", path, value);
}

static void wait_ksmd_done(void)
{
	long pages_shared, pages_sharing, pages_volatile, pages_unshared;
	long old_pages_shared = 0, old_pages_sharing = 0;
	long old_pages_volatile = 0, old_pages_unshared = 0;
	int changing = 1, count = 0;

	while (changing) {
		sleep(10);
		count++;

		SAFE_FILE_SCANF(cleanup, PATH_KSM "pages_shared",
				"%ld", &pages_shared);

		SAFE_FILE_SCANF(cleanup, PATH_KSM "pages_sharing",
				"%ld", &pages_sharing);

		SAFE_FILE_SCANF(cleanup, PATH_KSM "pages_volatile",
				"%ld", &pages_volatile);

		SAFE_FILE_SCANF(cleanup, PATH_KSM "pages_unshared",
				"%ld", &pages_unshared);

		if (pages_shared != old_pages_shared ||
		    pages_sharing != old_pages_sharing ||
		    pages_volatile != old_pages_volatile ||
		    pages_unshared != old_pages_unshared) {
			old_pages_shared = pages_shared;
			old_pages_sharing = pages_sharing;
			old_pages_volatile = pages_volatile;
			old_pages_unshared = pages_unshared;
		} else {
			changing = 0;
		}
	}

	tst_resm(TINFO, "ksm daemon takes %ds to scan all mergeable pages",
		 count * 10);
}

static void group_check(int run, int pages_shared, int pages_sharing,
			 int pages_volatile, int pages_unshared,
			 int sleep_millisecs, int pages_to_scan)
{
	/* wait for ksm daemon to scan all mergeable pages. */
	wait_ksmd_done();

	tst_resm(TINFO, "check!");
	check("run", run);
	check("pages_shared", pages_shared);
	check("pages_sharing", pages_sharing);
	check("pages_volatile", pages_volatile);
	check("pages_unshared", pages_unshared);
	check("sleep_millisecs", sleep_millisecs);
	check("pages_to_scan", pages_to_scan);
}

static void verify(char **memory, char value, int proc,
		    int start, int end, int start2, int end2)
{
	int i, j;
	void *s = NULL;

	s = malloc((end - start) * (end2 - start2));
	if (s == NULL)
		tst_brkm(TBROK | TERRNO, tst_exit, "malloc");

	tst_resm(TINFO, "child %d verifies memory content.", proc);
	memset(s, value, (end - start) * (end2 - start2));
	if (memcmp(memory[start], s, (end - start) * (end2 - start2))
	    != 0)
		for (j = start; j < end; j++)
			for (i = start2; i < end2; i++)
				if (memory[j][i] != value)
					tst_resm(TFAIL, "child %d has %c at "
						 "%d,%d,%d.",
						 proc, memory[j][i], proc,
						 j, i);
	free(s);
}

void write_memcg(void)
{
	SAFE_FILE_PRINTF(NULL, MEMCG_LIMIT, "%ld", TESTMEM);

	SAFE_FILE_PRINTF(NULL, MEMCG_PATH_NEW "/tasks", "%d", getpid());
}

static struct ksm_merge_data {
	char data;
	int mergeable_size;
};

static void ksm_child_memset(int child_num, int size, int total_unit,
		 struct ksm_merge_data ksm_merge_data, char **memory)
{
	int i, j;
	int unit = size / total_unit;

	tst_resm(TINFO, "child %d continues...", child_num);

	if (ksm_merge_data.mergeable_size == size * MB) {
		tst_resm(TINFO, "child %d allocates %d MB filled with '%c'",
			child_num, size, ksm_merge_data.data);

	} else {
		tst_resm(TINFO, "child %d allocates %d MB filled with '%c'"
				" except one page with 'e'",
				child_num, size, ksm_merge_data.data);
	}

	for (j = 0; j < total_unit; j++) {
		for (i = 0; i < unit * MB; i++)
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
	memory = (char **)malloc(total_unit * sizeof(char *));
	for (j = 0; j < total_unit; j++) {
		memory[j] = mmap(NULL, unit * MB, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
		if (memory[j] == MAP_FAILED)
			tst_brkm(TBROK|TERRNO, tst_exit, "mmap");
#ifdef HAVE_MADV_MERGEABLE
		if (madvise(memory[j], unit * MB, MADV_MERGEABLE) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "madvise");
#endif
	}

	tst_resm(TINFO, "child %d stops.", child_num);
	if (raise(SIGSTOP) == -1)
		tst_brkm(TBROK|TERRNO, tst_exit, "kill");
	fflush(stdout);

	for (j = 0; j < 4; j++) {

		ksm_child_memset(child_num, size, total_unit,
				  ksm_merge_data[j], memory);

		fflush(stdout);

		tst_resm(TINFO, "child %d stops.", child_num);
		if (raise(SIGSTOP) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "kill");

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

	tst_resm(TINFO, "child %d finished.", child_num);
}

static void stop_ksm_children(int *child, int num)
{
	int k, status;

	tst_resm(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		if (waitpid(child[k], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup, "child %d was not stopped", k);
	}
}

static void resume_ksm_children(int *child, int num)
{
	int k, status;

	tst_resm(TINFO, "resume all children.");
	for (k = 0; k < num; k++) {
		if (kill(child[k], SIGCONT) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "kill child[%d]", k);
	}
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

	ksm_data = (struct ksm_merge_data **)malloc
		   ((num - 3) * sizeof(struct ksm_merge_data *));
	/* Since from third child, the data is same with the first child's */
	for (i = 0; i < num - 3; i++) {
		ksm_data[i] = (struct ksm_merge_data *)malloc
			      (4 * sizeof(struct ksm_merge_data));
		for (j = 0; j < 4; j++) {
			ksm_data[i][j].data = ksm_data0[j].data;
			ksm_data[i][j].mergeable_size =
				ksm_data0[j].mergeable_size;
		}
	}

	child = (int *)malloc(num * sizeof(int));
	if (child == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc");

	for (i = 0; i < num; i++) {
		fflush(stdout);
		switch (child[i] = fork()) {
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "fork");
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

	tst_resm(TINFO, "KSM merging...");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "1");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "pages_to_scan", "%ld",
			 size * pages *num);
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "sleep_millisecs", "0");

	resume_ksm_children(child, num);
	group_check(1, 2, size * num * pages - 2, 0, 0, 0, size * pages * num);

	stop_ksm_children(child, num);
	resume_ksm_children(child, num);
	group_check(1, 3, size * num * pages - 3, 0, 0, 0, size * pages * num);

	stop_ksm_children(child, num);
	resume_ksm_children(child, num);
	group_check(1, 1, size * num * pages - 1, 0, 0, 0, size * pages * num);

	stop_ksm_children(child, num);
	resume_ksm_children(child, num);
	group_check(1, 1, size * num * pages - 2, 0, 1, 0, size * pages * num);

	stop_ksm_children(child, num);

	tst_resm(TINFO, "KSM unmerging...");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "2");

	resume_ksm_children(child, num);
	group_check(2, 0, 0, 0, 0, 0, size * pages * num);

	tst_resm(TINFO, "stop KSM.");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "0");
	group_check(0, 0, 0, 0, 0, 0, size * pages * num);

	while (waitpid(-1, &status, WUNTRACED | WCONTINUED) > 0)
		if (WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "child exit status is %d",
				 WEXITSTATUS(status));
}

void test_ksm_merge_across_nodes(unsigned long nr_pages)
{
	char **memory;
	int i, ret;
	int num_nodes, *nodes;
	unsigned long length;
	unsigned long pagesize;
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };

	ret = get_allowed_nodes_arr(NH_MEMS|NH_CPUS, &num_nodes, &nodes);
	if (ret != 0)
		tst_brkm(TBROK|TERRNO, cleanup, "get_allowed_nodes_arr");
	if (num_nodes < 2) {
		tst_resm(TINFO, "need NUMA system support");
		free(nodes);
		return;
	}

	pagesize = sysconf(_SC_PAGE_SIZE);
	length = nr_pages * pagesize;

	memory = (char **)malloc(num_nodes * sizeof(char *));
	for (i = 0; i < num_nodes; i++) {
		memory[i] = mmap(NULL, length, PROT_READ|PROT_WRITE,
			    MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
		if (memory[i] == MAP_FAILED)
			tst_brkm(TBROK|TERRNO, tst_exit, "mmap");
#ifdef HAVE_MADV_MERGEABLE
		if (madvise(memory[i], length, MADV_MERGEABLE) == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "madvise");
#endif

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS
		clean_node(nmask);
		set_node(nmask, nodes[i]);
		/*
		 * Use mbind() to make sure each node contains
		 * length size memory.
		 */
		ret = mbind(memory[i], length, MPOL_BIND, nmask, MAXNODES, 0);
		if (ret == -1)
			tst_brkm(TBROK|TERRNO, tst_exit, "mbind");
#endif

		memset(memory[i], 10, length);
	}

	SAFE_FILE_PRINTF(cleanup, PATH_KSM "sleep_millisecs", "0");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "pages_to_scan", "%ld",
			 nr_pages * num_nodes);
	/*
	 * merge_across_nodes setting can be changed only when there
	 * are no ksm shared pages in system, so set run 2 to unmerge
	 * pages first, then to 1 after changing merge_across_nodes,
	 * to remerge according to the new setting.
	 */
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "2");
	wait_ksmd_done();
	tst_resm(TINFO, "Start to test KSM with merge_across_nodes=1");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "merge_across_nodes", "1");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "1");
	group_check(1, 1, nr_pages * num_nodes - 1, 0, 0, 0,
		    nr_pages * num_nodes);

	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "2");
	wait_ksmd_done();
	tst_resm(TINFO, "Start to test KSM with merge_across_nodes=0");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "merge_across_nodes", "0");
	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "1");
	group_check(1, num_nodes, nr_pages * num_nodes - num_nodes,
		    0, 0, 0, nr_pages * num_nodes);

	SAFE_FILE_PRINTF(cleanup, PATH_KSM "run", "2");
	wait_ksmd_done();
}

void check_ksm_options(int *size, int *num, int *unit)
{
	if (opt_size) {
		*size = atoi(opt_sizestr);
		if (*size < 1)
			tst_brkm(TBROK, cleanup, "size cannot be less than 1.");
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

/* THP */

static int alloc_transparent_hugepages(int nr_thps, int hg_aligned)
{
	unsigned long hugepagesize, size;
	void *addr;
	int ret;

	hugepagesize = read_meminfo("Hugepagesize:") * KB;
	size = nr_thps * hugepagesize;

	if (hg_aligned) {
		ret = posix_memalign(&addr, hugepagesize, size);
		if (ret != 0) {
			printf("posix_memalign failed\n");
			return -1;
		}
	} else {
		addr = mmap(NULL, size, PROT_READ|PROT_WRITE,
			    MAP_PRIVATE|MAP_ANON, -1, 0);
		if (addr == MAP_FAILED) {
			perror("mmap");
			return -1;
		}
	}

	memset(addr, 10, size);

	tst_resm(TINFO, "child[%d] stop here", getpid());
	/*
	 * stop here, until the father finish to calculate
	 * all the transparent hugepages.
	 */
	if (raise(SIGSTOP) == -1) {
		perror("kill");
		return -1;
	}

	return 0;
}

static void khugepaged_scan_done(void)
{
	int changing = 1, count = 0, interval;
	long old_pages_collapsed, old_max_ptes_none, old_pages_to_scan;
	long pages_collapsed = 0, max_ptes_none = 0, pages_to_scan = 0;

	/*
	 * as 'khugepaged' run 100% during testing, so 5s is an
	 * enough interval for us to recognize if 'khugepaged'
	 * finish scanning proceses' anonymous hugepages or not.
	 */
	interval = 5;

	while (changing) {
		sleep(interval);
		count++;

		SAFE_FILE_SCANF(cleanup, PATH_KHPD "pages_collapsed",
			       "%ld", &pages_collapsed);
		SAFE_FILE_SCANF(cleanup, PATH_KHPD "max_ptes_none",
			       "%ld", &max_ptes_none);
		SAFE_FILE_SCANF(cleanup, PATH_KHPD "pages_to_scan",
			       "%ld", &pages_to_scan);

		if (pages_collapsed != old_pages_collapsed ||
		    max_ptes_none != old_max_ptes_none ||
		    pages_to_scan != old_pages_to_scan) {
			old_pages_collapsed = pages_collapsed;
			old_max_ptes_none = max_ptes_none;
			old_pages_to_scan = pages_to_scan;
		} else {
			changing = 0;
		}
	}

	tst_resm(TINFO, "khugepaged daemon takes %ds to scan all thp pages",
		 count * interval);
}

static void verify_thp_size(int *children, int nr_children, int nr_thps)
{
	FILE *fp;
	char path[BUFSIZ], buf[BUFSIZ], line[BUFSIZ];
	int i, ret;
	long expect_thps; /* the amount of per child's transparent hugepages */
	long val, actual_thps;
	long hugepagesize;

	hugepagesize = read_meminfo("Hugepagesize:");
	expect_thps = nr_thps * hugepagesize;

	for (i = 0; i < nr_children; i++) {
		actual_thps = 0;

		snprintf(path, BUFSIZ, "/proc/%d/smaps", children[i]);
		fp = fopen(path, "r");
		while (fgets(line, BUFSIZ, fp) != NULL) {
			ret = sscanf(line, "%64s %ld", buf, &val);
			if (ret == 2 && val != 0) {
				if (strcmp(buf, "AnonHugePages:") == 0)
					actual_thps += val;
			}
		}

		if (actual_thps != expect_thps)
			tst_resm(TFAIL, "child[%d] got %ldKB thps - expect %ld"
				"KB thps", getpid(), actual_thps, expect_thps);
		fclose(fp);
	}
}

void test_transparent_hugepage(int nr_children, int nr_thps,
			       int hg_aligned, int mempolicy)
{
	unsigned long hugepagesize, memfree;
	int i, *pids, ret, status;

	if (mempolicy)
		set_global_mempolicy(mempolicy);

	memfree = read_meminfo("MemFree:");
	tst_resm(TINFO, "The current MemFree is %luMB", memfree / KB);
	if (memfree < MB)
		tst_resm(TCONF, "Not enough memory for testing");

	hugepagesize = read_meminfo("Hugepagesize:");

	pids = malloc(nr_children * sizeof(int));
	if (pids == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc");

	for (i = 0; i < nr_children; i++) {
		switch (pids[i] = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork");

		case 0:
			ret = alloc_transparent_hugepages(nr_thps, hg_aligned);
			exit(ret);
		}
	}

	tst_resm(TINFO, "Stop all children...");
	for (i = 0; i < nr_children; i++) {
		if (waitpid(pids[i], &status, WUNTRACED) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
		if (!WIFSTOPPED(status))
			tst_brkm(TBROK, cleanup,
				 "child[%d] was not stoppted", pids[i]);
	}

	tst_resm(TINFO, "Start to scan all transparent hugepages...");
	khugepaged_scan_done();

	tst_resm(TINFO, "Start to verify transparent hugepage size...");
	verify_thp_size(pids, nr_children, nr_thps);

	tst_resm(TINFO, "Wake up all children...");
	for (i = 0; i < nr_children; i++) {
		if (kill(pids[i], SIGCONT) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "signal continue child[%d]", pids[i]);
	}

	/* wait all children finish their task */
	for (i = 0; i < nr_children; i++) {
		if (waitpid(pids[i], &status, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid %d", pids[i]);

		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			tst_resm(TFAIL, "the child[%d] unexpectedly failed:"
				 " %d", pids[i], status);
	}
}

void check_thp_options(int *nr_children, int *nr_thps)
{
	if (opt_nr_children)
		*nr_children = SAFE_STRTOL(NULL, opt_nr_children_str,
					   0, LONG_MAX);
	if (opt_nr_thps)
		*nr_thps = SAFE_STRTOL(NULL, opt_nr_thps_str, 0, LONG_MAX);
}

void thp_usage(void)
{
	printf("  -n      Number of processes\n");
	printf("  -N      Number of transparent hugepages\n");
}

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
				SAFE_FILE_SCANF(cleanup, path1, "%ld", &online);
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
			fd = open(path, O_RDONLY);
			if (fd == -1)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "open %s", path);
		} else
			tst_brkm(TBROK | TERRNO, cleanup, "open %s", path);
	}
	if (read(fd, retbuf, BUFSIZ) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "read %s", path);
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
				tst_brkm(TBROK | TERRNO, cleanup,
					 "open %s", path);
		} else
			tst_brkm(TBROK | TERRNO, cleanup, "open %s", path);
	}
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_brkm(TBROK | TERRNO, cleanup, "write %s", path);
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
		tst_resm(TINFO, "No CPUs in the node%ld; "
				"using only CPU0", nd);
		write_cpuset_files(CPATH_NEW, "cpus", "0");
	}

	SAFE_FILE_PRINTF(NULL, CPATH_NEW "/tasks", "%d", getpid());
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
		tst_resm(TWARN | TERRNO, "open %s", s);

	snprintf(s_new, BUFSIZ, "%s/tasks", path_new);
	fp = fopen(s_new, "r");
	if (fp == NULL)
		tst_resm(TWARN | TERRNO, "fopen %s", s_new);
	if ((fd != -1) && (fp != NULL)) {
		while (fgets(value, BUFSIZ, fp) != NULL)
			if (write(fd, value, strlen(value) - 1)
			    != strlen(value) - 1)
				tst_resm(TWARN | TERRNO, "write %s", s);
	}
	if (fd != -1)
		close(fd);
	if (fp != NULL)
		fclose(fp);
	if (rmdir(path_new) == -1)
		tst_resm(TWARN | TERRNO, "rmdir %s", path_new);
	if (umount(path) == -1)
		tst_resm(TWARN | TERRNO, "umount %s", path);
	if (rmdir(path) == -1)
		tst_resm(TWARN | TERRNO, "rmdir %s", path);
}

void mount_mem(char *name, char *fs, char *options, char *path, char *path_new)
{
	if (mkdir(path, 0777) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mkdir %s", path);
	if (mount(name, path, fs, 0, options) == -1) {
		if (errno == ENODEV) {
			if (rmdir(path) == -1)
				tst_resm(TWARN | TERRNO, "rmdir %s failed",
					 path);
			tst_brkm(TCONF, NULL,
				 "file system %s is not configured in kernel",
				 fs);
		}
		tst_brkm(TBROK | TERRNO, cleanup, "mount %s", path);
	}
	if (mkdir(path_new, 0777) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mkdir %s", path_new);
}

/* shared */

/* Warning: *DO NOT* use this function in child */
unsigned int get_a_numa_node(void (*cleanup_fn) (void))
{
	unsigned int nd1, nd2;
	int ret;

	ret = get_allowed_nodes(0, 2, &nd1, &nd2);
	switch (ret) {
	case 0:
		break;
	case -3:
		tst_brkm(TCONF, cleanup_fn, "requires a NUMA system.");
	default:
		tst_brkm(TBROK | TERRNO, cleanup_fn, "1st get_allowed_nodes");
	}

	ret = get_allowed_nodes(NH_MEMS | NH_CPUS, 1, &nd1);
	switch (ret) {
	case 0:
		tst_resm(TINFO, "get node%lu.", nd1);
		return nd1;
	case -3:
		tst_brkm(TCONF, cleanup_fn, "requires a NUMA system that has "
			 "at least one node with both memory and CPU "
			 "available.");
	default:
		break;
	}
	tst_brkm(TBROK | TERRNO, cleanup_fn, "2nd get_allowed_nodes");
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
		tst_brkm(TBROK | TERRNO, cleanup, "fopen %s", PATH_MEMINFO);

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
	char path[BUFSIZ];

	tst_resm(TINFO, "set %s to %ld", sys_file, tune);

	snprintf(path, BUFSIZ, PATH_SYSVM "%s", sys_file);
	SAFE_FILE_PRINTF(NULL, path, "%ld", tune);

	if (check) {
		val = get_sys_tune(sys_file);
		if (val != tune)
			tst_brkm(TBROK, cleanup, "%s = %ld, but expect %ld",
				 sys_file, val, tune);
	}
}

long get_sys_tune(char *sys_file)
{
	char path[BUFSIZ];
	long tune;

	snprintf(path, BUFSIZ, PATH_SYSVM "%s", sys_file);
	SAFE_FILE_SCANF(NULL, path, "%ld", &tune);

	return tune;
}

void update_shm_size(size_t * shm_size)
{
	size_t shmmax;

	SAFE_FILE_SCANF(cleanup, PATH_SHMMAX, "%ld", &shmmax);
	if (*shm_size > shmmax) {
		tst_resm(TINFO, "Set shm_size to shmmax: %ld", shmmax);
		*shm_size = shmmax;
	}
}
