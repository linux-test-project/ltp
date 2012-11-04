/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
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
#ifndef _FFSB_FS_H_
#define _FFSB_FS_H_

#include "filelist.h"
#include "ffsb_op.h"
#include "ffsb_tg.h"
#include "ffsb_stats.h"

/* These are the base names for the different file types on a
 * filesystem.
*/
#define FILES_BASE "data"
#define META_BASE  "meta"
#define AGE_BASE   "fill"

struct ffsb_tg;

typedef struct size_weight {
	uint64_t size;
	int weight;
} size_weight_t;

/* A filesystem object
 * --
 * represents a filesystem on disk, and maintains access to the different
 * directories within it.  Currently there are two directories created in
 * each filesystem: data and meta
 * "data" contains the aging files and the working set files
 * "meta" only contains directories for use in the metadata operation
 *
 * Aging
 * This object contains methods for aging the filesystem if needed
 * a properly set up threadgroup is supplied by the parser which is run
 * until the filesystem reaches the desired utilization
 *
 * Operations
 * The semantics of a ffsb operation are always such that they occur on
 * a filesystem, so the filesystem also hold operation specific data as
 * an opaque type
 * One example of how this is useful is the aging process, where a different
 * set of files is operated upon than in the regular benchmark and the
 * op_data is pointing to the "fill" set rather than the "files" set
 */

typedef struct ffsb_fs {
	char *basedir;

	struct benchfiles files;
	struct benchfiles meta;
	struct benchfiles fill;

	int flags;
#define FFSB_FS_DIRECTIO   (1 << 0)
#define FFSB_FS_ALIGNIO4K  (1 << 1)
#define FFSB_FS_LIBCIO     (1 << 2)
#define FFSB_FS_REUSE_FS   (1 << 3)

	/* These pararmeters pertain to files in the files and fill
	 * dirs.  Meta dir only contains directories, starting with 0.
	 */
	uint32_t num_dirs;
	uint32_t num_start_files;
	uint64_t minfilesize, maxfilesize;
	double init_fsutil;
	uint64_t init_size;

	/* These two parameters specify the blocksize to use for
	 * writes when creating and aging the fs.
	 */
	uint32_t create_blocksize, age_blocksize;
#define FFSB_FS_DEFAULT_CREATE_BLOCKSIZE 4096
#define FFSB_FS_DEFAULT_AGE_BLOCKSIZE    4096

	double start_fsutil;

	/* Aging data/parameters */
	double desired_fsutil;
	int age_fs;
	uint32_t num_age_dirs;

	/* Use an ffsb thread group to do the aging work */
	struct ffsb_tg *aging_tg;

	/* If a particular operation wants to maintain fs-specific
	 * data, it should use this array.  Naturally, the ops must
	 * synchonize access to the data for now, they are all just
	 * putting pointers to a particular benchfiles struct here,
	 * which is already sync'ed
	 */
	void *op_data[FFSB_NUMOPS];

	/* per-fs stats */
	ffsb_statsc_t fsc;
	ffsb_statsd_t fsd;

	size_weight_t *size_weights;
	unsigned num_weights;
	unsigned sum_weights;

} ffsb_fs_t;

/* Set up the structure, zeros everything out and dups the basedir
 * string
 */
void init_ffsb_fs(ffsb_fs_t *fs, char *basedir, uint32_t num_data_dirs,
		  uint32_t num_start_files, unsigned flags);

/* Does not remove files/dirs on disk, only frees up data
 * structures
 */
void destroy_ffsb_fs(ffsb_fs_t *fs);

/* Set up the files and such on the disk including aging if requested.
 * Should call back into each op, which initialize its op_data[]
 * entry.  Aging is done by starting the aging_tg thread group, and
 * waiting until the desired utilization is achieved.  It can (and is)
 * be used with pthread_create().  Parameter should be a ffsb_fs_t * ,
 * and it will return the same type
 */
void *construct_ffsb_fs(void *ffsb_fs_ptr);

/* Shallow clone, original should simply be discarded (not destroyed).
 * Generally should only be used by parser to write into the config
 * object
 */
void clone_ffsb_fs(ffsb_fs_t *target, ffsb_fs_t *original);

void fs_print_config(ffsb_fs_t *fs);

char *fs_get_basedir(ffsb_fs_t *fs);
int fs_get_directio(ffsb_fs_t *fs);
void fs_set_directio(ffsb_fs_t *fs, int dio);
int fs_get_alignio(ffsb_fs_t *fs);
void fs_set_alignio(ffsb_fs_t *fs, int aio);
int fs_get_libcio(ffsb_fs_t *fs);
void fs_set_libcio(ffsb_fs_t *fs, int lio);
int fs_get_reuse_fs(ffsb_fs_t *fs);
void fs_set_reuse_fs(ffsb_fs_t *fs, int rfs);

struct benchfiles *fs_get_datafiles(ffsb_fs_t *fs);
struct benchfiles *fs_get_metafiles(ffsb_fs_t *fs);
struct benchfiles *fs_get_agefiles(ffsb_fs_t *fs);

void fs_set_aging_tg(ffsb_fs_t *fs, struct ffsb_tg *, double util);
struct ffsb_tg *fs_get_aging_tg(ffsb_fs_t *fs);
int fs_get_agefs(ffsb_fs_t *fs);

void fs_set_opdata(ffsb_fs_t *fs, void *data, unsigned opnum);
void *fs_get_opdata(ffsb_fs_t *fs, unsigned opnum);
void fs_set_min_filesize(ffsb_fs_t *fs, uint64_t size);
void fs_set_max_filesize(ffsb_fs_t *fs, uint64_t size);
void fs_set_create_blocksize(ffsb_fs_t *fs, uint32_t blocksize);
void fs_set_age_blocksize(ffsb_fs_t *fs, uint32_t blocksize);
uint32_t fs_get_create_blocksize(ffsb_fs_t *fs);
uint32_t fs_get_age_blocksize(ffsb_fs_t *fs);
uint64_t fs_get_min_filesize(ffsb_fs_t *fs);
uint64_t fs_get_max_filesize(ffsb_fs_t *fs);
uint32_t fs_get_numstartfiles(ffsb_fs_t *fs);
uint32_t fs_get_numdirs(ffsb_fs_t *fs);

double fs_get_desired_fsutil(ffsb_fs_t *fs);

/* For these two, fs == NULL is OK */
int fs_needs_stats(ffsb_fs_t *fs, syscall_t s);
void fs_add_stat(ffsb_fs_t *fs, syscall_t sys, uint32_t val);

#endif /* _FFSB_FS_H_ */
