// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2007 IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * This Test perform mmap, munmap and write operation on hugetlb file
 * based mapping. Mapping can be shared or private. and it checks for
 * Hugetlb counter (Total, Free, Reserve, Surplus) in /proc/meminfo and
 * compare them with expected (calculated) value. if all checks are
 * successful, the test passes.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <linux/mempolicy.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"
#include "lapi/syscalls.h"

#define MNTPOINT "hugetlbfs/"
#define ULONG_BITS (sizeof(unsigned long) * CHAR_BIT)

static long hpage_size;
static int private_resv;

#define NR_SLOTS	2
#define SL_SETUP	0
#define SL_TEST		1
static int map_fd[NR_SLOTS] = { -1, -1 };
static char *map_addr[NR_SLOTS];
static unsigned long map_size[NR_SLOTS];
static unsigned int touched[NR_SLOTS];

static long prev_total;
static long prev_free;
static long prev_resv;
static long prev_surp;

static void read_meminfo_huge(long *total, long *free, long *resv, long *surp)
{
	*total = SAFE_READ_MEMINFO(MEMINFO_HPAGE_TOTAL);
	*free = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	*resv = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	*surp = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SURP);
}

static int node_isset(unsigned long node, const unsigned long *nodemask)
{
	return !!(nodemask[node / ULONG_BITS] & (1UL << (node % ULONG_BITS)));
}

static void node_set(unsigned long node, unsigned long *nodemask)
{
	nodemask[node / ULONG_BITS] |= 1UL << (node % ULONG_BITS);
}

static int nodemasks_intersect(const unsigned long *left,
			       const unsigned long *right, unsigned long maxnode)
{
	unsigned long node;

	for (node = 0; node < maxnode; node++) {
		if (node_isset(node, left) && node_isset(node, right))
			return 1;
	}

	return 0;
}

static void resolve_relative_nodemask(unsigned long *nodemask,
				      const unsigned long *allowed,
				      unsigned long maxnode)
{
	unsigned long *folded;
	unsigned long node, rank = 0, nr_allowed = 0;

	for (node = 0; node < maxnode; node++)
		nr_allowed += node_isset(node, allowed);
	if (!nr_allowed)
		return;

	/* Match the kernel's nodes_fold() followed by nodes_onto(). */
	folded = SAFE_CALLOC(maxnode / ULONG_BITS, sizeof(*folded));
	for (node = 0; node < maxnode; node++) {
		if (node_isset(node, nodemask))
			node_set(node % nr_allowed, folded);
	}

	memset(nodemask, 0, maxnode / CHAR_BIT);
	for (node = 0; node < maxnode; node++) {
		if (!node_isset(node, allowed))
			continue;
		if (node_isset(rank++, folded))
			node_set(node, nodemask);
	}
	free(folded);
}

static unsigned long surplus_pages_needed(int hpages, unsigned long allowed_free)
{
	unsigned long global_shortfall = 0;
	unsigned long allowed_shortfall = 0;

	if (hpages + prev_resv > prev_free)
		global_shortfall = hpages - prev_free + prev_resv;

	if ((unsigned long)hpages > allowed_free)
		allowed_shortfall = hpages - allowed_free;

	return MAX(global_shortfall, allowed_shortfall);
}

static int mapping_creates_reservation(int flags)
{
	return (flags & MAP_SHARED) || private_resv;
}

