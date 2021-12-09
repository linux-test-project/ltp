// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Red Hat, Inc.
 * Copyright (c) 2020 Li Wang <liwang@redhat.com>
 * Copyright (c) 2020-2021 SUSE LLC <rpalethorpe@suse.com>
 */
/*\
 * [Description]
 *
 * The LTP CGroups API tries to present a consistent interface to the
 * many possible CGroup configurations a system could have.
 *
 * You may ask; "Why don't you just mount a simple CGroup hierarchy,
 * instead of scanning the current setup?". The short answer is that
 * it is not possible unless no CGroups are currently active and
 * almost all of our users will have CGroups active. Even if
 * unmounting the current CGroup hierarchy is a reasonable thing to do
 * to the sytem manager, it is highly unlikely the CGroup hierarchy
 * will be destroyed. So users would be forced to remove their CGroup
 * configuration and reboot the system.
 *
 * The core library tries to ensure an LTP CGroup exists on each
 * hierarchy root. Inside the LTP group it ensures a 'drain' group
 * exists and creats a test group for the current test. In the worst
 * case we end up with a set of hierarchies like the follwoing. Where
 * existing system-manager-created CGroups have been omitted.
 *
 *	(V2 Root)	(V1 Root 1)	...	(V1 Root N)
 *	    |		     |			     |
 *	  (ltp)		   (ltp)	...	   (ltp)
 *	 /     \	  /	\		  /	\
 *  (drain) (test-n) (drain)  (test-n)  ...  (drain)  (test-n)
 *
 * V2 CGroup controllers use a single unified hierarchy on a single
 * root. Two or more V1 controllers may share a root or have their own
 * root. However there may exist only one instance of a controller.
 * So you can not have the same V1 controller on multiple roots.
 *
 * It is possible to have both a V2 hierarchy and V1 hierarchies
 * active at the same time. Which is what is shown above. Any
 * controllers attached to V1 hierarchies will not be available in the
 * V2 hierarchy. The reverse is also true.
 *
 * Note that a single hierarchy may be mounted multiple
 * times. Allowing it to be accessed at different locations. However
 * subsequent mount operations will fail if the mount options are
 * different from the first.
 *
 * The user may pre-create the CGroup hierarchies and the ltp CGroup,
 * otherwise the library will try to create them. If the ltp group
 * already exists and has appropriate permissions, then admin
 * privileges will not be required to run the tests.
 *
 * Because the test may not have access to the CGroup root(s), the
 * drain CGroup is created. This can be used to store processes which
 * would otherwise block the destruction of the individual test CGroup
 * or one of its descendants.
 *
 * The test author may create child CGroups within the test CGroup
 * using the CGroup Item API. The library will create the new CGroup
 * in all the relevant hierarchies.
 *
 * There are many differences between the V1 and V2 CGroup APIs. If a
 * controller is on both V1 and V2, it may have different parameters
 * and control files. Some of these control files have a different
 * name, but similar functionality. In this case the Item API uses
 * the V2 names and aliases them to the V1 name when appropriate.
 *
 * Some control files only exist on one of the versions or they can be
 * missing due to other reasons. The Item API allows the user to check
 * if the file exists before trying to use it.
 *
 * Often a control file has almost the same functionality between V1
 * and V2. Which means it can be used in the same way most of the
 * time, but not all. For now this is handled by exposing the API
 * version a controller is using to allow the test author to handle
 * edge cases. (e.g. V2 memory.swap.max accepts "max", but V1
 * memory.memsw.limit_in_bytes does not).
 */

#ifndef TST_CGROUP_H
#define TST_CGROUP_H

#include <sys/types.h>

/* CGroups Kernel API version */
enum tst_cgroup_ver {
	TST_CGROUP_V1 = 1,
	TST_CGROUP_V2 = 2,
};

/* Used to specify CGroup hierarchy configuration options, allowing a
 * test to request a particular CGroup structure.
 */
struct tst_cgroup_opts {
	/* Only try to mount V1 CGroup controllers. This will not
	 * prevent V2 from being used if it is already mounted, it
	 * only indicates that we should mount V1 controllers if
	 * nothing is present. By default we try to mount V2 first. */
	int only_mount_v1:1;
};

/* A Control Group in LTP's aggregated hierarchy */
struct tst_cgroup_group;

