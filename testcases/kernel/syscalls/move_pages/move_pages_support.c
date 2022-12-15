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

#include "config.h"
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <semaphore.h>
#include "test.h"
#include "move_pages_support.h"

long get_page_size(void)
{
	return sysconf(_SC_PAGESIZE);
}

/*
 * free_pages() - free an array of pages
 * @pages: array of page pointers to be freed
 * @num: no. of pages in the array
 */
void free_pages(void **pages, unsigned int num)
{
#ifdef HAVE_NUMA_V2
	int i;
	size_t onepage = get_page_size();

	for (i = 0; i < num; i++) {
		if (pages[i] != NULL) {
			numa_free(pages[i], onepage);
		}
	}
#endif
}

/*
 * alloc_pages_on_nodes() - allocate pages on specified NUMA nodes
 * @pages: array in which the page pointers will be stored
 * @num: no. of pages to allocate
 * @nodes: array of NUMA nodes
 *
 * A page will be allocated in each node specified by @nodes, and the
 * page pointers will be stored in @pages array.
 *
 * RETURNS:
 * 0 on success, -1 on allocation failure.
 */
int alloc_pages_on_nodes(void **pages, unsigned int num, int *nodes)
{
	int i;
#ifdef HAVE_NUMA_V2
	size_t onepage = get_page_size();
#endif

	for (i = 0; i < (int)num; i++) {
		pages[i] = NULL;
	}

	for (i = 0; i < (int)num; i++) {
		char *page;

#ifdef HAVE_NUMA_V2
		pages[i] = numa_alloc_onnode(onepage, nodes[i]);
#endif
		if (pages[i] == NULL) {
			tst_resm(TBROK, "allocation of page on node "
				 "%d failed", nodes[i]);
			break;
		}

		/* Touch the page, to force allocation. */
		page = pages[i];
		page[0] = i;
	}

	if (i == (int)num)
		return 0;

	free_pages(pages, num);

	return -1;
}

/*
 * alloc_pages_linear() - allocate pages in each NUMA node
 * @pages: array in which the page pointers will be stored
 * @num: no. of pages to allocate
 *
 * Pages will be allocated one from each NUMA node, in an interleaved
 * fashion.
 *
 * RETURNS:
 * 0 on success, -1 on allocation failure.
 */
int alloc_pages_linear(void **pages, unsigned int num)
{
	int nodes[num];

#ifdef HAVE_NUMA_V2
	unsigned int i;
	unsigned int n = 0;
	int num_allowed_nodes;
	int *allowed_nodes;
	int ret;

	ret = get_allowed_nodes_arr(NH_MEMS, &num_allowed_nodes,
				    &allowed_nodes);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes(): %d", ret);

	for (i = 0; i < num; i++) {
		nodes[i] = allowed_nodes[n];
		n++;
		if (n >= num_allowed_nodes)
			n = 0;
	}
	free(allowed_nodes);
#endif

	return alloc_pages_on_nodes(pages, num, nodes);
}

/*
 * alloc_pages_on_node() - allocate pages on specified NUMA node
 * @pages: array in which the page pointers will be stored
 * @num: no. of pages to allocate
 * @node: NUMA node from which to allocate pages
 *
 * Pages will be allocated from the NUMA node @node, and the page
 * pointers will be stored in the @pages array.
 *
 * RETURNS:
 * 0 on success, -1 on allocation failure.
 */
int alloc_pages_on_node(void **pages, unsigned int num, int node)
{
	unsigned int i;
	int nodes[num];

	for (i = 0; i < num; i++)
		nodes[i] = node;

	return alloc_pages_on_nodes(pages, num, nodes);
}

/*
 * verify_pages_on_nodes() - verify pages are in specified nodes
 * @pages: array of pages to be verified
 * @status: the NUMA node of each page
 * @num: the no. of pages
 * @nodes: the expected NUMA nodes
 */
void
verify_pages_on_nodes(void **pages, int *status, unsigned int num, int *nodes)
{
#ifdef HAVE_NUMA_V2
	unsigned int i;
	int which_node;
	int ret;

	for (i = 0; i < num; i++) {
		if (status[i] != nodes[i]) {
			tst_resm(TFAIL, "page %d on node %d, "
				 "expected on node %d", i, status[i], nodes[i]);
			return;
		}

		/* Based on inputs from Andi Kleen.
		 *
		 * Retrieves numa node for the given page. This does
		 * not seem to be documented in the man pages.
		 */
		ret = get_mempolicy(&which_node, NULL, 0,
				    pages[i], MPOL_F_NODE | MPOL_F_ADDR);
		if (ret == -1) {
			tst_resm(TBROK | TERRNO, "error getting memory policy "
				 "for page %p", pages[i]);
			return;
		}

		if (which_node != nodes[i]) {
			tst_resm(TFAIL, "page %p is not in node %d ",
				 pages[i], nodes[i]);
			return;
		}
	}

	tst_resm(TPASS, "pages are present in expected nodes");
#else
	tst_resm(TCONF, "NUMA support not provided");
#endif
}

/*
 * verify_pages_linear() - verify pages are interleaved
 * @pages: array of pages to be verified
 * @status: the NUMA node of each page
 * @num: the no. of pages
 */
