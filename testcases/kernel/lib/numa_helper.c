/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include <errno.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "tso_safe_macros.h"
#include "numa_helper.h"
#include "lapi/syscalls.h"

unsigned long get_max_node(void)
{
	unsigned long max_node = 0;
#ifdef HAVE_NUMA_V2
	max_node = numa_max_possible_node() + 1;
#endif
	return max_node;
}

#ifdef HAVE_NUMA_V2
static void get_nodemask_allnodes(nodemask_t * nodemask, unsigned long max_node)
{
	unsigned long nodemask_size = max_node / 8;
	int i;
	char fn[64];
	struct stat st;

	memset(nodemask, 0, nodemask_size);
	for (i = 0; i < (int)max_node; i++) {
		sprintf(fn, "/sys/devices/system/node/node%d", i);
		if (stat(fn, &st) == 0)
			nodemask_set(nodemask, i);
	}
}

static int filter_nodemask_mem(nodemask_t * nodemask, unsigned long max_node)
{
#ifdef MPOL_F_MEMS_ALLOWED
	unsigned long nodemask_size = max_node / 8;
	memset(nodemask, 0, nodemask_size);
	/*
	 * avoid numa_get_mems_allowed(), because of bug in getpol()
	 * utility function in older versions:
	 * http://www.spinics.net/lists/linux-numa/msg00849.html
	 *
	 * At the moment numa_available() implementation also uses
	 * get_mempolicy, but let's make explicit check for ENOSYS
	 * here as well in case it changes in future. Silent ignore
	 * of ENOSYS is OK, because without NUMA caller gets empty
	 * set of nodes anyway.
	 */
	if (syscall(__NR_get_mempolicy, NULL, nodemask->n,
		    max_node, 0, MPOL_F_MEMS_ALLOWED) < 0) {
		if (errno == ENOSYS)
			return 0;
		return -2;
	}
#else
	int i;
	/*
	 * old libnuma/kernel don't have MPOL_F_MEMS_ALLOWED, so let's assume
	 * that we can use any node with memory > 0
	 */
	for (i = 0; i < (int)max_node; i++) {
		if (!nodemask_isset(nodemask, i))
			continue;
		if (numa_node_size64(i, NULL) <= 0)
			nodemask_clr(nodemask, i);
	}
#endif /* MPOL_F_MEMS_ALLOWED */
	return 0;
}

static int cpumask_has_cpus(char *cpumask, size_t len)
{
	unsigned int j;
	for (j = 0; j < len; j++)
		if (cpumask[j] == '\0')
			return 0;
		else if ((cpumask[j] > '0' && cpumask[j] <= '9') ||
			 (cpumask[j] >= 'a' && cpumask[j] <= 'f'))
			return 1;
	return 0;

}

static void filter_nodemask_cpu(nodemask_t * nodemask, unsigned long max_node)
{
	char *cpumask = NULL;
	char fn[64];
	FILE *f;
	size_t len;
	int i, ret;

	for (i = 0; i < (int)max_node; i++) {
		if (!nodemask_isset(nodemask, i))
			continue;
		sprintf(fn, "/sys/devices/system/node/node%d/cpumap", i);
		f = fopen(fn, "r");
		if (f) {
			ret = getdelim(&cpumask, &len, '\n', f);
			if ((ret > 0) && (!cpumask_has_cpus(cpumask, len)))
				nodemask_clr(nodemask, i);
			fclose(f);
		}
	}
	free(cpumask);
}
#endif /* HAVE_NUMA_V2 */

/*
 * get_allowed_nodes_arr - get number and array of available nodes
 * @num_nodes: pointer where number of available nodes will be stored
 * @nodes: array of available node ids, this is MPOL_F_MEMS_ALLOWED
 *                 node bitmask compacted (without holes), so that each field
 *                 contains node number. If NULL only num_nodes is
 *                 returned, otherwise it cotains new allocated array,
 *                 which caller is responsible to free.
 * RETURNS:
 *     0 on success
 *    -1 on allocation failure
 *    -2 on get_mempolicy failure
 */
