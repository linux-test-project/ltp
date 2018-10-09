/*
 * memtoy:  commands.c - command line interface
 *
 * A brute force/ad hoc command interpreter:
 * + parse commands [interactive or batch]
 * + convert/validate arguments
 * + some general/administrative commands herein
 * + actual segment management routines in segment.c
 */
/*
 *  Copyright (c) 2005 Hewlett-Packard, Inc
 *  All rights reserved.
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "config.h"

#ifdef HAVE_NUMA_V2
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <ctype.h>
#include <errno.h>
#include <numa.h>
#include <numaif.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "memtoy.h"
#include "test.h"

#define CMD_SUCCESS 0
#define CMD_ERROR   1

#ifndef __NR_migrate_pages
#define __NR_migrate_pages 0
#endif

#ifndef MPOL_MF_WAIT
#define MPOL_MF_WAIT    (1<<2)	/* Wait for existing pages to migrate */
#endif

static inline int nodemask_isset(nodemask_t * mask, int node)
{
	if ((unsigned)node >= NUMA_NUM_NODES)
		return 0;
	if (mask->n[node / (8 * sizeof(unsigned long))] &
	    (1UL << (node % (8 * sizeof(unsigned long)))))
		return 1;
	return 0;
}

static inline void nodemask_set(nodemask_t * mask, int node)
{
	mask->n[node / (8 * sizeof(unsigned long))] |=
	    (1UL << (node % (8 * sizeof(unsigned long))));
}

static char *whitespace = " \t";

/*
 * =========================================================================
 */
static int help_me(char *);	/* forward reference */

/*
 * required_arg -- check for a required argument; issue message if not there
 *
 * return true if arg [something] exists; else return false
 */
static bool required_arg(char *arg, char *arg_name)
{
	glctx_t *gcp = &glctx;

	if (*arg != '\0')
		return true;

	fprintf(stderr, "%s:  command '%s' missing required argument: %s\n\n",
		gcp->program_name, gcp->cmd_name, arg_name);
	help_me(gcp->cmd_name);

	return false;
}

/*
 *  size_kmgp() -- convert ascii arg to numeric and scale as requested
 */
#define KILO_SHIFT 10
static size_t size_kmgp(char *arg)
{
	size_t argval;
	char *next;

	argval = strtoul(arg, &next, 0);
	if (*next == '\0')
		return argval;

	switch (tolower(*next)) {
	case 'p':		/* pages */
		argval *= glctx.pagesize;
		break;

	case 'k':
		argval <<= KILO_SHIFT;
		break;

	case 'm':
		argval <<= KILO_SHIFT * 2;
		break;

	case 'g':
		argval <<= KILO_SHIFT * 3;
		break;

	default:
		return BOGUS_SIZE;	/* bogus chars after number */
	}

	return argval;
}

static size_t get_scaled_value(char *args, char *what)
{
	glctx_t *gcp = &glctx;
	size_t size = size_kmgp(args);

	if (size == BOGUS_SIZE) {
		fprintf(stderr, "%s:  segment %s must be numeric value"
			" followed by optional k, m, g or p [pages] scale factor.\n",
			gcp->program_name, what);
	}

	return size;
}

static int get_range(char *args, range_t * range, char **nextarg)
{

	if (isdigit(*args)) {
		char *nextarg;

		args = strtok_r(args, whitespace, &nextarg);
		range->offset = get_scaled_value(args, "offset");
		if (range->offset == BOGUS_SIZE)
			return CMD_ERROR;
		args = nextarg + strspn(nextarg, whitespace);

		/*
		 * <length> ... only if offset specified
		 */
		if (*args != '\0') {
			args = strtok_r(args, whitespace, &nextarg);
			if (*args != '*') {
				range->length =
				    get_scaled_value(args, "length");
				if (range->length == BOGUS_SIZE)
					return CMD_ERROR;
			} else
				range->length = 0;	/* map to end of file */
			args = nextarg + strspn(nextarg, whitespace);
		}
	}

	*nextarg = args;
	return CMD_SUCCESS;
}

