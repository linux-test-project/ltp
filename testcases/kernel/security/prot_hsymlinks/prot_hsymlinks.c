/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author:
 * Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Test checks following preconditions:
 *
 * Symlinks
 * ---------
 * Users who own sticky world-writable directory can't follow symlinks
 * inside that directory if their don't own ones. All other users can follow.
 *
 * Hardlinks
 * ---------
 * Hard links restriction applies only to non-privileged users. Only
 * non-privileged user can't create hard links to files if he isn't owner
 * of the file or he doesn't have write access to the file.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "prot_hsymlinks";
int TST_TOTAL = 396;

/* create 3 files and 1 dir in each base dir */
#define MAX_FILES_CREATED	4
#define MAX_PATH		128
#define MAX_CMD_LEN		64
#define MAX_USER_NAME		16

enum {
	ROOT = 0,
	TEST_USER,
	USERS_NUM
};

#define BASE_DIR_NUM		(USERS_NUM + 1)
/*
 * max test files and directories
 * that will be created during the test
 * is't not include symlinks and hardlinks
 * and base directories
 */
#define MAX_ENTITIES		(MAX_FILES_CREATED * BASE_DIR_NUM)

struct dir_params {
	char path[MAX_PATH];
	int world_writable;
	int sticky;
	int owner;
};

static struct dir_params bdirs[BASE_DIR_NUM];

static const char file_ext[] = ".hs";

enum {
	IS_FILE = 0,
	IS_DIRECTORY,
};

struct user_file {
	char path[MAX_PATH];
	int is_dir;
};

struct test_user {
	char name[MAX_USER_NAME];
	struct user_file file[MAX_ENTITIES];
	int num;
};

static struct test_user users[USERS_NUM];

struct link_info {
	char path[MAX_PATH];
	int owner;
	int source_owner;
	int in_world_write;
	int in_sticky;
	int is_dir;
	int dir_owner;
};

/* test flags */
enum {
	CANNOT_FOLLOW = -1,
	CAN_FOLLOW = 0,
};

enum {
	CANNOT_CREATE = -1,
	CAN_CREATE = 0,
};

static char *tmp_user_name;
static char *default_user = "hsym";
static int nflag;
static int skip_cleanup;

static const option_t options[] = {
	{"u:", &nflag, &tmp_user_name},	/* -u #user name */
	{"s", &skip_cleanup, NULL},
	{NULL, NULL, NULL}
};
/* full length of the test tmpdir path in /tmp */
static size_t cwd_offset;

static const char hrdlink_proc_path[]	= "/proc/sys/fs/protected_hardlinks";
static const char symlink_proc_path[]	= "/proc/sys/fs/protected_symlinks";

static void help(void);
static void setup(int argc, char *argv[]);
static void cleanup(void);

static void test_user_cmd(const char *user_cmd);

static int disable_protected_slinks;
static int disable_protected_hlinks;

/*
 * changes links restrictions
 * @param value can be:
 * 0 - restrictions is off
 * 1 - restrictions is on
 */
static void switch_protected_slinks(int value);
static void switch_protected_hlinks(int value);

static int get_protected_slinks(void);
static int get_protected_hlinks(void);

static void create_link_path(char *buffer, int size, const char *path);
static int create_check_hlinks(const struct user_file *ufile, int owner);
static int create_check_slinks(const struct user_file *ufile, int owner);
static int check_symlink(const struct link_info *li);
static int try_open(const char *name, int mode);
/* try to open symlink in diff modes */
static int try_symlink(const char *name);

static int test_run(void);
static void init_base_dirs(void);
static void init_files_dirs(void);

/* change effective user id and group id by name
 * pass NULL to set root
 */
static void set_user(const char *name);

/* add new created files to user struct */
static void ufiles_add(int usr, char *path, int type);

int main(int argc, char *argv[])
{
	setup(argc, argv);

	test_run();

	cleanup();

	tst_exit();
}

static void setup(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, &help);

	tst_require_root();

	if (tst_kvercmp(3, 7, 0) < 0)
		tst_brkm(TCONF, NULL,
			"Test must be run with kernel 3.7 or newer");

	if (eaccess("/etc/passwd", W_OK)) {
		tst_brkm(TCONF, NULL,
			"/etc/passwd is not accessible");
	}

	/* initialize user names */
	strcpy(users[ROOT].name, "root");

	if (tmp_user_name == NULL)
		tmp_user_name = default_user;
	snprintf(users[TEST_USER].name, MAX_USER_NAME, "%s", tmp_user_name);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	test_user_cmd("useradd");
	/*
	 * enable hardlinks and symlinks restrictions,
	 * it's not defualt but have to check
	 */
	if (!get_protected_hlinks()) {
		switch_protected_hlinks(1);
		disable_protected_hlinks = 1;
	}
	if (!get_protected_slinks()) {
		switch_protected_slinks(1);
		disable_protected_slinks = 1;
	}

	tst_tmpdir();

	init_base_dirs();

	init_files_dirs();
}

