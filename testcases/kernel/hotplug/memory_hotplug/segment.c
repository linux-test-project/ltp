/*
 * memtoy:  segment.c - manage memory segments
 *
 * create/destroy/map/unmap - anonymous, file and SysV shmem segments
 * touch [read or write] - ranges of segments
 * mbind - ranges of segments
 * show mappings or locations of segment pages
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
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <numa.h>
#include <numaif.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "memtoy.h"
#include "segment.h"

struct segment {
	char *seg_name;
	void *seg_start;
	size_t seg_length;

	off_t seg_offset;	/* memory mapped files */
	char *seg_path;		/*   "      "      "   */

	seg_type_t seg_type;
	int seg_slot;
	int seg_flags;		/* shared|private */
	int seg_prot;
	int seg_fd;		/* saved file descriptor */
	int seg_shmid;

};

#define MAX_SEGMENTS 63		/* arbitrary max */
#define SEG_FD_NONE (-1)
#define SHM_ID_NONE (-1)

#define SEG_ERR (0)
#define SEG_OK  (1)

#define SEG_OFFSET(SEGP, ADDR) ((char *)(ADDR) - (char *)(SEGP->seg_start))

/*
 * =========================================================================
 */
void segment_init(struct global_context *gcp)
{
	/*
	 * one extra slot to terminate the list
	 */
	gcp->seglist = calloc(MAX_SEGMENTS + 1, sizeof(segment_t *));
	if (!gcp->seglist)
		die(4, "%s: can't alloc segment table\n", gcp->program_name);
	gcp->seg_avail = NULL;

}

static segment_t *new_segment(void)
{
	glctx_t *gcp = &glctx;
	segment_t *segp = (segment_t *) calloc(1, sizeof(segment_t));

	if (segp == NULL)
		fprintf(stderr, "%s:  failed to allocate segment\n",
			gcp->program_name);
	return segp;
}

/*
 * get_seg_slot() -- allocate a segment table slot for a new segment
 */
static segment_t *get_seg_slot(void)
{
	glctx_t *gcp = &glctx;
	segment_t *segp, **segpp;

	/*
	 * consume saved slot, if any
	 */
	segp = gcp->seg_avail;
	if (segp != NULL) {
		gcp->seg_avail = NULL;
		return segp;
	}

	/*
	 * simple linear scan for first available slot
	 */
	for (segpp = gcp->seglist; (segp = *segpp); ++segpp) {
		if (segp->seg_type == SEGT_NONE)
			return segp;
	}

	if (segpp < &gcp->seglist[MAX_SEGMENTS]) {
		/*
		 * previously unused slot
		 */
		*segpp = segp = new_segment();
		segp->seg_slot = segpp - gcp->seglist;
		return segp;
	}

	fprintf(stderr, "%s:  segment table full\n", gcp->program_name);
	return NULL;
}

static void unmap_segment(segment_t * segp)
{

	if (segp->seg_start == MAP_FAILED)
		return;		/* already unmapped */

	switch (segp->seg_type) {
	case SEGT_ANON:
	case SEGT_FILE:
		munmap(segp->seg_start, segp->seg_length);
		break;

	case SEGT_SHM:
		shmdt(segp->seg_start);
		break;

	default:
		// shouldn't happen?
		break;
	}

	segp->seg_start = MAP_FAILED;
}

/*
 * free up a segment table slot, freeing any string storage
 * and removing shm segment, if necessary
 * clear out the segment, but preserve slot #
 */
static void free_seg_slot(segment_t * segp)
{
	glctx_t *gcp = &glctx;
	int slot = segp->seg_slot;

	if (segp->seg_name != NULL)
		free(segp->seg_name);

	if (segp->seg_path != NULL)
		free(segp->seg_path);

	if (segp->seg_type == SEGT_FILE && segp->seg_fd != SEG_FD_NONE)
		close(segp->seg_fd);

	if (segp->seg_type == SEGT_SHM && segp->seg_shmid != SHM_ID_NONE)
		shmctl(segp->seg_shmid, IPC_RMID, NULL);

	(void)memset(segp, 0, sizeof(*segp));

	segp->seg_slot = slot;
	if (gcp->seg_avail == NULL)
		gcp->seg_avail = segp;

}

