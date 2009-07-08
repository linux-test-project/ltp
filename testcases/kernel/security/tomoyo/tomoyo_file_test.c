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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * tomoyo_file_test.c
 *
 * Testing program for security/tomoyo/
 *
 * Copyright (C) 2005-2009  NTT DATA CORPORATION
 *
 * Version: 2.2.0   2009/06/23
 *
 */
#include "include.h"

static void show_prompt(const char *str)
{
	printf("Testing %35s:", str);
	errno = 0;
}

static void show_result(int result, int should_success)
{
	const int err = errno;
	if (should_success) {
		if (result != EOF)
			printf("OK\n");
		else
			printf("%s\n", strerror(err));
	} else {
		if (result == EOF) {
			if (err == EPERM)
				printf("OK: Permission denied.\n");
			else
				printf("FAILED: %s\n", strerror(err));
		} else {
			printf("BUG!\n");
		}
	}
}

static const char *dev_null_path       = "/dev/null";
static const char *truncate_path       = "/tmp/truncate_test";
static const char *ftruncate_path      = "/tmp/ftruncate_test";
static const char *open_creat_path     = "/tmp/open_test";
static const char *mknod_reg_path      = "/tmp/mknod_reg_test";
static const char *mknod_chr_path      = "/tmp/mknod_chr_test";
static const char *mknod_blk_path      = "/tmp/mknod_blk_test";
static const char *mknod_fifo_path     = "/tmp/mknod_fifo_test";
static const char *mknod_sock_path     = "/tmp/mknod_sock_test";
static const char *unlink_path         = "/tmp/unlink_test";
static const char *mkdir_path          = "/tmp/mkdir_test";
static const char *rmdir_path          = "/tmp/rmdir_test";
static const char *link_source_path    = "/tmp/link_source_test";
static const char *link_dest_path      = "/tmp/link_dest_test";
static const char *symlink_source_path = "/tmp/symlink_source_test";
static const char *symlink_dest_path   = "/tmp/symlink_dest_test";
static const char *rename_source_path  = "/tmp/rename_source_test";
static const char *rename_dest_path    = "/tmp/rename_dest_test";
static const char *socket_path         = "/tmp/socket_test";

static int ftruncate_fd = EOF;

