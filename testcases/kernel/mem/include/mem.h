#ifndef _MEM_H
#define _MEM_H
#include "config.h"
#include "test.h"
#include "usctest.h"

#define LENGTH			(3UL<<30)
#define SYSFS_OVER		"/proc/sys/vm/overcommit_memory"
#define OVERCOMMIT		1
#define NORMAL			2
#define MLOCK			3
#define KSM			4
#define CPATH			"/dev/cpuset"
#define CPATH_NEW		CPATH "/1"
#if defined(__powerpc__) || defined(__powerpc64__)
#define MAXNODES		256
#else
#define MAXNODES		512
#endif
#define MEMCG_PATH		"/dev/cgroup"
#define MEMCG_PATH_NEW		MEMCG_PATH "/1"
#define TESTMEM			(1UL<<30)
#define MB			(1UL<<20)
#define KB			(1UL<<10)
#define PATH_SYS_SYSTEM		"/sys/devices/system"
#define PATH_KSM		"/sys/kernel/mm/ksm/"
#define PATH_SYSVM		"/proc/sys/vm/"
#define PATH_MEMINFO		"/proc/meminfo"

char overcommit[BUFSIZ];
int opt_num, opt_size, opt_unit;
char *opt_numstr, *opt_sizestr, *opt_unitstr;
/* memory pointer to identify per process, MB unit, and byte like
   memory[process No.][MB unit No.][byte No.]. */
char ***memory;

/* For mm/oom* tests */
int  _alloc_mem(long int length, int testcase);
void _test_alloc(int testcase, int lite);
void oom(int testcase, int mempolicy, int lite);
void testoom(int mempolicy, int lite, int numa);

/* For mm/ksm* tests */
void _gather_cpus(char *cpus);
void _check(char *path, long int value);
void _group_check(int run, int pages_shared, int pages_sharing,
		int pages_volatile, int pages_unshared, int sleep_millisecs,
		int pages_to_scan);
void _verify(char value, int proc, int start, int end, int start2, int end2);
void write_cpusets(void);
void write_memcg(void);
void create_same_memory(int size, int num, int unit);
void check_ksm_options(int *size, int *num, int *unit);
void ksm_usage(void);

/* For mm/oom* and mm/ksm* tests*/
void umount_mem(char *path, char *path_new);
void mount_mem(char *name, char *fs, char *options, char *path, char *path_new);

/* general function */
long count_numa(void);
int  path_exist(const char *path, ...);
long read_meminfo(char *item);
void set_sys_tune(char *sys_file, long tune, int check);
long get_sys_tune(char *sys_file);
void cleanup(void);
void setup(void);

#endif