static int get_shared(char *args)
{
	glctx_t *gcp = &glctx;
	int segflag = MAP_PRIVATE;

	if (!strcmp(args, "shared"))
		segflag = MAP_SHARED;
	else if (*args != '\0' && strcmp(args, "private")) {
		fprintf(stderr, "%s:  anon seg access type must be one of:  "
			"'private' or 'shared'\n", gcp->program_name);
		return -1;
	}
	return segflag;
}

/*
 * get_access() - check args for 'read'\'write'
 * return:
 *	1 = read
 *	2 = write
 *	0 = neither [error]
 */
static int get_access(char *args)
{
	glctx_t *gcp = &glctx;
	int axcs = 1;
	int len = strlen(args);

	if (tolower(*args) == 'w')
		axcs = 2;
	else if (len != 0 && tolower(*args) != 'r') {
		fprintf(stderr,
			"%s:  segment access must be 'r[ead]' or 'w[rite]'\n",
			gcp->program_name);
		return 0;
	}

	return axcs;
}

static bool numa_supported(void)
{
	glctx_t *gcp = &glctx;

	if (gcp->numa_max_node <= 0) {
		fprintf(stderr, "%s:  no NUMA support on this platform\n",
			gcp->program_name);
		return false;
	}
	return true;
}

static struct policies {
	char *pol_name;
	int pol_flag;
} policies[] = {
	{
	"default", MPOL_DEFAULT}, {
	"preferred", MPOL_PREFERRED}, {
	"bind", MPOL_BIND}, {
	"interleaved", MPOL_INTERLEAVE}, {
	NULL, -1}
};

/*
 * get_mbind_policy() - parse <policy> argument to mbind command
 *
 * format:  <mpol>[+<flags>]
 * <mpol> is one of the policies[] above.
 * '+<flags>' = modifiers to mbind() call.  parsed by get_mbind_flags()
 */
static int get_mbind_policy(char *args, char **nextarg)
{
	glctx_t *gcp = &glctx;
	struct policies *polp;
	char *pol;

	pol = args;
	args += strcspn(args, " 	+");

	for (polp = policies; polp->pol_name != NULL; ++polp) {
		size_t plen = args - pol;

		if (strncmp(pol, polp->pol_name, plen))
			continue;

		*nextarg = args;
		return polp->pol_flag;
	}

	fprintf(stderr, "%s:  unrecognized policy %s\n",
		gcp->program_name, pol);
	return CMD_ERROR;
}

/*
 * get_mbind_flags() - parse mbind(2) modifier flags
 *
 * format: +move[+wait]
 * 'move' specifies that currently allocated pages should be migrated.
 *        => MPOL_MF_MOVE
 * 'wait' [only if 'move' specified] specifies that mbind(2) should not
 *        return until all pages that can be migrated have been.
 *        => MPOL_MF_WAIT
 *
 * returns flags on success; -1 on error
 */
static int get_mbind_flags(char *args, char **nextarg)
{
	glctx_t *gcp = &glctx;
	char *arg;
	int flags = 0;

	arg = args;
	args += strcspn(args, " 	+");

	if (strncmp(arg, "move", args - arg))
		goto flags_err;

	flags = MPOL_MF_MOVE;

	if (*args == '+') {
		++args;
		if (*args == '\0') {
			fprintf(stderr, "%s:  expected 'wait' after '+'\n",
				gcp->program_name);
			return -1;
		}
		arg = strtok_r(args, "  ", &args);
		if (strncmp(arg, "wait", strlen(arg)))
			goto flags_err;

		flags |= MPOL_MF_WAIT;
	}

	*nextarg = args;
	return flags;

flags_err:
	fprintf(stderr, "%s: unrecognized mbind flag: %s\n",
		gcp->program_name, arg);
	return -1;

}

/*
 * get_nodemask() -- get nodemask from comma-separated list of node ids.
 *
 * N.B., caller must free returned nodemask
 */
