#ifndef _MEM_H
#define _MEM_H
#include "config.h"
#include "tst_test.h"
#include "ksm_helper.h"

#if defined(__powerpc__) || defined(__powerpc64__)
#define MAXNODES		256
#else
#define MAXNODES		512
#endif
#define MB			(1UL<<20)
#define KB			(1UL<<10)
#define PATH_SYS_SYSTEM		"/sys/devices/system"
#define PATH_SYSVM		"/proc/sys/vm/"
#define PATH_MEMINFO		"/proc/meminfo"
#define BITS_PER_LONG           (8 * sizeof(long))

static inline void set_node(unsigned long *array, unsigned int node)
{
	array[node / BITS_PER_LONG] |= 1UL << (node % BITS_PER_LONG);
}

static inline void clean_node(unsigned long *array)
{
	unsigned int i;

	for (i = 0; i < MAXNODES / BITS_PER_LONG; i++)
		array[i] &= 0UL;
}

/* OOM */

#define LENGTH			(3UL<<30)
#define TESTMEM			(1UL<<30)
#define NORMAL			1
#define MLOCK			2
#define KSM			3

extern long overcommit;
void oom(int testcase, int lite, int retcode, int allow_sigkill);
void testoom(int mempolicy, int lite, int retcode, int allow_sigkill);

/* KSM */

void create_same_memory(int size, int num, int unit);
void test_ksm_merge_across_nodes(unsigned long nr_pages);

/* THP */

#define PATH_THP		"/sys/kernel/mm/transparent_hugepage/"
#define PATH_KHPD		PATH_THP "khugepaged/"

/* HUGETLB */

#define PATH_HUGEPAGES		"/sys/kernel/mm/hugepages/"
#define PATH_SHMMAX		"/proc/sys/kernel/shmmax"

void check_hugepage(void);
void write_memcg(void);

/* cpuset/memcg */

#define CPATH			"/dev/cpuset"
#define CPATH_NEW		CPATH "/1"
#define MEMCG_PATH		"/dev/cgroup"
#define MEMCG_PATH_NEW		MEMCG_PATH "/1"
#define MEMCG_LIMIT		MEMCG_PATH_NEW "/memory.limit_in_bytes"
#define MEMCG_SW_LIMIT		MEMCG_PATH_NEW "/memory.memsw.limit_in_bytes"
#if HAVE_SYS_EVENTFD_H
#define PATH_OOMCTRL		MEMCG_PATH_NEW "/memory.oom_control"
#define PATH_EVTCTRL		MEMCG_PATH_NEW "/cgroup.event_control"
#endif

void read_cpuset_files(char *prefix, char *filename, char *retbuf);
void write_cpuset_files(char *prefix, char *filename, char *buf);
void write_cpusets(long nd);
void mount_mem(char *name, char *fs, char *options, char *path, char *path_new);
void umount_mem(char *path, char *path_new);

/* shared */
unsigned int get_a_numa_node(void);
int  path_exist(const char *path, ...);
void set_sys_tune(char *sys_file, long tune, int check);
long get_sys_tune(char *sys_file);

void update_shm_size(size_t *shm_size);

/* MMAP */
int range_is_mapped(unsigned long low, unsigned long high);
#endif