/*
 * called from memtoy "at exit" cleanup().
 * primarily to remove any shm segments created.
 */
void segment_cleanup(struct global_context *gcp)
{
	segment_t *segp, **segpp;

	segpp = gcp->seglist;
	if (segpp == NULL)
		return;

	for (; (segp = *segpp); ++segpp) {
		if (segp->seg_type != SEGT_SHM) {
			continue;
		}
		free_seg_slot(segp);	/* to remove shared mem */
	}
}

static size_t round_up_to_pagesize(size_t size)
{
	glctx_t *gcp = &glctx;
	size_t pagemask = gcp->pagesize - 1;

	return ((size + pagemask) & ~pagemask);

}

static size_t round_down_to_pagesize(size_t size)
{
	glctx_t *gcp = &glctx;
	size_t pagemask = gcp->pagesize - 1;

	return (size & ~pagemask);

}

/*
 * get_node() -- fetch numa node id of page at vaddr
 * [from Ray Bryant's [SGI] memory migration tests]
 */
static int get_node(void *vaddr)
{
	int rc, node;

	rc = get_mempolicy(&node, NULL, 0, vaddr, MPOL_F_NODE | MPOL_F_ADDR);
	if (rc)
		return -1;

	return node;
}

/*
 * =========================================================================
 */
static int map_anon_segment(segment_t * segp)
{
	glctx_t *gcp = &glctx;

	char *memp;
	int flags = segp->seg_flags;

	if (!flags)
		flags = MAP_PRIVATE;	/* default */

	memp = (char *)mmap(0, segp->seg_length, segp->seg_prot, flags | MAP_ANONYMOUS, 0,	/* fd -- ignored */
			    0);	/* offset -- ignored */

	if (memp == MAP_FAILED) {
		int err = errno;
		fprintf(stderr, "%s:  anonymous mmap failed - %s\n",
			__FUNCTION__, strerror(err));
		return SEG_ERR;
	}

	vprint("%s:  mmap()ed anon seg %s at 0x%lx-0x%lx\n",
	       gcp->program_name, segp->seg_name,
	       memp, memp + segp->seg_length - 1);

	segp->seg_start = memp;

	return SEG_OK;
}

/*
 * open_file() -- open and validate file when registering a file segment.
 * remember fd in segment struct.
 */
static int open_file(segment_t * segp)
{
	glctx_t *gcp = &glctx;

	struct stat stbuf;
	int fd, flags;

	if (stat(segp->seg_path, &stbuf) < 0) {
		int err = errno;
		fprintf(stderr, "%s:  can't stat %s - %s\n",
			gcp->program_name, segp->seg_path, strerror(err));
		free_seg_slot(segp);
		return SEG_ERR;
	}

	/*
	 * TODO:  for now, just regular files.  later?
	 */
	if (!S_ISREG(stbuf.st_mode)) {
		fprintf(stderr, "%s:  %s - is not a regular file\n",
			gcp->program_name, segp->seg_path);
		free_seg_slot(segp);
		return SEG_ERR;
	}

	/*
	 * Open file with maximal privileges;  adjust segment mapping
	 * protections if permissions don't allow full R/W access.
	 */
	if (!access(segp->seg_path, R_OK | W_OK))
		flags = O_RDWR;
	else if (!access(segp->seg_path, R_OK)) {
		flags = O_RDONLY;
		segp->seg_prot &= ~PROT_WRITE;
	} else if (!access(segp->seg_path, W_OK)) {
		flags = O_WRONLY;
		segp->seg_prot &= ~PROT_READ;
	} else {
		fprintf(stderr, "%s:  can't access %s\n",
			gcp->program_name, segp->seg_path);
		free_seg_slot(segp);
		return SEG_ERR;
	}

	fd = open(segp->seg_path, flags);
	if (fd < 0) {
		int err = errno;
		fprintf(stderr, "%s:  can't open %s - %s\n",
			gcp->program_name, segp->seg_path, strerror(err));
		free_seg_slot(segp);
		return SEG_ERR;
	}

	segp->seg_fd = fd;
	return SEG_OK;
}

