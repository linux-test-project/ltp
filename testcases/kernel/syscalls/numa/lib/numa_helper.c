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
	struct bitmask *allowed_nodemask = NULL;
#endif
	*num_allowed_nodes = 0;
	if (allowed_nodes)
		*allowed_nodes = NULL;

#if HAVE_NUMA_H && HAVE_MPOL_CONSTANTS
	if ((allowed_nodemask = numa_allocate_nodemask()) == NULL)
		return -1;

	if (allowed_nodes) {
		*allowed_nodes = malloc(sizeof(int)*allowed_nodemask->size);
		if (*allowed_nodes == NULL)
			return -1;
	}

	/*
	 * avoid numa_get_mems_allowed(), because of bug in getpol()
	 * utility function in older versions:
	 * http://www.spinics.net/lists/linux-numa/msg00849.html
	 */
	if (syscall(__NR_get_mempolicy, NULL, allowed_nodemask->maskp,
		allowed_nodemask->size, 0, MPOL_F_MEMS_ALLOWED) < 0) {
		numa_bitmask_free(allowed_nodemask);
		if (allowed_nodes) {
			free(*allowed_nodes);
			*allowed_nodes = NULL;
		}
		return -2;
	}

	for (i = 0; i <	allowed_nodemask->size; i++) {
		if (numa_bitmask_isbitset(allowed_nodemask, i)) {
			if (allowed_nodes)
				(*allowed_nodes)[*num_allowed_nodes] = i;
			(*num_allowed_nodes)++;
		}
	}
	numa_bitmask_free(allowed_nodemask);
#endif
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
