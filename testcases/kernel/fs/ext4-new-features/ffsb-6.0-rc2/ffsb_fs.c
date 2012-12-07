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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "ffsb_fs.h"
#include "util.h"
#include "fh.h"

/* First zero out struct, set num_dirs, and strdups basedir */
void init_ffsb_fs(ffsb_fs_t * fs, char *basedir, uint32_t num_data_dirs,
		  uint32_t numstartfiles, unsigned flags)
{
	memset(fs, 0, sizeof(ffsb_fs_t));
	fs->basedir = ffsb_strdup(basedir);
	fs->num_dirs = num_data_dirs;
	fs->num_start_files = numstartfiles;
	fs->flags = flags;
	fs->create_blocksize = FFSB_FS_DEFAULT_CREATE_BLOCKSIZE;
	fs->age_blocksize = FFSB_FS_DEFAULT_AGE_BLOCKSIZE;
	fs->age_fs = 0;
}

/*
 * Does not remove files/dirs on disk, only frees up data
 * structures
*/
void destroy_ffsb_fs(ffsb_fs_t * fs)
{
	free(fs->basedir);
	destroy_filelist(&fs->files);
	destroy_filelist(&fs->fill);
	destroy_filelist(&fs->meta);
}

void clone_ffsb_fs(ffsb_fs_t * target, ffsb_fs_t * orig)
{
	target->basedir = orig->basedir;
	target->flags = orig->flags;

	/* !!!! hackish, write a filelist_clone() function later */
	memcpy(&target->files, &orig->files, sizeof(orig->files));
	memcpy(&target->fill, &orig->fill, sizeof(orig->fill));
	memcpy(&target->meta, &orig->meta, sizeof(orig->meta));

	target->num_dirs = orig->num_dirs;
	target->num_start_files = orig->num_start_files;
	target->minfilesize = orig->minfilesize;
	target->maxfilesize = orig->maxfilesize;

	target->start_fsutil = orig->start_fsutil;
	target->desired_fsutil = orig->desired_fsutil;

	target->age_fs = orig->age_fs;
	target->num_age_dirs = orig->num_age_dirs;
	target->aging_tg = orig->aging_tg;

	target->create_blocksize = orig->create_blocksize;
	target->age_blocksize = orig->age_blocksize;

	memcpy(target->op_data, orig->op_data, sizeof(void *) * FFSB_NUMOPS);
}

static void add_files(ffsb_fs_t * fs, struct benchfiles *bf, int num,
		      uint64_t minsize, uint64_t maxsize, unsigned blocksize)
{
	struct ffsb_file *cur;
	int i, fd, condition = 0, has_directio = 0;
	randdata_t rd;
	char *buf = ffsb_malloc(blocksize);
	uint64_t initial_free = getfsutil_size(fs->basedir);

	if (fs_get_directio(fs)) {
		has_directio = 1;
		fs_set_directio(fs, 0);
	}

	assert(blocksize);

	init_random(&rd, 0);

	if (num)
		condition = num;
	else if (fs->init_size) {
		if (getfsutil(fs->basedir) != initial_free ||
		    fs->init_size > (getfsutil_size(fs->basedir) -
				     initial_free))
			condition = 1;
		else
			condition = 0;
	} else if (fs->init_fsutil) {
		if (fs->init_fsutil > getfsutil(fs->basedir))
			condition = 1;
		else
			condition = 0;
	}

	while (condition) {
		uint64_t size;
		if (fs->num_weights) {
			int num = 1 + getrandom(&rd, fs->sum_weights);
			int curop = 0;

			while (fs->size_weights[curop].weight < num) {
				num -= fs->size_weights[curop].weight;
				curop++;
			}
			size = fs->size_weights[curop].size;
		} else
			size = minsize + getllrandom(&rd, maxsize - minsize);

		cur = add_file(bf, size, &rd);
		fd = fhopencreate(cur->name, NULL, fs);
		writefile_helper(fd, size, blocksize, buf, NULL, fs);
		fhclose(fd, NULL, fs);
		unlock_file_writer(cur);

		if (num)
			condition--;
		else if (fs->init_size) {
			if (fs->init_size > getfsutil_size(fs->basedir) -
			    initial_free)
				condition = 1;
			else
				condition = 0;
		} else if (fs->init_fsutil) {
			if (fs->init_fsutil > getfsutil(fs->basedir))
				condition = 1;
			else
				condition = 0;
		}

	}
	free(buf);
	if (has_directio)
		fs_set_directio(fs, 1);
}

