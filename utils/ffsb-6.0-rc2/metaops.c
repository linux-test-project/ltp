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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ffsb.h"
#include "metaops.h"
#include "rand.h"
#include "filelist.h"

/* metaops:
 *  createdir
 *  removedir
 *  renamedir
 *  renamefile
 */

void metaops_metadir(ffsb_fs_t * fs, unsigned opnum)
{
	fs_set_opdata(fs, fs_get_metafiles(fs), opnum);
}

static void createdir(struct benchfiles *dirs, randdata_t * rd)
{
	struct ffsb_file *newdir;

	newdir = add_file(dirs, 0, rd);
	if (mkdir(newdir->name, S_IRWXU) < 0) {
		perror("mkdir");
		exit(1);
	}
	unlock_file_writer(newdir);
}

static void removedir(struct benchfiles *dirs, randdata_t * rd)
{
	struct ffsb_file *deldir;

	deldir = choose_file_writer(dirs, rd);
	remove_file(dirs, deldir);

	if (rmdir(deldir->name) < 0) {
		perror("rmdir");
		exit(1);
	}
	unlock_file_writer(deldir);
}

static void renamedir(struct benchfiles *dirs, randdata_t * rd)
{
	struct ffsb_file *dir;
	char *oldname;

	dir = choose_file_writer(dirs, rd);
	oldname = dir->name;
	rename_file(dir);

	if (rename(oldname, dir->name) < 0) {
		perror("rename");
		exit(1);
	}
	unlock_file_writer(dir);
	free(oldname);
}

void ffsb_metaops(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	randdata_t *rd = ft_get_randdata(ft);

	createdir(bf, rd);
	createdir(bf, rd);
	removedir(bf, rd);
	renamedir(bf, rd);

	ft_incr_op(ft, opnum, 1, 0);
}

void ffsb_createdir(ffsb_thread_t * ft, ffsb_fs_t * fs, unsigned opnum)
{
	struct benchfiles *bf = (struct benchfiles *)fs_get_opdata(fs, opnum);
	struct ffsb_file *newdir;
	randdata_t *rd = ft_get_randdata(ft);

	newdir = add_dir(bf, 0, rd);
	if (mkdir(newdir->name, S_IRWXU) < 0) {
		perror("mkdir");
		exit(1);
	}
	unlock_file_writer(newdir);

	ft_incr_op(ft, opnum, 1, 0);
}
