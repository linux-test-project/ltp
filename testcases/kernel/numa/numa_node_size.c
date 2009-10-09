/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        numa_node_size.c                                              */
/*                                                                            */
/* Description: Invokes numa_node_size() API                                  */
/*                                                                            */
/* Author:     Pradeep Kumar Surisetty pradeepkumars@in.ibm.com               */
/*                                                                            */
/* History:     Created - Nov 28 2007 - Pradeep Kumar Surisetty               */
/*                                                 pradeepkumars@in.ibm.com   */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#if HAS_NUMA_H
#include <numa.h>
#endif

int numa_exit_on_error = 0;
char *fmt_mem(unsigned long long mem, char *buf)
{
        if (mem == -1L)
            sprintf(buf, "<not available>");
        else
            sprintf(buf, "%Lu MB", mem >> 20);
        return buf;
}
void hardware(void)
{
#if HAS_NUMA_H
        int i;
        int maxnode = numa_max_node();
        printf("available: %d nodes (0-%d)\n", 1+maxnode, maxnode);
        for (i = 0; i <= maxnode; i++) {
            char buf[64];
            long fr;
            unsigned long sz = numa_node_size(i, &fr);
            printf("node %d cpus:", i);
            printf("node %d size: %s\n", i, fmt_mem(sz, buf));
            printf("node %d free: %s\n", i, fmt_mem(fr, buf));
        }
#endif
}
int main()
{
#if HAS_NUMA_H
        nodemask_t nodemask;
        void hardware();
        if (numa_available() < 0)
        {
            printf("This system does not support NUMA policy\n");
            numa_error("numa_available");
            numa_exit_on_error = 1;
            exit(numa_exit_on_error);
        }
        nodemask_zero(&nodemask);
        nodemask_set(&nodemask,1);
        numa_bind(&nodemask);
        hardware();
        return numa_exit_on_error;
#else
        printf("NUMA is not available\n");
        return 1;
#endif
}