/*
 * re-fetch file size at map time -- just in case it's changed
 */
static size_t file_size(int fd)
{
	struct stat stbuf;

	if (fstat(fd, &stbuf) != 0) {
		return BOGUS_SIZE;
	}

	return stbuf.st_size;
}

/*
 * map_file_segment() -- map a [range of a] registered file segment.
 */
static int map_file_segment(segment_t * segp)
{
	glctx_t *gcp = &glctx;

	char *memp;
	size_t size;
	int fd;
	int flags = segp->seg_flags;

	if (!flags)
		flags = MAP_PRIVATE;	/* default */

	if ((fd = segp->seg_fd) == SEG_FD_NONE) {
		fprintf(stderr, "%s:  file %s not open\n",
			gcp->program_name, segp->seg_path);
		return SEG_ERR;
	}

	size = file_size(fd);

	/*
	 * page align offset/length;  verify fit in file
	 */
	segp->seg_offset = round_down_to_pagesize(segp->seg_offset);
	if (segp->seg_offset > size) {
		fprintf(stderr, "%s: offset 0x%lx beyond end of file %s\n",
			gcp->program_name, segp->seg_offset, segp->seg_path);
		return SEG_ERR;
	}

	if (segp->seg_length == 0)
		segp->seg_length = round_up_to_pagesize(size) -
		    segp->seg_offset;
	else
		segp->seg_length = round_up_to_pagesize(segp->seg_length);

	memp = (char *)mmap(0, segp->seg_length,
			    segp->seg_prot, flags, fd, segp->seg_offset);

	if (memp == MAP_FAILED) {
		int err = errno;
		fprintf(stderr, "%s:  mmap of %s failed - %s\n",
			__FUNCTION__, segp->seg_path, strerror(err));
		return SEG_ERR;
	}

	vprint("%s:  mmap()ed file seg %s at 0x%lx-0x%lx\n",
	       gcp->program_name, segp->seg_name,
	       memp, memp + segp->seg_length - 1);

	segp->seg_start = memp;

	return SEG_OK;
}

/*
 * get_shm_segment() -- create [shmget] a new shared memory segment
 */
static int get_shm_segment(segment_t * segp)
{
	glctx_t *gcp = &glctx;

	int shmid;

	shmid = shmget(IPC_PRIVATE, segp->seg_length, SHM_R | SHM_W);
	if (shmid == -1) {
		int err = errno;
		fprintf(stderr, "%s:  failed to get shm segment %s - %s\n",
			gcp->program_name, segp->seg_name, strerror(err));
		free_seg_slot(segp);
		return SEG_ERR;
	}

	segp->seg_shmid = shmid;
	vprint("%s:  shm seg %s id:  %d\n",
	       gcp->program_name, segp->seg_name, segp->seg_shmid);
	return SEG_OK;
}

/*
 * map_shm_segment() -- attach [shmat] a shared memory segment
 */
static int map_shm_segment(segment_t * segp)
{
	glctx_t *gcp = &glctx;

	segp->seg_start = shmat(segp->seg_shmid, NULL, 0);
	if (segp->seg_start == MAP_FAILED) {
		int err = errno;
		fprintf(stderr, "%s:  failed to attach shm segment %s: %s\n",
			gcp->program_name, segp->seg_name, strerror(err));
		return SEG_ERR;
	}

	vprint("%s:  mmap()ed shm seg %s at 0x%lx-0x%lx\n",
	       gcp->program_name, segp->seg_name,
	       segp->seg_start, segp->seg_start + segp->seg_length - 1);

	return SEG_OK;
}

/*
 * =========================================================================
 * segment API
 */