static void stage_file_test(int res)
{
	int fd;
	{
		static int name[] = { CTL_NET, NET_IPV4,
				      NET_IPV4_LOCAL_PORT_RANGE };
		int buffer[2] = { 32768, 61000 };
		size_t size = sizeof(buffer);
		show_prompt("sysctl(READ)");
		show_result(sysctl(name, 3, buffer, &size, 0, 0), res);
		show_prompt("sysctl(WRITE)");
		show_result(sysctl(name, 3, 0, 0, buffer, size), res);
	}

	/* QUESTION: Is there a file which can be passed to uselib()? */
	show_prompt("uselib()");
	show_result(uselib("/bin/true"), res);

	{
		int pipe_fd[2] = { EOF, EOF };
		int err = 0;
		fflush(stdout);
		fflush(stderr);
		pipe(pipe_fd);
		if (fork() == 0) {
			execl("/bin/true", "/bin/true", NULL);
			err = errno;
			write(pipe_fd[1], &err, sizeof(err));
			_exit(0);
		}
		close(pipe_fd[1]);
		read(pipe_fd[0], &err, sizeof(err));
		show_prompt("execve()");
		errno = err;
		show_result(err ? EOF : 0, res);
	}

	show_prompt("open(O_RDONLY)");
	fd = open(dev_null_path, O_RDONLY);
	show_result(fd, res);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_WRONLY)");
	fd = open(dev_null_path, O_WRONLY);
	show_result(fd, res);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_RDWR)");
	fd = open(dev_null_path, O_RDWR);
	show_result(fd, res);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_CREAT | O_EXCL)");
	fd = open(open_creat_path, O_CREAT | O_EXCL, 0666);
	show_result(fd, res);
	if (fd != EOF)
		close(fd);

	show_prompt("open(O_TRUNC)");
	fd = open(truncate_path, O_TRUNC);
	show_result(fd, res);
	if (fd != EOF)
		close(fd);

	show_prompt("truncate()");
	show_result(truncate(truncate_path, 0), res);

	show_prompt("ftruncate()");
	show_result(ftruncate(ftruncate_fd, 0), res);

	show_prompt("mknod(S_IFREG)");
	show_result(mknod(mknod_reg_path, S_IFREG, 0), res);

	show_prompt("mknod(S_IFCHR)");
	show_result(mknod(mknod_chr_path, S_IFCHR, MKDEV(1, 3)), res);

	show_prompt("mknod(S_IFBLK)");
	show_result(mknod(mknod_blk_path, S_IFBLK, MKDEV(1, 0)), res);

	show_prompt("mknod(S_IFIFO)");
	show_result(mknod(mknod_fifo_path, S_IFIFO, 0), res);

	show_prompt("mknod(S_IFSOCK)");
	show_result(mknod(mknod_sock_path, S_IFSOCK, 0), res);

	show_prompt("mkdir()");
	show_result(mkdir(mkdir_path, 0600), res);

	show_prompt("rmdir()");
	show_result(rmdir(rmdir_path), res);

	show_prompt("unlink()");
	show_result(unlink(unlink_path), res);

	show_prompt("symlink()");
	show_result(symlink(symlink_dest_path, symlink_source_path), res);

	show_prompt("link()");
	show_result(link(link_source_path, link_dest_path), res);

	show_prompt("rename()");
	show_result(rename(rename_source_path, rename_dest_path), res);

	{
		struct sockaddr_un addr;
		int fd;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		show_prompt("unix_bind()");
		show_result(bind(fd, (struct sockaddr *) &addr, sizeof(addr)),
			    res);
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
	if (enforce)
		write_profile("255-MAC_FOR_FILE=enforcing\n");
	else
		write_profile("255-MAC_FOR_FILE=permissive\n");
}

static const char *policy = "";

static int write_policy(void)
{
	FILE *fp;
	char buffer[8192];
	char *cp;
	int domain_found = 0;
	int policy_found = 0;
	memset(buffer, 0, sizeof(buffer));
	write_profile("255-MAC_FOR_FILE=disabled\n");
	fp = fopen(proc_policy_domain_policy, "r");
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	fprintf(fp_domain, "%s\n", policy);
	fflush(fp_domain);
	if (!fp) {
		printf("%s : BUG: policy read failed\n", policy);
		return 0;
	}
	while (fgets(buffer, sizeof(buffer) - 1, fp)) {
		cp = strchr(buffer, '\n');
		if (cp)
			*cp = '\0';
		if (!strncmp(buffer, "<kernel>", 8))
			domain_found = !strcmp(self_domain, buffer);
		if (domain_found) {
			/* printf("<%s>\n", buffer); */
			if (!strcmp(buffer, policy)) {
				policy_found = 1;
				break;
			}
		}
	}
	fclose(fp);
	if (!policy_found) {
		printf("%s : BUG: policy write failed\n", policy);
		return 0;
	}
	errno = 0;
	return 1;
}

static void delete_policy(void)
{
	fprintf(fp_domain, "delete %s\n", policy);
	fflush(fp_domain);
	errno = 0;
}

static void show_result2(int result, char should_success)
{
	int err = errno;
	printf("%s : ", policy);
	if (should_success) {
		if (result != EOF)
			printf("OK\n");
		else
			printf("FAILED: %s\n", strerror(err));
	} else {
		if (result == EOF) {
			if (err == EPERM)
				printf("OK: Permission denied.\n");
			else
				printf("FAILED: %s\n", strerror(err));
		} else {
			printf("BUG: didn't fail.\n");
		}
	}
}

static void create2(const char *pathname)
{
	write_profile("255-MAC_FOR_FILE=disabled\n");
	close(creat(pathname, 0600));
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	errno = 0;
}

static void mkdir2(const char *pathname)
{
	write_profile("255-MAC_FOR_FILE=disabled\n");
	mkdir(pathname, 0600);
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	errno = 0;
}