void verify_pages_linear(void **pages, int *status, unsigned int num)
{
#ifdef HAVE_NUMA_V2
	unsigned int i;
	unsigned int n = 0;
	int nodes[num];
	int num_allowed_nodes;
	int *allowed_nodes;
	int ret;

	ret = get_allowed_nodes_arr(NH_MEMS, &num_allowed_nodes,
				    &allowed_nodes);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes(): %d", ret);

	for (i = 0; i < num; i++) {
		nodes[i] = allowed_nodes[n];
		n++;
		if (n >= num_allowed_nodes)
			n = 0;
	}
	free(allowed_nodes);

	verify_pages_on_nodes(pages, status, num, nodes);
#endif
}

/*
 * verify_pages_on_node() - verify pages are in specified node
 * @pages: array of pages to be verified
 * @status: the NUMA node of each page
 * @num: the no. of pages
 * @node: the expected NUMA node
 */
void verify_pages_on_node(void **pages, int *status, unsigned int num, int node)
{
	unsigned int i;
	int nodes[num];

	for (i = 0; i < num; i++) {
		nodes[i] = node;
	}

	verify_pages_on_nodes(pages, status, num, nodes);
}

/*
 * alloc_shared_pages_on_node() - allocate shared pages on a NUMA node
 * @pages: array to store the allocated pages
 * @num: the no. of pages to allocate
 * @node: the node in which the pages should be allocated
 *
 * RETURNS:
 * 0 on success, -1 on allocation failure
 */
int alloc_shared_pages_on_node(void **pages, unsigned int num, int node)
{
#ifdef HAVE_NUMA_V2
	char *shared;
	unsigned int i;
	int nodes[num];
	size_t total_size = num * get_page_size();

	shared = mmap(NULL, total_size,
		      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if (shared == MAP_FAILED) {
		tst_resm(TBROK | TERRNO, "allocation of shared pages failed");
		return -1;
	}

	numa_tonode_memory(shared, total_size, node);

	for (i = 0; i < num; i++) {
		char *page;

		pages[i] = shared;
		shared += get_page_size();

		nodes[i] = node;

		/* Touch the page to force allocation */
		page = pages[i];
		page[0] = i;
	}

	return 0;
#else
	return -1;
#endif
}

/*
 * free_shared_pages() - free shared pages
 * @pages: array of pages to be freed
 * @num: the no. of pages to free
 */
void free_shared_pages(void **pages, unsigned int num)
{
	int ret;

	ret = munmap(pages[0], num * get_page_size());
	if (ret == -1)
		tst_resm(TWARN | TERRNO, "unmapping of shared pages failed");
}

/*
 * alloc_sem() - allocate semaphores
 * @num - no. of semaphores to create
 *
 * Allocate and initialize semaphores in a shared memory area, so that
 * the semaphore can be used across processes.
 *
 * RETURNS:
 * Array of initialized semaphores.
 */
sem_t *alloc_sem(int num)
{
	sem_t *sem;
	void *sem_mem;
	int i, ret;

	sem_mem = mmap(NULL, get_page_size(),
		       PROT_READ | PROT_WRITE,
		       MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if (sem_mem == MAP_FAILED) {
		tst_resm(TBROK | TERRNO, "allocation of semaphore page failed");
		goto err_exit;
	}

	sem = sem_mem;

	for (i = 0; i < num; i++) {
		ret = sem_init(&sem[i], 1, 0);
		if (ret == -1) {
			tst_resm(TBROK | TERRNO, "semaphore initialization "
				 "failed");
			goto err_free_mem;
		}
	}

	return sem;

err_free_mem:
	ret = munmap(sem_mem, get_page_size());
	if (ret == -1)
		tst_resm(TWARN | TERRNO, "error freeing semaphore memory");
err_exit:
	return NULL;
}

/*
 * free_sem() - free semaphores
 * @sem - array of semphores to be freed
 * @num - no. of semaphores in the array
 */
void free_sem(sem_t * sem, int num)
{
	int i;
	int ret;

	for (i = 0; i < num; i++) {
		ret = sem_destroy(&sem[i]);
		if (ret == -1)
			tst_resm(TWARN | TERRNO, "error destroying semaphore");
	}

	ret = munmap(sem, get_page_size());
	if (ret == -1)
		tst_resm(TWARN | TERRNO, "error freeing semaphore memory");
}

/*
 * check_config() - check for required configuration
 * @min_nodes: the minimum required NUMA nodes
 *
 * Checks if numa support is availabe, kernel is >= 2.6.18, arch is
 * one of the supported architectures.
 */
void check_config(unsigned int min_nodes)
{
#ifdef HAVE_NUMA_V2
	int num_allowed_nodes;
	int ret;

	ret = get_allowed_nodes_arr(NH_MEMS, &num_allowed_nodes, NULL);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes(): %d", ret);

	if (numa_available() < 0)
		tst_brkm(TCONF, NULL, "NUMA support is not available");

	if (num_allowed_nodes < min_nodes) {
		tst_brkm(TCONF, NULL, "at least %d allowed NUMA nodes"
			 " are required", min_nodes);
	}
#else
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
#endif
}
