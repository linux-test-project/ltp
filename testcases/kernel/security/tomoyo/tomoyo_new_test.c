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
 * tomoyo_new_test.c
 *
 * Testing program for security/tomoyo/
 *
 * Copyright (C) 2005-2010  NTT DATA CORPORATION
 */
#include "include.h"

static int result;
static int error;

static void show_result(const char *test, int should_success)
{
	error = errno;
	printf("%s : ", test);
	if (should_success) {
		if (error == 0)
			printf("OK (%d)\n", result);
		else
			printf("FAILED: %s\n", strerror(error));
	} else {
		if (error == 0)
			printf("BUG: Didn't fail (%d)\n", result);
		else if (error == EPERM)
			printf("OK: permission denied\n");
		else
			printf("FAILED: %s\n", strerror(error));
	}
}

static void test_read_etc_fstab(void)
{
	result = open("/etc/fstab", O_RDONLY);
}

static void test_write_dev_null(void)
{
	result = open("/dev/null", O_WRONLY);
}

static void cleanup_file_open(void)
{
	if (result != EOF)
		close(result);
}

static void test_mkdir_testdir(void)
{
	result = mkdir("/tmp/testdir", 0755);
}

static void cleanup_mkdir_testdir(void)
{
	rmdir("/tmp/testdir");
}

static void setup_mkdir_testdir(void)
{
	mkdir("/tmp/testdir", 0755);
}

static void test_rmdir_testdir(void)
{
	result = rmdir("/tmp/testdir");
}

static void setup_execute_bin_true(void)
{
	fprintf(domain_fp, "%s /bin/true\n", self_domain);
	fprintf(domain_fp, "use_profile 0\n");
	fprintf(domain_fp, "select pid=%u\n", pid);
}

static void cleanup_execute_bin_true(void)
{
	wait(NULL);
	fprintf(domain_fp, "delete %s /bin/true\n", self_domain);
	fprintf(domain_fp, "select pid=%u\n", pid);
}

static void test_execute_bin_true(void)
{
	char *argv[] = { "/bin/true", NULL };
	char *envp[] = { "HOME=/", NULL };
	int pipe_fd[2] = { EOF, EOF };
	if (pipe(pipe_fd) == -1)
		err(1, "pipe");
	switch (fork()) {
	case 0:
		execve("/bin/true", argv, envp);
		error = errno;
		if (write(pipe_fd[1], &error, sizeof(error)) == -1)
			err(1, "write");
		_exit(0);
		break;
	case -1:
		error = ENOMEM;
		break;
	}
	close(pipe_fd[1]);
	(void)read(pipe_fd[0], &error, sizeof(error));
	close(pipe_fd[0]);
	result = error ? EOF : 0;
	errno = error;
}

static void test_chmod_dev_null(void)
{
	result = chmod("/dev/null", 0666);
}

static void test_chown_dev_null(void)
{
	result = chown("/dev/null", 0, -1);
}

static void test_chgrp_dev_null(void)
{
	result = chown("/dev/null", -1, 0);
}

static void test_ioctl_dev_null(void)
{
	int fd = open("/dev/null", O_RDWR);
	errno = 0;
	result = ioctl(fd, 0x5451, NULL);
	error = errno;
	close(fd);
	errno = error;
}

static void setup_chmod_group(void)
{
	write_exception_policy("path_group CHMOD_TARGET /dev/null", 0);
	write_exception_policy("number_group CHMOD_MODES 0666", 0);
}

static void cleanup_chmod_group(void)
{
	write_exception_policy("path_group CHMOD_TARGET /dev/null", 1);
	write_exception_policy("number_group CHMOD_MODES 0666", 1);
}

static void setup_chown_group(void)
{
	write_exception_policy("path_group CHOWN_TARGET /dev/\\*", 0);
	write_exception_policy("number_group CHOWN_IDS 0x0-0xFFFE", 0);
}

static void cleanup_chown_group(void)
{
	write_exception_policy("path_group CHOWN_TARGET /dev/\\*", 1);
	write_exception_policy("number_group CHOWN_IDS 0x0-0xFFFE", 1);
}

static void setup_ioctl_group(void)
{
	write_exception_policy("path_group IOCTL_TARGET /dev/\\*", 0);
	write_exception_policy("number_group IOCTL_NUMBERS 0x5450-0x5452", 0);
}