static void unlink2(const char *pathname)
{
	write_profile("255-MAC_FOR_FILE=disabled\n");
	unlink(pathname);
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	errno = 0;
}

static void rmdir2(const char *pathname)
{
	write_profile("255-MAC_FOR_FILE=disabled\n");
	rmdir(pathname);
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	errno = 0;
}

static void mkfifo2(const char *pathname)
{
	write_profile("255-MAC_FOR_FILE=disabled\n");
	mkfifo(pathname, 0600);
	write_profile("255-MAC_FOR_FILE=enforcing\n");
	errno = 0;
}

static void stage_file_test2(void)
{
	char *filename = "";
	policy = "allow_read /proc/sys/net/ipv4/ip_local_port_range";
	if (write_policy()) {
		static int name[] = { CTL_NET, NET_IPV4,
				      NET_IPV4_LOCAL_PORT_RANGE };
		int buffer[2] = { 32768, 61000 };
		size_t size = sizeof(buffer);
		show_result2(sysctl(name, 3, buffer, &size, 0, 0), 1);
		delete_policy();
		show_result2(sysctl(name, 3, buffer, &size, 0, 0), 0);
	}
	policy = "allow_write /proc/sys/net/ipv4/ip_local_port_range";
	if (write_policy()) {
		static int name[] = { CTL_NET, NET_IPV4,
				      NET_IPV4_LOCAL_PORT_RANGE };
		int buffer[2] = { 32768, 61000 };
		size_t size = sizeof(buffer);
		show_result2(sysctl(name, 3, 0, 0, buffer, size), 1);
		delete_policy();
		show_result2(sysctl(name, 3, 0, 0, buffer, size), 0);
	}
	policy = "allow_read/write /proc/sys/net/ipv4/ip_local_port_range";
	if (write_policy()) {
		static int name[] = { CTL_NET, NET_IPV4,
				      NET_IPV4_LOCAL_PORT_RANGE };
		int buffer[2] = { 32768, 61000 };
		size_t size = sizeof(buffer);
		show_result2(sysctl(name, 3, buffer, &size, buffer, size), 1);
		delete_policy();
		show_result2(sysctl(name, 3, buffer, &size, buffer, size), 0);
	}

	policy = "allow_read /bin/true";
	if (write_policy()) {
		show_result2(uselib("/bin/true"), 1);
		delete_policy();
		show_result2(uselib("/bin/true"), 0);
	}

	policy = "allow_execute /bin/true";
	if (write_policy()) {
		int pipe_fd[2] = { EOF, EOF };
		int err = 0;
		fflush(stdout);
		fflush(stderr);
		pipe(pipe_fd);
		if (fork() == 0) {
			execl("/bin/true", "/bin/true", NULL);
			err = errno;
			write(pipe_fd[1], &err, sizeof(err));
			_exit(0);
		}
		close(pipe_fd[1]);
		read(pipe_fd[0], &err, sizeof(err));
		close(pipe_fd[0]);
		wait(NULL);
		errno = err;
		show_result2(err ? EOF : 0, 1);
		delete_policy();
		fflush(stdout);
		fflush(stderr);
		pipe(pipe_fd);
		if (fork() == 0) {
			execl("/bin/true", "/bin/true", NULL);
			err = errno;
			write(pipe_fd[1], &err, sizeof(err));
			_exit(0);
		}
		close(pipe_fd[1]);
		read(pipe_fd[0], &err, sizeof(err));
		close(pipe_fd[0]);
		wait(NULL);
		errno = err;
		show_result2(err ? EOF : 0, 0);
	}

	policy = "allow_read /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_read /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_read /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_read /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_mkfifo /tmp/mknod_fifo_test";
	if (write_policy()) {
		filename = "/tmp/mknod_fifo_test";
		show_result2(mknod(filename, S_IFIFO, 0), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFIFO, 0), 0);
	}

	{
		char buffer[1024];
		struct stat sbuf;
		memset(buffer, 0, sizeof(buffer));
		memset(&sbuf, 0, sizeof(sbuf));
		filename = "/dev/null";
		stat(filename, &sbuf);
		snprintf(buffer, sizeof(buffer) - 1, "allow_write %s",
			 filename);
		policy = buffer;
		if (write_policy()) {
			int fd = open(filename, O_WRONLY);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);
			delete_policy();
			fd = open(filename, O_WRONLY);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);
		}
	}

	policy = "allow_read/write /tmp/fifo";
	mkfifo2("/tmp/fifo");
	if (write_policy()) {
		int fd = open("/tmp/fifo", O_RDWR);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/tmp/fifo", O_RDWR);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_read /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_write /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_WRONLY);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_WRONLY);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_read/write /dev/null";
	if (write_policy()) {
		int fd = open("/dev/null", O_RDWR);
		show_result2(fd, 1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		fd = open("/dev/null", O_RDWR);
		show_result2(fd, 0);
		if (fd != EOF)
			close(fd);
	}

	policy = "allow_create /tmp/open_test";
	if (write_policy()) {
		policy = "allow_write /tmp/open_test";
		if (write_policy()) {
			int fd = open("/tmp/open_test",
				      O_WRONLY | O_CREAT | O_EXCL, 0666);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);
			unlink2("/tmp/open_test");
			delete_policy();
			fd = open("/tmp/open_test",
				  O_WRONLY | O_CREAT | O_EXCL, 0666);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);
			unlink2("/tmp/open_test");
		}
		policy = "allow_create /tmp/open_test";
		delete_policy();
	}

	policy = "allow_write /tmp/open_test";
	if (write_policy()) {
		policy = "allow_create /tmp/open_test";
		if (write_policy()) {
			int fd = open("/tmp/open_test",
				      O_WRONLY | O_CREAT | O_EXCL, 0666);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);
			unlink2("/tmp/open_test");
			delete_policy();
			fd = open("/tmp/open_test",
				  O_WRONLY | O_CREAT | O_EXCL, 0666);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);
			unlink2("/tmp/open_test");
		}
		policy = "allow_write /tmp/open_test";
		delete_policy();
	}

	filename = "/tmp/truncate_test";
	create2(filename);

	policy = "allow_truncate /tmp/truncate_test";
	if (write_policy()) {
		policy = "allow_write /tmp/truncate_test";
		if (write_policy()) {
			int fd = open(filename, O_WRONLY | O_TRUNC);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);
			delete_policy();
			fd = open(filename, O_WRONLY | O_TRUNC);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);
		}
		policy = "allow_truncate /tmp/truncate_test";
		delete_policy();
	}

	policy = "allow_write /tmp/truncate_test";
	if (write_policy()) {
		policy = "allow_truncate /tmp/truncate_test";
		if (write_policy()) {
			int fd = open(filename, O_WRONLY | O_TRUNC);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);
			delete_policy();
			fd = open(filename, O_WRONLY | O_TRUNC);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);
		}
		policy = "allow_write /tmp/truncate_test";
		delete_policy();
	}

	policy = "allow_truncate /tmp/truncate_test";
	if (write_policy()) {
		show_result2(truncate(filename, 0), 1);
		delete_policy();
		show_result2(truncate(filename, 0), 0);
	}

	policy = "allow_truncate /tmp/truncate_test";
	if (write_policy()) {
		int fd;
		write_profile("255-MAC_FOR_FILE=disabled\n");
		fd = open(filename, O_WRONLY);
		write_profile("255-MAC_FOR_FILE=enforcing\n");
		errno = 0;
		show_result2(ftruncate(fd, 0), 1);
		delete_policy();
		show_result2(ftruncate(fd, 0), 0);
		if (fd != EOF)
			close(fd);
	}

	unlink2(filename);

	policy = "allow_create /tmp/mknod_reg_test";
	if (write_policy()) {
		filename = "/tmp/mknod_reg_test";
		show_result2(mknod(filename, S_IFREG, 0), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFREG, 0), 0);
	}

	policy = "allow_mkchar /tmp/mknod_chr_test";
	if (write_policy()) {
		filename = "/tmp/mknod_chr_test";
		show_result2(mknod(filename, S_IFCHR, MKDEV(1, 3)), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFCHR, MKDEV(1, 3)), 0);
	}

	policy = "allow_mkblock /tmp/mknod_blk_test";
	if (write_policy()) {
		filename = "/tmp/mknod_blk_test";
		show_result2(mknod(filename, S_IFBLK, MKDEV(1, 0)), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFBLK, MKDEV(1, 0)), 0);
	}

	policy = "allow_mkfifo /tmp/mknod_fifo_test";
	if (write_policy()) {
		filename = "/tmp/mknod_fifo_test";
		show_result2(mknod(filename, S_IFIFO, 0), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFIFO, 0), 0);
	}

	policy = "allow_mksock /tmp/mknod_sock_test";
	if (write_policy()) {
		filename = "/tmp/mknod_sock_test";
		show_result2(mknod(filename, S_IFSOCK, 0), 1);
		delete_policy();
		unlink2(filename);
		show_result2(mknod(filename, S_IFSOCK, 0), 0);
	}

	policy = "allow_mkdir /tmp/mkdir_test/";
	if (write_policy()) {
		filename = "/tmp/mkdir_test";
		show_result2(mkdir(filename, 0600), 1);
		delete_policy();
		rmdir2(filename);
		show_result2(mkdir(filename, 0600), 0);
	}

	policy = "allow_rmdir /tmp/rmdir_test/";
	if (write_policy()) {
		filename = "/tmp/rmdir_test";
		mkdir2(filename);
		show_result2(rmdir(filename), 1);
		delete_policy();
		mkdir2(filename);
		show_result2(rmdir(filename), 0);
		rmdir2(filename);
	}

	policy = "allow_unlink /tmp/unlink_test";
	if (write_policy()) {
		filename = "/tmp/unlink_test";
		create2(filename);
		show_result2(unlink(filename), 1);
		delete_policy();
		create2(filename);
		show_result2(unlink(filename), 0);
		unlink2(filename);
	}

	policy = "allow_symlink /tmp/symlink_source_test";
	if (write_policy()) {
		filename = "/tmp/symlink_source_test";
		show_result2(symlink("/tmp/symlink_dest_test", filename), 1);
		delete_policy();
		unlink2(filename);
		show_result2(symlink("/tmp/symlink_dest_test", filename), 0);
	}

	policy = "allow_link /tmp/link_source_test /tmp/link_dest_test";
	if (write_policy()) {
		filename = "/tmp/link_source_test";
		create2(filename);
		show_result2(link(filename, "/tmp/link_dest_test"), 1);
		delete_policy();
		unlink2("/tmp/link_dest_test");
		show_result2(link(filename, "/tmp/link_dest_test"), 0);
		unlink2(filename);
	}

	policy = "allow_rename /tmp/rename_source_test /tmp/rename_dest_test";
	if (write_policy()) {
		filename = "/tmp/rename_source_test";
		create2(filename);
		show_result2(rename(filename, "/tmp/rename_dest_test"), 1);
		delete_policy();
		unlink2("/tmp/rename_dest_test");
		create2(filename);
		show_result2(rename(filename, "/tmp/rename_dest_test"), 0);
		unlink2(filename);
	}

	policy = "allow_mksock /tmp/socket_test";
	if (write_policy()) {
		struct sockaddr_un addr;
		int fd;
		filename = "/tmp/socket_test";
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, filename, sizeof(addr.sun_path) - 1);
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		show_result2(bind(fd, (struct sockaddr *) &addr, sizeof(addr)),
			     1);
		if (fd != EOF)
			close(fd);
		delete_policy();
		unlink2(filename);
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		show_result2(bind(fd, (struct sockaddr *) &addr, sizeof(addr)),
			     0);
		if (fd != EOF)
			close(fd);
	}

	filename = "/tmp/rewrite_test";
	create2(filename);
	policy = "allow_read/write /tmp/rewrite_test";
	if (write_policy()) {
		fprintf(fp_exception, "deny_rewrite /tmp/rewrite_test\n");
		fflush(fp_exception);
		policy = "allow_truncate /tmp/rewrite_test";
		if (write_policy()) {
			int fd;

			fd = open(filename, O_RDONLY);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);

			fd = open(filename, O_WRONLY | O_APPEND);
			show_result2(fd, 1);
			if (fd != EOF)
				close(fd);

			fd = open(filename, O_WRONLY);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);

			fd = open(filename, O_WRONLY | O_TRUNC);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);

			fd = open(filename, O_WRONLY | O_TRUNC | O_APPEND);
			show_result2(fd, 0);
			if (fd != EOF)
				close(fd);

			show_result2(truncate(filename, 0), 0);

			write_profile("255-MAC_FOR_FILE=disabled\n");
			fd = open(filename, O_WRONLY | O_APPEND);
			write_profile("255-MAC_FOR_FILE=enforcing\n");
			show_result2(ftruncate(fd, 0), 0);
			show_result2(fcntl(fd, F_SETFL,
					   fcntl(fd, F_GETFL) & ~O_APPEND), 0);
			if (fd != EOF)
				close(fd);

			delete_policy();
		}
		policy = "allow_read/write /tmp/rewrite_test";
		delete_policy();
		fprintf(fp_exception, "delete deny_rewrite "
			"/tmp/rewrite_test\n");
		fflush(fp_exception);

	}
	unlink2(filename);
}

