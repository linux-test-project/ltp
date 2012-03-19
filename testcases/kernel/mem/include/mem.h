#ifndef _MEM_H
#define _MEM_H
#include "config.h"
#include "test.h"
#include "usctest.h"

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

/* OOM */

#define LENGTH			(3UL<<30)
#define TESTMEM			(1UL<<30)
#define OVERCOMMIT		1
#define NORMAL			2
#define MLOCK			3
#define KSM			4

long overcommit;
void oom(int testcase, int mempolicy, int lite);
void testoom(int mempolicy, int lite, int numa);

/* KSM */

#define PATH_KSM		"/sys/kernel/mm/ksm/"

/*
 * memory pointer to identify per process, MB unit, and byte like
 * memory[process No.][MB unit No.][byte No.].
 */
char ***memory;
void write_memcg(void);
void create_same_memory(int size, int num, int unit);
int  opt_num, opt_size, opt_unit;
char *opt_numstr, *opt_sizestr, *opt_unitstr;
void check_ksm_options(int *size, int *num, int *unit);
void ksm_usage(void);

/* cpuset/memcg */

#define CPATH			"/dev/cpuset"
#define CPATH_NEW		CPATH "/1"
#define MEMCG_PATH		"/dev/cgroup"
#define MEMCG_PATH_NEW		MEMCG_PATH "/1"
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
long count_numa(long nodes[]);
int  path_exist(const char *path, ...);
long read_meminfo(char *item);
void set_sys_tune(char *sys_file, long tune, int check);
long get_sys_tune(char *sys_file);
void write_file(char *filename, char *buf);
void read_file(char *filename, char *retbuf);
void cleanup(void);
void setup(void);

#endif
