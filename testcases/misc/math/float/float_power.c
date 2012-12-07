/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/******************************************************************************/
/*                                                                            */
/* Dec-03-2001  Created: Jacky Malcles & Jean Noel Cordenner                  */
/*              These tests are adapted from AIX float PVT tests.             */
/*                                                                            */
/******************************************************************************/
#include "tfloat.h"

const TH_FUNC th_func[] = {
	{FUNC_NORMAL, 50, ceil, "ceil", "ceil_inp.ref", "ceil_out.ref",
	 ""},
	{FUNC_NORMAL, 50, fabs, "fabs", "fabs_inp.ref", "fabs_out.ref",
	 ""},
	{FUNC_NORMAL, 50, floor, "floor", "floor_inp.ref", "floor_out.ref",
	 ""},
	{FUNC_FMOD, 50, fmod, "fmod", "fmod_inp.ref", "fmod_out.ref",
	 "1fmod_inp.ref"},
	{FUNC_POW, 47, pow, "pow", "pow_inp.ref", "pow_out.ref",
	 "1pow_inp.ref"},
	{FUNC_NORMAL, 50, sqrt, "sqrt", "sqrt_inp.ref", "sqrt_out.ref",
	 ""}
};

#define NB_FUNC  (sizeof(th_func)/sizeof(TH_FUNC))

#define GENERATOR "genpower"
#include "main.c"
#include "thread_code.c"