static void add_domain_policy(const char *data)
{
	set_file_enforce(0);
	fprintf(fp_domain, "%s\n", self_domain);
	fprintf(fp_domain, "%s\n", data);
	fflush(fp_domain);
}

static void add_exception_policy(const char *data)
{
	set_file_enforce(0);
	fprintf(fp_exception, "%s\n", data);
	fflush(fp_exception);
}

#define REWRITE_PATH "/tmp/rewrite_test"

static void stage_rewrite_test(void)
{
	int fd;

	/* Start up */
	add_domain_policy("allow_read/write " REWRITE_PATH);
	add_domain_policy("allow_truncate " REWRITE_PATH);
	add_domain_policy("allow_create " REWRITE_PATH);
	add_domain_policy("allow_unlink " REWRITE_PATH);
	add_exception_policy("deny_rewrite " REWRITE_PATH);
	close(open(REWRITE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0600));

	/* Enforce mode */
	set_file_enforce(1);

	show_prompt("open(O_RDONLY)");
	fd = open(REWRITE_PATH, O_RDONLY);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY)");
	fd = open(REWRITE_PATH, O_WRONLY);
	show_result(fd, 0);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC);
	show_result(fd, 0);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC | O_APPEND);
	show_result(fd, 0);
	close(fd);

	show_prompt("truncate()");
	show_result(truncate(REWRITE_PATH, 0), 0);

	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_prompt("ftruncate()");
	show_result(ftruncate(fd, 0), 0);

	show_prompt("fcntl(F_SETFL, ~O_APPEND)");
	show_result(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_APPEND), 0);
	close(fd);

	/* Permissive mode */
	set_file_enforce(0);

	show_prompt("open(O_RDONLY)");
	fd = open(REWRITE_PATH, O_RDONLY);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY)");
	fd = open(REWRITE_PATH, O_WRONLY);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC);
	show_result(fd, 1);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC | O_APPEND);
	show_result(fd, 1);
	close(fd);

	show_prompt("truncate()");
	show_result(truncate(REWRITE_PATH, 0), 1);

	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_prompt("ftruncate()");
	show_result(ftruncate(fd, 0), 1);

	show_prompt("fcntl(F_SETFL, ~O_APPEND)");
	show_result(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_APPEND), 1);
	close(fd);

	/* Clean up */
	unlink(REWRITE_PATH);
	add_exception_policy("delete " "deny_rewrite " REWRITE_PATH);
	printf("\n\n");
}