static void age_fs(ffsb_fs_t * fs, double utilization);
static ffsb_fs_t *construct_new_fileset(ffsb_fs_t * fs);
static ffsb_fs_t *check_existing_fileset(ffsb_fs_t * fs);

void *construct_ffsb_fs(void *data)
{
	ffsb_fs_t *fs = (ffsb_fs_t *) data;
	ffsb_fs_t *ret = NULL;

	if (fs_get_reuse_fs(fs)) {
		printf("checking existing fs: %s\n", fs->basedir);
		ret = check_existing_fileset(fs);
		if (ret == NULL) {
			printf("recreating new fileset\n");
			ret = construct_new_fileset(fs);
		}
	} else {
		printf("creating new fileset %s\n", fs->basedir);
		ret = construct_new_fileset(fs);
	}
	if (ret == NULL) {
		printf("fs setup on %s failed\n", fs->basedir);
		exit(1);
	}
	return ret;
}

static int verify_file(struct benchfiles *bf, char *fname, void *fs_ptr)
{
	ffsb_fs_t *fs = (ffsb_fs_t *) fs_ptr;
	uint64_t minsize = fs->minfilesize;
	uint64_t maxsize = fs->maxfilesize;
	uint64_t filesize = 0;
	int fd = 0;
	DIR *dirptr = NULL;

	/* If it is a directory and it passed the name verification we
	 * don't need to do anything here
	 */
	dirptr = opendir(fname);
	if (dirptr) {
		closedir(dirptr);
		return 0;
	}

	fd = open(fname, O_RDONLY);
	/* If we can't open it for read we're done */
	if (fd < 0) {
		printf("verify_file: error opening %s for readonly\n", fname);
		perror(fname);
		return 1;
	}
	close(fd);
	filesize = ffsb_get_filesize(fname);

	if (filesize < minsize || filesize > maxsize) {
		printf("size %llu bytes for file %s is invalid\n",
		       filesize, fname);
		return 1;
	}

	return 0;
}

