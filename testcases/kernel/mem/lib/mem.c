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
		int ret = alloc_mem(TESTMEM * 2 + MB, testcase);
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

	tst_enable_oom_protection(0);

	switch (pid = SAFE_FORK()) {
	case 0:
		tst_disable_oom_protection(0);
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
	int ksm_run_orig;

	tst_res(TINFO, "check!");
	check("run", run);

	/*
	 * Temporarily stop the KSM scan during the checks: during the
	 * KSM scan the rmap_items in the stale unstable tree of the
	 * old pass are removed from it and are later reinserted in
	 * the new unstable tree of the current pass. So if the checks
	 * run in the race window between removal and re-insertion, it
	 * can lead to unexpected false positives where page_volatile
	 * is elevated and page_unshared is recessed.
	 */
	SAFE_FILE_SCANF(PATH_KSM "run", "%d", &ksm_run_orig);
	SAFE_FILE_PRINTF(PATH_KSM "run", "0");

	check("pages_shared", pages_shared);
	check("pages_sharing", pages_sharing);
	check("pages_volatile", pages_volatile);
	check("pages_unshared", pages_unshared);
	check("sleep_millisecs", sleep_millisecs);
	check("pages_to_scan", pages_to_scan);

	SAFE_FILE_PRINTF(PATH_KSM "run", "%d", ksm_run_orig);
}

void ksm_group_check(int run, int pages_shared, int pages_sharing,
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
	ksm_group_check(1, 2, size * num * pages - 2, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	ksm_group_check(1, 3, size * num * pages - 3, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	ksm_group_check(1, 1, size * num * pages - 1, 0, 0, 0, size * pages * num);

	resume_ksm_children(child, num);
	stop_ksm_children(child, num);
	ksm_group_check(1, 1, size * num * pages - 2, 0, 1, 0, size * pages * num);

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

/* THP */

/* cpuset/memcg */
static void gather_node_cpus(char *cpus, long nd)
{
	int ncpus = 0;
	int i;
	long online;
	char buf[BUFSIZ];
	char path[BUFSIZ], path1[BUFSIZ];

	while (tst_path_exists(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	for (i = 0; i < ncpus; i++) {
		snprintf(path, BUFSIZ,
			 PATH_SYS_SYSTEM "/node/node%ld/cpu%d", nd, i);
		if (tst_path_exists("%s", path)) {
			snprintf(path1, BUFSIZ, "%s/online", path);
			/*
			 * if there is no online knob, then the cpu cannot
			 * be taken offline
			 */
			if (tst_path_exists("%s", path1)) {
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

void write_cpusets(const struct tst_cg_group *cg, long nd)
{
	char cpus[BUFSIZ] = "";

	SAFE_CG_PRINTF(cg, "cpuset.mems", "%ld", nd);

	gather_node_cpus(cpus, nd);
	/*
	 * If the 'nd' node doesn't contain any CPUs,
	 * the first ID of CPU '0' will be used as
	 * the value of cpuset.cpus.
	 */
	if (strlen(cpus) != 0) {
		SAFE_CG_PRINT(cg, "cpuset.cpus", cpus);
	} else {
		tst_res(TINFO, "No CPUs in the node%ld; "
				"using only CPU0", nd);
		SAFE_CG_PRINT(cg, "cpuset.cpus", "0");
	}
}
