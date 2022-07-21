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
enum tst_cg_ver {
	TST_CG_V1 = 1,
	TST_CG_V2 = 2,
};

/* Used to specify CGroup hierarchy configuration options, allowing a
 * test to request a particular CGroup structure.
 */
struct tst_cg_opts {
	/* Call tst_brk with TCONF if the controller is not on this
	 * version. Defautls to zero to accept any version.
	 */
	enum tst_cg_ver needs_ver;
	/* Pass in a specific pid to create and identify the test
	 * directory as opposed to the default pid of the calling process.
	 */
	int test_pid;
};

/* A Control Group in LTP's aggregated hierarchy */
struct tst_cg_group;

/* Populated with a reference to this tests's CGroup */
extern const struct tst_cg_group *const tst_cg;
extern const struct tst_cg_group *const tst_cg_drain;

/* Search the system for mounted cgroups and available
 * controllers. Called automatically by tst_cg_require.
 */
void tst_cg_scan(void);
/* Print the config detected by tst_cg_scan and print the internal
 * state associated with each controller. Output can be passed to
 * tst_cg_load_config to configure the internal state to that of the
 * config between program invocations.
 */
void tst_cg_print_config(void);

/* Load the config printed out by tst_cg_print_config and configure the internal
 * libary state to match the config. Used to allow tst_cg_cleanup to properly
 * cleanup mounts and directories created by tst_cg_require between program
 * invocations.
 */
void tst_cg_load_config(const char *const config);

/* Ensure the specified controller is available in the test's default
 * CGroup, mounting/enabling it if necessary. Usually this is not
 * necessary use tst_test.needs_cgroup_ctrls instead.
 */
void tst_cg_require(const char *const ctrl_name,
			const struct tst_cg_opts *const options)
			__attribute__ ((nonnull));

/* Tear down any CGroups created by calls to tst_cg_require */
void tst_cg_cleanup(void);

/* Call this in setup after you call tst_cg_require and want to
 * initialize tst_cg and tst_cg_drain. See tst_cg_require.
 */
void tst_cg_init(void);

/* Create a descendant CGroup */
struct tst_cg_group *
tst_cg_group_mk(const struct tst_cg_group *const parent,
		    const char *const group_name_fmt, ...)
	    __attribute__ ((nonnull, warn_unused_result, format (printf, 2, 3)));

const char *
tst_cg_group_name(const struct tst_cg_group *const cg)
		      __attribute__ ((nonnull, warn_unused_result));

/* Remove a descendant CGroup */
struct tst_cg_group *
tst_cg_group_rm(struct tst_cg_group *const cg)
		    __attribute__ ((nonnull, warn_unused_result));

#define TST_CG_VER(cg, ctrl_name) \
	tst_cg_ver(__FILE__, __LINE__, (cg), (ctrl_name))

enum tst_cg_ver tst_cg_ver(const char *const file, const int lineno,
				   const struct tst_cg_group *const cg,
				   const char *const ctrl_name)
				   __attribute__ ((nonnull, warn_unused_result));

#define TST_CG_VER_IS_V1(cg, ctrl_name) \
	(TST_CG_VER((cg), (ctrl_name)) == TST_CG_V1)

#define SAFE_CG_HAS(cg, file_name) \
	safe_cg_has(__FILE__, __LINE__, (cg), (file_name))

int safe_cg_has(const char *const file, const int lineno,
		    const struct tst_cg_group *const cg,
		    const char *const file_name)
		    __attribute__ ((nonnull, warn_unused_result));

#define SAFE_CG_READ(cg, file_name, out, len)			\
	safe_cg_read(__FILE__, __LINE__,				\
			 (cg), (file_name), (out), (len))

ssize_t safe_cg_read(const char *const file, const int lineno,
			 const struct tst_cg_group *const cg,
			 const char *const file_name,
			 char *const out, const size_t len)
			 __attribute__ ((nonnull));

#define SAFE_CG_PRINTF(cg, file_name, fmt, ...)			\
	safe_cg_printf(__FILE__, __LINE__,				\
			   (cg), (file_name), (fmt), __VA_ARGS__)

#define SAFE_CG_PRINT(cg, file_name, str)				\
	safe_cg_printf(__FILE__, __LINE__, (cg), (file_name), "%s", (str))

void safe_cg_printf(const char *const file, const int lineno,
			const struct tst_cg_group *const cg,
			const char *const file_name,
			const char *const fmt, ...)
			__attribute__ ((format (printf, 5, 6), nonnull));

#define SAFE_CG_SCANF(cg, file_name, fmt, ...)			\
	safe_cg_scanf(__FILE__, __LINE__,				\
			  (cg), (file_name), (fmt), __VA_ARGS__)

void safe_cg_scanf(const char *file, const int lineno,
		       const struct tst_cg_group *const cg,
		       const char *const file_name,
		       const char *const fmt, ...)
		       __attribute__ ((format (scanf, 5, 6), nonnull));

#define SAFE_CG_LINES_SCANF(cg, file_name, fmt, ...)		\
	safe_cg_lines_scanf(__FILE__, __LINE__,			\
				(cg), (file_name), (fmt), __VA_ARGS__)

void safe_cg_lines_scanf(const char *const file, const int lineno,
			     const struct tst_cg_group *const cg,
			     const char *const file_name,
			     const char *const fmt, ...)
			__attribute__ ((format (scanf, 5, 6), nonnull));

#define SAFE_CG_OCCURSIN(cg, file_name, needle)		\
	safe_cg_occursin(__FILE__, __LINE__,		\
			     (cg), (file_name), (needle))

int safe_cg_occursin(const char *file, const int lineno,
			 const struct tst_cg_group *const cg,
			 const char *const file_name,
			 const char *const needle);

#endif /* TST_CGROUP_H */
