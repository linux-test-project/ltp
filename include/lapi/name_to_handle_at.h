// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Linaro Limited. All rights reserved.
 * Author: Viresh Kumar <viresh.kumar@linaro.org>
 */

#ifndef LAPI_NAME_TO_HANDLE_AT_H__
#define LAPI_NAME_TO_HANDLE_AT_H__

#include <sys/syscall.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "lapi/fcntl.h"
#include "tst_buffers.h"

#ifndef HAVE_NAME_TO_HANDLE_AT
static inline int name_to_handle_at(int dfd, const char *pathname,
                                    struct file_handle *handle,
                                    int *mount_id, int flags)
{
	return tst_syscall(__NR_name_to_handle_at, dfd, pathname, handle,
			   mount_id, flags);
}

static inline int open_by_handle_at(int mount_fd, struct file_handle *handle,
                                    int flags)
{
	return tst_syscall(__NR_open_by_handle_at, mount_fd, handle, flags);
}
#endif /* HAVE_NAME_TO_HANDLE_AT */

/* Returns a valid pointer on success, NULL on errors */
static inline struct file_handle *
allocate_file_handle(int dfd, const char *pathname)
{
	long ret;
	struct file_handle fh = {}, *fhp;
	int mount_id;

	/*
	 * Make an initial call to name_to_handle_at() to discover the size
	 * required for the file handle.
	 */
	ret = name_to_handle_at(dfd, pathname, &fh, &mount_id, 0);
	if (ret != -1 || errno != EOVERFLOW) {
		tst_res(TFAIL | TERRNO,
			"name_to_handle_at() should fail with EOVERFLOW");
		return NULL;
	}

	/* Valid file handle */
	fhp = tst_alloc(sizeof(*fhp) + fh.handle_bytes);
	fhp->handle_type = fh.handle_type;
	fhp->handle_bytes = fh.handle_bytes;

	return fhp;
}

#endif /* LAPI_NAME_TO_HANDLE_AT_H__ */
