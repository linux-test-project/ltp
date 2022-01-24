// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Red Hat, Inc.
 * Copyright (c) 2020 Li Wang <liwang@redhat.com>
 * Copyright (c) 2020-2021 SUSE LLC <rpalethorpe@suse.com>
 */

#define TST_NO_DEFAULT_MAIN

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/mount.h"
#include "lapi/mkdirat.h"
#include "tst_safe_file_at.h"

struct cgroup_root;

/* A node in a single CGroup hierarchy. It exists mainly for
 * convenience so that we do not have to traverse the CGroup structure
 * for frequent operations.
 *
 * This is actually a single-linked list not a tree. We only need to
 * traverse from leaf towards root.
 */
struct cgroup_dir {
	const char *dir_name;
	const struct cgroup_dir *dir_parent;

	/* Shortcut to root */
	const struct cgroup_root *dir_root;

	/* Subsystems (controllers) bit field. Only controllers which
	 * were required and configured for this node are added to
	 * this field. So it may be different from root->css_field.
	 */
	uint32_t ctrl_field;

	/* In general we avoid having sprintfs everywhere and use
	 * openat, linkat, etc.
	 */
	int dir_fd;

	int we_created_it:1;
};

/* The root of a CGroup hierarchy/tree */
struct cgroup_root {
	enum tst_cgroup_ver ver;
	/* A mount path */
	char mnt_path[PATH_MAX];
	/* Subsystems (controllers) bit field. Includes all
	 * controllers found while scanning this root.
	 */
	uint32_t ctrl_field;

	/* CGroup hierarchy: mnt -> ltp -> {drain, test -> ??? } We
	 * keep a flat reference to ltp, drain and test for
	 * convenience.
	 */

	/* Mount directory */
	struct cgroup_dir mnt_dir;
	/* LTP CGroup directory, contains drain and test dirs */
	struct cgroup_dir ltp_dir;
	/* Drain CGroup, see cgroup_cleanup */
	struct cgroup_dir drain_dir;
	/* CGroup for current test. Which may have children. */
	struct cgroup_dir test_dir;

	int we_mounted_it:1;
	/* cpuset is in compatability mode */
	int no_cpuset_prefix:1;
};

/* Controller sub-systems */
enum cgroup_ctrl_indx {
	CTRL_MEMORY = 1,
	CTRL_CPU,
	CTRL_CPUSET,
};
#define CTRLS_MAX CTRL_CPUSET

/* At most we can have one cgroup V1 tree for each controller and one
 * (empty) v2 tree.
 */
#define ROOTS_MAX (CTRLS_MAX + 1)

/* Describes a controller file or knob
 *
 * The primary purpose of this is to map V2 names to V1
 * names.
 */
struct cgroup_file {
	/* Canonical name. Is the V2 name unless an item is V1 only */
	const char *const file_name;
	/* V1 name or NULL if item is V2 only */
	const char *const file_name_v1;

	/* The controller this item belongs to or zero for
	 * 'cgroup.<item>'.
	 */
	const enum cgroup_ctrl_indx ctrl_indx;
};

/* Describes a Controller or subsystem
 *
 * Internally the kernel seems to call controllers subsystems and uses
 * the abbreviations subsys and css.
 */
struct cgroup_ctrl {
	/* Userland name of the controller (e.g. 'memory' not 'memcg') */
	const char *const ctrl_name;
	/* List of files belonging to this controller */
	const struct cgroup_file *const files;
	/* Our index for the controller */
	const enum cgroup_ctrl_indx ctrl_indx;

	/* Runtime; hierarchy the controller is attached to */
	struct cgroup_root *ctrl_root;
	/* Runtime; whether we required the controller */
	int we_require_it:1;
};

struct tst_cgroup_group {
	char group_name[NAME_MAX + 1];
	/* Maps controller ID to the tree which contains it. The V2
	 * tree is at zero even if it contains no controllers.
	 */
	struct cgroup_dir *dirs_by_ctrl[ROOTS_MAX];
	/* NULL terminated list of trees */
	struct cgroup_dir *dirs[ROOTS_MAX + 1];
};

/* If controllers are required via the tst_test struct then this is
 * populated with the test's CGroup.
 */
static struct tst_cgroup_group test_group;
static struct tst_cgroup_group drain_group;
const struct tst_cgroup_group *const tst_cgroup = &test_group;
const struct tst_cgroup_group *const tst_cgroup_drain = &drain_group;

/* Always use first item for unified hierarchy */
static struct cgroup_root roots[ROOTS_MAX + 1];

static const struct cgroup_file cgroup_ctrl_files[] = {
	/* procs exists on V1, however it was read-only until kernel v3.0. */
	{ "cgroup.procs", "tasks", 0 },
	{ "cgroup.controllers", NULL, 0 },
	{ "cgroup.subtree_control", NULL, 0 },
	{ "cgroup.clone_children", "cgroup.clone_children", 0 },
	{ }
};