int get_allowed_nodes_arr(int flag, int *num_nodes, int **nodes)
{
	int ret = 0;
#ifdef HAVE_NUMA_V2
	int i;
	nodemask_t *nodemask = NULL;
#endif
	*num_nodes = 0;
	if (nodes)
		*nodes = NULL;

#ifdef HAVE_NUMA_V2
	unsigned long max_node, nodemask_size;

	if (numa_available() == -1)
		return 0;

	max_node = LTP_ALIGN(get_max_node(), sizeof(unsigned long)*8);
	nodemask_size = max_node / 8;

	nodemask = malloc(nodemask_size);
	if (nodes)
		*nodes = malloc(sizeof(int) * max_node);

	do {
		if (nodemask == NULL || (nodes && (*nodes == NULL))) {
			ret = -1;
			break;
		}

		/* allow all nodes at start, then filter based on flags */
		get_nodemask_allnodes(nodemask, max_node);
		if ((flag & NH_MEMS) == NH_MEMS) {
			ret = filter_nodemask_mem(nodemask, max_node);
			if (ret < 0)
				break;
		}
		if ((flag & NH_CPUS) == NH_CPUS)
			filter_nodemask_cpu(nodemask, max_node);

		for (i = 0; i < (int)max_node; i++) {
			if (nodemask_isset(nodemask, i)) {
				if (nodes)
					(*nodes)[*num_nodes] = i;
				(*num_nodes)++;
			}
		}
	} while (0);
	free(nodemask);
#endif /* HAVE_NUMA_V2 */
	return ret;
}

/*
 * get_allowed_nodes - convenience function to get fixed number of nodes
 * @count: how many nodes to get
 * @...: int pointers, where node ids will be stored
 * RETURNS:
 *     0 on success
 *    -1 on allocation failure
 *    -2 on get_mempolicy failure
 *    -3 on not enough allowed nodes
 */
int get_allowed_nodes(int flag, int count, ...)
{
	int ret;
	int i, *nodep;
	va_list ap;
	int num_nodes = 0;
	int *nodes = NULL;

	ret = get_allowed_nodes_arr(flag, &num_nodes, &nodes);
	if (ret < 0)
		return ret;

	va_start(ap, count);
	for (i = 0; i < count; i++) {
		nodep = va_arg(ap, int *);
		if (i < num_nodes) {
			*nodep = nodes[i];
		} else {
			ret = -3;
			errno = EINVAL;
			break;
		}
	}
	free(nodes);
	va_end(ap);

	return ret;
}

static void print_node_info(int flag)
{
	int *allowed_nodes = NULL;
	int i, ret, num_nodes;

	ret = get_allowed_nodes_arr(flag, &num_nodes, &allowed_nodes);
	printf("nodes (flag=%d): ", flag);
	if (ret == 0) {
		for (i = 0; i < num_nodes; i++)
			printf("%d ", allowed_nodes[i]);
		printf("\n");
	} else
		printf("error(%d)\n", ret);
	free(allowed_nodes);
}

/*
 * nh_dump_nodes - dump info about nodes to stdout
 */
void nh_dump_nodes(void)
{
	print_node_info(0);
	print_node_info(NH_MEMS);
	print_node_info(NH_CPUS);
	print_node_info(NH_MEMS | NH_CPUS);
}

/*
 * is_numa - judge a system is NUMA system or not
 * @flag: NH_MEMS and/or NH_CPUS
 * @min_nodes: find at least 'min_nodes' nodes with memory
 * NOTE: the function is designed to try to find at least 'min_nodes'
 * available nodes, where each node contains memory.
 * WARN: Don't use this func in child, as it calls tst_brkm()
 * RETURNS:
 *     0 - it's not a NUMA system
 *     1 - it's a NUMA system
 */
int is_numa(void (*cleanup_fn)(void), int flag, int min_nodes)
{
	int ret;
	int numa_nodes = 0;

	ret = get_allowed_nodes_arr(flag, &numa_nodes, NULL);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup_fn, "get_allowed_nodes_arr");

	if (numa_nodes >= min_nodes)
		return 1;
	else
		return 0;
}
