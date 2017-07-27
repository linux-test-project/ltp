/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
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

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_personality.h"

int tst_personality(const char *filename, unsigned int lineno,
		    unsigned long persona)
{
	int prev_persona = personality(persona);

	if (prev_persona < 0) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			 "persona(%ld) failed", persona);
	}

	return prev_persona;
}
