/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MOVE_PAGES_SUPPORT_H
#define MOVE_PAGES_SUPPORT_H

#include "config.h"
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <semaphore.h>
#include "numa_helper.h"

long get_page_size();

void free_pages(void **pages, unsigned int num);

int alloc_pages_on_nodes(void **pages, unsigned int num, int *nodes);
int alloc_pages_linear(void **pages, unsigned int num);
int alloc_pages_on_node(void **pages, unsigned int num, int node);

void verify_pages_on_nodes(void **pages, int *status,
			   unsigned int num, int *nodes);
void verify_pages_linear(void **pages, int *status, unsigned int num);
void verify_pages_on_node(void **pages, int *status,
			  unsigned int num, int node);

int alloc_shared_pages_on_node(void **pages, unsigned int num, int node);
void free_shared_pages(void **pages, unsigned int num);

sem_t *alloc_sem(int num);
void free_sem(sem_t *sem, int num);

void check_config(unsigned int min_nodes);

#endif