static unsigned long read_allowed_free_hugepages(void)
{
	char path[PATH_MAX];
	size_t mask_size = getpagesize();
	unsigned long allowed_free = 0;
	unsigned long maxnode = mask_size * CHAR_BIT;
	unsigned long *policy_nodemask;
	unsigned long *cpuset_nodemask;
	unsigned long node, node_free;
	int mode, mode_flags, use_bind;

	policy_nodemask = SAFE_CALLOC(1, mask_size);
	cpuset_nodemask = SAFE_CALLOC(1, mask_size);

	if (syscall(__NR_get_mempolicy, &mode, policy_nodemask,
		    maxnode, NULL, 0)) {
		if (errno == ENOSYS) {
			allowed_free = prev_free;
			goto out;
		}
		tst_brk(TBROK | TERRNO, "get_mempolicy() failed");
	}

	if (syscall(__NR_get_mempolicy, NULL, cpuset_nodemask,
		    maxnode, NULL, MPOL_F_MEMS_ALLOWED))
		tst_brk(TBROK | TERRNO,
			"get_mempolicy(MPOL_F_MEMS_ALLOWED) failed");

	mode_flags = mode & MPOL_MODE_FLAGS;
	mode &= ~MPOL_MODE_FLAGS;
	if (mode == MPOL_BIND && (mode_flags & MPOL_F_RELATIVE_NODES))
		resolve_relative_nodemask(policy_nodemask, cpuset_nodemask, maxnode);

	/* Hugetlb ignores a bind policy that no longer overlaps the cpuset. */
	use_bind = mode == MPOL_BIND &&
		nodemasks_intersect(policy_nodemask, cpuset_nodemask, maxnode);
	for (node = 0; node < maxnode; node++) {
		if (!node_isset(node, cpuset_nodemask))
			continue;
		if (use_bind && !node_isset(node, policy_nodemask))
			continue;

		snprintf(path, sizeof(path),
			 "/sys/devices/system/node/node%lu/hugepages/"
			 "hugepages-%ldkB/free_hugepages",
			 node, hpage_size / 1024);
		if (access(path, R_OK))
			continue;

		SAFE_FILE_SCANF(path, "%lu", &node_free);
		allowed_free += node_free;
	}

out:
	free(policy_nodemask);
	free(cpuset_nodemask);
	return allowed_free;
}

static int kernel_has_private_reservations(void)
{
	int fd;
	long t, f, r, s;
	long nt, nf, nr, ns;
	void *p;

	read_meminfo_huge(&t, &f, &r, &s);
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

	read_meminfo_huge(&nt, &nf, &nr, &ns);

	SAFE_MUNMAP(p, hpage_size);
	SAFE_CLOSE(fd);

	/*
	 * There are only three valid cases:
	 * 1) If a surplus page was allocated to create a reservation, all
	 *    four pool counters increment
	 * 2) All counters remain the same except for Hugepages_Rsvd, then
	 *    a reservation was created using an existing pool page.
	 * 3) All counters remain the same, indicates that no reservation has
	 *    been created
	 */
	if ((nt == t + 1) && (nf == f + 1) && (ns == s + 1) && (nr == r + 1))
		return 1;
	else if ((nt == t) && (nf == f) && (ns == s)) {
		if (nr == r + 1)
			return 1;
		else if (nr == r)
			return 0;
	}
	tst_brk(TCONF, "bad counter state - "
	      "T:%li F:%li R:%li S:%li -> T:%li F:%li R:%li S:%li",
		  t, f, r, s, nt, nf, nr, ns);
	return -1;
}

static int verify_counters(int line, char *desc, long et, long ef, long er, long es)
{
	long t, f, r, s;
	long fail = 0;

	read_meminfo_huge(&t, &f, &r, &s);

	if (t != et) {
		tst_res_(__FILE__, line, TFAIL, "While %s: Bad "MEMINFO_HPAGE_TOTAL
				" expected %li, actual %li", desc, et, t);
		fail++;
	}
	if (f != ef) {
		tst_res_(__FILE__, line, TFAIL, "While %s: Bad "MEMINFO_HPAGE_FREE
				" expected %li, actual %li", desc, ef, f);
		fail++;
	}
	if (r != er) {
		tst_res_(__FILE__, line, TFAIL, "While %s: Bad "MEMINFO_HPAGE_RSVD
				" expected %li, actual %li", desc, er, r);
		fail++;
	}
	if (s != es) {
		tst_res_(__FILE__, line, TFAIL, "While %s: Bad "MEMINFO_HPAGE_SURP
				" expected %li, actual %li", desc, es, s);
		fail++;
	}

	if (fail)
		return -1;

	prev_total = t;
	prev_free = f;
	prev_resv = r;
	prev_surp = s;
	return 0;
}

