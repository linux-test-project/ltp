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

#define TST_BTRFS_MAGIC    0x9123683E
#define TST_NFS_MAGIC      0x6969
#define TST_RAMFS_MAGIC    0x858458f6
#define TST_TMPFS_MAGIC    0x01021994
#define TST_V9FS_MAGIC     0x01021997
#define TST_XFS_MAGIC      0x58465342
#define TST_EXT2_OLD_MAGIC 0xEF51
/* ext2, ext3, ext4 have the same magic number */
#define TST_EXT234_MAGIC   0xEF53
#define TST_MINIX_MAGIC    0x137F
#define TST_MINIX_MAGIC2   0x138F
#define TST_MINIX2_MAGIC   0x2468
#define TST_MINIX2_MAGIC2  0x2478
#define TST_MINIX3_MAGIC   0x4D5A
#define TST_UDF_MAGIC      0x15013346
#define TST_SYSV2_MAGIC    0x012FF7B6
#define TST_SYSV4_MAGIC    0x012FF7B5
#define TST_UFS_MAGIC      0x00011954
#define TST_UFS2_MAGIC     0x19540119
#define TST_F2FS_MAGIC     0xF2F52010
#define TST_NILFS_MAGIC    0x3434
#define TST_EXOFS_MAGIC    0x5DF5

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
