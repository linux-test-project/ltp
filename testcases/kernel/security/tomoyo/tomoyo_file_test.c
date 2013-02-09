/******************************************************************************/
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * tomoyo_file_test.c
 *
 * Testing program for security/tomoyo/
 *
 * Copyright (C) 2005-2010  NTT DATA CORPORATION
 */
#include "include.h"

static int should_fail = 0;

static void show_prompt(const char *str)
{
	printf("Testing %35s: (%s) ", str,
	       should_fail ? "must fail" : "should success");
	errno = 0;
}

static void show_result(int result)
{
	if (should_fail) {
		if (result == EOF) {
			if (errno == EPERM)
				printf("OK: Permission denied.\n");
			else
				printf("FAILED: %s\n", strerror(errno));
		} else {
			printf("BUG!\n");
		}
	} else {
		if (result != EOF)
			printf("OK\n");
		else
			printf("%s\n", strerror(errno));
	}
}

static const char *dev_null_path = "/dev/null";
static const char *truncate_path = "/tmp/truncate_test";
static const char *ftruncate_path = "/tmp/ftruncate_test";
static const char *open_creat_path = "/tmp/open_test";
static const char *mknod_reg_path = "/tmp/mknod_reg_test";
static const char *mknod_chr_path = "/tmp/mknod_chr_test";
static const char *mknod_blk_path = "/tmp/mknod_blk_test";
static const char *mknod_fifo_path = "/tmp/mknod_fifo_test";
static const char *mknod_sock_path = "/tmp/mknod_sock_test";
static const char *unlink_path = "/tmp/unlink_test";
static const char *mkdir_path = "/tmp/mkdir_test";
static const char *rmdir_path = "/tmp/rmdir_test";
static const char *link_source_path = "/tmp/link_source_test";
static const char *link_dest_path = "/tmp/link_dest_test";
static const char *symlink_source_path = "/tmp/symlink_source_test";
static const char *symlink_dest_path = "/tmp/symlink_dest_test";
static const char *rename_source_path = "/tmp/rename_source_test";
static const char *rename_dest_path = "/tmp/rename_dest_test";
static const char *socket_path = "/tmp/socket_test";

static int ftruncate_fd = EOF;

static void stage_file_test(void)
{
	int fd;
	{
		const char buffer[] = "32768 61000";
		show_prompt("sysctl(READ)");
		show_result(read_sysctl(TEST_SYSCTL_PATH, NULL, 0));
		show_prompt("sysctl(WRITE)");
		show_result(write_sysctl(TEST_SYSCTL_PATH, buffer));
	}

	/* QUESTION: Is there a file which can be passed to uselib()? */
	show_prompt("uselib()");
	show_result(uselib("/bin/true"));

	{
		int pipe_fd[2] = { EOF, EOF };
		int error = 0;
		fflush(stdout);
		fflush(stderr);
		if (pipe(pipe_fd) == -1)
			err(1, "pipe");
		if (fork() == 0) {
			execl("/bin/true", "/bin/true", NULL);
			if (write(pipe_fd[1], &errno, sizeof(errno)) == -1)
				err(1, "write");
			_exit(0);
		}
		close(pipe_fd[1]);
		(void)read(pipe_fd[0], &error, sizeof(error));
		show_prompt("execve()");
		errno = error;
		show_result(error ? EOF : 0);
	}

	show_prompt("open(O_RDONLY)");
	fd = open(dev_null_path, O_RDONLY);
	show_result(fd);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_WRONLY)");
	fd = open(dev_null_path, O_WRONLY);
	show_result(fd);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_RDWR)");
	fd = open(dev_null_path, O_RDWR);
	show_result(fd);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_CREAT | O_EXCL)");
	fd = open(open_creat_path, O_CREAT | O_EXCL, 0666);
	show_result(fd);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_TRUNC)");
	fd = open(truncate_path, O_TRUNC);
	show_result(fd);
	if (fd != EOF)
		close(fd);

	show_prompt("truncate()");
	show_result(truncate(truncate_path, 0));

	show_prompt("ftruncate()");
	show_result(ftruncate(ftruncate_fd, 0));

	show_prompt("mknod(S_IFREG)");
	show_result(mknod(mknod_reg_path, S_IFREG, 0));

	show_prompt("mknod(S_IFCHR)");
	show_result(mknod(mknod_chr_path, S_IFCHR, MKDEV(1, 3)));

	show_prompt("mknod(S_IFBLK)");
	show_result(mknod(mknod_blk_path, S_IFBLK, MKDEV(1, 0)));

	show_prompt("mknod(S_IFIFO)");
	show_result(mknod(mknod_fifo_path, S_IFIFO, 0));

	show_prompt("mknod(S_IFSOCK)");
	show_result(mknod(mknod_sock_path, S_IFSOCK, 0));

	show_prompt("mkdir()");
	show_result(mkdir(mkdir_path, 0600));

	show_prompt("rmdir()");
	show_result(rmdir(rmdir_path));

	show_prompt("unlink()");
	show_result(unlink(unlink_path));

	show_prompt("symlink()");
	show_result(symlink(symlink_dest_path, symlink_source_path));

	show_prompt("link()");
	show_result(link(link_source_path, link_dest_path));

	show_prompt("rename()");
	show_result(rename(rename_source_path, rename_dest_path));

	{
		struct sockaddr_un addr;
		int fd;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		show_prompt("unix_bind()");
		show_result(bind(fd, (struct sockaddr *)&addr, sizeof(addr)));
		if (fd != EOF)
			close(fd);
	}

	printf("\n\n");
}

