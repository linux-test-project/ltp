/*
 *    AUTHOR
 *    	Ricky Ng-Adam <rngadam@yahoo.com>, 2005-01-01
 *
 *    DESCRIPTION
 * 	Check if current directory is on a tmpfs filesystem
 * 	If current directory is tmpfs, return 1
 * 	If current directory is NOT tmpfs, return 0
 *
 *
 */

#include <sys/vfs.h>

#define TMPFS_MAGIC 0x01021994 /* man 2 statfs */

int 
tst_is_cwd_tmpfs() 
{
	struct statfs sf;
	statfs(".", &sf);

	/* Verify that the file is not on a tmpfs (in-memory) filesystem */
	return sf.f_type == TMPFS_MAGIC?1:0;
}
