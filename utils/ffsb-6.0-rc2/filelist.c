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
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "rand.h"
#include "filelist.h"
#include "util.h"
#include "rwlock.h"
#include "rbt.h"
#include "cirlist.h"

#if 0
static
void print_cl(struct cirlist *cl)
{
	struct cnode *cur = cl->head;
	printf("curlist: ");
	if (cur == NULL) {
		printf("\n");
		return;
	}
	do {
		printf("%d ", cur->obj->num);
		cur = cur->next;
	} while (cur != cl->head);
	printf("\n");
}
#endif

#if 0
static
int node_cmp(struct ffsb_file *a, struct ffsb_file *b)
{
	return a->num - b->num;
}
#endif

static
void build_dirs(struct benchfiles *bf)
{
	char buf[FILENAME_MAX];
	int i;

	if (mkdir(bf->basedir, S_IRWXU) < 0)
		if (errno != EEXIST) {
			perror(bf->basedir);
			exit(1);
		}
	for (i = 0; i < bf->numsubdirs; i++) {
		snprintf(buf, FILENAME_MAX, "%s/%s%s%d",
			 bf->basedir, bf->basename, SUBDIRNAME_BASE, i);
		if (mkdir(buf, S_IRWXU) == -1)
			if (errno != EEXIST) {
				perror(buf);
				exit(1);
			}
	}
}

void init_filelist(struct benchfiles *b, char *basedir, char *basename,
		   uint32_t numsubdirs, int builddirs)
{
	memset(b, 0, sizeof(struct benchfiles));
	b->basedir = ffsb_strdup(basedir);
	b->basename = ffsb_strdup(basename);
	b->numsubdirs = numsubdirs;
	init_rwlock(&b->fileslock);
	b->files = rbtree_construct();
	b->dirs = rbtree_construct();
	b->holes = ffsb_malloc(sizeof(struct cirlist));
	b->dholes = ffsb_malloc(sizeof(struct cirlist));
	init_cirlist(b->holes);
	init_cirlist(b->dholes);

	if (builddirs)
		build_dirs(b);
}

static void file_destructor(struct ffsb_file *file)
{
	free(file->name);
	free(file);
}

void destroy_filelist(struct benchfiles *bf)
{
	free(bf->basedir);
	free(bf->basename);

	while (!cl_empty(bf->holes)) {
		struct ffsb_file *cur = cl_remove_head(bf->holes);
		file_destructor(cur);
	}
	free(bf->holes);
	rbtree_clean(bf->files, file_destructor);
	free(bf->files);
}

struct ffsb_file *add_file(struct benchfiles *b, uint64_t size, randdata_t * rd)
{
	struct ffsb_file *newfile, *oldfile = NULL;
	int filenum = 0;

	/* We pre-allocate here, because I don't want to spend time
	 * malloc'ing while the list is locked we free it later if
	 * necessary
	 */
	newfile = ffsb_malloc(sizeof(struct ffsb_file));

	newfile->size = size;
	init_rwlock(&(newfile->lock));

	/* Write lock the filelist, begin critical section */
	rw_lock_write(&b->fileslock);

	/* First check "holes" for a file  */
	if (!cl_empty(b->holes)) {
		oldfile = cl_remove_head(b->holes);
		rbtree_insert(b->files, oldfile);
		rw_lock_write(&oldfile->lock);
	} else {
		filenum = b->listsize;
		b->listsize++;

		newfile->num = filenum;
		rbtree_insert(b->files, newfile);

		rw_lock_write(&newfile->lock);
	}

	/* unlock filelist */
	rw_unlock_write(&b->fileslock);

	if (oldfile == NULL) {
		char buf[FILENAME_MAX];
		int randdir = getrandom(rd, b->numsubdirs + 1);
		int namesize = 0;
		if (randdir == 0)
			namesize = snprintf(buf, FILENAME_MAX, "%s/%s%s%d",
					    b->basedir, b->basename,
					    FILENAME_BASE, filenum);
		else
			namesize = snprintf(buf, FILENAME_MAX,
					    "%s/%s%s%d/%s%s%d", b->basedir,
					    b->basename, SUBDIRNAME_BASE,
					    randdir - 1, b->basename,
					    FILENAME_BASE, filenum);
		if (namesize >= FILENAME_MAX)
			/* !!! do something about this ? */
			printf("warning: filename \"%s\" too long\n", buf);
		newfile->name = ffsb_strdup(buf);
		return newfile;
	} else {
		free(newfile);
		return oldfile;
	}
}

struct ffsb_file *add_dir(struct benchfiles *b, uint64_t size, randdata_t * rd)
{
	struct ffsb_file *newdir, *olddir = NULL;
	int dirnum = 0;

