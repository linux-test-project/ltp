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
	{FUNC_NORMAL, 30, j0, "j0", "j0_inp.ref", "j0_out.ref2",
	 ""},
	{FUNC_NORMAL, 30, j1, "j1", "j1_inp.ref", "j1_out.ref2",
	 ""},
	{FUNC_NORMAL, 30, y0, "y0", "y0_inp.ref", "y0_out.ref2",
	 ""},
	{FUNC_NORMAL, 30, y1, "y1", "y1_inp.ref", "y1_out.ref2",
	 ""},
	{FUNC_GAM, 30, lgamma, "lgamma", "gamma_inp.ref", "gamma_out.ref",
	 "gamma_sign.ref"}
};

#define NB_FUNC  (sizeof(th_func)/sizeof(TH_FUNC))

#define GENERATOR "genbessel"
#include "main.c"
#include "thread_code.c"
