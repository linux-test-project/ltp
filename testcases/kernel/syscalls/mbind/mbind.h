/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef MBIND_H__
#define MBIND_H__

static inline const char *mbind_flag_name(unsigned flag)
{
	switch (flag) {
	case 0:
		return "0";
	case MPOL_MF_STRICT:
		return "MPOL_MF_STRICT";
	case MPOL_MF_MOVE:
		return "MPOL_MF_MOVE";
	case MPOL_MF_MOVE_ALL:
		return "MPOL_MF_MOVE_ALL";
	default:
		return "???";
	}
}

#endif /* MBIND_H__ */
