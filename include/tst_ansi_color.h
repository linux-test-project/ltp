/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#ifndef TST_ANSI_COLOR_H__
#define TST_ANSI_COLOR_H__
/*
 * NOTE: these colors should match colors defined in tst_flag2color() in
 * testcases/lib/tst_ansi_color.sh
 */
#define ANSI_COLOR_BLUE		"\033[1;34m"
#define ANSI_COLOR_GREEN	"\033[1;32m"
#define ANSI_COLOR_MAGENTA	"\033[1;35m"
#define ANSI_COLOR_RED		"\033[1;31m"
#define ANSI_COLOR_YELLOW	"\033[1;33m"

#define ANSI_COLOR_RESET	"\033[0m"

char* tst_ttype2color(int ttype);
int tst_color_enabled(int fd);

#endif	/* TST_ANSI_COLOR_H__ */