static void cleanup_ioctl_group(void)
{
	write_exception_policy("path_group IOCTL_TARGET /dev/\\*", 1);
	write_exception_policy("number_group IOCTL_NUMBERS 0x5450-0x5452", 1);
}

static void setup_open_group(void)
{
	write_exception_policy("path_group READABLE /etc/\\*", 0);
	write_exception_policy("number_group READABLE_IDS 0-0xFFF", 0);
}

static void cleanup_open_group(void)
{
	cleanup_file_open();
	write_exception_policy("path_group READABLE /etc/\\*", 1);
	write_exception_policy("number_group READABLE_IDS 0-0xFFF", 1);
}

static void test_file_open_0(void)
{
	result = open("/tmp/testfile0", O_RDONLY, 0600);
}

static void test_file_open_1(void)
{
	result = open("/tmp/testfile1", O_CREAT | O_RDONLY, 0600);
}

static void test_file_open_2(void)
{
	result = open("/tmp/testfile2", O_TRUNC | O_RDONLY, 0600);
}

static void test_file_open_3(void)
{
	result = open("/tmp/testfile3", O_TRUNC | O_CREAT | O_RDONLY, 0600);
}

static void test_file_open_4(void)
{
	result = open("/tmp/testfile4", O_APPEND | O_RDONLY, 0600);
}

static void test_file_open_5(void)
{
	result = open("/tmp/testfile5", O_APPEND | O_CREAT | O_RDONLY, 0600);
}

static void test_file_open_6(void)
{
	result = open("/tmp/testfile6", O_APPEND | O_TRUNC | O_RDONLY, 0600);
}

static void test_file_open_7(void)
{
	result = open("/tmp/testfile7",
		      O_APPEND | O_TRUNC | O_CREAT | O_RDONLY, 0600);
}

static void test_file_open_8(void)
{
	result = open("/tmp/testfile8", O_WRONLY, 0600);
}

static void test_file_open_9(void)
{
	result = open("/tmp/testfile9", O_CREAT | O_WRONLY, 0600);
}

static void test_file_open_10(void)
{
	result = open("/tmp/testfile10", O_TRUNC | O_WRONLY, 0600);
}

static void test_file_open_11(void)
{
	result = open("/tmp/testfile11", O_TRUNC | O_CREAT | O_WRONLY, 0600);
}

static void test_file_open_12(void)
{
	result = open("/tmp/testfile12", O_APPEND | O_WRONLY, 0600);
}

static void test_file_open_13(void)
{
	result = open("/tmp/testfile13", O_APPEND | O_CREAT | O_WRONLY, 0600);
}

static void test_file_open_14(void)
{
	result = open("/tmp/testfile14", O_APPEND | O_TRUNC | O_WRONLY, 0600);
}

static void test_file_open_15(void)
{
	result = open("/tmp/testfile15",
		      O_APPEND | O_TRUNC | O_CREAT | O_WRONLY, 0600);
}

static void test_file_open_16(void)
{
	result = open("/tmp/testfile16", O_RDWR, 0600);
}

static void test_file_open_17(void)
{
	result = open("/tmp/testfile17", O_CREAT | O_RDWR, 0600);
}

static void test_file_open_18(void)
{
	result = open("/tmp/testfile18", O_TRUNC | O_RDWR, 0600);
}

static void test_file_open_19(void)
{
	result = open("/tmp/testfile19", O_TRUNC | O_CREAT | O_RDWR, 0600);
}

static void test_file_open_20(void)
{
	result = open("/tmp/testfile20", O_APPEND | O_RDWR, 0600);
}

static void test_file_open_21(void)
{
	result = open("/tmp/testfile21", O_APPEND | O_CREAT | O_RDWR, 0600);
}

static void test_file_open_22(void)
{
	result = open("/tmp/testfile22", O_APPEND | O_TRUNC | O_RDWR, 0600);
}

static void test_file_open_23(void)
{
	result = open("/tmp/testfile23", O_APPEND | O_TRUNC | O_CREAT | O_RDWR,
		      0600);
}