static int test_run(void)
{
	tst_resm(TINFO, " --- HARDLINKS AND SYMLINKS RESTRICTIONS TEST ---\n");

	int	result_slink = 0,
		result_hlink = 0,
		usr,
		file;

	const struct user_file *ufile;
	/*
	 * create symlinks and hardlinks from each user's files
	 * to each world writable directory
	 */
	for (usr = 0; usr < USERS_NUM; ++usr) {
		/* get all users files and directories */
		for (file = 0; file < users[usr].num; ++file) {
			ufile = &users[usr].file[file];
			result_slink |= create_check_slinks(ufile, usr);
			result_hlink |= create_check_hlinks(ufile, usr);
		}
	}

	/* final results */
	tst_resm(TINFO, "All test-cases have been completed, summary:\n"
		" - symlinks  test:\t%s\n"
		" - hardlinks test:\t%s",
		(result_slink == 1) ? "FAIL" : "PASS",
		(result_hlink == 1) ? "FAIL" : "PASS");

	return result_slink | result_hlink;
}

static void cleanup(void)
{
	/* call cleanup function only once */
	static int first_call = 1;
	if (!first_call)
		return;
	first_call = 0;

	set_user(NULL);

	if (skip_cleanup)
		return;

	test_user_cmd("userdel -r");

	if (disable_protected_hlinks) {
		tst_resm(TINFO, "Disable protected hardlinks mode back");
		switch_protected_hlinks(0);
	}
	if (disable_protected_slinks) {
		tst_resm(TINFO, "Disable protected symlinks mode back");
		switch_protected_slinks(0);
	}

	tst_rmdir();
}

static int get_protected_hlinks(void)
{
	int value = 0;
	SAFE_FILE_SCANF(cleanup, hrdlink_proc_path, "%d", &value);
	return value;
}

static int get_protected_slinks(void)
{
	int value = 0;
	SAFE_FILE_SCANF(cleanup, symlink_proc_path, "%d", &value);
	return value;
}

static void switch_protected_hlinks(int value)
{
	SAFE_FILE_PRINTF(cleanup, hrdlink_proc_path, "%d", value == 1);
}

static void switch_protected_slinks(int value)
{
	SAFE_FILE_PRINTF(cleanup, symlink_proc_path, "%d", value == 1);
}

static void test_user_cmd(const char *user_cmd)
{
	char cmd[MAX_CMD_LEN];
	snprintf(cmd, MAX_CMD_LEN, "%s %s", user_cmd, users[TEST_USER].name);
	if (system(cmd) != 0) {
		tst_brkm(TBROK, cleanup, "Failed to run cmd: %s %s",
			user_cmd, users[TEST_USER].name);
	}
}

static void help(void)
{
	printf("  -s      Skip cleanup.\n");
	printf("  -u #user name : Define test user\n");
}

static void create_sub_dir(const char *path,
	struct dir_params *bdir, mode_t mode)
{
	snprintf(bdir->path, MAX_PATH, "%s/tmp_%s",
		path, users[bdir->owner].name);
	SAFE_MKDIR(cleanup, bdir->path, mode);

	if (bdir->sticky)
		mode |= S_ISVTX;
	chmod(bdir->path, mode);
}

static void init_base_dirs(void)
{
	char *cwd = tst_get_tmpdir();
	cwd_offset = strlen(cwd);

	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
	chmod(cwd, mode);

	strcpy(bdirs[0].path, cwd);
	free(cwd);

	bdirs[0].sticky  = 0;
	bdirs[0].world_writable = 1;

	/* create subdir for each user */
	int dir, usr;
	for (usr = 0; usr < USERS_NUM; ++usr) {
		set_user(users[usr].name);
		dir = usr + 1;
		bdirs[dir].sticky  = 1;
		bdirs[dir].world_writable = 1;
		bdirs[dir].owner = usr;

		create_sub_dir(bdirs[0].path, &bdirs[dir], mode);
	}
}

static void init_files_dirs(void)
{
	unsigned int dir, usr;
	/* create all other dirs and files */
	for (dir = 0; dir < ARRAY_SIZE(bdirs); ++dir) {
		for (usr = 0; usr < USERS_NUM; ++usr) {
			set_user(users[usr].name);
			char path[MAX_PATH];

			/* create file in the main directory */
			snprintf(path, MAX_PATH, "%s/%s%s",
				bdirs[dir].path, users[usr].name, file_ext);
			ufiles_add(usr, path, IS_FILE);

			/* create file with S_IWOTH bit set */
			strcat(path, "_w");
			ufiles_add(usr, path, IS_FILE);

			chmod(path, S_IRUSR | S_IRGRP | S_IWOTH | S_IROTH);

			/* create sub directory */
			snprintf(path, MAX_PATH, "%s/%s", bdirs[dir].path,
				users[usr].name);
			ufiles_add(usr, path, IS_DIRECTORY);

			/* create local file inside sub directory */
			snprintf(path + strlen(path), MAX_PATH - strlen(path),
				"/local_%s%s", users[usr].name, file_ext);
			ufiles_add(usr, path, IS_FILE);
		}
	}
}