/* Memory operations:
 * Each of these has a predefined effect on the counters
 */
static int set_nr_hugepages_(long count, char *desc, int line)
{
	long min_size;
	long et, ef, er, es;

	SAFE_FILE_PRINTF(PATH_VM_NR_HPAGES, "%lu", count);

	/* The code below is based on set_max_huge_pages in mm/hugetlb.c */
	es = prev_surp;
	et = prev_total;
	ef = prev_free;
	er = prev_resv;

	/*
	 * Increase the pool size
	 * First take pages out of surplus state.  Then make up the
	 * remaining difference by allocating fresh huge pages.
	 */
	while (es && count > et - es)
		es--;
	while (count > et - es) {
		et++;
		ef++;
	}
	if (count >= et - es)
		goto out;

	/*
	 * Decrease the pool size
	 * First return free pages to the buddy allocator (being careful
	 * to keep enough around to satisfy reservations).  Then place
	 * pages into surplus state as needed so the pool will shrink
	 * to the desired size as pages become free.
	 */
	min_size = MAX(count, er + et - ef);
	while (min_size < et - es) {
		ef--;
		et--;
	}
	while (count < et - es)
		es++;

out:
	return verify_counters(line, desc, et, ef, er, es);
}
#define SET_NR_HUGEPAGES(c, d) set_nr_hugepages_(c, d, __LINE__)

static int map_(int s, int hpages, int flags, char *desc, int line)
{
	unsigned long allowed_free = 0;
	long et, ef, er, es;
	int creates_reservation = mapping_creates_reservation(flags);

	map_fd[s] = tst_creat_unlinked(MNTPOINT, 0, 0600);
	map_size[s] = hpages * hpage_size;
	if (creates_reservation)
		allowed_free = read_allowed_free_hugepages();
	map_addr[s] = SAFE_MMAP(NULL, map_size[s], PROT_READ|PROT_WRITE, flags,
				map_fd[s], 0);
	touched[s] = 0;

	et = prev_total;
	ef = prev_free;
	er = prev_resv;
	es = prev_surp;
	/*
	 * When using MAP_SHARED, a reservation will be created to guarantee
	 * pages to the process.  If not enough pages are available to
	 * satisfy the reservation, surplus pages are added to the pool.
	 * NOTE: This code assumes that the whole mapping needs to be
	 * reserved and hence, will not work with partial reservations.
	 *
	 * If the kernel supports private reservations, then MAP_PRIVATE
	 * mappings behave like MAP_SHARED at mmap time.  Otherwise,
	 * no counter updates will occur.
	 */
	if (creates_reservation) {
		unsigned long shortfall = surplus_pages_needed(hpages, allowed_free);

		et += shortfall;
		ef += shortfall;
		er += hpages;
		es += shortfall;
	}

	return verify_counters(line, desc, et, ef, er, es);
}
#define MAP(s, h, f, d) map_(s, h, f, d, __LINE__)

static int unmap_(int s, int hpages, int flags, char *desc, int line)
{
	long et, ef, er, es;
	unsigned long i;

	SAFE_MUNMAP(map_addr[s], map_size[s]);
	SAFE_CLOSE(map_fd[s]);
	map_addr[s] = NULL;
	map_size[s] = 0;

	et = prev_total;
	ef = prev_free;
	er = prev_resv;
	es = prev_surp;

	/*
	 * When a VMA is unmapped, the instantiated (touched) pages are
	 * freed.  If the pool is in a surplus state, pages are freed to the
	 * buddy allocator, otherwise they go back into the hugetlb pool.
	 * NOTE: This code assumes touched pages have only one user.
	 */
	for (i = 0; i < touched[s]; i++) {
		if (es) {
			et--;
			es--;
		} else
			ef++;
	}

	/*
	 * mmap may have created some surplus pages to accommodate a
	 * reservation.  If those pages were not touched, then they will
	 * not have been freed by the code above.  Free them here.
	 */
	if ((flags & MAP_SHARED) || private_resv) {
		int unused_surplus = MIN(hpages - touched[s], es);

		et -= unused_surplus;
		ef -= unused_surplus;
		er -= hpages - touched[s];
		es -= unused_surplus;
	}

	return verify_counters(line, desc, et, ef, er, es);
}
#define UNMAP(s, h, f, d) unmap_(s, h, f, d, __LINE__)

