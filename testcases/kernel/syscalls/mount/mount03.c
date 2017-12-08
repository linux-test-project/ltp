/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 * DESCRIPTION
 *	Check for basic mount(2) system call flags.
 *
 *	Verify that mount(2) syscall passes for each flag setting and validate
 *	the flags
 *	1) MS_RDONLY - mount read-only.
 *	2) MS_NODEV - disallow access to device special files.
 *	3) MS_NOEXEC - disallow program execution.
 *	4) MS_SYNCHRONOUS - writes are synced at once.
 *	5) MS_REMOUNT - alter flags of a mounted FS.
 *	6) MS_NOSUID - ignore suid and sgid bits.
 *	7) MS_NOATIME - do not update access times.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);
static int test_rwflag(int, int);

char *TCID = "mount03";
int TST_TOTAL = 7;

#define TEMP_FILE	"temp_file"
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define SUID_MODE	(S_ISUID|S_IRUSR|S_IXUSR|S_IXGRP|S_IXOTH)

static const char mntpoint[] = "mntpoint";
static const char *device;
static const char *fs_type;
static int fildes;

static char write_buffer[BUFSIZ];
static char read_buffer[BUFSIZ];
static char path_name[PATH_MAX];
static char file[PATH_MAX];

long rwflags[] = {
	MS_RDONLY,
	MS_NODEV,
	MS_NOEXEC,
	MS_SYNCHRONOUS,
	MS_RDONLY,
	MS_NOSUID,
	MS_NOATIME,
};

int main(int argc, char *argv[])
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			TEST(mount(device, mntpoint, fs_type, rwflags[i],
				   NULL));

			if (TEST_RETURN != 0) {
				tst_resm(TFAIL | TTERRNO, "mount(2) failed");
				continue;
			}

			/* Validate the rwflag */
			if (test_rwflag(i, lc) == 1)
				tst_resm(TFAIL, "mount(2) failed while"
					 " validating %ld", rwflags[i]);
			else
				tst_resm(TPASS, "mount(2) passed with "
					 "rwflag = %ld", rwflags[i]);

			TEST(tst_umount(mntpoint));
			if (TEST_RETURN != 0)
				tst_brkm(TBROK | TTERRNO, cleanup,
					 "umount(2) failed for %s", mntpoint);
		}
	}

	cleanup();
	tst_exit();
}

/*
 * test_rwflag(int i, int cnt)
 * Validate the mount system call for rwflags.
 */