static const struct cgroup_file memory_ctrl_files[] = {
	{ "memory.current", "memory.usage_in_bytes", CTRL_MEMORY },
	{ "memory.min", NULL, CTRL_MEMORY },
	{ "memory.max", "memory.limit_in_bytes", CTRL_MEMORY },
	{ "memory.stat", "memory.stat", CTRL_MEMORY },
	{ "memory.swappiness", "memory.swappiness", CTRL_MEMORY },
	{ "memory.swap.current", "memory.memsw.usage_in_bytes", CTRL_MEMORY },
	{ "memory.swap.max", "memory.memsw.limit_in_bytes", CTRL_MEMORY },
	{ "memory.kmem.usage_in_bytes", "memory.kmem.usage_in_bytes", CTRL_MEMORY },
	{ "memory.kmem.limit_in_bytes", "memory.kmem.limit_in_bytes", CTRL_MEMORY },
	{ }
};

static const struct cgroup_file cpu_ctrl_files[] = {
	/* The V1 quota and period files were combined in the V2 max
	 * file. The quota is in the first column and if we just print
	 * a single value to the file, it will be treated as the
	 * quota. To get or set the period we need to branch on the
	 * API version.
	 */
	{ "cpu.max", "cpu.cfs_quota_us", CTRL_CPU },
	{ "cpu.cfs_period_us", "cpu.cfs_period_us", CTRL_CPU },
	{ }
};

static const struct cgroup_file cpuset_ctrl_files[] = {
	{ "cpuset.cpus", "cpuset.cpus", CTRL_CPUSET },
	{ "cpuset.mems", "cpuset.mems", CTRL_CPUSET },
	{ "cpuset.memory_migrate", "cpuset.memory_migrate", CTRL_CPUSET },
	{ }
};

/* Lookup tree for item names. */
static struct cgroup_ctrl controllers[] = {
	[0] = { "cgroup", cgroup_ctrl_files, 0, NULL, 0 },
	[CTRL_MEMORY] = {
		"memory", memory_ctrl_files, CTRL_MEMORY, NULL, 0
	},
	[CTRL_CPU] = {
		"cpu", cpu_ctrl_files, CTRL_CPU, NULL, 0
	},
	[CTRL_CPUSET] = {
		"cpuset", cpuset_ctrl_files, CTRL_CPUSET, NULL, 0
	},
	{ }
};

/* We should probably allow these to be set in environment
 * variables
 */
static const char *cgroup_ltp_dir = "ltp";
static const char *cgroup_ltp_drain_dir = "drain";
static char cgroup_test_dir[NAME_MAX + 1];
static const char *cgroup_mount_ltp_prefix = "/tmp/cgroup_";
static const char *cgroup_v2_ltp_mount = "unified";

#define first_root				\
	(roots[0].ver ? roots : roots + 1)
#define for_each_root(r)			\
	for ((r) = first_root; (r)->ver; (r)++)
#define for_each_v1_root(r)			\
	for ((r) = roots + 1; (r)->ver; (r)++)
#define for_each_ctrl(ctrl)			\
	for ((ctrl) = controllers; (ctrl)->ctrl_name; (ctrl)++)

/* In all cases except one, this only loops once.
 *
 * If (ctrl) == 0 and multiple V1 (and a V2) hierarchies are mounted,
 * then we need to loop over multiple directories. For example if we
 * need to write to "tasks"/"cgroup.procs" which exists for each
 * hierarchy.
 */
#define for_each_dir(cg, ctrl, t)					\
	for ((t) = (ctrl) ? (cg)->dirs_by_ctrl + (ctrl) : (cg)->dirs;	\
	     *(t);							\
	     (t) = (ctrl) ? (cg)->dirs + ROOTS_MAX : (t) + 1)

__attribute__ ((nonnull))
static int has_ctrl(const uint32_t ctrl_field,
		    const struct cgroup_ctrl *const ctrl)
{
	return !!(ctrl_field & (1 << ctrl->ctrl_indx));
}

__attribute__ ((nonnull))
static void add_ctrl(uint32_t *const ctrl_field,
		     const struct cgroup_ctrl *const ctrl)
{
	*ctrl_field |= 1 << ctrl->ctrl_indx;
}

static int cgroup_v2_mounted(void)
{
	return !!roots[0].ver;
}

static int cgroup_v1_mounted(void)
{
	return !!roots[1].ver;
}

static int cgroup_mounted(void)
{
	return cgroup_v2_mounted() || cgroup_v1_mounted();
}

__attribute__ ((nonnull, warn_unused_result))
static int cgroup_ctrl_on_v2(const struct cgroup_ctrl *const ctrl)
{
	return ctrl->ctrl_root && ctrl->ctrl_root->ver == TST_CGROUP_V2;
}

__attribute__ ((nonnull))
static void cgroup_dir_mk(const struct cgroup_dir *const parent,
			  const char *const dir_name,
			  struct cgroup_dir *const new)
{
	const char *dpath;

	new->dir_root = parent->dir_root;
	new->dir_name = dir_name;
	new->dir_parent = parent;
	new->ctrl_field = parent->ctrl_field;
	new->we_created_it = 0;

