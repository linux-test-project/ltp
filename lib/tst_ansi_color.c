// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "tst_res_flags.h"
#include "tst_ansi_color.h"

char* tst_ttype2color(int ttype)
{
	switch (TTYPE_RESULT(ttype)) {
	case TPASS:
		return ANSI_COLOR_GREEN;
	break;
	case TFAIL:
		return ANSI_COLOR_RED;
	break;
	case TBROK:
		return ANSI_COLOR_RED;
	break;
	case TCONF:
		return ANSI_COLOR_YELLOW;
	break;
	case TWARN:
		return ANSI_COLOR_MAGENTA;
	break;
	case TINFO:
		return ANSI_COLOR_BLUE;
	break;
	default:
		return "";
	}
}

int tst_color_enabled(int fd)
{
	static int color;

	if (color)
		return color - 1;

	char *env = getenv("LTP_COLORIZE_OUTPUT");

	if (env) {
		if (!strcmp(env, "n") || !strcmp(env, "0"))
			color = 1;

		if (!strcmp(env, "y") || !strcmp(env, "1"))
			color = 2;

		return color - 1;
	}

	if (isatty(fd) == 0)
		color = 1;
	else
		color = 2;

	return color - 1;
}