static void setup_test_file(void)
{
	int i;
	char buffer[32];
	buffer[31] = '\0';
	for (i = 0; i < 24; i += 2) {
		snprintf(buffer, sizeof(buffer) - 1, "/tmp/testfile%u", i);
		close(open(buffer, O_WRONLY | O_CREAT, 0600));
	}
	write_exception_policy("deny_rewrite /tmp/testfile\\$", 0);
}

static void setup_test_file_truncate(void)
{
	setup_test_file();
	write_domain_policy("allow_truncate /tmp/testfile\\$", 0);
	set_profile(3, "file::truncate");
}

static void setup_all_test_file(void)
{
	int i;
	char buffer[32];
	buffer[31] = '\0';
	for (i = 0; i < 24; i++) {
		snprintf(buffer, sizeof(buffer) - 1, "/tmp/testfile%u", i);
		close(open(buffer, O_WRONLY | O_CREAT, 0600));
	}
	write_exception_policy("deny_rewrite /tmp/testfile\\$", 0);
}

static void setup_all_test_file_truncate(void)
{
	setup_all_test_file();
	write_domain_policy("allow_truncate /tmp/testfile\\$", 0);
	set_profile(3, "file::truncate");
}

static void cleanup_test_file(void)
{
	int i;
	char buffer[32];
	buffer[31] = '\0';
	for (i = 0; i < 24; i++) {
		snprintf(buffer, sizeof(buffer) - 1, "/tmp/testfile%u", i);
		unlink(buffer);
	}
	write_exception_policy("deny_rewrite /tmp/testfile\\$", 1);
	cleanup_file_open();
}

static void cleanup_test_file_truncate(void)
{
	cleanup_test_file();
	write_domain_policy("allow_truncate /tmp/testfile\\$", 1);
	set_profile(0, "file::truncate");
}