	if (!mkdirat(parent->dir_fd, dir_name, 0777)) {
		new->we_created_it = 1;
		goto opendir;
	}

	if (errno == EEXIST)
		goto opendir;

	dpath = tst_decode_fd(parent->dir_fd);

	if (errno == EACCES) {
		tst_brk(TCONF | TERRNO,
			"Lack permission to make '%s/%s'; premake it or run as root",
			dpath, dir_name);
	} else {
		tst_brk(TBROK | TERRNO,
			"mkdirat(%d<%s>, '%s', 0777)",
			parent->dir_fd, dpath, dir_name);
	}

opendir:
	new->dir_fd = SAFE_OPENAT(parent->dir_fd, dir_name,
				  O_PATH | O_DIRECTORY);
}

void tst_cgroup_print_config(void)
{
	struct cgroup_root *root;
	const struct cgroup_ctrl *ctrl;

	tst_res(TINFO, "Detected Controllers:");

	for_each_ctrl(ctrl) {
		root = ctrl->ctrl_root;

		if (!root)
			continue;

		tst_res(TINFO, "\t%.10s %s @ %s:%s",
			ctrl->ctrl_name,
			root->no_cpuset_prefix ? "[noprefix]" : "",
			root->ver == TST_CGROUP_V1 ? "V1" : "V2",
			root->mnt_path);
	}
}

__attribute__ ((nonnull, warn_unused_result))
static struct cgroup_ctrl *cgroup_find_ctrl(const char *const ctrl_name)
{
	struct cgroup_ctrl *ctrl;

	for_each_ctrl(ctrl) {
		if (!strcmp(ctrl_name, ctrl->ctrl_name))
			return ctrl;
	}

	return NULL;
}

/* Determine if a mounted cgroup hierarchy is unique and record it if so.
 *
 * For CGroups V2 this is very simple as there is only one
 * hierarchy. We just record which controllers are available and check
 * if this matches what we read from any previous mount points.
 *
 * For V1 the set of controllers S is partitioned into sets {P_1, P_2,
 * ..., P_n} with one or more controllers in each partion. Each
 * partition P_n can be mounted multiple times, but the same
 * controller can not appear in more than one partition. Usually each
 * partition contains a single controller, but, for example, cpu and
 * cpuacct are often mounted together in the same partiion.
 *
 * Each controller partition has its own hierarchy (root) which we
 * must track and update independently.
 */
__attribute__ ((nonnull))
static void cgroup_root_scan(const char *const mnt_type,
			     const char *const mnt_dir,
			     char *const mnt_opts)
{
	struct cgroup_root *root = roots;
	const struct cgroup_ctrl *const_ctrl;
	struct cgroup_ctrl *ctrl;
	uint32_t ctrl_field = 0;
	int no_prefix = 0;
	char buf[BUFSIZ];
	char *tok;
	const int mnt_dfd = SAFE_OPEN(mnt_dir, O_PATH | O_DIRECTORY);

	if (!strcmp(mnt_type, "cgroup"))
		goto v1;

	SAFE_FILE_READAT(mnt_dfd, "cgroup.controllers", buf, sizeof(buf));

	for (tok = strtok(buf, " "); tok; tok = strtok(NULL, " ")) {
		const_ctrl = cgroup_find_ctrl(tok);
		if (const_ctrl)
			add_ctrl(&ctrl_field, const_ctrl);
	}

	if (root->ver && ctrl_field == root->ctrl_field)
		goto discard;

	if (root->ctrl_field)
		tst_brk(TBROK, "Available V2 controllers are changing between scans?");

	root->ver = TST_CGROUP_V2;

	goto backref;

v1:
	for (tok = strtok(mnt_opts, ","); tok; tok = strtok(NULL, ",")) {
		const_ctrl = cgroup_find_ctrl(tok);
		if (const_ctrl)
			add_ctrl(&ctrl_field, const_ctrl);

		no_prefix |= !strcmp("noprefix", tok);
	}

	if (!ctrl_field)
		goto discard;

	for_each_v1_root(root) {
		if (!(ctrl_field & root->ctrl_field))
			continue;

		if (ctrl_field == root->ctrl_field)
			goto discard;

		tst_brk(TBROK,
			"The intersection of two distinct sets of mounted controllers should be null? "
			"Check '%s' and '%s'", root->mnt_path, mnt_dir);
	}

	if (root >= roots + ROOTS_MAX) {
		tst_brk(TBROK,
			"Unique controller mounts have exceeded our limit %d?",
			ROOTS_MAX);
	}

	root->ver = TST_CGROUP_V1;

backref:
	strcpy(root->mnt_path, mnt_dir);
	root->mnt_dir.dir_root = root;
	root->mnt_dir.dir_name = root->mnt_path;
	root->mnt_dir.dir_fd = mnt_dfd;
	root->ctrl_field = ctrl_field;
	root->no_cpuset_prefix = no_prefix;

	for_each_ctrl(ctrl) {
		if (has_ctrl(root->ctrl_field, ctrl))
			ctrl->ctrl_root = root;
	}

	return;

discard:
	close(mnt_dfd);
}

