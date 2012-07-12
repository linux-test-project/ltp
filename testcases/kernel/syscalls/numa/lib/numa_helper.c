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
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "numa_helper.h"
#include "linux_syscall_numbers.h"

/*
 * get_allowed_nodes_arr - get number and array of available nodes
 * @num_allowed_nodes: pointer where number of available nodes will be stored
 * @allowed_nodes: array of available node ids, this is MPOL_F_MEMS_ALLOWED
 *                 node bitmask compacted (without holes), so that each field
 *                 contains node number. If NULL only num_allowed_nodes is
 *                 returned, otherwise it cotains new allocated array,
 *                 which caller is responsible to free.
 * RETURNS:
 *     0 on success
 *    -1 on allocation failure
 *    -2 on get_mempolicy failure
 */
int get_allowed_nodes_arr(int *num_allowed_nodes, int **allowed_nodes)
{
#if HAVE_NUMA_H
	int i;
	nodemask_t *allowed_nodemask = NULL;
	unsigned long max_node;

#if !defined(LIBNUMA_API_VERSION) || LIBNUMA_API_VERSION < 2
	max_node = NUMA_NUM_NODES;
	/*
	 * NUMA_NUM_NODES is not reliable, libnuma >=2 is looking
	 * at /proc/self/status to figure out correct number.
	 * If buffer is not large enough get_mempolicy will fail with EINVAL.
	 */
	if (max_node < 1024)
		max_node = 1024;
#else
	max_node = numa_max_possible_node() + 1;
#endif
#endif /* HAVE_NUMA_H */

	*num_allowed_nodes = 0;
	if (allowed_nodes)
		*allowed_nodes = NULL;

#if HAVE_NUMA_H
	if ((allowed_nodemask = malloc(max_node/8+1)) == NULL)
		return -1;
	nodemask_zero(allowed_nodemask);

	if (allowed_nodes) {
		*allowed_nodes = malloc(sizeof(int)*max_node);
		if (*allowed_nodes == NULL) {
			free(allowed_nodemask);
			return -1;
		}
	}

#if MPOL_F_MEMS_ALLOWED
	/*
	 * avoid numa_get_mems_allowed(), because of bug in getpol()
	 * utility function in older versions:
	 * http://www.spinics.net/lists/linux-numa/msg00849.html
	 */
	if (syscall(__NR_get_mempolicy, NULL, allowed_nodemask->n,
		max_node, 0, MPOL_F_MEMS_ALLOWED) < 0) {
		free(allowed_nodemask);
		if (allowed_nodes) {
			free(*allowed_nodes);
			*allowed_nodes = NULL;
		}
		return -2;
	}
#else
	/*
	 * old libnuma/kernel don't have MPOL_F_MEMS_ALLOWED, so let's assume
	 * that we can use any node with memory > 0
	 */
	for (i = 0; i < max_node; i++)
		if (numa_node_size64(i, NULL) > 0)
			nodemask_set(allowed_nodemask, i);

#endif /* MPOL_F_MEMS_ALLOWED */
	for (i = 0; i < max_node; i++) {
		if (nodemask_isset(allowed_nodemask, i)) {
			if (allowed_nodes)
				(*allowed_nodes)[*num_allowed_nodes] = i;
			(*num_allowed_nodes)++;
		}
	}
	free(allowed_nodemask);
#endif /* HAVE_NUMA_H */
	return 0;
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
int get_allowed_nodes(int count, ...)
{
	int ret;
	int i, *nodep;
	va_list ap;
	int num_nodes = 0;
	int *nodes = NULL;

	if ((ret = get_allowed_nodes_arr(&num_nodes, &nodes)) < 0)
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