static void ufiles_add(int usr, char *path, int type)
{
	int file = users[usr].num;

	if (file >= MAX_ENTITIES)
		tst_brkm(TBROK, cleanup, "Unexpected number of files");

	struct user_file *ufile = &users[usr].file[file];

	if (type == IS_FILE)
		SAFE_TOUCH(cleanup, path, 0644, NULL);
	else
		SAFE_MKDIR(cleanup, path, 0755);

	strcpy(ufile->path, path);

	ufile->is_dir = (type == IS_DIRECTORY);
	++users[usr].num;
}

static void create_link_path(char *buffer, int size, const char *path)
{
	/* to make sure name is unique */
	static int count;
	++count;

	/* construct link name */
	snprintf(buffer, size, "%s/link_%d", path, count);
}

static int create_check_slinks(const struct user_file *ufile, int owner)
{
	int result = 0, usr;
	unsigned int dir;
	for (dir = 0; dir < ARRAY_SIZE(bdirs); ++dir) {
		for (usr = 0; usr < USERS_NUM; ++usr) {
			/* set user who will create symlink */
			set_user(users[usr].name);

			struct link_info slink_info;
			create_link_path(slink_info.path, MAX_PATH,
				bdirs[dir].path);

			slink_info.owner = usr;
			slink_info.source_owner = owner;
			slink_info.in_world_write = bdirs[dir].world_writable;
			slink_info.in_sticky = bdirs[dir].sticky;
			slink_info.dir_owner = bdirs[dir].owner;

			SAFE_SYMLINK(cleanup, ufile->path, slink_info.path);
			result |= check_symlink(&slink_info);
		}
	}
	return result;
}

static int create_check_hlinks(const struct user_file *ufile, int owner)
{
	int result = 0, usr;
	unsigned int dir;
	for (dir = 0; dir < ARRAY_SIZE(bdirs); ++dir) {
		for (usr = 0; usr < USERS_NUM; ++usr) {
			/* can't create hardlink to directory */
			if (ufile->is_dir)
				continue;
			/* set user who will create hardlink */
			set_user(users[usr].name);

			struct link_info hlink_info;
			create_link_path(hlink_info.path, MAX_PATH,
				bdirs[dir].path);

			int can_write = try_open(ufile->path, O_WRONLY) == 0;

			int tst_flag = (can_write || usr == owner ||
				usr == ROOT) ? CAN_CREATE : CANNOT_CREATE;

			int fail;
			fail = tst_flag != link(ufile->path, hlink_info.path);

			result |= fail;
			tst_resm((fail) ? TFAIL : TPASS,
				"Expect: %s create hardlink '...%s' to '...%s', "
				"owner '%s', curr.user '%s', w(%d)",
				(tst_flag == CAN_CREATE) ? "can" : "can't",
				ufile->path + cwd_offset,
				hlink_info.path + cwd_offset,
				users[owner].name, users[usr].name,
				can_write);
		}
	}
	return result;
}

static int check_symlink(const struct link_info *li)
{
	int symlink_result = 0;
	int usr;
	for (usr = 0; usr < USERS_NUM; ++usr) {
		set_user(users[usr].name);
		int tst_flag = (usr == li->dir_owner &&
			li->in_world_write && li->in_sticky &&
			usr != li->owner) ? CANNOT_FOLLOW : CAN_FOLLOW;

		int fail = tst_flag != try_symlink(li->path);

		symlink_result |= fail;

		tst_resm((fail) ? TFAIL : TPASS,
			"Expect: %s follow symlink '...%s', "
			"owner '%s', src.owner '%s', "
			"curr.user '%s', dir.owner '%s'",
			(tst_flag == CAN_FOLLOW) ? "can" : "can't",
			li->path + cwd_offset, users[li->owner].name,
			users[li->source_owner].name, users[usr].name,
			users[li->dir_owner].name);
	}
	return symlink_result;
}

/* differenet modes to try in the test */
static const int o_modes[] = {
	O_RDONLY,
	O_WRONLY,
	O_RDWR,
	O_RDONLY | O_NONBLOCK | O_DIRECTORY,
};

static int try_symlink(const char *name)
{
	unsigned int mode;
	for (mode = 0; mode < ARRAY_SIZE(o_modes); ++mode) {
		if (try_open(name, o_modes[mode]) != -1)
			return CAN_FOLLOW;
	}

	return CANNOT_FOLLOW;
}

static int try_open(const char *name, int mode)
{
	int fd = open(name, mode);

	if (fd == -1)
		return fd;

	SAFE_CLOSE(cleanup, fd);

	return 0;
}

static void set_user(const char *name)
{
	uid_t user_id = 0;
	gid_t user_gr = 0;

	if (name != NULL) {
		struct passwd *pswd = getpwnam(name);

		if (pswd == 0) {
			tst_brkm(TBROK, cleanup,
				"Failed to find user '%s'", name);
		}
		user_id = pswd->pw_uid;
		user_gr = pswd->pw_gid;
	}

	SAFE_SETEGID(cleanup, user_gr);
	SAFE_SETEUID(cleanup, user_id);
}
