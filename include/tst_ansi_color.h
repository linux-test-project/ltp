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