static int touch_(int s, int hpages, int flags, char *desc, int line)
{
	long et, ef, er, es;
	int nr;
	char *c;

	for (c = map_addr[s], nr = hpages;
			hpages && c < map_addr[s] + map_size[s];
			c += hpage_size, nr--)
		*c = (char) (nr % 2);
	/*
	 * Keep track of how many pages were touched since we can't easily
	 * detect that from user space.
	 * NOTE: Calling this function more than once for a mmap may yield
	 * results you don't expect.  Be careful :)
	 */
	touched[s] = MAX(touched[s], hpages);

	/*
	 * Shared (and private when supported) mappings and consume resv pages
	 * that were previously allocated. Also deduct them from the free count.
	 *
	 * Unreserved private mappings may need to allocate surplus pages to
	 * satisfy the fault.  The surplus pages become part of the pool
	 * which could elevate total, free, and surplus counts.  resv is
	 * unchanged but free must be decreased.
	 */
	if (flags & MAP_SHARED || private_resv) {
		et = prev_total;
		ef = prev_free - hpages;
		er = prev_resv - hpages;
		es = prev_surp;
	} else {
		if (hpages + prev_resv > prev_free)
			et = prev_total + (hpages - prev_free + prev_resv);
		else
			et = prev_total;
		er = prev_resv;
		es = prev_surp + et - prev_total;
		ef = prev_free - hpages + et - prev_total;
	}
	return verify_counters(line, desc, et, ef, er, es);
}
#define TOUCH(s, h, f, d) touch_(s, h, f, d, __LINE__)

static int test_counters(char *desc, int base_nr)
{
	tst_res(TINFO, "%s...", desc);

	if (SET_NR_HUGEPAGES(base_nr, "initializing hugepages pool"))
		return -1;

	/* untouched, shared mmap */
	if (MAP(SL_TEST, 1, MAP_SHARED, "doing mmap shared with no touch") ||
		UNMAP(SL_TEST, 1, MAP_SHARED, "doing munmap on shared with no touch"))
		return -1;

	/* untouched, private mmap */
	if (MAP(SL_TEST, 1, MAP_PRIVATE, "doing mmap private with no touch") ||
		UNMAP(SL_TEST, 1, MAP_PRIVATE, "doing munmap private with on touch"))
		return -1;

	/* touched, shared mmap */
	if (MAP(SL_TEST, 1, MAP_SHARED, "doing mmap shared followed by touch") ||
		TOUCH(SL_TEST, 1, MAP_SHARED, "touching the addr after mmap shared") ||
		UNMAP(SL_TEST, 1, MAP_SHARED, "doing munmap shared after touch"))
		return -1;

	/* touched, private mmap */
	if (MAP(SL_TEST, 1, MAP_PRIVATE, "doing mmap private followed by touch") ||
		TOUCH(SL_TEST, 1, MAP_PRIVATE, "touching the addr after mmap private") ||
		UNMAP(SL_TEST, 1, MAP_PRIVATE, "doing munmap private after touch"))
		return -1;

	/*
	 * Explicit resizing during outstanding surplus
	 * Consume surplus when growing pool
	 */
	if (MAP(SL_TEST, 2, MAP_SHARED, "doing mmap to consume surplus") ||
		SET_NR_HUGEPAGES(MAX(base_nr, 1), "setting hugepages pool to consume surplus"))
		return -1;

	/* Add pages once surplus is consumed */
	if (SET_NR_HUGEPAGES(MAX(base_nr, 3), "adding more pages after consuming surplus"))
		return -1;

	/* Release free huge pages first */
	if (SET_NR_HUGEPAGES(MAX(base_nr, 2), "releasing free huge pages"))
		return -1;

	/* When shrinking beyond committed level, increase surplus */
	if (SET_NR_HUGEPAGES(base_nr, "increasing surplus counts"))
		return -1;

	/* Upon releasing the reservation, reduce surplus counts */
	if (UNMAP(SL_TEST, 2, MAP_SHARED, "reducing surplus counts"))
		return -1;

	tst_res(TINFO, "OK");
	return 0;
}