static nodemask_t *get_nodemask(char *args)
{
	glctx_t *gcp = &glctx;
	nodemask_t *nmp = (nodemask_t *) calloc(1, sizeof(nodemask_t));
	char *next;
	int node;
	while (*args != '\0') {
		if (!isdigit(*args)) {
			fprintf(stderr, "%s:  expected digit for <node/list>\n",
				gcp->program_name);
			goto out_err;
		}

		node = strtoul(args, &next, 10);

		if (node > gcp->numa_max_node) {
			fprintf(stderr, "%s:  node ids must be <= %d\n",
				gcp->program_name, gcp->numa_max_node);
			goto out_err;
		}

		nodemask_set(nmp, node);

		if (*next == '\0')
			return nmp;
		if (*next != ',') {
			break;
		}
		args = next + 1;
	}

out_err:
	free(nmp);
	return NULL;
}

/*
 * get_arg_nodeid_list() -- get list [array] of node ids from comma-separated list.
 *
 * on success, returns count of id's in list; on error -1
 */
static int get_arg_nodeid_list(char *args, unsigned int *list)
{
	glctx_t *gcp;
	char *next;
	nodemask_t my_allowed_nodes;
	int node, count = 0;

	gcp = &glctx;
	my_allowed_nodes = numa_get_membind_compat();
	while (*args != '\0') {
		if (!isdigit(*args)) {
			fprintf(stderr, "%s:  expected digit for <node/list>\n",
				gcp->program_name);
			return -1;
		}

		node = strtoul(args, &next, 10);

		if (node > gcp->numa_max_node) {
			fprintf(stderr, "%s:  node ids must be <= %d\n",
				gcp->program_name, gcp->numa_max_node);
			return -1;
		}

		if (!nodemask_isset(&my_allowed_nodes, node)) {
			fprintf(stderr,
				"%s:  node %d is not in my allowed node mask\n",
				gcp->program_name, node);
			return -1;
		}

		*(list + count++) = node;

		if (*next == '\0')
			return count;
		if (*next != ',') {
			break;
		}

		if (count >= gcp->numa_max_node) {
			fprintf(stderr, "%s:  too many node ids in list\n",
				gcp->program_name);
		}
		args = next + 1;
	}

	return -1;
}

/*
 * get_current_nodeid_list() - fill arg array with nodes from
 * current thread's allowed node mask.  return # of nodes in
 * mask.
 */
static int get_current_nodeid_list(unsigned int *fromids)
{
	/*
	 * FIXME (garrcoop): gcp is uninitialized and shortly hereafter used in
	 * an initialization statement..... UHHHHHHH... test writer fail?
	 */
	glctx_t *gcp;
	nodemask_t my_allowed_nodes;
	int nr_nodes = 0, max_node = gcp->numa_max_node;
	int node;

	gcp = &glctx;
	my_allowed_nodes = numa_get_membind_compat();
	for (node = 0; node <= max_node; ++node) {
		if (nodemask_isset(&my_allowed_nodes, node))
			*(fromids + nr_nodes++) = node;
	}

	/*
	 * shouldn't happen, but let 'em know if it does
	 */
	if (nr_nodes == 0)
		fprintf(stderr, "%s:  my allowed node mask is empty !!???\n",
			gcp->program_name);
	return nr_nodes;
}

/*
 * NOTE (garrcoop): Get rid of an -Wunused warning. This wasn't deleted because
 * I don't know what the original intent was for this code.
 */
#if 0
static void not_implemented()
{
	glctx_t *gcp = &glctx;

	fprintf(stderr, "%s:  %s not implemented yet\n",
		gcp->program_name, gcp->cmd_name);
}
#endif

/*
 * =========================================================================
 */
static int quit(char *args)
{
	exit(0);		/* let cleanup() do its thing */
}

static int show_pid(char *args)
{
	glctx_t *gcp = &glctx;

	printf("%s:  pid = %d\n", gcp->program_name, getpid());

	return CMD_SUCCESS;
}

static int pause_me(char *args)
{
	// glctx_t *gcp = &glctx;

	pause();
	reset_signal();

	return CMD_SUCCESS;
}