static struct test_struct {
	void (*do_setup) (void);
	void (*do_test) (void);
	void (*do_cleanup) (void);
	const char *name;
	const char *policy;
} tests[] = {
	{
	NULL, test_read_etc_fstab, cleanup_file_open, "file::open",
		    "allow_read /etc/fstab"}, {
	NULL, test_read_etc_fstab, cleanup_file_open, "file::open",
		    "allow_read /etc/fstab"}, {
	NULL, test_read_etc_fstab, cleanup_file_open, "file::open",
		    "allow_read /etc/fstab"}, {
	setup_open_group, test_read_etc_fstab, cleanup_open_group,
		    "file::open", "allow_read @READABLE"}, {
	NULL, test_write_dev_null, cleanup_file_open, "file::open",
		    "allow_write /dev/null"}, {
	NULL, test_write_dev_null, cleanup_file_open, "file::open",
		    "allow_write /dev/null"}, {
	NULL, test_write_dev_null, cleanup_file_open, "file::open",
		    "allow_write /dev/null"}, {
	cleanup_mkdir_testdir, test_mkdir_testdir,
		    cleanup_mkdir_testdir, "file::mkdir",
		    "allow_mkdir /tmp/testdir/ 0755"}, {
	cleanup_mkdir_testdir, test_mkdir_testdir,
		    cleanup_mkdir_testdir, "file::mkdir",
		    "allow_mkdir /tmp/testdir/ 0755"}, {
	cleanup_mkdir_testdir, test_mkdir_testdir,
		    cleanup_mkdir_testdir, "file::mkdir",
		    "allow_mkdir /tmp/testdir/ 0755"}, {
	setup_mkdir_testdir, test_rmdir_testdir, cleanup_mkdir_testdir,
		    "file::rmdir", "allow_rmdir /tmp/testdir/"}, {
	setup_mkdir_testdir, test_rmdir_testdir, cleanup_mkdir_testdir,
		    "file::rmdir", "allow_rmdir /tmp/testdir/"}, {
	setup_mkdir_testdir, test_rmdir_testdir, cleanup_mkdir_testdir,
		    "file::rmdir", "allow_rmdir /tmp/testdir/"}, {
	setup_execute_bin_true, test_execute_bin_true,
		    cleanup_execute_bin_true, "file::execute",
		    "allow_execute /bin/true"}, {
	setup_execute_bin_true, test_execute_bin_true,
		    cleanup_execute_bin_true, "file::execute",
		    "allow_execute /bin/true"}, {
	setup_execute_bin_true, test_execute_bin_true,
		    cleanup_execute_bin_true, "file::execute",
		    "allow_execute /bin/true"}, {
	NULL, test_chmod_dev_null, NULL, "file::chmod",
		    "allow_chmod /dev/null 0666"}, {
	NULL, test_chown_dev_null, NULL, "file::chown",
		    "allow_chown /dev/null 0"}, {
	NULL, test_chgrp_dev_null, NULL, "file::chgrp",
		    "allow_chgrp /dev/null 0"}, {
	NULL, test_ioctl_dev_null, NULL, "file::ioctl",
		    "allow_ioctl /dev/null 0x5451"}, {
	setup_chmod_group, test_chmod_dev_null, cleanup_chmod_group,
		    "file::chmod", "allow_chmod @CHMOD_TARGET @CHMOD_MODES"}, {
	setup_chown_group, test_chown_dev_null, cleanup_chown_group,
		    "file::chown", "allow_chown @CHOWN_TARGET @CHOWN_IDS"}, {
	setup_chown_group, test_chgrp_dev_null, cleanup_chown_group,
		    "file::chgrp", "allow_chgrp @CHOWN_TARGET @CHOWN_IDS"}, {
	setup_ioctl_group, test_ioctl_dev_null, cleanup_ioctl_group,
		    "file::ioctl", "allow_ioctl @IOCTL_TARGET @IOCTL_NUMBERS"},
	{
	setup_test_file, test_file_open_0, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile0"}, {
	setup_test_file, test_file_open_1, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile1"}, {
	setup_test_file, test_file_open_1, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile1 0600"}, {
	setup_test_file, test_file_open_2, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile2"}, {
	setup_test_file, test_file_open_2, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile2"}, {
	setup_test_file_truncate, test_file_open_2,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile2"}, {
	setup_test_file, test_file_open_3, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile3"}, {
	setup_test_file, test_file_open_3, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile3 0600"}, {
	setup_test_file, test_file_open_4, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile4"}, {
	setup_test_file, test_file_open_5, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile5"}, {
	setup_test_file, test_file_open_5, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile5 0600"}, {
	setup_test_file, test_file_open_6, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile6"}, {
	setup_test_file, test_file_open_6, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile6"}, {
	setup_test_file_truncate, test_file_open_6,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile6"}, {
	setup_test_file, test_file_open_7, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile7"}, {
	setup_test_file, test_file_open_7, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile7 0600"}, {
	setup_test_file, test_file_open_8, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile8"}, {
	setup_test_file, test_file_open_8, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile8"}, {
	setup_test_file, test_file_open_9, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile9"}, {
	setup_test_file, test_file_open_9, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile9 0600"}, {
	setup_test_file, test_file_open_9, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile9"}, {
	setup_test_file, test_file_open_10, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile10"}, {
	setup_test_file, test_file_open_10, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile10"}, {
	setup_test_file, test_file_open_10, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile10"}, {
	setup_test_file, test_file_open_11, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile11"}, {
	setup_test_file, test_file_open_11, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile11 0600"}, {
	setup_test_file, test_file_open_11, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile11"}, {
	setup_test_file, test_file_open_12, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile12"}, {
	setup_test_file, test_file_open_13, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile13"}, {
	setup_test_file, test_file_open_13, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile13 0600"}, {
	setup_test_file, test_file_open_14, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile14"}, {
	setup_test_file, test_file_open_14, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile14"}, {
	setup_test_file_truncate, test_file_open_14,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile14"}, {
	setup_test_file, test_file_open_15, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile15"}, {
	setup_test_file, test_file_open_15, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile15 0600"}, {
	setup_test_file, test_file_open_16, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile16"}, {
	setup_test_file, test_file_open_16, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile16"}, {
	setup_test_file, test_file_open_17, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile17"}, {
	setup_test_file, test_file_open_17, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile17 0600"}, {
	setup_test_file, test_file_open_17, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile17"}, {
	setup_test_file, test_file_open_18, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile18"}, {
	setup_test_file, test_file_open_18, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile18"}, {
	setup_test_file, test_file_open_18, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile18"}, {
	setup_test_file, test_file_open_19, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile19"}, {
	setup_test_file, test_file_open_19, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile19 0600"}, {
	setup_test_file, test_file_open_19, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile19"}, {
	setup_test_file, test_file_open_20, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile20"}, {
	setup_test_file, test_file_open_21, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile21"}, {
	setup_test_file, test_file_open_21, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile21 0600"}, {
	setup_test_file, test_file_open_22, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile22"}, {
	setup_test_file, test_file_open_22, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile22"}, {
	setup_test_file_truncate, test_file_open_22,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile22"}, {
	setup_test_file, test_file_open_23, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile23"}, {
	setup_test_file, test_file_open_23, cleanup_test_file,
		    "file::create", "allow_create /tmp/testfile23 0600"}, {
	setup_all_test_file, test_file_open_0, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile0"}, {
	setup_all_test_file, test_file_open_2, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile2"}, {
	setup_all_test_file, test_file_open_2, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile2"}, {
	setup_all_test_file_truncate, test_file_open_2,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile2"}, {
	setup_all_test_file, test_file_open_4, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile4"}, {
	setup_all_test_file, test_file_open_6, cleanup_test_file,
		    "file::open", "allow_read /tmp/testfile6"}, {
	setup_all_test_file, test_file_open_6, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile6"}, {
	setup_all_test_file_truncate, test_file_open_6,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile6"}, {
	setup_all_test_file, test_file_open_8, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile8"}, {
	setup_all_test_file, test_file_open_8, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile8"}, {
	setup_all_test_file, test_file_open_10, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile10"}, {
	setup_all_test_file, test_file_open_10, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile10"}, {
	setup_all_test_file, test_file_open_10, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile10"}, {
	setup_all_test_file, test_file_open_12, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile12"}, {
	setup_all_test_file, test_file_open_14, cleanup_test_file,
		    "file::open", "allow_write /tmp/testfile14"}, {
	setup_all_test_file, test_file_open_14, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile14"}, {
	setup_all_test_file_truncate, test_file_open_14,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile14"}, {
	setup_all_test_file, test_file_open_16, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile16"}, {
	setup_all_test_file, test_file_open_16, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile16"}, {
	setup_all_test_file, test_file_open_18, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile18"}, {
	setup_all_test_file, test_file_open_18, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile18"}, {
	setup_all_test_file, test_file_open_18, cleanup_test_file,
		    "file::rewrite", "allow_rewrite /tmp/testfile18"}, {
	setup_all_test_file, test_file_open_20, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile20"}, {
	setup_all_test_file, test_file_open_22, cleanup_test_file,
		    "file::open", "allow_read/write /tmp/testfile22"}, {
	setup_all_test_file, test_file_open_22, cleanup_test_file,
		    "file::truncate", "allow_truncate /tmp/testfile22"}, {
	setup_all_test_file_truncate, test_file_open_22,
		    cleanup_test_file_truncate, "file::rewrite",
		    "allow_rewrite /tmp/testfile22"}, {
	NULL}
};