void tst_cgroup_scan(void)
{
	struct mntent *mnt;
	FILE *f = setmntent("/proc/self/mounts", "r");

	if (!f) {
		tst_brk(TBROK | TERRNO, "Can't open /proc/self/mounts");
		return;
	}

	mnt = getmntent(f);
	if (!mnt) {
		tst_brk(TBROK | TERRNO, "Can't read mounts or no mounts?");
		return;
	}

	do {
		if (strncmp(mnt->mnt_type, "cgroup", 6))
			continue;

		cgroup_root_scan(mnt->mnt_type, mnt->mnt_dir, mnt->mnt_opts);
	} while ((mnt = getmntent(f)));
}

static void cgroup_mount_v2(void)
{
	char mnt_path[PATH_MAX];

	sprintf(mnt_path, "%s%s", cgroup_mount_ltp_prefix, cgroup_v2_ltp_mount);

	if (!mkdir(mnt_path, 0777)) {
		roots[0].mnt_dir.we_created_it = 1;
		goto mount;
	}

	if (errno == EEXIST)
		goto mount;

	if (errno == EACCES) {
		tst_res(TINFO | TERRNO,
			"Lack permission to make %s, premake it or run as root",
			mnt_path);
		return;
	}

	tst_brk(TBROK | TERRNO, "mkdir(%s, 0777)", mnt_path);
	return;

mount:
	if (!mount("cgroup2", mnt_path, "cgroup2", 0, NULL)) {
		tst_res(TINFO, "Mounted V2 CGroups on %s", mnt_path);
		tst_cgroup_scan();
		roots[0].we_mounted_it = 1;
		return;
	}

	tst_res(TINFO | TERRNO, "Could not mount V2 CGroups on %s", mnt_path);

	if (roots[0].mnt_dir.we_created_it) {
		roots[0].mnt_dir.we_created_it = 0;
		SAFE_RMDIR(mnt_path);
	}
}

__attribute__ ((nonnull))
static void cgroup_mount_v1(struct cgroup_ctrl *const ctrl)
{
	char mnt_path[PATH_MAX];
	int made_dir = 0;

	sprintf(mnt_path, "%s%s", cgroup_mount_ltp_prefix, ctrl->ctrl_name);

	if (!mkdir(mnt_path, 0777)) {
		made_dir = 1;
		goto mount;
	}

	if (errno == EEXIST)
		goto mount;

	if (errno == EACCES) {
		tst_res(TINFO | TERRNO,
			"Lack permission to make %s, premake it or run as root",
			mnt_path);
		return;
	}

	tst_brk(TBROK | TERRNO, "mkdir(%s, 0777)", mnt_path);
	return;

mount:
	if (mount(ctrl->ctrl_name, mnt_path, "cgroup", 0, ctrl->ctrl_name)) {
		tst_res(TINFO | TERRNO,
			"Could not mount V1 CGroup on %s", mnt_path);

		if (made_dir)
			SAFE_RMDIR(mnt_path);
		return;
	}

	tst_res(TINFO, "Mounted V1 %s CGroup on %s", ctrl->ctrl_name, mnt_path);
	tst_cgroup_scan();
	if (!ctrl->ctrl_root)
		return;

	ctrl->ctrl_root->we_mounted_it = 1;
	ctrl->ctrl_root->mnt_dir.we_created_it = made_dir;

	if (ctrl->ctrl_indx == CTRL_MEMORY) {
		SAFE_FILE_PRINTFAT(ctrl->ctrl_root->mnt_dir.dir_fd,
				   "memory.use_hierarchy", "%d", 1);
	}
}

__attribute__ ((nonnull))
static void cgroup_copy_cpuset(const struct cgroup_root *const root)
{
	char knob_val[BUFSIZ];
	int i;
	const char *const n0[] = {"mems", "cpus"};
	const char *const n1[] = {"cpuset.mems", "cpuset.cpus"};
	const char *const *const fname = root->no_cpuset_prefix ? n0 : n1;

	for (i = 0; i < 2; i++) {
		SAFE_FILE_READAT(root->mnt_dir.dir_fd,
				 fname[i], knob_val, sizeof(knob_val));
		SAFE_FILE_PRINTFAT(root->ltp_dir.dir_fd,
				   fname[i], "%s", knob_val);
	}
}

/* Ensure the specified controller is available.
 *
 * First we check if the specified controller has a known mount point,
 * if not then we scan the system. If we find it then we goto ensuring
 * the LTP group exists in the hierarchy the controller is using.
 *
 * If we can't find the controller, then we try to create it. First we
 * check if the V2 hierarchy/tree is mounted. If it isn't then we try
 * mounting it and look for the controller. If it is already mounted
 * then we know the controller is not available on V2 on this system.
 *
 * If we can't mount V2 or the controller is not on V2, then we try
 * mounting it on its own V1 tree.
 *
 * Once we have mounted the controller somehow, we create a hierarchy
 * of cgroups. If we are on V2 we first need to enable the controller
 * for all children of root. Then we create hierarchy described in
 * tst_cgroup.h.
 *
 * If we are using V1 cpuset then we copy the available mems and cpus
 * from root to the ltp group and set clone_children on the ltp group
 * to distribute these settings to the test cgroups. This means the
 * test author does not have to copy these settings before using the
 * cpuset.
 *
 */