/*
 * segment_get(name) - lookup named segment
TODO:  move to segment private functions?
 */
segment_t *segment_get(char *name)
{
	glctx_t *gcp = &glctx;
	segment_t *segp, **segpp;

	for (segpp = gcp->seglist; (segp = *segpp); ++segpp) {
		if (segp->seg_type == SEGT_NONE) {
			if (gcp->seg_avail == NULL)
				gcp->seg_avail = *segpp;
			continue;
		}
		if (!strcmp(name, segp->seg_name))
			return segp;
	}

	if (gcp->seg_avail == NULL && segpp < &gcp->seglist[MAX_SEGMENTS]) {
		/*
		 * prealloc an available segment
		 */
		*segpp = segp = new_segment();
		if (segp != NULL) {
			segp->seg_slot = segpp - gcp->seglist;
			gcp->seg_avail = segp;
		}
	}

	return NULL;
}

/*
 * segment_register:  register an anon, file or shm segment based on args.
 *	for anon and shm, 'name' = segment name.
 *	for file, 'name' = path name; segment name = basename(path)
 *
 * returns: !0 on success; 0 on failure
 */
int segment_register(seg_type_t type, char *name, range_t * range, int flags)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;
	char *path;

	segp = segment_get(basename(name));	/* ensure unique name */
	if (segp != NULL) {
		fprintf(stderr, "%s:  segment %s already exists\n",
			gcp->program_name, segp->seg_name);
		return SEG_ERR;
	}

	segp = get_seg_slot();
	if (segp == NULL)
		return SEG_ERR;

	path = strdup(name);	/* save a copy */
	segp->seg_name = strdup(basename(name));
	segp->seg_start = MAP_FAILED;
	segp->seg_length = round_up_to_pagesize(range->length);
	segp->seg_offset = round_down_to_pagesize(range->offset);
	segp->seg_type = type;
	segp->seg_flags = flags;	/* possibly 0 */
	segp->seg_prot = PROT_READ | PROT_WRITE;	/* default */
	segp->seg_fd = SEG_FD_NONE;
	segp->seg_shmid = SHM_ID_NONE;

	switch (type) {
	case SEGT_ANON:
		free(path);
		break;

	case SEGT_FILE:
		segp->seg_path = path;
		return open_file(segp);
		break;

	case SEGT_SHM:
		free(path);
		return get_shm_segment(segp);
		break;

	default:
		free(path);
	}
	return SEG_OK;
}

static char *segment_header =
    "  _____address______ ____length____ ____offset____ prot  share  name\n";

static char seg_type[] = { '.', 'a', 'f', 's' };

static int show_one_segment(segment_t * segp, bool header)
{
	char *protection, *share, *name;

	switch (segp->seg_prot & (PROT_READ | PROT_WRITE)) {
	case PROT_READ | PROT_WRITE:
		protection = "rw";
		break;

	case PROT_READ:
		protection = "r-";
		break;

	case PROT_WRITE:
		protection = "-w";
		break;

	default:
		protection = "--";
		break;
	}

	if (segp->seg_flags)
		share = (segp->seg_flags & MAP_SHARED) ? "shared " : "private";
	else
		share = "default";

	name = (segp->seg_type == SEGT_FILE) ? segp->seg_path : segp->seg_name;

	if (header)
		puts(segment_header);

	if (segp->seg_start != MAP_FAILED) {
		printf("%c 0x%p 0x%012zx 0x%012lx  %s  %s %s\n",
		       seg_type[segp->seg_type],
		       segp->seg_start,
		       segp->seg_length,
		       segp->seg_offset, protection, share, name);
	} else {
		printf("%c *** not-mapped *** 0x%012zx 0x%012lx  %s  %s %s\n",
		       seg_type[segp->seg_type],
		       segp->seg_length,
		       segp->seg_offset, protection, share, name);
	}

	return SEG_OK;
}

/*
 * segment_show() -- show specified segment, or all, if none specified.
 */
