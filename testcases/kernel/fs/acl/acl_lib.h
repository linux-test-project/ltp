/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2026 IBM
 * Original shell test by Kai Zhao (ltcd3@cn.ibm.com)
 * Converted to C by Sachin Sant <sachinp@linux.ibm.com>
 *
 * Common library for ACL and extended attribute tests using xattr API
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef ACL_LIB_H
#define ACL_LIB_H

#include "config.h"

#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <sys/fsuid.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif
#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_capability.h"

#define MNTPOINT	"mntpoint"
#define TESTDIR		MNTPOINT "/testdir"
#define TESTFILE	TESTDIR "/testfile"
#define TESTSYMLINK	TESTDIR "/testsymlink"

/*
 * POSIX ACL xattr format definitions
 * These match the kernel's internal representation
 */
#define POSIX_ACL_XATTR_VERSION	0x0002

/* ACL entry tag types */
#define ACL_UNDEFINED_TAG	0x00
#define ACL_USER_OBJ		0x01
#define ACL_USER		0x02
#define ACL_GROUP_OBJ		0x04
#define ACL_GROUP		0x08
#define ACL_MASK		0x10
#define ACL_OTHER		0x20

/* ACL permissions */
#define ACL_READ		0x04
#define ACL_WRITE		0x02
#define ACL_EXECUTE		0x01

/* ACL xattr names */
#define XATTR_NAME_POSIX_ACL_ACCESS	"system.posix_acl_access"
#define XATTR_NAME_POSIX_ACL_DEFAULT	"system.posix_acl_default"

/* ACL type for set/get operations */
#define ACL_TYPE_ACCESS		1
#define ACL_TYPE_DEFAULT	2

/* Convert host to little-endian */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#else
#define cpu_to_le16(x) __builtin_bswap16(x)
#define cpu_to_le32(x) __builtin_bswap32(x)
#define le16_to_cpu(x) __builtin_bswap16(x)
#define le32_to_cpu(x) __builtin_bswap32(x)
#endif

/*
 * POSIX ACL xattr format as stored in kernel
 * This is the on-disk/in-xattr representation
 */
struct posix_acl_xattr_header {
	uint32_t a_version;
};

struct posix_acl_xattr_entry {
	uint16_t e_tag;
	uint16_t e_perm;
	uint32_t e_id;
};

/*
 * Combined xattr structure for ACL operations
 */
struct acl_xattr {
	struct posix_acl_xattr_header header;
	struct posix_acl_xattr_entry entries[];
};

/*
 * In-memory ACL representation for building ACLs
 */
#define MAX_ACL_ENTRIES 32

struct acl_entry {
	uint16_t tag;
	uint16_t perm;
	uint32_t id;
};

struct acl {
	int count;
	struct acl_entry entries[MAX_ACL_ENTRIES];
};

/*
 * try_unlink - unlink a file, ignoring ENOENT
 * Won't TBROK if file doesn't exist
 */
static inline void try_unlink(const char *path)
{
	if (unlink(path) == -1 && errno != ENOENT)
		tst_brk(TBROK | TERRNO, "unlink(%s) failed", path);
}

/*
 * try_rmdir - remove a directory, ignoring ENOENT
 * Won't TBROK if directory doesn't exist
 */
static inline void try_rmdir(const char *path)
{
	if (rmdir(path) == -1 && errno != ENOENT)
		tst_brk(TBROK | TERRNO, "rmdir(%s) failed", path);
}

static inline void cleanup_test_paths(void)
{
	try_unlink(TESTSYMLINK);
	try_unlink(TESTFILE);
	try_rmdir(TESTDIR);
}

static inline void reset_test_path(void)
{
	cleanup_test_paths();

	SAFE_MKDIR(TESTDIR, 0755);
}

static inline void cleanup_testfile(void)
{
	try_unlink(TESTFILE);
}

#ifdef HAVE_SYS_XATTR_H

/*
 * Initialize an empty ACL structure
 */
static inline struct acl *acl_init(void)
{
	struct acl *acl = SAFE_MALLOC(sizeof(struct acl));

	acl->count = 0;
	return acl;
}

/*
 * Free an ACL structure
 */
static inline void acl_free(struct acl *acl)
{
	free(acl);
}

/*
 * Add an ACL entry to the ACL structure
 */
static inline int acl_add_entry(struct acl *acl, uint16_t tag, uint16_t perm,
				uint32_t id)
{
	if (acl->count >= MAX_ACL_ENTRIES) {
		errno = ENOMEM;
		return -1;
	}

	acl->entries[acl->count].tag = tag;
	acl->entries[acl->count].perm = perm;
	acl->entries[acl->count].id = id;
	acl->count++;
	return 0;
}

