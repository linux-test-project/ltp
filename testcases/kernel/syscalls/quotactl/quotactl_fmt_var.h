// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

#ifndef LTP_QUOTACTL_FMT_VAR_H
#define LTP_QUOTACTL_FMT_VAR_H

#include "lapi/quotactl.h"

#define QUOTACTL_FMT_VARIANTS 2

static struct quotactl_fmt_variant {
	int32_t fmt_id;
	const char *fmt_name;
} fmt_variants[] = {
	{.fmt_id = QFMT_VFS_V0, .fmt_name = "vfsv0"},
	{.fmt_id = QFMT_VFS_V1, .fmt_name = "vfsv1"}
};

#endif /* LAPI_QUOTACTL_FMT_VAR_H__ */