/* Record the number of files and directorys there are supposed to be
 * grab (check and build the structures) the regular data fileset then
 * check to make sure the number of directories and files in that
 * filelist matches up.  Then grab the meta filelist and verify that
 * the meta filelist is empty.  Set up the filelist for fill (aging)
 * and setup the ops for the benchmark.
*/
static ffsb_fs_t *check_existing_fileset(ffsb_fs_t * fs)
{
	char buf[FILENAME_MAX * 3];
	int retval = 0;
	uint32_t num_dirs = fs->num_dirs;
	uint32_t num_files = fs->num_start_files;

	if (fs->age_fs) {
		printf("Aging and reusing the fileset are mutually "
		       "exclusive\n");
		printf("aborting\n");
		return NULL;
	}

	/* Set up bench/age dir */
	if (FILENAME_MAX <=
	    snprintf(buf, FILENAME_MAX, "%s/%s", fs->basedir, FILES_BASE)) {
		printf("pathname \"%s\" is too long, aborting\n", buf);
		return NULL;
	}

	/* Make a "dummy" filelist that has numsubdirs set to 0 and
	 * numstartfiles set to 0
	 */
	init_filelist(&fs->files, buf, FILES_BASE, 0, 0);

	retval = grab_old_fileset(&fs->files, buf, verify_file, fs);

	if (retval)
		return NULL;

	if ((get_listsize(&fs->files) != num_files) ||
	    (get_numsubdirs(&fs->files) != num_dirs)) {
		printf("check_existing_fileset: number of files (%u)"
		       " or directorys (%u) don't match up\n",
		       get_listsize(&fs->files), get_numsubdirs(&fs->files));
		destroy_filelist(&fs->files);
		return NULL;
	}

	if (FILENAME_MAX <=
	    snprintf(buf, FILENAME_MAX, "%s/%s", fs->basedir, META_BASE)) {
		printf("pathname \"%s\" is too long, aborting\n", buf);
		return NULL;
	}

	init_filelist(&fs->meta, buf, META_BASE, 0, 1);
	retval = grab_old_fileset(&fs->meta, buf, verify_file, fs);

	if (retval) {
		destroy_filelist(&fs->files);
		return NULL;
	}

	if ((get_listsize(&fs->meta) != 0) || (get_numsubdirs(&fs->meta) != 0)) {
		printf("check_existing_fileset: meta directory isn't empty\n"
		       "aborting\n");
		destroy_filelist(&fs->files);
		destroy_filelist(&fs->meta);
		return NULL;
	}

	/* Even though we won't use it, we still need to be consistent
	 * here.
	 */
	init_filelist(&fs->fill, buf, AGE_BASE, 0, 0);

	/* Have to do this or everything else could break. */
	ops_setup_bench(fs);

	return fs;
}

/*
 *  clean up fs, "rm -rf data meta"
 *  record utilization
 *  set up the dirs: files, meta
 *  age filesystem
 *  have ffsb_ops setup their data
 *  create starting files in files
 */
static ffsb_fs_t *construct_new_fileset(ffsb_fs_t * fs)
{
	char buf[FILENAME_MAX * 3];

	/* TODO: Convert this quick and dirty rm -rf to a "real"
	 * programmatic version, that doesn't rely on the rm command.
	 */
	if (FILENAME_MAX * 3 <= snprintf(buf, FILENAME_MAX * 3,
					 "rm -rf %s/data %s/meta",
					 fs->basedir, fs->basedir)) {
		printf("pathname too long for command \"%s\"\n", buf);
		return NULL;
	}

	if (ffsb_system(buf) < 0) {
		perror(buf);
		return NULL;
	}

	fs->start_fsutil = getfsutil(fs->basedir);

	/* Set up bench/age dir */
	if (FILENAME_MAX <=
	    snprintf(buf, FILENAME_MAX, "%s/%s", fs->basedir, FILES_BASE)) {
		printf("pathname \"%s\" is too long, aborting\n", buf);
		return NULL;
	}

	ffsb_mkdir(buf);

	/* Regular files and aging share this directory */
	init_filelist(&fs->files, buf, FILES_BASE, fs->num_dirs, 1);
	init_filelist(&fs->fill, buf, AGE_BASE, fs->num_age_dirs, 1);

	/* Set up meta dir */
	snprintf(buf, FILENAME_MAX, "%s/%s", fs->basedir, META_BASE);

	ffsb_mkdir(buf);

	init_filelist(&fs->meta, buf, META_BASE, 0, 1);

	/* Do aging */
	if (fs->age_fs)
		age_fs(fs, fs->desired_fsutil);

	/* Call back into ops, set for benchmark */
	ops_setup_bench(fs);

	/* Create initial fileset */
	add_files(fs, &fs->files, fs->num_start_files, fs->minfilesize,
		  fs->maxfilesize, fs->create_blocksize);
	return fs;
}

struct poll_data {
	ffsb_fs_t *fs;
	double util;
};

static int fs_get_util(void *data)
{
	struct poll_data *pd = (struct poll_data *)data;
	double fsutil = getfsutil(pd->fs->basedir);

	if (fsutil >= pd->util)
		return 1;

	return 0;
}