static char *numa_header = "  Node  Total Mem[MB]  Free Mem[MB]\n";
static int numa_info(char *args)
{
	glctx_t *gcp = &glctx;
	unsigned int *nodeids;
	int nr_nodes, i;
	bool do_header = true;

	if (!numa_supported())
		return CMD_ERROR;

	nodeids = calloc(gcp->numa_max_node, sizeof(*nodeids));
	nr_nodes = get_current_nodeid_list(nodeids);
	if (nr_nodes < 0)
		return CMD_ERROR;

	for (i = 0; i < nr_nodes; ++i) {
		int node = nodeids[i];
		long node_size, node_free;

		node_size = numa_node_size(node, &node_free);
		if (node_size < 0) {
			fprintf(stderr,
				"%s:  numa_node_size() failed for node %d\n",
				gcp->program_name, node);
			return CMD_ERROR;
		}

		if (do_header) {
			do_header = false;
			puts(numa_header);
		}
		printf("  %3d  %9ld      %8ld\n", node,
		       node_size / (1024 * 1024), node_free / (1024 * 1024));
	}

	return CMD_SUCCESS;
}

/*
 * migrate <to-node-id[s]> [<from-node-id[s]>]
 *
 * Node id[s] - single node id or comma-separated list
 * <to-node-id[s]> - 1-for-1 with <from-node-id[s]>, OR
 * if <from-node-id[s]> omitted, <to-node-id[s]> must be
 * a single node id.
 */
static int migrate_process(char *args)
{
	glctx_t *gcp = &glctx;
	unsigned int *fromids, *toids;
	char *idlist, *nextarg;
	struct timeval t_start, t_end;
	int nr_to, nr_from;
	int nr_migrated;
	int ret = CMD_ERROR;

	if (!numa_supported())
		return CMD_ERROR;

	toids = calloc(gcp->numa_max_node, sizeof(*toids));
	fromids = calloc(gcp->numa_max_node, sizeof(*fromids));

	/*
	 * <to-node-id[s]>
	 */
	if (!required_arg(args, "<to-node-id[s]>"))
		return CMD_ERROR;
	idlist = strtok_r(args, whitespace, &nextarg);
	nr_to = get_arg_nodeid_list(idlist, toids);
	if (nr_to <= 0)
		goto out_free;
	args = nextarg + strspn(nextarg, whitespace);

	if (*args != '\0') {
		/*
		 * apparently, <from-node-id[s]> present
		 */
		idlist = strtok_r(args, whitespace, &nextarg);
		nr_from = get_arg_nodeid_list(idlist, fromids);
		if (nr_from <= 0)
			goto out_free;
		if (nr_from != nr_to) {
			fprintf(stderr,
				"%s:  # of 'from' ids must = # of 'to' ids\n",
				gcp->program_name);
			goto out_free;
		}
	} else {
		int i;

		/*
		 * no <from-node-id[s]>, nr_to must == 1,
		 * get fromids from memory policy.
		 */
		if (nr_to > 1) {
			fprintf(stderr, "%s:  # to ids must = 1"
				" when no 'from' ids specified\n",
				gcp->program_name);
			goto out_free;
		}
		nr_from = get_current_nodeid_list(fromids);
		if (nr_from <= 0)
			goto out_free;

		/*
		 * remove 'to' node from 'from' list.  to and from
		 * lists can't intersect.
		 */
		for (i = nr_from - 1; i >= 0; --i) {
			if (*toids == *(fromids + i)) {
				while (i <= nr_from) {
					*(fromids + i) = *(fromids + i + 1);
					++i;
				}
				--nr_from;
				break;
			}
		}

		/*
		 * fill out nr_from toids with the single 'to' node
		 */
		for (; nr_to < nr_from; ++nr_to)
			*(toids + nr_to) = *toids;	/* toids[0] */
	}

	gettimeofday(&t_start, NULL);
	nr_migrated =
	    syscall(__NR_migrate_pages, getpid(), nr_from, fromids, toids);
	if (nr_migrated < 0) {
		int err = errno;
		fprintf(stderr, "%s: migrate_pages failed - %s\n",
			gcp->program_name, strerror(err));
		goto out_free;
	}
	gettimeofday(&t_end, NULL);
	printf("%s:  migrated %d pages in %6.3fsecs\n",
	       gcp->program_name, nr_migrated,
	       (float)(tv_diff_usec(&t_start, &t_end)) / 1000000.0);
	ret = CMD_SUCCESS;

out_free:
	free(toids);
	free(fromids);
	return ret;
}