static void per_iteration_cleanup(void)
{
	int nr;

	prev_total = 0;
	prev_free = 0;
	prev_resv = 0;
	prev_surp = 0;
	for (nr = 0; nr < NR_SLOTS; nr++) {
		if (map_addr[nr])
			SAFE_MUNMAP(map_addr[nr], map_size[nr]);
		if (map_fd[nr] != -1)
			SAFE_CLOSE(map_fd[nr]);
	}
}

static int test_per_base_nr(int base_nr)
{
	tst_res(TINFO, "Base pool size: %i", base_nr);

	/* Run the tests with a clean slate */
	if (test_counters("Clean", base_nr))
		return -1;

	/* Now with a pre-existing untouched, shared mmap */
	if (MAP(SL_SETUP, 1, MAP_SHARED, "mmap for test having prior untouched shared mmap") ||
		test_counters("Untouched, shared", base_nr) ||
		UNMAP(SL_SETUP, 1, MAP_SHARED, "unmap after test having prior untouched shared mmap"))
		return -1;

	/* Now with a pre-existing untouched, private mmap */
	if (MAP(SL_SETUP, 1, MAP_PRIVATE, "mmap for test having prior untouched private mmap") ||
		test_counters("Untouched, private", base_nr) ||
		UNMAP(SL_SETUP, 1, MAP_PRIVATE, "unmap after test having prior untouched private mmap"))
		return -1;

	/* Now with a pre-existing touched, shared mmap */
	if (MAP(SL_SETUP, 1, MAP_SHARED, "mmap for test having prior touched shared mmap") ||
		TOUCH(SL_SETUP, 1, MAP_SHARED, "touching for test having prior touched shared mmap") ||
		test_counters("Touched, shared", base_nr) ||
		UNMAP(SL_SETUP, 1, MAP_SHARED, "unmap after test having prior touched shared mmap"))
		return -1;

	/* Now with a pre-existing touched, private mmap */
	if (MAP(SL_SETUP, 1, MAP_PRIVATE, "mmap for test with having touched private mmap") ||
		TOUCH(SL_SETUP, 1, MAP_PRIVATE, "touching for test with having touched private mmap") ||
		test_counters("Touched, private", base_nr) ||
		UNMAP(SL_SETUP, 1, MAP_PRIVATE,	"unmap after test having prior touched private mmap"))
		return -1;
	return 0;
}

static void run_test(void)
{
	int base_nr;

	for (base_nr = 0; base_nr <= 3; base_nr++) {
		if (test_per_base_nr(base_nr))
			break;
	}
	if (base_nr > 3)
		tst_res(TPASS, "Hugepages Counters works as expected.");
	per_iteration_cleanup();
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
	SAFE_FILE_PRINTF(PATH_VM_OVERCOMMIT_HPAGES, "%lu", tst_hugepages);
	private_resv = kernel_has_private_reservations();
}

static void cleanup(void)
{
	per_iteration_cleanup();
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.save_restore = (const struct tst_path_val[]) {
		{PATH_VM_OVERCOMMIT_HPAGES, NULL, TST_SR_TCONF},
		{PATH_VM_NR_HPAGES, NULL, TST_SR_TCONF},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {3, TST_NEEDS},
};
