/*
 * Copyright (c) 2014 Linux Test Project
 * Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __TST_FS_TYPE_H__
#define __TST_FS_TYPE_H__

/* man 2 statfs or kernel-source/include/linux/magic.h */

#define TST_BTRFS_MAGIC 0x9123683E
#define TST_NFS_MAGIC   0x6969
#define TST_RAMFS_MAGIC 0x858458f6
#define TST_TMPFS_MAGIC 0x01021994
#define TST_V9FS_MAGIC  0x01021997
#define TST_XFS_MAGIC   0x58465342

/*
 * Returns filesystem magick for a given path.
 *
 * The expected usage is:
 *
 *      if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC) {
 *		tst_brkm(TCONF, cleanup,
 *		         "Test not supported on NFS filesystem");
 *	}
 *
 * Or:
 *
 *	long type;
 *
 *	swtich ((type = tst_fs_type(cleanup, "."))) {
 *	case TST_NFS_MAGIC:
 *	case TST_TMPFS_MAGIC:
 *	case TST_RAMFS_MAGIC:
 *		tst_brkm(TCONF, cleanup, "Test not supported on %s filesystem",
 *		         tst_fs_type_name(type));
 *	break;
 *	}
 */
long tst_fs_type(void (*cleanup)(void), const char *path);

/*
 * Returns filesystem name given magic.
 */
const char *tst_fs_type_name(long f_type);

#endif	/* __TST_FS_TYPE_H__ */