void tst_cgroup_require(const char *const ctrl_name,
			const struct tst_cgroup_opts *options)
{
	const char *const cgsc = "cgroup.subtree_control";
	struct cgroup_ctrl *const ctrl = cgroup_find_ctrl(ctrl_name);
	struct cgroup_root *root;

	if (!ctrl) {
		tst_brk(TBROK, "'%s' controller is unknown to LTP", ctrl_name);
		tst_brk(TBROK, "Calling %s in cleanup?", __func__);
		return;
	}

	if (ctrl->we_require_it)
		tst_res(TWARN, "Duplicate %s(%s, )", __func__, ctrl->ctrl_name);

	ctrl->we_require_it = 1;

	if (ctrl->ctrl_root)
		goto mkdirs;

	tst_cgroup_scan();

	if (ctrl->ctrl_root)
		goto mkdirs;

	if (!cgroup_v2_mounted() && options->needs_ver != TST_CGROUP_V1)
		cgroup_mount_v2();

	if (ctrl->ctrl_root)
		goto mkdirs;

	if (options->needs_ver != TST_CGROUP_V2)
		cgroup_mount_v1(ctrl);

	if (!ctrl->ctrl_root) {
		tst_brk(TCONF,
			"'%s' controller required, but not available",
			ctrl->ctrl_name);
		return;
	}

mkdirs:
	root = ctrl->ctrl_root;
	add_ctrl(&root->mnt_dir.ctrl_field, ctrl);

	if (cgroup_ctrl_on_v2(ctrl) && options->needs_ver == TST_CGROUP_V1) {
		tst_brk(TCONF,
			"V1 '%s' controller required, but it's mounted on V2",
			ctrl->ctrl_name);
	}
	if (!cgroup_ctrl_on_v2(ctrl) && options->needs_ver == TST_CGROUP_V2) {
		tst_brk(TCONF,
			"V2 '%s' controller required, but it's mounted on V1",
			ctrl->ctrl_name);
	}

	if (cgroup_ctrl_on_v2(ctrl)) {
		if (root->we_mounted_it) {
			SAFE_FILE_PRINTFAT(root->mnt_dir.dir_fd,
					   cgsc, "+%s", ctrl->ctrl_name);
		} else {
			tst_file_printfat(root->mnt_dir.dir_fd,
					  cgsc, "+%s", ctrl->ctrl_name);
		}
	}

	if (!root->ltp_dir.dir_fd)
		cgroup_dir_mk(&root->mnt_dir, cgroup_ltp_dir, &root->ltp_dir);
	else
		root->ltp_dir.ctrl_field |= root->mnt_dir.ctrl_field;

	if (cgroup_ctrl_on_v2(ctrl)) {
		SAFE_FILE_PRINTFAT(root->ltp_dir.dir_fd,
				   cgsc, "+%s", ctrl->ctrl_name);
	} else {
		SAFE_FILE_PRINTFAT(root->ltp_dir.dir_fd,
				   "cgroup.clone_children", "%d", 1);

		if (ctrl->ctrl_indx == CTRL_CPUSET)
			cgroup_copy_cpuset(root);
	}

	cgroup_dir_mk(&root->ltp_dir, cgroup_ltp_drain_dir, &root->drain_dir);

	sprintf(cgroup_test_dir, "test-%d", getpid());
	cgroup_dir_mk(&root->ltp_dir, cgroup_test_dir, &root->test_dir);
}

static void cgroup_drain(const enum tst_cgroup_ver ver,
			 const int source_dfd, const int dest_dfd)
{
	char pid_list[BUFSIZ];
	char *tok;
	const char *const file_name =
		ver == TST_CGROUP_V1 ? "tasks" : "cgroup.procs";
	int fd;
	ssize_t ret;

	ret = SAFE_FILE_READAT(source_dfd, file_name,
			       pid_list, sizeof(pid_list));
	if (ret < 0)
		return;

	fd = SAFE_OPENAT(dest_dfd, file_name, O_WRONLY);
	if (fd < 0)
		return;

	for (tok = strtok(pid_list, "\n"); tok; tok = strtok(NULL, "\n")) {
		ret = dprintf(fd, "%s", tok);

		if (ret < (ssize_t)strlen(tok))
			tst_brk(TBROK | TERRNO, "Failed to drain %s", tok);
	}
	SAFE_CLOSE(fd);
}

__attribute__ ((nonnull))
static void close_path_fds(struct cgroup_root *const root)
{
	if (root->test_dir.dir_fd > 0)
		SAFE_CLOSE(root->test_dir.dir_fd);
	if (root->ltp_dir.dir_fd > 0)
		SAFE_CLOSE(root->ltp_dir.dir_fd);
	if (root->drain_dir.dir_fd > 0)
		SAFE_CLOSE(root->drain_dir.dir_fd);
	if (root->mnt_dir.dir_fd > 0)
		SAFE_CLOSE(root->mnt_dir.dir_fd);
}

