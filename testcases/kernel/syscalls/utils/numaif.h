/*
 * Crackerjack Project
 *
 * Copyright (C) 2007-2008, Hitachi, Ltd.
 * Author(s): Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *            Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 *
 *            Derived from 'numa.h' in numactl-0.9.8
 *            Copyright (C) 2003,2004 Andi Kleen, SuSE Labs.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id: numaif.h,v 1.6 2009/12/11 13:18:26 yaberauneya Exp $
 *
 */

#include "config.h"
#include "include_j_h.h"
#include "linux_syscall_numbers.h"

#if HAVE_NUMA_H
#include <numa.h>
#else /* numa.h doesn't exist on the system. */

#if defined(__x86_64__) || defined(__i386__)
#define NUMA_NUM_NODES 	128
#else 
#define NUMA_NUM_NODES	2048
#endif

typedef struct { 
	unsigned long n[NUMA_NUM_NODES/(sizeof(unsigned long)*8)];
} nodemask_t;

static inline void nodemask_zero(nodemask_t *mask)
{ 
	memset(mask->n, 0, sizeof(mask->n)); 
} 

static inline void nodemask_clr(nodemask_t *mask, int node)
{
	mask->n[node / (8*sizeof(unsigned long))] &= 
		~(1UL << (node % (8*sizeof(unsigned long))));	
}

static inline int nodemask_isset(const nodemask_t *mask, int node)
{
	if ((unsigned)node >= NUMA_NUM_NODES)
		return 0;
	if (mask->n[node / (8*sizeof(unsigned long))] & 
		(1UL << (node % (8*sizeof(unsigned long)))))
		return 1;
	return 0;	
}

static inline int nodemask_equal(const nodemask_t *a, const nodemask_t *b) 
{ 
	int i;
	for (i = 0; i < NUMA_NUM_NODES/(sizeof(unsigned long)*8); i++) 
		if (a->n[i] != b->n[i]) 
			return 0; 
	return 1;
}

static inline void nodemask_set(nodemask_t *mask, int node)
{
	mask->n[node / (8*sizeof(unsigned long))] |=
		(1UL << (node % (8*sizeof(unsigned long))));		
}
#endif

static inline void nodemask_dump(const char *header, const nodemask_t *mask)
{
	int i;
	EPRINTF("%s", header);
	for (i = 0; i < NUMA_NUM_NODES/(sizeof(unsigned long)*8); i++) 
		EPRINTF(" 0x%08lx", mask->n[i]);
	EPRINTF("\n");
}

#ifndef MPOL_DEFAULT
   // Policies
#  define MPOL_DEFAULT			0
#  define MPOL_PREFERRED		1
#  define MPOL_BIND			2
#  define MPOL_INTERLEAVE		3
   // Flags for get_mem_policy
#  define MPOL_F_NODE			(1<<0)
#  define MPOL_F_ADDR			(1<<1)
   // Flags for mbind
#  define MPOL_MF_STRICT		(1<<0)
#  define MPOL_MF_MOVE			(1<<1)
#  define MPOL_MF_MOVE_ALL		(1<<2)
#endif