static void set_level(const int i)
{
	fprintf(fp_profile, "255-MAC_FOR_FILE=%d\n", i);
	fflush(fp_profile);
}

static void test(int rw_loop, int truncate_loop, int append_loop,
		 int create_loop)
{
	static const int rw_flags[4] = { 0, O_RDONLY, O_WRONLY, O_RDWR };
	static const int create_flags[3] = { 0, O_CREAT /* nonexistent*/ ,
					     O_CREAT /* existent */ };
	static const int truncate_flags[2] = { 0, O_TRUNC };
	static const int append_flags[2] = { 0, O_APPEND };
	int level;
	int flags;
	int i;
	int fd;
	static char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, sizeof(buffer) - 1, "/tmp/file:a=%d:t=%d:c=%d:m=%d",
		 append_loop, truncate_loop, create_loop, rw_loop);
	fprintf(fp_exception, "deny_rewrite %s\n", buffer);
	fflush(fp_exception);
	flags = rw_flags[rw_loop] | truncate_flags[truncate_loop] |
		append_flags[append_loop] | create_flags[create_loop];
	for (i = 1; i < 8; i++)
		fprintf(fp_domain, "delete %d %s\n", i, buffer);
	fflush(fp_domain);
	for (level = 0; level < 4; level++) {
		set_level(0);
		if (create_loop == 1)
			unlink(buffer);
		else
			close(open(buffer, O_CREAT, 0666));
		set_level(level);
		fd = open(buffer, flags, 0666);
		if (fd != EOF)
			close(fd);
		else
			fprintf(stderr, "%d: open(%04o) failed\n", level,
				flags);
	}
	for (i = 1; i < 8; i++)
		fprintf(fp_domain, "delete %d %s\n", i, buffer);
	fprintf(fp_domain, "delete allow_truncate %s\n", buffer);
	fprintf(fp_domain, "delete allow_create %s\n", buffer);
	fprintf(fp_domain, "delete allow_rewrite %s\n", buffer);
	fflush(fp_domain);
	fd = open(buffer, flags, 0666);
	if (fd != EOF) {
		close(fd);
		fprintf(stderr, "%d: open(%04o) didn't fail\n", 3, flags);
	}
}