/* Maybe remove CGroups used during testing and clear our data
 *
 * This will never remove CGroups we did not create to allow tests to
 * be run in parallel.
 *
 * Each test process is given its own unique CGroup. Unless we want to
 * stress test the CGroup system. We should at least remove these
 * unique per test CGroups.
 *
 * We probably also want to remove the LTP parent CGroup, although
 * this may have been created by the system manager or another test
 * (see notes on parallel testing).
 *
 * On systems with no initial CGroup setup we may try to destroy the
 * CGroup roots we mounted so that they can be recreated by another
 * test. Note that successfully unmounting a CGroup root does not
 * necessarily indicate that it was destroyed.
 *
 * The ltp/drain CGroup is required for cleaning up test CGroups when
 * we can not move them to the root CGroup. CGroups can only be
 * removed when they have no members and only leaf or root CGroups may
 * have processes within them. As test processes create and destroy
 * their own CGroups they must move themselves either to root or
 * another leaf CGroup. So we move them to drain while destroying the
 * unique test CGroup.
 *
 * If we have access to root and created the LTP CGroup we then move
 * the test process to root and destroy the drain and LTP
 * CGroups. Otherwise we just leave the test process to die in the
 * drain, much like many a unwanted terrapin.
 *
 * Finally we clear any data we have collected on CGroups. This will
 * happen regardless of whether anything was removed.
 */
void tst_cgroup_cleanup(void)
{
	struct cgroup_root *root;
	struct cgroup_ctrl *ctrl;

	if (!cgroup_mounted())
		goto clear_data;

	for_each_root(root) {
		if (!root->test_dir.dir_name)
			continue;

		cgroup_drain(root->ver,
			     root->test_dir.dir_fd, root->drain_dir.dir_fd);
		SAFE_UNLINKAT(root->ltp_dir.dir_fd, root->test_dir.dir_name,
			      AT_REMOVEDIR);
	}

	for_each_root(root) {
		if (!root->ltp_dir.we_created_it)
			continue;

		cgroup_drain(root->ver,
			     root->drain_dir.dir_fd, root->mnt_dir.dir_fd);

		if (root->drain_dir.dir_name) {
			SAFE_UNLINKAT(root->ltp_dir.dir_fd,
				      root->drain_dir.dir_name, AT_REMOVEDIR);
		}

		if (root->ltp_dir.dir_name) {
			SAFE_UNLINKAT(root->mnt_dir.dir_fd,
				      root->ltp_dir.dir_name, AT_REMOVEDIR);
		}
	}

	for_each_ctrl(ctrl) {
		if (!cgroup_ctrl_on_v2(ctrl) || !ctrl->ctrl_root->we_mounted_it)
			continue;

		SAFE_FILE_PRINTFAT(ctrl->ctrl_root->mnt_dir.dir_fd,
				   "cgroup.subtree_control",
				   "-%s", ctrl->ctrl_name);
	}

	for_each_root(root) {
		if (!root->we_mounted_it)
			continue;

		/* This probably does not result in the CGroup root
		 * being destroyed
		 */
		if (umount2(root->mnt_path, MNT_DETACH))
			continue;

		SAFE_RMDIR(root->mnt_path);
	}

clear_data:
	for_each_ctrl(ctrl) {
		ctrl->ctrl_root = NULL;
		ctrl->we_require_it = 0;
	}

	for_each_root(root)
		close_path_fds(root);

	memset(roots, 0, sizeof(roots));
}

__attribute__((nonnull(1)))
static void cgroup_group_init(struct tst_cgroup_group *const cg,
			      const char *const group_name)
{
	memset(cg, 0, sizeof(*cg));

	if (!group_name)
		return;

	if (strlen(group_name) > NAME_MAX)
		tst_brk(TBROK, "Group name is too long");

	strcpy(cg->group_name, group_name);
}

__attribute__((nonnull(2, 3)))
static void cgroup_group_add_dir(const struct tst_cgroup_group *const parent,
				 struct tst_cgroup_group *const cg,
				 struct cgroup_dir *const dir)
{
	const struct cgroup_ctrl *ctrl;
	int i;

	if (dir->dir_root->ver != TST_CGROUP_V1)
		cg->dirs_by_ctrl[0] = dir;

	for_each_ctrl(ctrl) {
		if (!has_ctrl(dir->ctrl_field, ctrl))
			continue;

		cg->dirs_by_ctrl[ctrl->ctrl_indx] = dir;

		if (!parent || dir->dir_root->ver == TST_CGROUP_V1)
			continue;

		SAFE_CGROUP_PRINTF(parent, "cgroup.subtree_control",
				   "+%s", ctrl->ctrl_name);
	}

	for (i = 0; cg->dirs[i]; i++)
		;
	cg->dirs[i] = dir;
}

