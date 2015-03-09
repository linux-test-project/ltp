/*
 *
 *   Copyright © International Business Machines  Corp., 2007, 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation,Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

/*
 * NAME
 *	getcpu1.c
 *
 * DESCRIPTION
 *	getcpu01 - call getcpu() and make sure it succeeds
 *
 * ALGORITHM
 *	set cpu affinity of the process
 *	If setaffinity() fails exit from the test suite
 *	Store the node ID of the cpu which has been set in previous step
 *	Make a call to getcpu() system call
 *	Verify the returned valued with the set value
 *	if they match
 *	  test is considered PASS
 *	else
 *	  test is considered FAIL
 *
 * USAGE:  <for command-line>
 *  getcpu [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	06/2008 written by Sharyathi Nagesh <sharyathi@in.ibm.com>
 *
 *      05/2009         Suzuki K P <suzuki@in.ibm.com>
 *                      Use TCONF instead of TWARN for non-NUMA machines
 *
 * RESTRICTIONS
 *	none
 */

#define _GNU_SOURCE
#include <sched.h>
#include <errno.h>
#include "test.h"
#include <sys/types.h>
#include <dirent.h>

#if defined(__i386__) || defined(__x86_64__)
#if __GLIBC_PREREQ(2,6)
#if defined(__x86_64__)
#include <utmpx.h>
#endif
int sys_support = 1;
#elif defined(__i386__)
int sys_support = 1;
#else
int sys_support = 0;
#endif
#else
int sys_support = 0;
#endif

#if !(__GLIBC_PREREQ(2, 7))
#define CPU_FREE(ptr) free(ptr)
#endif

void cleanup(void);
void setup(void);
static inline int getcpu(unsigned int *, unsigned int *, void *);
unsigned int set_cpu_affinity(void);
unsigned int get_nodeid(unsigned int);
unsigned int max_cpuid(size_t, cpu_set_t *);

char *TCID = "getcpu01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	unsigned int cpu_id, node_id = 0;
	unsigned int cpu_set;
#ifdef __i386__
	unsigned int node_set;
#endif

	/* Check For Kernel Version */
	if (((tst_kvercmp(2, 6, 20)) < 0) || !(sys_support)) {
		tst_resm(TCONF, "This test can only run on kernels that are ");
		tst_resm(TCONF,
			 "2.6.20 and higher and glibc version 2.6 and above");
		tst_resm(TCONF, "Currently the test case has been");
		tst_resm(TCONF, "developed only for i386 and x86_64");
		exit(0);
	}

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* call the system call with the TEST() macro */
		cpu_set = set_cpu_affinity();
#ifdef __i386__
		node_set = get_nodeid(cpu_set);
#endif
		TEST(getcpu(&cpu_id, &node_id, NULL));
		if (TEST_RETURN == 0) {
			if (cpu_id != cpu_set) {
				tst_resm(TFAIL, "getcpu() returned wrong value"
					 " expected cpuid:%d, returned value cpuid: %d",
					 cpu_set, cpu_id);

			}
#ifdef __i386__
			else if (node_id != node_set) {
				tst_resm(TFAIL, "getcpu() returned wrong value"
					 " expected  node id:%d returned  node id:%d",
					 node_set, node_id);

			}
#endif
			else
				tst_resm(TPASS, "getcpu() returned proper"
					 " cpuid:%d, node id:%d", cpu_id,
					 node_id);
		} else {
			tst_resm(TFAIL, "getcpu() Failed, errno=%d:%s",
				 TEST_ERRNO, strerror(TEST_ERRNO));

		}
	}

	cleanup();

	tst_exit();
}

/*
 * getcpu() - calls the system call
 */
static inline int getcpu(unsigned *cpu_id, unsigned *node_id,
			 void *cache_struct)
{
#if defined(__i386__)
	return syscall(318, cpu_id, node_id, cache_struct);
#elif __GLIBC_PREREQ(2,6)
	*cpu_id = sched_getcpu();
#endif
	return 0;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	/* ?? */

	TEST_PAUSE;
}

