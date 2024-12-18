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

#ifndef NUMA_HELPER_H
#define NUMA_HELPER_H

#include "config.h"
#ifdef HAVE_NUMA_H
# include <numa.h>
#endif
#ifdef HAVE_NUMAIF_H
# include <numaif.h>
#endif

struct tst_cg_group;

#define NH_MEMS (1 << 0)
#define NH_CPUS (1 << 1)

#if defined(__powerpc__) || defined(__powerpc64__)
# define MAXNODES                256
#else
# define MAXNODES                512
#endif

#define TESTMEM                 (1UL<<30)

#define BITS_PER_LONG (8 * sizeof(long))

#define PATH_SYS_SYSTEM         "/sys/devices/system"

static inline void set_node(unsigned long *array, unsigned int node)
{
	array[node / BITS_PER_LONG] |= 1UL << (node % BITS_PER_LONG);
}

unsigned long get_max_node(void);
int get_allowed_nodes_arr(int flag, int *num_nodes, int **nodes);
int get_allowed_nodes(int flag, int count, ...);
void nh_dump_nodes(void);
int is_numa(void (*cleanup_fn)(void), int flag, int min_nodes);

void write_node_cpusets(const struct tst_cg_group *cg, long nd);

#endif /* NUMA_HELPER_H */
