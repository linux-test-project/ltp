/*
 * Copyright (c) 2005-2014 Linux Test Project
 *
 * Cyril Hrubis <chrubis@suse.cz> 2014
 * Michal Simek <monstr@monstr.eu> 2009
 * Kumar Gala <galak@kernel.crashing.org> 2007
 * Ricky Ng-Adam <rngadam@yahoo.com> 2005
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

#include <sys/vfs.h>
#include "test.h"
#include "tst_fs.h"

long tst_fs_type_(void (*cleanup)(void), const char *path)
{
	struct statfs sbuf;

	if (statfs(path, &sbuf)) {
		tst_brkm(TBROK | TERRNO, cleanup,
		         "tst_fs_type: Failed to statfs(%s)", path);
		return 0;
	}

	return sbuf.f_type;
}

const char *tst_fs_type_name(long f_type)
{
	switch (f_type) {
	case TST_TMPFS_MAGIC:
		return "tmpfs";
	case TST_NFS_MAGIC:
		return "nfs";
	case TST_V9FS_MAGIC:
		return "9p";
	case TST_RAMFS_MAGIC:
		return "ramfs";
	case TST_BTRFS_MAGIC:
		return "btrfs";
	case TST_XFS_MAGIC:
		return "xfs";
	case TST_EXT2_OLD_MAGIC:
		return "ext2";
	case TST_EXT234_MAGIC:
		return "ext2/ext3/ext4";
	case TST_MINIX_MAGIC:
	case TST_MINIX_MAGIC2:
	case TST_MINIX2_MAGIC:
	case TST_MINIX2_MAGIC2:
	case TST_MINIX3_MAGIC:
		return "minix";
	case TST_UDF_MAGIC:
		return "udf";
	case TST_SYSV2_MAGIC:
	case TST_SYSV4_MAGIC:
		return "sysv";
	case TST_UFS_MAGIC:
	case TST_UFS2_MAGIC:
		return "ufs";
	case TST_F2FS_MAGIC:
		return "f2fs";
	case TST_NILFS_MAGIC:
		return "nilfs";
	case TST_EXOFS_MAGIC:
		return "exofs";
	case TST_OVERLAYFS_MAGIC:
		return "overlayfs";
	case TST_FUSE_MAGIC:
		return "fuse";
	case TST_EXFAT_MAGIC:
		return "exfat";
	default:
		return "unknown";
	}
}