static int show_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname = NULL, *nextarg;

	args += strspn(args, whitespace);
	if (*args != '\0')
		segname = strtok_r(args, whitespace, &nextarg);

	if (!segment_show(segname))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * anon_seg:  <seg-name> <size>[kmgp] [private|shared]
 */
static int anon_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };
	int segflag = 0;

	args += strspn(args, whitespace);

	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	if (!required_arg(args, "<size>"))
		return CMD_ERROR;
	args = strtok_r(args, whitespace, &nextarg);
	range.length = get_scaled_value(args, "size");
	if (range.length == BOGUS_SIZE)
		return CMD_ERROR;
	args = nextarg + strspn(nextarg, whitespace);

	if (*args != '\0') {
		segflag = get_shared(args);
		if (segflag == -1)
			return CMD_ERROR;
	}

	if (!segment_register(SEGT_ANON, segname, &range, segflag))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * file_seg:  <path-name> [<offset>[kmgp] <length>[kmgp]  [private|shared]]
 */
static int file_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *pathname, *nextarg;
	range_t range = { 0L, 0L };
	int segflag = MAP_PRIVATE;

	args += strspn(args, whitespace);

	if (!required_arg(args, "<path-name>"))
		return CMD_ERROR;
	pathname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	/*
	 * offset, length are optional
	 */
	if (get_range(args, &range, &nextarg) == CMD_ERROR)
		return CMD_ERROR;
	args = nextarg;

	if (*args != '\0') {
		segflag = get_shared(args);
		if (segflag == -1)
			return CMD_ERROR;
	}

	if (!segment_register(SEGT_FILE, pathname, &range, segflag))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * remove_seg:  <seg-name> [<seg-name> ...]
 */
static int remove_seg(char *args)
{
	glctx_t *gcp = &glctx;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;

	while (*args != '\0') {
		char *segname, *nextarg;

		segname = strtok_r(args, whitespace, &nextarg);
		args = nextarg + strspn(nextarg, whitespace);

		segment_remove(segname);
	}
	return 0;
}

/*
 * touch_seg:  <seg-name> [<offset> <length>] [read|write]
 */
static int touch_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };
	int axcs;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	/*
	 * offset, length are optional
	 */
	if (get_range(args, &range, &nextarg) == CMD_ERROR)
		return CMD_ERROR;
	args = nextarg;

	axcs = get_access(args);
	if (axcs == 0)
		return CMD_ERROR;

	if (!segment_touch(segname, &range, axcs - 1))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * unmap <seg-name> - unmap specified segment, but remember name/size/...
 */
static int unmap_seg(char *args)
{
	glctx_t *gcp = &glctx;
	char *segname, *nextarg;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	if (!segment_unmap(segname))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * map <seg-name> [<offset>[k|m|g|p] <length>[k|m|g|p]] [<seg-share>]
 */
static int map_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };
	range_t *rangep = NULL;
	int segflag = MAP_PRIVATE;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	/*
	 * offset, length are optional
	 */
	if (get_range(args, &range, &nextarg) == CMD_ERROR)
		return CMD_ERROR;
	if (args != nextarg) {
		rangep = &range;	/* override any registered range */
		args = nextarg;
	}

	if (*args != '\0') {
		segflag = get_shared(args);
		if (segflag == -1)
			return CMD_ERROR;
	}

	if (!segment_map(segname, rangep, segflag))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * mbind <seg-name> [<offset>[kmgp] <length>[kmgp]] <policy> <node-list>
 */
static int mbind_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };
	nodemask_t *nodemask = NULL;
	int policy, flags = 0;
	int ret;

	if (!numa_supported())
		return CMD_ERROR;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	/*
	 * offset, length are optional
	 */
	if (get_range(args, &range, &nextarg) == CMD_ERROR)
		return CMD_ERROR;
	args = nextarg;

	if (!required_arg(args, "<policy>"))
		return CMD_ERROR;
	policy = get_mbind_policy(args, &nextarg);
	if (policy < 0)
		return CMD_ERROR;

	args = nextarg + strspn(nextarg, whitespace);
	if (*args == '+') {
		flags = get_mbind_flags(++args, &nextarg);
		if (flags == -1)
			return CMD_ERROR;
	}
	args = nextarg + strspn(nextarg, whitespace);

	if (policy != MPOL_DEFAULT) {
		if (!required_arg(args, "<node/list>"))
			return CMD_ERROR;
		nodemask = get_nodemask(args);
		if (nodemask == NULL)
			return CMD_ERROR;
	}

	ret = CMD_SUCCESS;
