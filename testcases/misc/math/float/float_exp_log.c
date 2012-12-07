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
	{FUNC_NORMAL, 50, exp, "exp", "exp_inp.ref", "exp_out.ref",
	 ""},
	{FUNC_NORMAL, 50, log, "log", "log_inp.ref", "log_out.ref2",
	 ""},
	{FUNC_NORMAL, 50, log10, "log10", "log10_inp.ref", "log10_out.ref",
	 ""},
	{FUNC_FREXP, 50, frexp, "frexp", "frexp_inp.ref", "frexp_out.ref",
	 "frexp1_out.ref"},
	{FUNC_HYPOT, 50, hypot, "hypot", "hypot_inp.ref", "hypot_out.ref",
	 ""},
	{FUNC_LDEXP, 50, ldexp, "ldexp", "ldexp_inp.ref", "ldexp_out.ref",
	 "ildexp_inp.ref"},
	{FUNC_MODF, 50, modf, "modf", "modf_inp.ref", "modf_out.ref",
	 "modf1_out.ref"}
};

#define NB_FUNC  (sizeof(th_func)/sizeof(TH_FUNC))

#define GENERATOR "genexp_log"
#include "main.c"
#include "thread_code.c"
