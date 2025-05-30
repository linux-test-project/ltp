// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2024
 */
#ifndef KSM_TEST_
#define KSM_TEST_

#include <sys/wait.h>

static inline void check(char *path, long int value)
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

static inline void final_group_check(int run, int pages_shared, int pages_sharing,
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

static inline void ksm_group_check(int run, int pages_shared, int pages_sharing,
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

static inline void verify(char **memory, char value, int proc,
		    int start, int end, int start2, int end2)
{
	int i, j;

	tst_res(TINFO, "child %d verifies memory content.", proc);

	for (j = start; j < end; j++)
		for (i = start2; i < end2; i++)
			if (memory[j][i] != value)
				tst_res(TFAIL, "child %d has %c at "
					"%d,%d,%d.",
					proc, memory[j][i], proc, j, i);
}

struct ksm_merge_data {
	char data;
	unsigned int mergeable_size;
};

static inline void ksm_child_memset(int child_num, unsigned int size,
				    unsigned int total_unit,
				    struct ksm_merge_data ksm_merge_data,
				    char **memory)
{
	unsigned int i = 0, j;
	unsigned int unit = size / total_unit;

	tst_res(TINFO, "child %d continues...", child_num);

	if (ksm_merge_data.mergeable_size == size * TST_MB) {
		tst_res(TINFO, "child %d allocates %u MB filled with '%c'",
			child_num, size, ksm_merge_data.data);

	} else {
		tst_res(TINFO, "child %d allocates %u MB filled with '%c'"
				" except one page with 'e'",
				child_num, size, ksm_merge_data.data);
	}

	for (j = 0; j < total_unit; j++) {
		for (i = 0; (unsigned int)i < unit * TST_MB; i++)
			memory[j][i] = ksm_merge_data.data;
	}

	/* if it contains unshared page, then set 'e' char
	 * at the end of the last page
	 */
	if (ksm_merge_data.mergeable_size < size * TST_MB)
		memory[j-1][i-1] = 'e';
}

static inline void create_ksm_child(int child_num, unsigned int size,
				    unsigned int unit,
				    struct ksm_merge_data *ksm_merge_data)
{
	unsigned int j, total_unit;
	char **memory;

	/* The total units in all */
	total_unit = size / unit;

	/* Apply for the space for memory */
	memory = SAFE_MALLOC(total_unit * sizeof(char *));
	for (j = 0; j < total_unit; j++) {
		memory[j] = SAFE_MMAP(NULL, unit * TST_MB, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
#ifdef HAVE_DECL_MADV_MERGEABLE
		if (madvise(memory[j], unit * TST_MB, MADV_MERGEABLE) == -1)
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

		if (ksm_merge_data[j].mergeable_size < size * TST_MB) {
			verify(memory, 'e', child_num, total_unit - 1,
				total_unit, unit * TST_MB - 1, unit * TST_MB);
			verify(memory, ksm_merge_data[j].data, child_num,
				0, total_unit, 0, unit * TST_MB - 1);
		} else {
			verify(memory, ksm_merge_data[j].data, child_num,
				0, total_unit, 0, unit * TST_MB);
		}
	}

	tst_res(TINFO, "child %d finished.", child_num);
}

static inline void stop_ksm_children(int *child, int num)
{
	int k, status;

	tst_res(TINFO, "wait for all children to stop.");
	for (k = 0; k < num; k++) {
		SAFE_WAITPID(child[k], &status, WUNTRACED);
		if (!WIFSTOPPED(status))
			tst_brk(TBROK, "child %d was not stopped", k);
	}
}

static inline void resume_ksm_children(int *child, int num)
{
	int k;

	tst_res(TINFO, "resume all children.");
	for (k = 0; k < num; k++)
		SAFE_KILL(child[k], SIGCONT);

	fflush(stdout);
}

static inline void create_same_memory(unsigned int size, int num, unsigned int unit)
{
	int i, j, status, *child;
	unsigned long ps, pages;
	struct ksm_merge_data **ksm_data;

	struct ksm_merge_data ksm_data0[] = {
	       {'c', size*TST_MB}, {'c', size*TST_MB}, {'d', size*TST_MB}, {'d', size*TST_MB},
	};
	struct ksm_merge_data ksm_data1[] = {
	       {'a', size*TST_MB}, {'b', size*TST_MB}, {'d', size*TST_MB}, {'d', size*TST_MB-1},
	};
	struct ksm_merge_data ksm_data2[] = {
	       {'a', size*TST_MB}, {'a', size*TST_MB}, {'d', size*TST_MB}, {'d', size*TST_MB},
	};

	ps = sysconf(_SC_PAGE_SIZE);
	pages = TST_MB / ps;

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

#endif /* KSM_TEST_ */