	newdir = ffsb_malloc(sizeof(struct ffsb_file));

	init_rwlock(&newdir->lock);

	/* write lock the filelist, beging critical section */
	rw_lock_write(&b->fileslock);

	/* First check "holes" for a file  */
	if (!cl_empty(b->dholes)) {
		olddir = cl_remove_head(b->dholes);
		rbtree_insert(b->files, olddir);
		rw_lock_write(&olddir->lock);
	} else {
		dirnum = b->numsubdirs;
		b->numsubdirs++;
		printf("dirnum: %d\n", dirnum);
		newdir->num = dirnum;
		rbtree_insert(b->dirs, newdir);

		rw_lock_write(&newdir->lock);
	}

	/* unlock filelist */
	rw_unlock_write(&b->fileslock);

	if (olddir == NULL) {
		char buf[FILENAME_MAX];
		int namesize = 0;
		namesize = snprintf(buf, FILENAME_MAX, "%s/%s%s%d",
				    b->basedir, b->basename,
				    SUBDIRNAME_BASE, dirnum);
		if (namesize >= FILENAME_MAX)
			printf("warning: filename \"%s\" too long\n", buf);
		/* TODO: take action here... */
		newdir->name = ffsb_strdup(buf);
		return newdir;
	} else {
		free(newdir);
		return olddir;
	}
}

/* Private version of above function used only for reusing a
 * fileset.
 */
static struct ffsb_file *add_file_named(struct benchfiles *b, uint64_t size,
					char *name)
{
	struct ffsb_file *newfile = NULL;

	newfile = ffsb_malloc(sizeof(struct ffsb_file));
	memset(newfile, 0, sizeof(struct ffsb_file));
	newfile->name = ffsb_strdup(name);
	newfile->size = size;
	init_rwlock(&newfile->lock);

	/* Write lock the filelist, begin critical section */
	rw_lock_write(&b->fileslock);

	newfile->num = b->listsize;
	b->listsize++;

	/* Add a new file to the rbtree */
	rbtree_insert(b->files, newfile);

	rw_lock_write(&newfile->lock);

	/* Unlock filelist */
	rw_unlock_write(&b->fileslock);

	return newfile;
}

#if 0
static void print_rb_helper(rb_node * cur)
{
	if (cur != NULL) {
		print_rb_helper(cur->left);
		printf("%d ", cur->object->num);
		print_rb_helper(cur->right);
	}
}

static void print_rb(rb_tree * tree)
{
	print_rb_helper(tree->root);
}
#endif

void remove_file(struct benchfiles *b, struct ffsb_file *entry)
{
	rw_lock_write(&b->fileslock);

	rbtree_remove(b->files, entry, NULL);
	/* add node to the cir. list of "holes" */
	cl_insert_tail(b->holes, entry);

	rw_unlock_write(&b->fileslock);
}

static struct ffsb_file *choose_file(struct benchfiles *b, randdata_t * rd)
{
	rb_node *cur = NULL;
	int chosen = 0;
	struct ffsb_file temp;
	temp.num = chosen;

	if (b->listsize == 0) {
		fprintf(stderr, "No more files to operate on,"
			" try making more initial files "
			"or fewer delete operations\n");
		exit(0);
	}

	while (cur == NULL) {
		chosen = getrandom(rd, b->listsize);
		temp.num = chosen;
		cur = rbtree_find(b->files, &temp);
	}
	return cur->object;
}

struct ffsb_file *choose_file_reader(struct benchfiles *bf, randdata_t * rd)
{
	struct ffsb_file *ret;

	rw_lock_read(&bf->fileslock);
	/* If b->holes->count == bf->listsize, all files have been
	 * deleted!
	 */
	assert(bf->holes->count != bf->listsize);

	ret = choose_file(bf, rd);
	if (rw_trylock_read(&ret->lock)) {
		rw_unlock_read(&bf->fileslock);
		return choose_file_reader(bf, rd);
	}

	rw_unlock_read(&bf->fileslock);
	return ret;
}

struct ffsb_file *choose_file_writer(struct benchfiles *bf, randdata_t * rd)
{
	struct ffsb_file *ret;

	rw_lock_read(&bf->fileslock);
	assert(bf->holes->count != bf->listsize);
	ret = choose_file(bf, rd);

	if (rw_trylock_write(&ret->lock)) {
		rw_unlock_read(&bf->fileslock);
		return choose_file_writer(bf, rd);
	}

	rw_unlock_read(&bf->fileslock);
	return ret;
}

void unlock_file_reader(struct ffsb_file *file)
{
	rw_unlock_read(&file->lock);
}

