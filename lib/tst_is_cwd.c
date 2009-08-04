/*
 * Michal Simek <monstr@monstr.eu>, 2009-08-03 - ramfs
 * Kumar Gala <galak@kernel.crashing.org>, 2007-11-14 - nfs
 * Ricky Ng-Adam <rngadam@yahoo.com>, 2005-01-01 - tmpfs
 *
 * DESCRIPTION
 *	Check if current directory is on a tmpfs/nfs/ramfs filesystem
 *	If current directory is tmpfs/nfs/ramfs, return 1
 *	If current directory is NOT tmpfs/nfs/ramfs, return 0
 */

#include <sys/vfs.h>

#define TMPFS_MAGIC 0x01021994 /* man 2 statfs */
int tst_is_cwd_tmpfs(void)
{
	struct statfs sf;
	statfs(".", &sf);

	/* Verify that the file is not on a tmpfs (in-memory) filesystem */
	return sf.f_type == TMPFS_MAGIC ? 1 : 0;
}

#define NFS_MAGIC 0x6969 /* man 2 statfs */
int tst_is_cwd_nfs(void)
{
	struct statfs sf;
	statfs(".", &sf);

	/* Verify that the file is not on a nfs filesystem */
	return sf.f_type == NFS_MAGIC ? 1 : 0;
}

#define RAMFS_MAGIC 0x858458f6
int tst_is_cwd_ramfs(void)
{
	struct statfs sf;
	statfs(".", &sf);

	/* Verify that the file is not on a ramfs (in-memory) filesystem */
	return sf.f_type == RAMFS_MAGIC ? 1 : 0;
}