static void stage_old_rewrite_test(void)
{
	write_profile("255-COMMENT=Test\n255-TOMOYO_VERBOSE=disabled\n"
		"255-MAC_FOR_FILE=disabled\n255-MAX_ACCEPT_ENTRY=2048\n");
	fprintf(fp_domain, "%s\n", self_domain);
	fprintf(fp_domain, "use_profile 255\n");
	fflush(fp_domain);

	{
		int append_loop;
		for (append_loop = 0; append_loop < 2; append_loop++) {
			int truncate_loop;
			for (truncate_loop = 0; truncate_loop < 2;
			     truncate_loop++) {
				int create_loop;
				for (create_loop = 0; create_loop < 3;
				     create_loop++) {
					int rw_loop;
					for (rw_loop = 0; rw_loop < 4;
					     rw_loop++)
						test(rw_loop, truncate_loop,
						     append_loop, create_loop);
				}
			}
		}
	}
	write_profile("255-MAC_FOR_FILE=disabled\n");
}

int main(int argc, char *argv[])
{
	ccs_test_init();

	printf("***** Testing file access in enforce mode. *****\n");
	create_files();
	set_file_enforce(1);
	stage_file_test(0);
	set_file_enforce(0);
	clear_status();
	creanup_files();

	printf("***** Testing file access in permissive mode. *****\n");
	create_files();
	set_file_enforce(0);
	stage_file_test(1);
	creanup_files();

	printf("***** Testing file access for rewrite operations. *****\n");
	stage_rewrite_test();
	stage_old_rewrite_test();

	fprintf(fp_domain, "%s /bin/true\n", self_domain);
	fprintf(fp_domain, "%s\nuse_profile 255\n", self_domain);
	fflush(fp_domain);

	printf("***** Testing file access with policy. *****\n");
	stage_file_test2();

	fprintf(fp_domain, "%s\nuse_profile 0\n", self_domain);
	fflush(fp_domain);

	clear_status();
	return 0;
}