void unlock_file_writer(struct ffsb_file *file)
{
	rw_unlock_write(&file->lock);
}

void rename_file(struct ffsb_file *file)
{
	char *newname = malloc(strlen(file->name) + 2);
	sprintf(newname, "%sa", file->name);
	file->name = newname;
}

int validate_filename(struct benchfiles *bf, char *name)
{
	int retval = -1;
	char fmt_str[FILENAME_MAX];
	if (FILENAME_MAX <= snprintf(fmt_str, FILENAME_MAX,
				     "%s%s%%d", bf->basename, FILENAME_BASE)) {
		printf("filename is too long declaring it invalid\n");
		return -1;
	}

	sscanf(name, fmt_str, &retval);
	return retval;
}

int validate_dirname(struct benchfiles *bf, char *name)
{
	int retval = -1;
	char fmt_str[FILENAME_MAX];
	if (FILENAME_MAX <= snprintf(fmt_str, FILENAME_MAX, "%s%s%%d",
				     bf->basename, SUBDIRNAME_BASE)) {
		printf("dirname is too long declaring it invalid\n");
		return -1;
	}

	sscanf(name, fmt_str, &retval);
	return retval;
}

/* Do all the dirty work of recursing through a directory structure
 * check everything for validitiy and update everything properly.
 * Note it does not check filesizes !!!, it doesn't know anything
 * about them
 */
static int add_dir_to_filelist(struct benchfiles *bf, DIR * subdir,
			       char *subdir_path, fl_validation_func_t vfunc,
			       void *vf_data)
{
	int retval = 0;
	struct dirent *d_ent = NULL;

	while ((d_ent = readdir(subdir)) != NULL) {
		DIR *tmp = NULL;
		char filename_buf[FILENAME_MAX * 2];

		if (FILENAME_MAX < snprintf(filename_buf, FILENAME_MAX, "%s/%s",
					    subdir_path, d_ent->d_name)) {
			printf("filename \"%s\" too long aborting\n",
			       filename_buf);
			return -1;
		}
		tmp = opendir(filename_buf);
		if (tmp == NULL) {
			struct ffsb_file *ffsb_file = NULL;

			if (validate_filename(bf, d_ent->d_name) < 0) {
				printf("filename \"%s\" is invalid aborting\n",
				       d_ent->d_name);
				return -1;
			}
			/* Verify size/other attributes via callback  */
			if (vfunc(bf, filename_buf, vf_data)) {
				printf("filename \"%s\" didn't pass "
				       "validation\n", d_ent->d_name);
				return -1;
			}
			/* Add file to data structure */
			ffsb_file =
			    add_file_named(bf, ffsb_get_filesize(filename_buf),
					   filename_buf);
			unlock_file_writer(ffsb_file);
		} else {
			/* Check for the usual suspects and skip them */
			if ((0 == strcmp(".", d_ent->d_name)) ||
			    (0 == strcmp("..", d_ent->d_name))) {
				closedir(tmp);
				continue;
			}
			if (validate_dirname(bf, d_ent->d_name) < 0) {
				printf("dirname \"%s\" is invalid aborting\n",
				       d_ent->d_name);
				closedir(tmp);
				return -1;
			}
			if (vfunc(bf, filename_buf, vf_data)) {
				printf("dir \"%s\" didn't pass validation\n",
				       d_ent->d_name);
				closedir(tmp);
				return -1;
			}
			/* Update filelist */
			bf->numsubdirs++;

			/* recurse */
			retval += add_dir_to_filelist(bf, tmp, filename_buf,
						      vfunc, vf_data);

			/* clean up */
			closedir(tmp);
		}
	}
	return retval;
}

int grab_old_fileset(struct benchfiles *bf, char *basename,
		     fl_validation_func_t vfunc, void *vfunc_data)
{
	int retval = 0;
	char buf[FILENAME_MAX * 2];
	DIR *lc_dir = NULL;

	if (FILENAME_MAX < snprintf(buf, FILENAME_MAX, "%s", bf->basedir)) {
		printf("filename \"%s\" is too long aborting\n", buf);
		return -1;
	}

	lc_dir = opendir(buf);
	if (lc_dir == NULL) {
		perror("opendir");
		return -1;
	}

	retval = add_dir_to_filelist(bf, lc_dir, buf, vfunc, vfunc_data);

	closedir(lc_dir);
	return retval;
}

/* Get the number of files */
uint32_t get_listsize(struct benchfiles * bf)
{
	return bf->listsize;
}

/* Get the number of subdirectories */
uint32_t get_numsubdirs(struct benchfiles * bf)
{
	return bf->numsubdirs;
}