#if 1				// for testing
	if (!segment_mbind(segname, &range, policy, nodemask, flags))
		ret = CMD_ERROR;
#endif

	if (nodemask != NULL)
		free(nodemask);
	return ret;
}

/*
 *  shmem_seg - create [shmget] and register a SysV shared memory segment
 *              of specified size
 */
static int shmem_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };

	args += strspn(args, whitespace);

	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	if (!required_arg(args, "<size>"))
		return CMD_ERROR;
	args = strtok_r(args, whitespace, &nextarg);
	range.length = get_scaled_value(args, "size");
	if (range.length == BOGUS_SIZE)
		return CMD_ERROR;
	args = nextarg + strspn(nextarg, whitespace);

	if (!segment_register(SEGT_SHM, segname, &range, MAP_SHARED))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

/*
 * where <seg-name> [<offset>[kmgp] <length>[kmgp]]  - show node location
 * of specified range of segment.
 *
 * NOTE: if neither <offset> nor <length> specified, <offset> defaults
 * to 0 [start of segment], as usual, and length defaults to 64 pages
 * rather than the entire segment.  Suitable for a "quick look" at where
 * segment resides.
 */
static int where_seg(char *args)
{
	glctx_t *gcp = &glctx;

	char *segname, *nextarg;
	range_t range = { 0L, 0L };
	int ret;

	if (!numa_supported())
		return CMD_ERROR;

	args += strspn(args, whitespace);
	if (!required_arg(args, "<seg-name>"))
		return CMD_ERROR;
	segname = strtok_r(args, whitespace, &nextarg);
	args = nextarg + strspn(nextarg, whitespace);

	/*
	 * offset, length are optional
	 */
	if (get_range(args, &range, &nextarg) == CMD_ERROR)
		return CMD_ERROR;
	if (args == nextarg)
		range.length = 64 * gcp->pagesize;	/* default length */

	if (!segment_location(segname, &range))
		return CMD_ERROR;

	return CMD_SUCCESS;
}

#if 0
static int command(char *args)
{
	glctx_t *gcp = &glctx;

	return CMD_SUCCESS;
}

#endif
/*
 * =========================================================================
 */
typedef int (*cmd_func_t) (char *);

