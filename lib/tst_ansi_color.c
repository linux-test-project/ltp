/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
