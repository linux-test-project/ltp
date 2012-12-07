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
	{FUNC_NORMAL, 50, acos, "acos", "dacos", "racos",
	 ""},
	{FUNC_NORMAL, 50, asin, "asin", "dasin", "rasin",
	 ""},
	{FUNC_NORMAL, 50, atan, "atan", "datan", "ratan",
	 ""},
	{FUNC_ATAN2, 50, atan2, "atan2", "datan2", "ratan2",
	 ""},
	{FUNC_NORMAL, 50, cos, "cos", "dcos", "rcos",
	 ""},
	{FUNC_NORMAL, 50, sin, "sin", "dsin", "rsin",
	 ""},
	{FUNC_NORMAL, 50, tan, "tan", "dtan", "rtan",
	 ""}
};

#define NB_FUNC  (sizeof(th_func)/sizeof(TH_FUNC))

#define GENERATOR "gentrigo"
#include "main.c"
#include "thread_code.c"