int segment_show(char *name)
{
	glctx_t *gcp = &glctx;
	segment_t *segp, **segpp;
	bool header;

	if (name != NULL) {
		segp = segment_get(name);
		if (segp == NULL) {
			fprintf(stderr, "%s:  no such segment:  %s\n",
				gcp->program_name, name);
			return SEG_ERR;
		}
		show_one_segment(segp, false);
		return SEG_OK;
	}

	/*
	 * show all
	 */
	header = true;
	for (segpp = gcp->seglist; (segp = *segpp); ++segpp) {
		if (segp->seg_type != SEGT_NONE) {
			show_one_segment(segp, header);
			header = false;	/* first time only */
		}
	}

	return SEG_OK;

}

/*
 * segment_remove() - remove the specified segment, if exists.
 */
int segment_remove(char *name)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	unmap_segment(segp);

	free_seg_slot(segp);

	return SEG_OK;
}

/*
 * segment_touch() - "touch" [read or write] each page of specified range
 *                   -- from offset to offset+length -- to fault in or to
 *                   test protection.
 * NOTE:  offset is relative to start of mapping, not start of file!
 */
int segment_touch(char *name, range_t * range, int rw)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;
	off_t offset;
	size_t length, maxlength;
	unsigned long *memp;
	struct timeval t_start, t_end;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	offset = round_down_to_pagesize(range->offset);
	if (offset >= segp->seg_length) {
		fprintf(stderr, "%s:  offset %ld is past end of segment %s\n",
			gcp->program_name, offset, name);
		return SEG_ERR;
	}

	memp = (unsigned long *)(segp->seg_start + offset);
	maxlength = segp->seg_length - offset;

	length = range->length;
	if (length)
		length = round_up_to_pagesize(length);

	/*
	 * note:  we silently truncate to max length [end of segment]
	 */
	if (length == 0 || length > maxlength)
		length = maxlength;

	gettimeofday(&t_start, NULL);
	touch_memory(rw, memp, length);
	gettimeofday(&t_end, NULL);
	printf("%s:  touched %d pages in %6.3f secs\n",
	       gcp->program_name, length / gcp->pagesize,
	       (float)(tv_diff_usec(&t_start, &t_end)) / 1000000.0);

	return SEG_OK;
}

/*
 * segment_unmap() -  unmap the specified segment, if any, from seg_start
 *                    to seg_start+seg_lenth.  Leave the segment in the
 *                    table;
 */
int segment_unmap(char *name)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	if (segp->seg_start == MAP_FAILED)
		return SEG_OK;	/* silent success */

	switch (segp->seg_type) {
	case SEGT_ANON:
	case SEGT_FILE:
		munmap(segp->seg_start, segp->seg_length);
		break;

	case SEGT_SHM:
		//TODO:  shmdt()...
		break;
		/* Handle default to get rid of -Wswitch-enum */
	default:
		break;
	}

	segp->seg_start = MAP_FAILED;

	return SEG_OK;
}

/*
 * segment_map() -- [re] map() a previously unmapped segment
 *                  no-op if already mapped.
 *                  range only applies to mapped file.
 */
int segment_map(char *name, range_t * range, int flags)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	if (segp->seg_start != MAP_FAILED) {
		fprintf(stderr, "%s:  segment %s already mapped\n",
			gcp->program_name, name);
		return SEG_OK;	/* treat as success */
	}

	if (flags != 0)
		segp->seg_flags = flags;

	switch (segp->seg_type) {
	case SEGT_ANON:
		return map_anon_segment(segp);
		break;

	case SEGT_FILE:
		if (range != NULL) {
			segp->seg_offset = range->offset;
			segp->seg_length = range->length;
		}
		return map_file_segment(segp);
		break;

	case SEGT_SHM:
		return map_shm_segment(segp);
		break;
		/* Handle default to get rid of -Wswitch-enum */
	default:
		break;
	}

	return SEG_ERR;		/* unrecognized segment type -- shouldn't happen */

}