/*
 * This will set the affinity to max cpu on which process can run
 * and return that cpu id to the calling process
 */
unsigned int set_cpu_affinity(void)
{
	unsigned cpu_max;
	cpu_set_t *set;
	size_t size;
	int nrcpus = 1024;
#if __GLIBC_PREREQ(2, 7)
realloc:
	set = CPU_ALLOC(nrcpus);
#else
	set = malloc(sizeof(cpu_set_t));
#endif
	if (set == NULL) {
		tst_brkm(TFAIL, NULL, "CPU_ALLOC:errno:%d", errno);
	}
#if __GLIBC_PREREQ(2, 7)
	size = CPU_ALLOC_SIZE(nrcpus);
	CPU_ZERO_S(size, set);
#else
	size = sizeof(cpu_set_t);
	CPU_ZERO(set);
#endif
	if (sched_getaffinity(0, size, set) < 0) {
		CPU_FREE(set);
#if __GLIBC_PREREQ(2, 7)
		if (errno == EINVAL && nrcpus < (1024 << 8)) {
			nrcpus = nrcpus << 2;
			goto realloc;
		}
#else
		if (errno == EINVAL)
			tst_resm(TFAIL,
				 "NR_CPUS of the kernel is more than 1024, so we'd better use a newer glibc(>= 2.7)");
		else
#endif
			tst_resm(TFAIL, "sched_getaffinity:errno:%d", errno);
		tst_exit();
	}
	cpu_max = max_cpuid(size, set);
#if __GLIBC_PREREQ(2, 7)
	CPU_ZERO_S(size, set);
	CPU_SET_S(cpu_max, size, set);
#else
	CPU_ZERO(set);
	CPU_SET(cpu_max, set);
#endif
	if (sched_setaffinity(0, size, set) < 0) {
		CPU_FREE(set);
		tst_brkm(TFAIL, NULL, "sched_setaffinity:errno:%d", errno);
	}
	CPU_FREE(set);
	return cpu_max;
}

/*
 * Return the maximum cpu id
 */
#define BITS_PER_BYTE 8
unsigned int max_cpuid(size_t size, cpu_set_t * set)
{
	unsigned int index, max = 0;
	for (index = 0; index < size * BITS_PER_BYTE; index++)
#if __GLIBC_PREREQ(2, 7)
		if (CPU_ISSET_S(index, size, set))
#else
		if (CPU_ISSET(index, set))
#endif
			max = index;
	return max;
}

/*
 * get_nodeid(cpuid) - This will return the node to which selected cpu belongs
 */
unsigned int get_nodeid(unsigned int cpu_id)
{
	DIR *directory_parent, *directory_node;
	struct dirent *de, *dn;
	char directory_path[255];
	unsigned int cpu;
	int node_id = 0;

	directory_parent = opendir("/sys/devices/system/node");
	if (!directory_parent) {
		tst_resm(TCONF,
			 "/sys not mounted or not a numa system. Assuming one node");
		tst_resm(TCONF,
			 "Error opening: /sys/devices/system/node :%s",
			 strerror(errno));
		return 0;	//By Default assume it to belong to node Zero
	} else {
		while ((de = readdir(directory_parent)) != NULL) {
			if (strncmp(de->d_name, "node", 4))
				continue;
			sprintf(directory_path, "/sys/devices/system/node/%s",
				de->d_name);
			directory_node = opendir(directory_path);
			while ((dn = readdir(directory_node)) != NULL) {
				if (strncmp(dn->d_name, "cpu", 3))
					continue;
				cpu = strtoul(dn->d_name + 3, NULL, 0);
				if (cpu == cpu_id) {
					node_id =
					    strtoul(de->d_name + 4, NULL, 0);
					break;
				}
			}
			closedir(directory_node);
		}
		closedir(directory_parent);
	}
	return node_id;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}