int test_rwflag(int i, int cnt)
{
	int ret, fd, pid, status;
	char nobody_uid[] = "nobody";
	time_t atime;
	struct passwd *ltpuser;
	struct stat file_stat;
	char readbuf[20];

	switch (i) {
	case 0:
		/* Validate MS_RDONLY flag of mount call */

		snprintf(file, PATH_MAX, "%stmp", path_name);
		fd = open(file, O_CREAT | O_RDWR, S_IRWXU);
		if (fd == -1) {
			if (errno == EROFS) {
				return 0;
			} else {
				tst_resm(TWARN | TERRNO,
					 "open didn't fail with EROFS");
				return 1;
			}
		}
		close(fd);
		return 1;
	case 1:
		/* Validate MS_NODEV flag of mount call */

		snprintf(file, PATH_MAX, "%smynod_%d_%d", path_name, getpid(),
			 cnt);
		if (mknod(file, S_IFBLK | 0777, 0) == 0) {
			fd = open(file, O_RDWR, S_IRWXU);
			if (fd == -1) {
				if (errno == EACCES) {
					return 0;
				} else {
					tst_resm(TWARN | TERRNO,
						 "open didn't fail with EACCES");
					return 1;
				}
			}
			close(fd);
		} else {
			tst_resm(TWARN | TERRNO, "mknod(2) failed to create %s",
				 file);
			return 1;
		}
		return 1;
	case 2:
		/* Validate MS_NOEXEC flag of mount call */

		snprintf(file, PATH_MAX, "%stmp1", path_name);
		fd = open(file, O_CREAT | O_RDWR, S_IRWXU);
		if (fd == -1) {
			tst_resm(TWARN | TERRNO, "opening %s failed", file);
		} else {
			close(fd);
			ret = execlp(file, basename(file), NULL);
			if ((ret == -1) && (errno == EACCES))
				return 0;
		}
		return 1;
	case 3:
		/*
		 * Validate MS_SYNCHRONOUS flag of mount call.
		 * Copy some data into data buffer.
		 */

		strcpy(write_buffer, "abcdefghijklmnopqrstuvwxyz");

		/* Creat a temporary file under above directory */
		snprintf(file, PATH_MAX, "%s%s", path_name, TEMP_FILE);
		fildes = open(file, O_RDWR | O_CREAT, FILE_MODE);
		if (fildes == -1) {
			tst_resm(TWARN | TERRNO,
				 "open(%s, O_RDWR|O_CREAT, %#o) failed",
				 file, FILE_MODE);
			return 1;
		}

		/* Write the buffer data into file */
		if (write(fildes, write_buffer, strlen(write_buffer)) !=
		    strlen(write_buffer)) {
			tst_resm(TWARN | TERRNO, "writing to %s failed", file);
			close(fildes);
			return 1;
		}

		/* Set the file ptr to b'nning of file */
		if (lseek(fildes, 0, SEEK_SET) < 0) {
			tst_resm(TWARN, "lseek() failed on %s, error="
				 " %d", file, errno);
			close(fildes);
			return 1;
		}

		/* Read the contents of file */
		if (read(fildes, read_buffer, sizeof(read_buffer)) > 0) {
			if (strcmp(read_buffer, write_buffer)) {
				tst_resm(TWARN, "Data read from %s and written "
					 "mismatch", file);
				close(fildes);
				return 1;
			} else {
				close(fildes);
				return 0;
			}
		} else {
			tst_resm(TWARN | TERRNO, "read() Fails on %s", file);
			close(fildes);
			return 1;
		}

	case 4:
		/* Validate MS_REMOUNT flag of mount call */

		TEST(mount(device, mntpoint, fs_type, MS_REMOUNT, NULL));
		if (TEST_RETURN != 0) {
			tst_resm(TWARN | TTERRNO, "mount(2) failed to remount");
			return 1;
		} else {
			snprintf(file, PATH_MAX, "%stmp2", path_name);
			fd = open(file, O_CREAT | O_RDWR, S_IRWXU);
			if (fd == -1) {
				tst_resm(TWARN, "open(%s) on readonly "
					 "filesystem passed", file);
				return 1;
			} else {
				close(fd);
				return 0;
			}
		}
	case 5:
		/* Validate MS_NOSUID flag of mount call */

		snprintf(file, PATH_MAX, "%smount03_setuid_test", path_name);

		pid = fork();
		switch (pid) {
		case -1:
			tst_resm(TBROK | TERRNO, "fork failed");
			return 1;
		case 0:
			ltpuser = getpwnam(nobody_uid);
			if (setreuid(ltpuser->pw_uid, ltpuser->pw_uid) == -1)
				tst_resm(TWARN | TERRNO,
					 "seteuid() failed to change euid to %d",
					 ltpuser->pw_uid);

			execlp(file, basename(file), NULL);
			exit(1);
		default:
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {
				/* reset the setup_uid */
				if (status)
					return 0;
			}
			return 1;
		}
	case 6:
		/* Validate MS_NOATIME flag of mount call */

		snprintf(file, PATH_MAX, "%satime", path_name);
		fd = open(file, O_CREAT | O_RDWR, S_IRWXU);
		if (fd == -1) {
			tst_resm(TWARN | TERRNO, "opening %s failed", file);
			return 1;
		}

		if (write(fd, "TEST_MS_NOATIME", 15) != 15) {
			tst_resm(TWARN | TERRNO, "write %s failed", file);
			close(fd);
			return 1;
		}

		if (fstat(fd, &file_stat) == -1) {
			tst_resm(TWARN | TERRNO, "stat %s failed #1", file);
			close(fd);
			return 1;
		}

		atime = file_stat.st_atime;

		sleep(1);

		if (read(fd, readbuf, sizeof(readbuf)) == -1) {
			tst_resm(TWARN | TERRNO, "read %s failed", file);
			close(fd);
			return 1;
		}

		if (fstat(fd, &file_stat) == -1) {
			tst_resm(TWARN | TERRNO, "stat %s failed #2", file);
			close(fd);
			return 1;
		}
		close(fd);

		if (file_stat.st_atime != atime) {
			tst_resm(TWARN, "access time is updated");
			return 1;
		}
		return 0;
	}
	return 0;
}

static void setup(void)
{
	char path[PATH_MAX];
	struct stat file_stat;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	tst_tmpdir();

	fs_type = tst_dev_fs_type();
	device = tst_acquire_device(cleanup);

	if (!device)
		tst_brkm(TCONF, cleanup, "Failed to obtain block device");

	tst_mkfs(cleanup, device, fs_type, NULL, NULL);

	SAFE_MKDIR(cleanup, mntpoint, DIR_MODE);

	if (getcwd(path_name, sizeof(path_name)) == NULL)
		tst_brkm(TBROK, cleanup, "getcwd failed");

	if (chmod(path_name, DIR_MODE) != 0)
		tst_brkm(TBROK, cleanup, "chmod(%s, %#o) failed",
			 path_name, DIR_MODE);

	strncpy(path, path_name, PATH_MAX);
	snprintf(path_name, PATH_MAX, "%s/%s/", path, mntpoint);

	SAFE_MOUNT(cleanup, device, mntpoint, fs_type, 0, NULL);
	TST_RESOURCE_COPY(cleanup, "mount03_setuid_test", path_name);

	snprintf(file, PATH_MAX, "%smount03_setuid_test", path_name);
	SAFE_STAT(cleanup, file, &file_stat);

	if (file_stat.st_mode != SUID_MODE &&
	    chmod(file, SUID_MODE) < 0)
		tst_brkm(TBROK, cleanup,
			 "setuid for setuid_test failed");
	SAFE_UMOUNT(cleanup, mntpoint);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (device)
		tst_release_device(device);

	tst_rmdir();
}
