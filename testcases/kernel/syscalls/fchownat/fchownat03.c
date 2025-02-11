// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 * Copyright (c) Linux Test Project, 2007-2024
 */

/*\
 * Verify that,
 * - fchownat() returns -1 and sets errno to EACCES if there is no permission
     to access to the file.
 * - fchownat() returns -1 and sets errno to EBADF if the file descriptor
 *   of the specified file is not valid.
 * - fchownat() returns -1 and sets errno to EFAULT if the filename points
     outside of accessable address space.
 * - fchownat() returns -1 and sets errno to EINVAL if the flag is invalid.
 * - fchownat() returns -1 and sets errno to ELOOP if too many symbolic links
 *   were encountered in resolving filename.
 * - fchownat() returns -1 and sets errno to ENAMETOOLONG if the filename is
     too long.
 * - fchownat() returns -1 and sets errno to ENOENT if the specified file does
 *   not exist.
 * - fchownat() returns -1 and sets errno to ENOTDIR if the file descriptor is
 *   a file.
 * - fchownat() returns -1 and sets errno to EPERM if the effective user id
 *   of process does not match the owner of the file and the process is not
 *   super user.
 * - fchownat() returns -1 and sets errno to EROFS if the file is readonly.
 */

#define _GNU_SOURCE
#include <pwd.h>
#include "tst_test.h"

#define TESTFILE		"testfile"
#define TESTFILE_EACCESS_DIR	"eaccess"
#define TESTFILE_EACCESS	TESTFILE_EACCESS_DIR"/eaccess"
#define TESTFILE_ELOOP		"testfile_eloop"
#define TESTFILE_ENOENT		"/tmp/does/not/exist"
#define TESTFILE_EPERM		"/dev/null"
#define TESTFILE_EROFS_MNT	"ro_mntpoint"
#define TESTFILE_EROFS		TESTFILE_EROFS_MNT"/file"

static int file_fd = -1;
static int no_fd = -1;
static int dir_fd = -1;

static char *file_eaccess;
static char *file_ebadf;
static char *file_efault;
static char *file_einval;
static char *file_eloop;
static char *file_enametoolong;
static char *file_enoent;
static char *file_enotdir;
static char *file_eperm;
static char *file_erofs;

static struct tcase {
	char **filename;
	char *desc;
	int *fd;
	int flag;
	int exp_errno;
} tcases[] = {
	{&file_eaccess,      TESTFILE_EACCESS,  &dir_fd,  0,    EACCES},
	{&file_ebadf,        TESTFILE,          &no_fd,   0,    EBADF},
	{&file_efault,       "Invalid address", &dir_fd,  0,    EFAULT},
	{&file_einval,       TESTFILE,          &dir_fd,  9999, EINVAL},
	{&file_eloop,        TESTFILE_ELOOP,    &dir_fd,  0,    ELOOP},
	{&file_enametoolong, "aaaa...",         &dir_fd,  0,    ENAMETOOLONG},
	{&file_enoent,       TESTFILE_ENOENT,   &dir_fd,  0,    ENOENT},
	{&file_enotdir,      TESTFILE,          &file_fd, 0,    ENOTDIR},
	{&file_eperm,        TESTFILE_EPERM,    &dir_fd,  0,    EPERM},
	{&file_erofs,        TESTFILE_EROFS,    &dir_fd,  0,    EROFS},
};

static void fchownat_verify(unsigned int n)
{
	uid_t euid = geteuid();
	gid_t egid = getegid();

	TST_EXP_FAIL(fchownat(*tcases[n].fd, *tcases[n].filename, euid, egid, tcases[n].flag),
		     tcases[n].exp_errno,
		     "fchownat(%d, %s, %d, %d, %d)",
		     *tcases[n].fd, tcases[n].desc, euid, egid, tcases[n].flag);
}

static void setup(void)
{
	struct passwd *ltpuser;

	SAFE_TOUCH(TESTFILE, 0600, NULL);
	dir_fd = SAFE_OPEN("./", O_DIRECTORY);

	/* EACCES setting */
	SAFE_SETEUID(0);
	SAFE_MKDIR(TESTFILE_EACCESS_DIR, S_IRWXU);
	SAFE_TOUCH(TESTFILE_EACCESS, 0666, NULL);
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);

	/* EFAULT setting */
	file_efault = tst_get_bad_addr(NULL);

	/* ENOTDIR setting */
	file_fd = SAFE_OPEN("file_fd", O_CREAT | O_RDWR, 0600);

	/* ELOOP setting */
	SAFE_SYMLINK(TESTFILE_ELOOP, "test_file_eloop2");
	SAFE_SYMLINK("test_file_eloop2", TESTFILE_ELOOP);

	/* ENAMETOOLONG setting */
	memset(file_enametoolong, 'a', PATH_MAX+1);
	file_enametoolong[PATH_MAX+1] = 0;
}

static void cleanup(void)
{
	if (dir_fd != -1)
		SAFE_CLOSE(dir_fd);
	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = TESTFILE_EROFS_MNT,
	.test = fchownat_verify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&file_eaccess,      .str = TESTFILE_EACCESS},
		{&file_ebadf,        .str = TESTFILE},
		{&file_einval,       .str = TESTFILE},
		{&file_eloop,        .str = TESTFILE_ELOOP},
		{&file_enametoolong, .size = PATH_MAX+2},
		{&file_enoent,       .str = TESTFILE_ENOENT},
		{&file_enotdir,      .str = TESTFILE},
		{&file_eperm,        .str = TESTFILE_EPERM},
		{&file_erofs,        .str = TESTFILE_EROFS},
		{}
	},
};