/*
 * Convert ACL type to xattr name
 */
static inline const char *acl_type_to_name(int type)
{
	if (type == ACL_TYPE_ACCESS)
		return XATTR_NAME_POSIX_ACL_ACCESS;
	else if (type == ACL_TYPE_DEFAULT)
		return XATTR_NAME_POSIX_ACL_DEFAULT;
	else
		return NULL;
}

/*
 * Set ACL on a file using xattr.
 *
 * The kernel stores access ACLs only when they differ from the file mode.
 * If the ACL is equivalent to st_mode, the xattr is removed and future
 * getxattr() calls return ENODATA. Mirror libacl semantics by treating
 * ENODATA as a valid minimal ACL derived from st_mode.
 */
static inline int acl_set_file(const char *path, int type, struct acl *acl)
{
	const char *xattr_name;
	size_t size;
	char *buf;
	struct posix_acl_xattr_header *header;
	struct posix_acl_xattr_entry *entries;
	int i, ret;

	xattr_name = acl_type_to_name(type);
	if (!xattr_name) {
		errno = EINVAL;
		return -1;
	}

	size = sizeof(struct posix_acl_xattr_header) +
	       acl->count * sizeof(struct posix_acl_xattr_entry);

	buf = malloc(size);
	if (!buf)
		return -1;

	header = (struct posix_acl_xattr_header *)buf;
	header->a_version = cpu_to_le32(POSIX_ACL_XATTR_VERSION);

	entries = (struct posix_acl_xattr_entry *)(buf + sizeof(*header));

	for (i = 0; i < acl->count; i++) {
		entries[i].e_tag = cpu_to_le16(acl->entries[i].tag);
		entries[i].e_perm = cpu_to_le16(acl->entries[i].perm);
		entries[i].e_id = cpu_to_le32(acl->entries[i].id);
	}

	ret = setxattr(path, xattr_name, buf, size, 0);
	free(buf);

	return ret;
}

static inline void safe_acl_set_file(const char *file, const int lineno,
                                     const char *path, int type, struct acl *acl)
{
	if (acl_set_file(path, type, acl) < 0) {
		if (errno == EOPNOTSUPP)
			tst_brk_(file, lineno, TCONF | TERRNO, "ACL not supported");
		tst_brk_(file, lineno, TBROK | TERRNO, "ACL setup failed");
	}
}

#define SAFE_ACL_SET_FILE(path, type, acl) \
	safe_acl_set_file(__FILE__, __LINE__, (path), (type), (acl))

static inline void acl_add_mode_entries(struct acl *acl, mode_t mode)
{
	acl_add_entry(acl, ACL_USER_OBJ, (mode >> 6) & 07, 0);
	acl_add_entry(acl, ACL_GROUP_OBJ, (mode >> 3) & 07, 0);
	acl_add_entry(acl, ACL_OTHER, mode & 07, 0);
}

/*
 * Synthesize an ACL from file mode bits.
 * Used when no xattr exists for an access ACL.
 */
static inline struct acl *acl_from_mode(const char *path)
{
	struct acl *acl;
	struct stat st;

	if (stat(path, &st) < 0)
		return NULL;

	acl = acl_init();
	acl_add_mode_entries(acl, st.st_mode);

	return acl;
}

/*
 * Get ACL from a file using xattr.
 *
 * Access ACLs equivalent to file mode may not have a backing xattr at all.
 * In that case synthesize the base ACL from st_mode so callers observe the
 * same behavior as acl_get_file(3).
 */
static inline struct acl *acl_get_file(const char *path, int type)
{
	const char *xattr_name;
	ssize_t size;
	struct acl_xattr *ax;
	struct acl *acl;
	int i, count;

	xattr_name = acl_type_to_name(type);
	if (!xattr_name) {
		errno = EINVAL;
		return NULL;
	}

	size = getxattr(path, xattr_name, NULL, 0);
	if (size < 0) {
		if (errno != ENODATA || type != ACL_TYPE_ACCESS)
			return NULL;

		return acl_from_mode(path);
	}

	/* Handle race: xattr removed between size check and actual read */
	if (size == 0)
		return acl_from_mode(path);

	ax = malloc(size);
	if (!ax)
		return NULL;

	size = getxattr(path, xattr_name, ax, size);
	if (size < 0) {
		free(ax);
		/* Handle race: xattr removed between size check and read */
		if (errno == ENODATA && type == ACL_TYPE_ACCESS)
			return acl_from_mode(path);
		return NULL;
	}

	if (le32_to_cpu(ax->header.a_version) != POSIX_ACL_XATTR_VERSION) {
		free(ax);
		errno = EINVAL;
		return NULL;
	}

	count = (size - sizeof(ax->header)) /
		sizeof(struct posix_acl_xattr_entry);

	acl = acl_init();
	if (!acl) {
		free(ax);
		return NULL;
	}

	for (i = 0; i < count; i++) {
		uint16_t tag = le16_to_cpu(ax->entries[i].e_tag);
		uint16_t perm = le16_to_cpu(ax->entries[i].e_perm);
		uint32_t id = le32_to_cpu(ax->entries[i].e_id);

		if (acl_add_entry(acl, tag, perm, id) < 0) {
			acl_free(acl);
			free(ax);
			return NULL;
		}
	}

	free(ax);
	return acl;
}