static void age_fs(ffsb_fs_t * fs, double utilization)
{
	ffsb_barrier_t barrier;
	pthread_t thread;
	struct poll_data pdata;
	ffsb_tg_t *tg = fs_get_aging_tg(fs);
	tg_run_params_t params;
	ffsb_config_t fc;

	printf("aging fs %s from %.2lf to %.2lf\n", fs->basedir,
	       fs->start_fsutil, utilization);
	ffsb_barrier_init(&barrier, tg_get_numthreads(tg));

	init_ffsb_config_1fs(&fc, fs, tg);

	pdata.fs = fs;
	pdata.util = utilization;

	params.tg = tg;
	params.poll_fn = fs_get_util;
	params.poll_data = &pdata;
	params.wait_time = 1;
	params.fc = &fc;

	params.tg_barrier = NULL;
	params.thread_barrier = &barrier;

	/* Call back into ops, setup for aging */
	ops_setup_age(fs);

	/* Throw in some files to start off, so there's something */
	add_files(fs, &fs->fill, 10, 0, 0, fs->age_blocksize);

	pthread_create(&thread, NULL, tg_run, &params);
	pthread_join(thread, NULL);
}

void fs_set_create_blocksize(ffsb_fs_t * fs, uint32_t blocksize)
{
	fs->create_blocksize = blocksize;
}

void fs_set_age_blocksize(ffsb_fs_t * fs, uint32_t blocksize)
{
	fs->age_blocksize = blocksize;
}

uint32_t fs_get_create_blocksize(ffsb_fs_t * fs)
{
	return fs->create_blocksize;
}

uint32_t fs_get_age_blocksize(ffsb_fs_t * fs)
{
	return fs->age_blocksize;
}

char *fs_get_basedir(ffsb_fs_t * fs)
{
	return fs->basedir;
}

uint32_t fs_get_numstartfiles(ffsb_fs_t * fs)
{
	return fs->num_start_files;
}

uint32_t fs_get_numdirs(ffsb_fs_t * fs)
{
	return fs->num_dirs;
}

int fs_get_libcio(ffsb_fs_t * fs)
{
	return fs->flags & FFSB_FS_LIBCIO;
}

void fs_set_libcio(ffsb_fs_t * fs, int lio)
{
	if (lio)
		fs->flags |= FFSB_FS_LIBCIO;
	else
		fs->flags &= ~0 & ~FFSB_FS_LIBCIO;
}

int fs_get_directio(ffsb_fs_t * fs)
{
	return fs->flags & FFSB_FS_DIRECTIO;
}

void fs_set_directio(ffsb_fs_t * fs, int dio)
{
	if (dio)
		fs->flags |= FFSB_FS_DIRECTIO;
	else
		fs->flags &= ~0 & ~FFSB_FS_DIRECTIO;
}

int fs_get_alignio(ffsb_fs_t * fs)
{
	return fs->flags & FFSB_FS_ALIGNIO4K;
}

void fs_set_alignio(ffsb_fs_t * fs, int aio)
{
	if (aio)
		fs->flags |= FFSB_FS_ALIGNIO4K;
	else
		fs->flags &= ~0 & ~FFSB_FS_ALIGNIO4K;
}

int fs_get_reuse_fs(ffsb_fs_t * fs)
{
	return fs->flags & FFSB_FS_REUSE_FS;
}

void fs_set_reuse_fs(ffsb_fs_t * fs, int rfs)
{
	if (rfs)
		fs->flags |= FFSB_FS_REUSE_FS;
	else
		fs->flags &= ~0 & ~FFSB_FS_REUSE_FS;
}

struct benchfiles *fs_get_datafiles(ffsb_fs_t * fs)
{
	return &fs->files;
}

struct benchfiles *fs_get_metafiles(ffsb_fs_t * fs)
{
	return &fs->meta;
}

struct benchfiles *fs_get_agefiles(ffsb_fs_t * fs)
{
	return &fs->fill;
}