int main(int argc, char *argv[])
{
	int i;
	tomoyo_test_init();
	for (i = 0; tests[i].do_test; i++) {
		int trial;
		for (trial = 0; trial < 2; trial++) {
			int should_fail;
			for (should_fail = 0; should_fail < 2; should_fail++) {
				if (tests[i].do_setup)
					tests[i].do_setup();
				if (!should_fail)
					write_domain_policy(tests[i].policy, 0);
				set_profile(3, tests[i].name);
				tests[i].do_test();
				show_result(tests[i].policy, !should_fail);
				set_profile(0, tests[i].name);
				if (tests[i].do_cleanup)
					tests[i].do_cleanup();
				if (!should_fail)
					write_domain_policy(tests[i].policy, 1);
			}
		}
	}
	for (i = 0; tests[i].do_test; i++) {
		int mode;
		for (mode = 0; mode < 4; mode++) {
			if (tests[i].do_setup)
				tests[i].do_setup();
			set_profile(mode, tests[i].name);
			tests[i].do_test();
			show_result(tests[i].name, 1);
			set_profile(0, tests[i].name);
			if (tests[i].do_cleanup)
				tests[i].do_cleanup();
		}
	}
	fprintf(domain_fp, "delete %s\n", self_domain);
	return 0;
}