/*
 * segment_mbind() - set memory policy for a range of specified segment
 *
 * NOTE:  offset is relative to start of mapping, not start of file
 */
int
segment_mbind(char *name, range_t * range, int policy,
	      nodemask_t * nodemask, int flags)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;
	char *start;
	off_t offset;
	size_t length, maxlength;
	int ret;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	if (segp->seg_start == MAP_FAILED) {
		fprintf(stderr, "%s:  segment %s not mapped\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	offset = round_down_to_pagesize(range->offset);
	if (offset >= segp->seg_length) {
		fprintf(stderr, "%s:  offset %ld is past end of segment %s\n",
			gcp->program_name, offset, name);
		return SEG_ERR;
	}

	start = segp->seg_start + offset;
	maxlength = segp->seg_length - offset;

	length = range->length;
	if (length)
		length = round_up_to_pagesize(length);

	/*
	 * note:  we silently truncate to max length [end of segment]
	 */
	if (length == 0 || length > maxlength)
		length = maxlength;

	ret = mbind(segp->seg_start + offset, length, policy, nodemask->n,
		    NUMA_NUM_NODES, flags);

	if (ret == -1) {
		int err = errno;
		fprintf(stderr, "%s:  mbind() of segment %s failed - %s\n",
			gcp->program_name, name, strerror(err));
		return SEG_ERR;
	}

	return SEG_OK;
}

/*
 * segment_location() - report node location of specified range of segment
 *
 * NOTE:  offset is relative to start of mapping, not start of file
 */
#define PG_PER_LINE 8
#define PPL_MASK (PG_PER_LINE - 1)
int segment_location(char *name, range_t * range)
{
	glctx_t *gcp = &glctx;
	segment_t *segp;
	char *apage, *end;
	off_t offset;
	size_t length, maxlength;
	int pgid, i;
	bool need_nl;

	segp = segment_get(name);
	if (segp == NULL) {
		fprintf(stderr, "%s:  no such segment:  %s\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	if (segp->seg_start == MAP_FAILED) {
		fprintf(stderr, "%s:  segment %s not mapped\n",
			gcp->program_name, name);
		return SEG_ERR;
	}

	offset = round_down_to_pagesize(range->offset);
	if (offset >= segp->seg_length) {
		fprintf(stderr, "%s:  offset %ld is past end of segment %s\n",
			gcp->program_name, offset, name);
		return SEG_ERR;
	}

	apage = segp->seg_start + offset;
	maxlength = segp->seg_length - offset;

	length = range->length;
	if (length)
		length = round_up_to_pagesize(length);

	/*
	 * note:  we silently truncate to max length [end of segment]
	 */
	if (length == 0 || length > maxlength)
		length = maxlength;

	end = apage + length;
	pgid = offset / gcp->pagesize;

	show_one_segment(segp, false);	/* show mapping, no header */

	printf("page offset   ");
	for (i = 0; i < PG_PER_LINE; ++i)
		printf(" +%02d", i);
	printf("\n");
	if (pgid & PPL_MASK) {
		/*
		 * start partial line
		 */
		int pgid2 = pgid & ~PPL_MASK;
		printf("%12x: ", pgid2);
		while (pgid2 < pgid) {
			printf("    ");
			++pgid2;
		}
		need_nl = true;
	} else
		need_nl = false;

	for (; apage < end; apage += gcp->pagesize, ++pgid) {
		int node;

		node = get_node(apage);
		if (node < 0) {
			fprintf(stderr, "\n%s:  "
				"failed to get node for segment %s, offset 0x%x\n",
				gcp->program_name, name, SEG_OFFSET(segp,
								    apage));
			return SEG_ERR;
		}

		if ((pgid & PPL_MASK) == 0) {
			if (need_nl)
				printf("\n");
			printf("%12x: ", pgid);	/* start a new line */
			need_nl = true;
		}
		printf(" %3d", node);

		if (signalled(gcp)) {
			reset_signal();
			break;
		}
	}
	printf("\n");

	return SEG_OK;
}
#endif