struct command {
	char *cmd_name;
	cmd_func_t cmd_func;	/* */
	char *cmd_help;

} cmd_table[] = {
	{
	.cmd_name = "quit",.cmd_func = quit,.cmd_help =
		    "quit           - just what you think\n"
		    "\tEOF on stdin has the same effect\n"}, {
	.cmd_name = "help",.cmd_func = help_me,.cmd_help =
		    "help           - show this help\n"
		    "help <command> - display help for just <command>\n"}, {
	.cmd_name = "pid",.cmd_func = show_pid,.cmd_help =
		    "pid            - show process id of this session\n"}, {
	.cmd_name = "pause",.cmd_func = pause_me,.cmd_help =
		    "pause          - pause program until signal"
		    " -- e.g., INT, USR1\n"}, {
	.cmd_name = "numa",.cmd_func = numa_info,.cmd_help =
		    "numa          - display numa info as seen by this program.\n"
		    "\tshows nodes from which program may allocate memory\n"
		    "\twith total and free memory.\n"}, {
	.cmd_name = "migrate",.cmd_func = migrate_process,.cmd_help =
		    "migrate <to-node-id[s]> [<from-node-id[s]>] - \n"
		    "\tmigrate this process' memory from <from-node-id[s]>\n"
		    "\tto <to-node-id[s]>.  Specify multiple node ids as a\n"
		    "\tcomma-separated list. TODO - more info\n"}, {
	.cmd_name = "show",.cmd_func = show_seg,.cmd_help =
		    "show [<name>]  - show info for segment[s]; default all\n"},
	{
	.cmd_name = "anon",.cmd_func = anon_seg,.cmd_help =
		    "anon <seg-name> <seg-size>[k|m|g|p] [<seg-share>] -\n"
		    "\tdefine a MAP_ANONYMOUS segment of specified size\n"
		    "\t<seg-share> := private|shared - default = private\n"}, {
	.cmd_name = "file",.cmd_func = file_seg,.cmd_help =
		    "file <pathname> [<offset>[k|m|g|p] <length>[k|m|g|p]] [<seg-share>] -\n"
		    "\tdefine a mapped file segment of specified length starting at the\n"
		    "\tspecified offset into the file.  <offset> and <length> may be\n"
		    "\tomitted and specified on the map command.\n"
		    "\t<seg-share> := private|shared - default = private\n"}, {
	.cmd_name = "shm",.cmd_func = shmem_seg,.cmd_help =
		    "shm <seg-name> <seg-size>[k|m|g|p] - \n"
		    "\tdefine a shared memory segment of specified size.\n"
		    "\tYou may need to increase limits [/proc/sys/kernel/shmmax].\n"
		    "\tUse map/unmap to attach/detach\n"}, {
	.cmd_name = "remove",.cmd_func = remove_seg,.cmd_help =
		    "remove <seg-name> [<seg-name> ...] - remove the named segment[s]\n"},
	{
	.cmd_name = "map",.cmd_func = map_seg,.cmd_help =
		    "map <seg-name> [<offset>[k|m|g|p] <length>[k|m|g|p]] [<seg-share>] - \n"
		    "\tmmap()/shmat() a previously defined, currently unmapped() segment.\n"
		    "\t<offset> and <length> apply only to mapped files.\n"
		    "\tUse <length> of '*' or '0' to map to the end of the file.\n"},
	{
	.cmd_name = "unmap",.cmd_func = unmap_seg,.cmd_help =
		    "unmap <seg-name> - unmap specified segment, but remember name/size/...\n"},
	{
	.cmd_name = "touch",.cmd_func = touch_seg,.cmd_help =
		    "touch <seg-name> [<offset>[k|m|g|p] <length>[k|m|g|p]] [read|write] - \n"
		    "\tread [default] or write the named segment from <offset> through\n"
		    "\t<offset>+<length>.  If <offset> and <length> omitted, touches all\n"
		    "\t of mapped segment.\n"}, {
	.cmd_name = "mbind",.cmd_func = mbind_seg,.cmd_help =
		    "mbind <seg-name> [<offset>[k|m|g|p] <length>[k|m|g|p]]\n"
		    "      <policy>[+move[+wait]] [<node/list>] - \n"
		    "\tset the numa policy for the specified range of the name segment\n"
		    "\tto policy --  one of {default, bind, preferred, interleaved}.\n"
		    "\t<node/list> specifies a node id or a comma separated list of\n"
		    "\tnode ids.  <node> is ignored for 'default' policy, and only\n"
		    "\tthe first node is used for 'preferred' policy.\n"
		    "\t'+move' specifies that currently allocated pages be prepared\n"
		    "\t        for migration on next touch\n"
		    "\t'+wait' [valid only with +move] specifies that pages mbind()\n"
		    "          touch the pages and wait for migration before returning.\n"},
	{
	.cmd_name = "where",.cmd_func = where_seg,.cmd_help =
		    "where <seg-name> [<offset>[k|m|g|p] <length>[k|m|g|p]] - \n"
		    "\tshow the node location of pages in the specified range\n"
		    "\tof the specified segment.  <offset> defaults to start of\n"
		    "\tsegment; <length> defaults to 64 pages.\n"},
#if 0				/* template for new commands */
	{
	.cmd_name = "",.cmd_func =,.cmd_help =},
#endif
	{
	.cmd_name = NULL}
};