struct tst_cgroup_group *
tst_cgroup_group_mk(const struct tst_cgroup_group *const parent,
		    const char *const group_name)
{
	struct tst_cgroup_group *cg;
	struct cgroup_dir *const *dir;
	struct cgroup_dir *new_dir;

	cg = SAFE_MALLOC(sizeof(*cg));
	cgroup_group_init(cg, group_name);

	for_each_dir(parent, 0, dir) {
		new_dir = SAFE_MALLOC(sizeof(*new_dir));
		cgroup_dir_mk(*dir, group_name, new_dir);
		cgroup_group_add_dir(parent, cg, new_dir);
	}

	return cg;
}

const char *tst_cgroup_group_name(const struct tst_cgroup_group *const cg)
{
	return cg->group_name;
}

struct tst_cgroup_group *tst_cgroup_group_rm(struct tst_cgroup_group *const cg)
{
	struct cgroup_dir **dir;

	for_each_dir(cg, 0, dir) {
		close((*dir)->dir_fd);
		SAFE_UNLINKAT((*dir)->dir_parent->dir_fd,
			      (*dir)->dir_name,
			      AT_REMOVEDIR);
		free(*dir);
	}

	free(cg);
	return NULL;
}

__attribute__ ((nonnull, warn_unused_result))
static const struct cgroup_file *cgroup_file_find(const char *const file,
						  const int lineno,
						  const char *const file_name)
{
	const struct cgroup_file *cfile;
	const struct cgroup_ctrl *ctrl;
	char ctrl_name[32];
	const char *const sep = strchr(file_name, '.');
	size_t len;

	if (!sep) {
		tst_brk_(file, lineno, TBROK,
			 "Invalid file name '%s'; did not find controller separator '.'",
			 file_name);
		return NULL;
	}

	len = sep - file_name;
	memcpy(ctrl_name, file_name, len);
	ctrl_name[len] = '\0';

	ctrl = cgroup_find_ctrl(ctrl_name);

	if (!ctrl) {
		tst_brk_(file, lineno, TBROK,
			 "Did not find controller '%s'\n", ctrl_name);
		return NULL;
	}

	for (cfile = ctrl->files; cfile->file_name; cfile++) {
		if (!strcmp(file_name, cfile->file_name))
			break;
	}

	if (!cfile->file_name) {
		tst_brk_(file, lineno, TBROK,
			 "Did not find '%s' in '%s'\n",
			 file_name, ctrl->ctrl_name);
		return NULL;
	}

	return cfile;
}

enum tst_cgroup_ver tst_cgroup_ver(const char *const file, const int lineno,
				    const struct tst_cgroup_group *const cg,
				    const char *const ctrl_name)
{
	const struct cgroup_ctrl *const ctrl = cgroup_find_ctrl(ctrl_name);
	const struct cgroup_dir *dir;

	if (!strcmp(ctrl_name, "cgroup")) {
		tst_brk_(file, lineno,
			 TBROK,
			 "cgroup may be present on both V1 and V2 hierarchies");
		return 0;
	}

	if (!ctrl) {
		tst_brk_(file, lineno,
			 TBROK, "Unknown controller '%s'", ctrl_name);
		return 0;
	}

	dir = cg->dirs_by_ctrl[ctrl->ctrl_indx];

	if (!dir) {
		tst_brk_(file, lineno,
			 TBROK, "%s controller not attached to CGroup %s",
			 ctrl_name, cg->group_name);
		return 0;
	}

	return dir->dir_root->ver;
}

__attribute__ ((nonnull, warn_unused_result))
static const char *cgroup_file_alias(const struct cgroup_file *const cfile,
				     const struct cgroup_dir *const dir)
{
	if (dir->dir_root->ver != TST_CGROUP_V1)
		return cfile->file_name;

	if (cfile->ctrl_indx == CTRL_CPUSET &&
	    dir->dir_root->no_cpuset_prefix &&
	    cfile->file_name_v1) {
		return strchr(cfile->file_name_v1, '.') + 1;
	}

	return cfile->file_name_v1;
}

int safe_cgroup_has(const char *const file, const int lineno,
		    const struct tst_cgroup_group *cg,
		    const char *const file_name)
{
	const struct cgroup_file *const cfile =
		cgroup_file_find(file, lineno, file_name);
	struct cgroup_dir *const *dir;
	const char *alias;

	if (!cfile)
		return 0;

	for_each_dir(cg, cfile->ctrl_indx, dir) {
		alias = cgroup_file_alias(cfile, *dir);
		if (!alias)
			continue;

		if (!faccessat((*dir)->dir_fd, alias, F_OK, 0))
			return 1;

		if (errno == ENOENT)
			continue;

		tst_brk_(file, lineno, TBROK | TERRNO,
			 "faccessat(%d<%s>, %s, F_OK, 0)",
			 (*dir)->dir_fd, tst_decode_fd((*dir)->dir_fd), alias);
	}

	return 0;
}

