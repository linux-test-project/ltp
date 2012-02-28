#ifndef __INTERNAL_H
#define __INTERNAL_H
/* Internal APIs */

/* OOM */
static int  _alloc_mem(long int length, int testcase);
static void _test_alloc(int testcase, int lite);

/* KSM */
static void _check(char *path, long int value);
static void _group_check(int run, int pages_shared, int pages_sharing,
		int pages_volatile, int pages_unshared,
		int sleep_millisecs, int pages_to_scan);
static void _verify(char value, int proc, int start, int end,
		int start2, int end2);

/* cpuset/memcg */
static void _gather_cpus(char *cpus, long nd);

/* shared */

#endif /* __INTERNAL_H */
