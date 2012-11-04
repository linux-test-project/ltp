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
#ifndef _FILELIST_H_
#define _FILELIST_H_

#include <pthread.h>
#include "rand.h"
#include "rwlock.h"
#include "cirlist.h"
#include "rbt.h"

#define SUBDIRNAME_BASE "dir"
#define FILENAME_BASE "file"

struct ffsb_file {
	char *name;
	uint64_t size;
	struct rwlock lock;
	uint32_t num;
};

struct cirlist;

/* Tree of ffsb_file structs and associated state info struct must be
 * locked during use.
 */
struct benchfiles {
	/* The base directory in which all subdirs and files are
	 * created
	 */
	char *basedir;

	/* The name to prepend to all directory and file names */
	char *basename;
	uint32_t numsubdirs;

	/* Files which currently exist on the filesystem */
	struct red_black_tree *files;

	/* Directories which currently exist on the filesystem */
	struct red_black_tree *dirs;

	/* Files which have been deleted, and whose numbers should be
	 * reused
	 */
	struct cirlist *holes;
	struct cirlist *dholes;

	/* This lock must be held while manipulating the structure */
	struct rwlock fileslock;
	uint32_t listsize; /* Sum size of nodes in files and holes */
};

/* Initializes the list, user must call this before anything else it
 * will create the basedir and subdirs on the filesystem automatically
 * if the builddirs arg. is nonzero
 */
void init_filelist(struct benchfiles *, char *, char *, uint32_t, int);
void destroy_filelist(struct benchfiles *);

/* Allocates a new file, adds to list, (write) locks it, and returns
 * it.  This function also randomly selects a filename + path to
 * assign to the new file.
 *
 * It first checks the "holes" list for any available filenames.
 * Caller must ensure file is actually created on disk
 */
struct ffsb_file *add_file(struct benchfiles *b, uint64_t size, randdata_t *rd);
struct ffsb_file *add_dir(struct benchfiles *, uint64_t, randdata_t *);

/* Removes file from list, decrements listsize.
 *
 * File should be writer-locked before calling this function.
 *
 * This function does not unlock file after removal from list.
 *
 * Caller must ensure file is actually removed on disk.
 *
 * Caller must NOT free file->name and file, since oldfiles are being
 * put into holes list.
 */
void remove_file(struct benchfiles *, struct ffsb_file *);

/* Picks a file at random, locks it for reading and returns it
 * locked
 */
struct ffsb_file *choose_file_reader(struct benchfiles *, randdata_t *);

/* Picks a file at random, locks it for writing and returns it
 * locked
 */
struct ffsb_file *choose_file_writer(struct benchfiles *, randdata_t *);

/* changes the file->name of a file, file must be write locked
 * it does not free the old file->name, so caller must keep a ref to it
 * and free after the call
 */
void rename_file(struct ffsb_file *);

void unlock_file_reader(struct ffsb_file *);
void unlock_file_writer(struct ffsb_file *);

/* Uses SUBDIRNAME_BASE/FILENAME_BASE + bf->basename to validate a
 * name returns a negative on invalid names, and the actual file
 * number if valid
 */
int validate_filename(struct benchfiles *, char *);
int validate_dirname(struct benchfiles *, char *);

/* Function type which, does some validation of existing files
 * currently only used by ffsb_fs stuff, returns 0 on success
 */
typedef int (*fl_validation_func_t)(struct benchfiles *, char *, void *);

/* Provided for re-use of filesets.  Also runs the validation callback
 * on each file/dir that is found, after verifying the name is
 * conformant.  The fileset should be initialized with init_fileset()
 * beforehand.
 * Returns 0 on success
 */
int grab_old_fileset(struct benchfiles *, char *, fl_validation_func_t,
		      void *);

/* Get the number of files */
uint32_t get_listsize(struct benchfiles *);

/* Get the number of subdirectories */
uint32_t get_numsubdirs(struct benchfiles *);

#endif /* _FILELIST_H_ */