void fs_set_aging_tg(ffsb_fs_t * fs, struct ffsb_tg *tg, double util)
{
	fs->aging_tg = tg;
	fs->age_fs = 1;
	fs->desired_fsutil = util;
}

struct ffsb_tg *fs_get_aging_tg(ffsb_fs_t * fs)
{
	return fs->aging_tg;
}

int fs_get_agefs(ffsb_fs_t * fs)
{
	return fs->age_fs;
}

/* TODO: Implement this!!!*/
void fs_set_num_age_dirs(ffsb_fs_t * fs, uint32_t numdirs)
{
	fs->num_age_dirs = numdirs;
}

void fs_set_opdata(ffsb_fs_t * fs, void *data, unsigned opnum)
{
	fs->op_data[opnum] = data;
}

void *fs_get_opdata(ffsb_fs_t * fs, unsigned opnum)
{
	return fs->op_data[opnum];
}

void fs_set_min_filesize(ffsb_fs_t * fs, uint64_t size)
{
	fs->minfilesize = size;
}

void fs_set_max_filesize(ffsb_fs_t * fs, uint64_t size)
{
	fs->maxfilesize = size;
}

uint64_t fs_get_min_filesize(ffsb_fs_t * fs)
{
	return fs->minfilesize;
}

uint64_t fs_get_max_filesize(ffsb_fs_t * fs)
{
	return fs->maxfilesize;
}

double fs_get_desired_fsutil(ffsb_fs_t * fs)
{
	return fs->desired_fsutil;
}

void fs_print_config(ffsb_fs_t * fs)
{
	char buf[256];

	printf("FileSystem %s\n", fs->basedir);
	printf("==========\n");
	printf("\t num_dirs         = %u\n", fs->num_dirs);
	printf("\t starting files   = %u\n", fs->num_start_files);
	printf("\t\n");
	if (fs->num_weights) {
		int i;
		printf("\t Fileset weight:\n");
		for (i = 0; i < fs->num_weights; i++)
			printf("\t\t %12llu (%6s) -> %u (%.2f\%)\n",
			       fs->size_weights[i].size,
			       ffsb_printsize(buf, fs->size_weights[i].size,
					      256), fs->size_weights[i].weight,
			       ((float)fs->size_weights[i].weight /
				(float)fs->sum_weights) * 100);
	} else {
		printf("\t min file size    = %llu\t(%s)\n", fs->minfilesize,
		       ffsb_printsize(buf, fs->minfilesize, 256));
		printf("\t max file size    = %llu\t(%s)\n", fs->maxfilesize,
		       ffsb_printsize(buf, fs->maxfilesize, 256));
	}
	printf("\t directio         = %s\n", (fs->flags & FFSB_FS_DIRECTIO) ?
	       "on" : "off");
	printf("\t alignedio        = %s\n", (fs->flags & FFSB_FS_ALIGNIO4K) ?
	       "on" : "off");
	printf("\t bufferedio       = %s\n", (fs->flags & FFSB_FS_LIBCIO) ?
	       "on" : "off");
	printf("\t\n");
	printf("\t aging is %s\n", (fs->age_fs) ? "on" : "off");
	printf("\t current utilization = %.2f\%\n",
	       getfsutil(fs->basedir) * 100);
	if (fs->age_fs) {
		printf("\t desired utilization = %.2lf%\n",
		       fs->desired_fsutil * 100);
		printf("\t \n");
		tg_print_config_aging(fs->aging_tg, fs->basedir);
	}
	printf("\t\n");
}

int fs_needs_stats(ffsb_fs_t * fs, syscall_t sys)
{
	return (fs != NULL) ? (int)fs->fsd.config : 0;
}

void fs_add_stat(ffsb_fs_t * fs, syscall_t sys, uint32_t val)
{
	if (fs)
		ffsb_add_data(&fs->fsd, sys, val);
}