static void create_files(void)
{
	mkdir(rmdir_path, 0700);
	close(creat(link_source_path, 0600));
	close(creat(rename_source_path, 0600));
	close(creat(truncate_path, 0600));
	close(creat(unlink_path, 0600));
	ftruncate_fd = open(ftruncate_path, O_WRONLY | O_CREAT, 0600);
}

static void creanup_files(void)
{
	if (ftruncate_fd != EOF)
		close(ftruncate_fd);
	ftruncate_fd = EOF;
	unlink(open_creat_path);
	unlink(mknod_reg_path);
	unlink(mknod_chr_path);
	unlink(mknod_blk_path);
	unlink(mknod_fifo_path);
	unlink(mknod_sock_path);
	rmdir(mkdir_path);
	unlink(symlink_source_path);
	unlink(symlink_dest_path);
	unlink(link_source_path);
	unlink(link_dest_path);
	unlink(rename_source_path);
	unlink(rename_dest_path);
	unlink(truncate_path);
	unlink(ftruncate_path);
	unlink(socket_path);
}

static void set_file_enforce(int enforce)
{
	if (enforce) {
		set_profile(3, "file::execute");
		set_profile(3, "file::open");
		set_profile(3, "file::create");
		set_profile(3, "file::unlink");
		set_profile(3, "file::mkdir");
		set_profile(3, "file::rmdir");
		set_profile(3, "file::mkfifo");
		set_profile(3, "file::mksock");
		set_profile(3, "file::truncate");
		set_profile(3, "file::symlink");
		set_profile(3, "file::rewrite");
		set_profile(3, "file::mkblock");
		set_profile(3, "file::mkchar");
		set_profile(3, "file::link");
		set_profile(3, "file::rename");
		set_profile(3, "file::chmod");
		set_profile(3, "file::chown");
		set_profile(3, "file::chgrp");
		set_profile(3, "file::ioctl");
		set_profile(3, "file::chroot");
		set_profile(3, "file::mount");
		set_profile(3, "file::umount");
		set_profile(3, "file::pivot_root");
	} else {
		set_profile(0, "file::execute");
		set_profile(0, "file::open");
		set_profile(0, "file::create");
		set_profile(0, "file::unlink");
		set_profile(0, "file::mkdir");
		set_profile(0, "file::rmdir");
		set_profile(0, "file::mkfifo");
		set_profile(0, "file::mksock");
		set_profile(0, "file::truncate");
		set_profile(0, "file::symlink");
		set_profile(0, "file::rewrite");
		set_profile(0, "file::mkblock");
		set_profile(0, "file::mkchar");
		set_profile(0, "file::link");
		set_profile(0, "file::rename");
		set_profile(0, "file::chmod");
		set_profile(0, "file::chown");
		set_profile(0, "file::chgrp");
		set_profile(0, "file::ioctl");
		set_profile(0, "file::chroot");
		set_profile(0, "file::mount");
		set_profile(0, "file::umount");
		set_profile(0, "file::pivot_root");
	}
}

int main(int argc, char *argv[])
{
	tomoyo_test_init();

	printf("***** Testing file hooks in enforce mode. *****\n");
	create_files();
	should_fail = 1;
	set_file_enforce(1);
	stage_file_test();
	set_file_enforce(0);
	clear_status();
	creanup_files();

	printf("***** Testing file hooks in permissive mode. *****\n");
	should_fail = 0;
	create_files();
	set_file_enforce(0);
	stage_file_test();
	creanup_files();

	clear_status();
	return 0;
}
