// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that all landlock filesystem rules are working properly.
 * The way we do it is to verify that all disabled syscalls are not working but
 * the one we enabled via specifc landlock rules.
 */

#include "landlock_common.h"
#include "landlock_tester.h"
#include "tst_safe_stdio.h"

static struct landlock_ruleset_attr *ruleset_attr;
static struct landlock_path_beneath_attr *path_beneath_attr;
static int ruleset_fd = -1;

static struct tvariant {
	int access;
	char *desc;
} tvariants[] = {
	{
		LANDLOCK_ACCESS_FS_READ_FILE | LANDLOCK_ACCESS_FS_EXECUTE,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_EXECUTE)
	},
	{
		LANDLOCK_ACCESS_FS_WRITE_FILE,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_WRITE_FILE)
	},
	{
		LANDLOCK_ACCESS_FS_READ_FILE,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_READ_FILE)
	},
	{
		LANDLOCK_ACCESS_FS_READ_DIR,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_READ_DIR)
	},
	{
		LANDLOCK_ACCESS_FS_REMOVE_DIR,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_REMOVE_DIR)
	},
	{
		LANDLOCK_ACCESS_FS_REMOVE_FILE,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_REMOVE_FILE)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_CHAR,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_CHAR)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_BLOCK,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_BLOCK)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_REG,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_REG)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_SOCK,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_SOCK)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_FIFO,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_FIFO)
	},
	{
		LANDLOCK_ACCESS_FS_MAKE_SYM,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_MAKE_SYM)
	},
	{
		LANDLOCK_ACCESS_FS_WRITE_FILE | LANDLOCK_ACCESS_FS_TRUNCATE,
		TST_TO_STR_(LANDLOCK_ACCESS_FS_TRUNCATE)
	},
};

static void run(void)
{
	struct tvariant  variant = tvariants[tst_variant];

	tester_setup_files();

	if (!SAFE_FORK()) {
		enforce_ruleset(ruleset_fd);
		tester_run_all_fs_rules(variant.access);

		_exit(0);
	}

	tst_reap_children();
	tester_cleanup_files();
}

static void enable_exec_libs(const int ruleset_fd)
{
	FILE *fp;
	char line[1024];
	char path[PATH_MAX];
	char dependency[8][PATH_MAX];
	int count = 0;
	int duplicate = 0;

	fp = SAFE_FOPEN("/proc/self/maps", "r");

	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, ".so") == NULL)
			continue;

		SAFE_SSCANF(line, "%*x-%*x %*s %*x %*s %*d %s", path);

		for (int i = 0; i < count; i++) {
			if (strcmp(path, dependency[i]) == 0) {
				duplicate = 1;
				break;
			}
		}

		if (duplicate) {
			duplicate = 0;
			continue;
		}

		strncpy(dependency[count], path, PATH_MAX);
		count++;

		tst_res(TINFO, "Enable read/exec permissions for %s", path);

		path_beneath_attr->allowed_access =
			LANDLOCK_ACCESS_FS_READ_FILE |
			LANDLOCK_ACCESS_FS_EXECUTE;
		path_beneath_attr->parent_fd = SAFE_OPEN(path, O_PATH | O_CLOEXEC);

		SAFE_LANDLOCK_ADD_RULE(
			ruleset_fd, LANDLOCK_RULE_PATH_BENEATH,	path_beneath_attr, 0);

		SAFE_CLOSE(path_beneath_attr->parent_fd);
	}

	SAFE_FCLOSE(fp);
}

static void setup(void)
{
	struct tvariant variant = tvariants[tst_variant];

	verify_landlock_is_enabled();

	tst_res(TINFO, "Testing %s", variant.desc);

	ruleset_attr->handled_access_fs = tester_get_all_fs_rules();

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(
		ruleset_attr, sizeof(struct landlock_ruleset_attr), 0);

	/* since our binary is dynamically linked, we need to enable dependences
	 * to be read and executed
	 */
	enable_exec_libs(ruleset_fd);

	/* sandbox folder has to exist before creating the rule */
	if (access(SANDBOX_FOLDER, F_OK) == -1)
		SAFE_MKDIR(SANDBOX_FOLDER, PERM_MODE);

	path_beneath_attr->allowed_access = variant.access;
	path_beneath_attr->parent_fd = SAFE_OPEN(
		SANDBOX_FOLDER, O_PATH | O_CLOEXEC);

	SAFE_LANDLOCK_ADD_RULE(
		ruleset_fd, LANDLOCK_RULE_PATH_BENEATH,	path_beneath_attr, 0);

	SAFE_CLOSE(path_beneath_attr->parent_fd);
}

static void cleanup(void)
{
	if (ruleset_fd != -1)
		SAFE_CLOSE(ruleset_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.test_variants = ARRAY_SIZE(tvariants),
	.resource_files = (const char *[]) {
		TESTAPP,
		NULL,
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_SECURITY_LANDLOCK=y",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{&path_beneath_attr, .size = sizeof(struct landlock_path_beneath_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		TST_CAP(TST_CAP_REQ, CAP_MKNOD),
		{}
	},
	.mount_device = 1,
	.mntpoint = SANDBOX_FOLDER,
	.all_filesystems = 1,
	.max_runtime = 360,
};
