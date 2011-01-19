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
#define MAXNODES		512
#define MEMCG_PATH		"/dev/cgroup"
#define MEMCG_PATH_NEW		MEMCG_PATH "/1"
#define TESTMEM			(1UL<<30)
#define MB			(1UL<<20)
#define PATH_SYS_SYSTEM		"/sys/devices/system"

char overcommit[BUFSIZ];

void oom(int testcase, int mempolicy, int lite);
void testoom(int mempolicy, int lite, int numa);
long count_numa(void);
int path_exist(const char *path, ...);
int alloc_mem(long int length, int testcase);
void test_alloc(int testcase, int lite);
void gather_cpus(char *cpus);
void umount_mem(char *path, char *path_new);
void mount_mem(char *name, char *fs, char *options, char *path, char *path_new);
void cleanup(void) LTP_ATTRIBUTE_NORETURN;