/*
 * Check if an ACL entry has a specific permission
 */
static inline int acl_entry_has_perm(struct acl_entry *entry, uint16_t perm)
{
	return (entry->perm & perm) == perm;
}

/*
 * Check if an ACL entry has all rwx permissions
 */
static inline int acl_entry_has_rwx(struct acl_entry *entry)
{
	return acl_entry_has_perm(entry,
				  ACL_READ | ACL_WRITE | ACL_EXECUTE);
}

/*
 * Find an ACL entry by tag type
 */
static inline struct acl_entry *acl_find_entry(struct acl *acl, uint16_t tag,
					       uint32_t id)
{
	int i;

	for (i = 0; i < acl->count; i++) {
		if (acl->entries[i].tag == tag) {
			if (tag == ACL_USER || tag == ACL_GROUP) {
				if (acl->entries[i].id == id)
					return &acl->entries[i];
			} else {
				return &acl->entries[i];
			}
		}
	}

	return NULL;
}

/*
 * Update ACL mask permissions
 */
static inline int acl_set_mask_perms(struct acl *acl, uint16_t perm)
{
	struct acl_entry *mask_entry = acl_find_entry(acl, ACL_MASK, 0);

	if (!mask_entry) {
		errno = ENOENT;
		return -1;
	}

	mask_entry->perm = perm;
	return 0;
}

static inline void create_file_as(uid_t uid, gid_t gid, mode_t mode,
				   int use_umask, mode_t mask, int exp_errno)
{
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid) {
		uid_t fsuid;
		gid_t fsgid;
		struct tst_cap_user_header hdr = {
			.version = 0x20080522,
			.pid = 0,
		};
		struct tst_cap_user_data data[2] = {};

		SAFE_SETGROUPS(0, NULL);
		SAFE_SETRESGID(gid, gid, gid);
		SAFE_SETRESUID(uid, uid, uid);

		/* Drop all capabilities to ensure DAC checks are enforced */
		if (tst_capset(&hdr, data) < 0)
			tst_brk(TBROK | TERRNO, "capset failed");

		/*
		 * setfsuid()/setfsgid() return the previous value, not -1 on
		 * failure. Verify the effective filesystem credentials by
		 * reading them back.
		 */
		setfsuid(uid);
		fsuid = setfsuid((uid_t)-1);
		if (fsuid != uid)
			tst_brk(TBROK,
				"setfsuid verification failed, expected %u got %u",
				(unsigned int)uid, (unsigned int)fsuid);

		setfsgid(gid);
		fsgid = setfsgid((gid_t)-1);
		if (fsgid != gid)
			tst_brk(TBROK,
				"setfsgid verification failed, expected %u got %u",
				(unsigned int)gid, (unsigned int)fsgid);

		if (use_umask)
			umask(mask);

		if (exp_errno) {
			TST_EXP_FAIL2(open(TESTFILE, O_CREAT | O_WRONLY, mode),
				      exp_errno, "open(%s)", TESTFILE);
		} else {
			TST_EXP_FD(open(TESTFILE, O_CREAT | O_WRONLY, mode),
				   "open(%s)", TESTFILE);
			if (TST_RET >= 0)
				SAFE_CLOSE(TST_RET);
		}

		exit(0);
	}

	tst_reap_children();
}

static inline void try_create_as(uid_t uid, gid_t gid, mode_t mode,
				  int exp_errno)
{
	create_file_as(uid, gid, mode, 0, 0, exp_errno);
}

static inline void create_with_umask_as(uid_t uid, gid_t gid, mode_t mode,
					 mode_t mask, int exp_errno)
{
	create_file_as(uid, gid, mode, 1, mask, exp_errno);
}

#endif /* HAVE_SYS_XATTR_H */

#endif /* ACL_LIB_H */