/* Search the system for mounted cgroups and available
 * controllers. Called automatically by tst_cgroup_require.
 */
void tst_cgroup_scan(void);
/* Print the config detected by tst_cgroup_scan */
void tst_cgroup_print_config(void);

/* Ensure the specified controller is available in the test's default
 * CGroup, mounting/enabling it if necessary */
void tst_cgroup_require(const char *const ctrl_name,
			const struct tst_cgroup_opts *const options)
			__attribute__ ((nonnull (1)));

/* Tear down any CGroups created by calls to tst_cgroup_require */
void tst_cgroup_cleanup(void);

/* Get the default CGroup for the test. It allocates memory (in a
 * guarded buffer) so should always be called from setup
 */
const struct tst_cgroup_group *tst_cgroup_get_test_group(void)
	__attribute__ ((warn_unused_result));
/* Get the shared drain group. Also should be called from setup */
const struct tst_cgroup_group *tst_cgroup_get_drain_group(void)
	__attribute__ ((warn_unused_result));

/* Create a descendant CGroup */
struct tst_cgroup_group *
tst_cgroup_group_mk(const struct tst_cgroup_group *const parent,
		    const char *const group_name)
		    __attribute__ ((nonnull, warn_unused_result));
const char *
tst_cgroup_group_name(const struct tst_cgroup_group *const cg)
		      __attribute__ ((nonnull, warn_unused_result));

/* Remove a descendant CGroup */
struct tst_cgroup_group *
tst_cgroup_group_rm(struct tst_cgroup_group *const cg)
		    __attribute__ ((nonnull, warn_unused_result));

#define TST_CGROUP_VER(cg, ctrl_name) \
	tst_cgroup_ver(__FILE__, __LINE__, (cg), (ctrl_name))

enum tst_cgroup_ver tst_cgroup_ver(const char *const file, const int lineno,
				   const struct tst_cgroup_group *const cg,
				   const char *const ctrl_name)
				   __attribute__ ((nonnull, warn_unused_result));

#define SAFE_CGROUP_HAS(cg, file_name) \
	safe_cgroup_has(__FILE__, __LINE__, (cg), (file_name))

int safe_cgroup_has(const char *const file, const int lineno,
		    const struct tst_cgroup_group *const cg,
		    const char *const file_name)
		    __attribute__ ((nonnull, warn_unused_result));

#define SAFE_CGROUP_READ(cg, file_name, out, len)			\
	safe_cgroup_read(__FILE__, __LINE__,				\
			 (cg), (file_name), (out), (len))

ssize_t safe_cgroup_read(const char *const file, const int lineno,
			 const struct tst_cgroup_group *const cg,
			 const char *const file_name,
			 char *const out, const size_t len)
			 __attribute__ ((nonnull));

#define SAFE_CGROUP_PRINTF(cg, file_name, fmt, ...)			\
	safe_cgroup_printf(__FILE__, __LINE__,				\
			   (cg), (file_name), (fmt), __VA_ARGS__)

#define SAFE_CGROUP_PRINT(cg, file_name, str)				\
	safe_cgroup_printf(__FILE__, __LINE__, (cg), (file_name), "%s", (str))

void safe_cgroup_printf(const char *const file, const int lineno,
			const struct tst_cgroup_group *const cg,
			const char *const file_name,
			const char *const fmt, ...)
			__attribute__ ((format (printf, 5, 6), nonnull));

#define SAFE_CGROUP_SCANF(cg, file_name, fmt, ...)			\
	safe_cgroup_scanf(__FILE__, __LINE__,				\
			  (cg), (file_name), (fmt), __VA_ARGS__)

void safe_cgroup_scanf(const char *file, const int lineno,
		       const struct tst_cgroup_group *const cg,
		       const char *const file_name,
		       const char *fmt, ...)
		       __attribute__ ((format (scanf, 5, 6), nonnull));

#define SAFE_CGROUP_OCCURSIN(cg, file_name, needle)		\
	safe_cgroup_occursin(__FILE__, __LINE__,		\
			     (cg), (file_name), (needle))

int safe_cgroup_occursin(const char *file, const int lineno,
			 const struct tst_cgroup_group *const cg,
			 const char *const file_name,
			 const char *const needle);

#endif /* TST_CGROUP_H */