static void group_from_roots(struct tst_cgroup_group *const cg)
{
	struct cgroup_root *root;

	if (cg->group_name[0]) {
		tst_brk(TBROK,
			"%s CGroup already initialized",
			cg == &test_group ? "Test" : "Drain");
	}

	for_each_root(root) {
		struct cgroup_dir *dir =
			cg == &test_group ? &root->test_dir : &root->drain_dir;

		if (dir->ctrl_field)
			cgroup_group_add_dir(NULL, cg, dir);
	}

	if (cg->dirs[0]) {
		strncpy(cg->group_name, cg->dirs[0]->dir_name, NAME_MAX);
		return;
	}

	tst_brk(TBROK,
		"No CGroups found; maybe you forgot to call tst_cgroup_require?");
}

void tst_cgroup_init(void)
{
	group_from_roots(&test_group);
	group_from_roots(&drain_group);
}

ssize_t safe_cgroup_read(const char *const file, const int lineno,
			 const struct tst_cgroup_group *const cg,
			 const char *const file_name,
			 char *const out, const size_t len)
{
	const struct cgroup_file *const cfile =
		cgroup_file_find(file, lineno, file_name);
	struct cgroup_dir *const *dir;
	const char *alias;
	size_t prev_len = 0;
	char prev_buf[BUFSIZ];
	ssize_t read_ret = 0;

	for_each_dir(cg, cfile->ctrl_indx, dir) {
		alias = cgroup_file_alias(cfile, *dir);
		if (!alias)
			continue;

		if (prev_len)
			memcpy(prev_buf, out, prev_len);

		read_ret = safe_file_readat(file, lineno,
					    (*dir)->dir_fd, alias, out, len);
		if (read_ret < 0)
			continue;

		if (prev_len && memcmp(out, prev_buf, prev_len)) {
			tst_brk_(file, lineno, TBROK,
				 "%s has different value across roots",
				 file_name);
			break;
		}

		prev_len = MIN(sizeof(prev_buf), (size_t)read_ret);
	}

	out[MAX(read_ret, 0)] = '\0';

	return read_ret;
}

void safe_cgroup_printf(const char *const file, const int lineno,
			const struct tst_cgroup_group *cg,
			const char *const file_name,
			const char *const fmt, ...)
{
	const struct cgroup_file *const cfile =
		cgroup_file_find(file, lineno, file_name);
	struct cgroup_dir *const *dir;
	const char *alias;
	va_list va;

	for_each_dir(cg, cfile->ctrl_indx, dir) {
		alias = cgroup_file_alias(cfile, *dir);
		if (!alias)
			continue;

		va_start(va, fmt);
		safe_file_vprintfat(file, lineno,
				    (*dir)->dir_fd, alias, fmt, va);
		va_end(va);
	}
}

void safe_cgroup_scanf(const char *const file, const int lineno,
		       const struct tst_cgroup_group *const cg,
		       const char *const file_name,
		       const char *const fmt, ...)
{
	va_list va;
	char buf[BUFSIZ];
	ssize_t len = safe_cgroup_read(file, lineno,
				       cg, file_name, buf, sizeof(buf));
	const int conv_cnt = tst_count_scanf_conversions(fmt);
	int ret;

	if (len < 1)
		return;

	va_start(va, fmt);
	ret = vsscanf(buf, fmt, va);
	if (ret < 1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "'%s': vsscanf('%s', '%s', ...)", file_name, buf, fmt);
	}
	va_end(va);

	if (conv_cnt == ret)
		return;

	tst_brk_(file, lineno, TBROK,
		 "'%s': vsscanf('%s', '%s', ..): Less conversions than expected: %d != %d",
		 file_name, buf, fmt, ret, conv_cnt);
}

void safe_cgroup_lines_scanf(const char *const file, const int lineno,
			     const struct tst_cgroup_group *const cg,
			     const char *const file_name,
			     const char *const fmt, ...)
{
	va_list va;
	char buf[BUFSIZ];
	ssize_t len = safe_cgroup_read(file, lineno,
				       cg, file_name, buf, sizeof(buf));
	const int conv_cnt = tst_count_scanf_conversions(fmt);
	int ret = 0;
	char *line, *buf_ptr;

	if (len < 1)
		return;

	line = strtok_r(buf, "\n", &buf_ptr);
	while (line && ret != conv_cnt) {
		va_start(va, fmt);
		ret = vsscanf(line, fmt, va);
		va_end(va);

		line = strtok_r(NULL, "\n", &buf_ptr);
	}

	if (conv_cnt == ret)
		return;

	tst_brk_(file, lineno, TBROK,
		 "'%s': vsscanf('%s', '%s', ..): Less conversions than expected: %d != %d",
		 file_name, buf, fmt, ret, conv_cnt);
}

int safe_cgroup_occursin(const char *const file, const int lineno,
			 const struct tst_cgroup_group *const cg,
			 const char *const file_name,
			 const char *const needle)
{
	char buf[BUFSIZ];

	safe_cgroup_read(file, lineno, cg, file_name, buf, sizeof(buf));

	return !!strstr(buf, needle);
}