static int help_me(char *args)
{
	struct command *cmdp = cmd_table;
	char *cmd, *nextarg;
	int cmdlen;
	bool match = false;

	args += strspn(args, whitespace);
	if (*args != '\0') {
		cmd = strtok_r(args, whitespace, &nextarg);
		cmdlen = strlen(cmd);
	} else {
		cmd = NULL;
		cmdlen = 0;
	}

	for (cmdp = cmd_table; cmdp->cmd_name != NULL; ++cmdp) {
		if (cmd == NULL || !strncmp(cmd, cmdp->cmd_name, cmdlen)) {
			printf("%s\n", cmdp->cmd_help);
			match = true;
		}
	}

	if (!match) {
		printf("unrecognized command:  %s\n", cmd);
		printf("\tuse 'help' for a complete list of commands\n");
		return CMD_ERROR;
	}

	return CMD_SUCCESS;
}

/*
 * =========================================================================
 */
#define CMDBUFSZ 256

static bool unique_abbrev(char *cmd, size_t clen, struct command *cmdp)
{
	for (; cmdp->cmd_name != NULL; ++cmdp) {
		if (!strncmp(cmd, cmdp->cmd_name, clen))
			return false;	/* match: not unique */
	}
	return true;
}

static int parse_command(char *cmdline)
{
	glctx_t *gcp = &glctx;
	char *cmd, *args;
	struct command *cmdp;

	cmdline += strspn(cmdline, whitespace);	/* possibly redundant */

	cmd = strtok_r(cmdline, whitespace, &args);

	for (cmdp = cmd_table; cmdp->cmd_name != NULL; ++cmdp) {
		size_t clen = strlen(cmd);
		int ret;

		if (strncmp(cmd, cmdp->cmd_name, clen))
			continue;
		if (!unique_abbrev(cmd, clen, cmdp + 1)) {
			fprintf(stderr, "%s:  ambiguous command:  %s\n",
				gcp->program_name, cmd);
			return CMD_ERROR;
		}
		gcp->cmd_name = cmdp->cmd_name;
		ret = cmdp->cmd_func(args);
		gcp->cmd_name = NULL;
		return ret;
	}

	fprintf(stderr, "%s:  unrecognized command %s\n", __FUNCTION__, cmd);
	return CMD_ERROR;
}

void process_commands()
{
	glctx_t *gcp = &glctx;

	char cmdbuf[CMDBUFSZ];

	do {
		char *cmdline;
		size_t cmdlen;

		if (is_option(INTERACTIVE))
			printf("%s>", gcp->program_name);

		cmdline = fgets(cmdbuf, CMDBUFSZ, stdin);
		if (cmdline == NULL) {
			printf("%s\n",
			       is_option(INTERACTIVE) ? "" : "EOF on stdin");
			exit(0);	/* EOF */
		}
		if (cmdline[0] == '\n')
			continue;

		/*
		 * trim trailing newline, if any
		 */
		cmdlen = strlen(cmdline);
		if (cmdline[cmdlen - 1] == '\n')
			cmdline[--cmdlen] = '\0';

		cmdline += strspn(cmdline, whitespace);
		cmdlen -= (cmdline - cmdbuf);

		if (cmdlen == 0) {
			//TODO:  interactive help?
			continue;	/* ignore blank lines */
		}

		if (*cmdline == '#')
			continue;	/* comments */

		/*
		 * trim trailing whitespace for ease of parsing
		 */
		while (strchr(whitespace, cmdline[cmdlen - 1]))
			cmdline[--cmdlen] = '\0';

		if (cmdlen == 0)
			continue;

		/*
		 * reset signals just before parsing a command.
		 * non-interactive:  exit on SIGQUIT
		 */
		if (signalled(gcp)) {
			if (!is_option(INTERACTIVE) &&
			    gcp->siginfo->si_signo == SIGQUIT)
				exit(0);
			reset_signal();
		}

		/*
		 * non-interactive:  errors are fatal
		 */
		if (!is_option(INTERACTIVE)) {
			vprint("%s>%s\n", gcp->program_name, cmdline);
			if (parse_command(cmdline) == CMD_ERROR) {
				fprintf(stderr, "%s:  command error\n",
					gcp->program_name);
				exit(4);
			}
		} else
			parse_command(cmdline);

	} while (1);
}
#endif
