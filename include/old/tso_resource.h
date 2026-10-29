// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
 * Copyright (c) Linux Test Project, 2026
 */

 /*
  * Small helper for preparing files the test needs to copy before the testing.
  * We need to support two scenarios.
  *
  * 1. Test is executed in local directory and this is also the place
  *     we should look for files
  *
  * 2. Test is executed after LTP has been installed, in this case we
  *     look for env LTPROOT (usually /opt/ltp/)
  */

#ifndef TST_RESOURCE
#define TST_RESOURCE

const char *tst_dataroot(void);

/*
 * Copy a file to the CWD. The destination is apended to CWD.
 */
#define TST_RESOURCE_COPY(cleanup_fn, filename, dest) \
	tst_resource_copy(__FILE__, __LINE__, (cleanup_fn), \
	                  (filename), (dest))

void tst_resource_copy(const char *file, const int lineno,
                       void (*cleanup_fn)(void),
		       const char *filename, const char *dest);

#endif /* TST_RESOURCE */
