/*
 * Default cleanup logic because linux_syscall_numbers.h's need for cleanup
 * and binutils bugs suck.
 *
 * Copyright (c) 2009 Cisco Systems, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
 
#ifndef __CLEANUP_C__
#define __CLEANUP_C__

/* Did the user define a cleanup function? */
#ifndef CLEANUP
# define USING_DUMMY_CLEANUP 1
# define CLEANUP dummy_cleanup
#endif

/* A freebie for defining the function prototype. */
static void CLEANUP() __attribute__((unused));

#ifdef USING_DUMMY_CLEANUP
/* The stub function. Wewt.. */
static void dummy_cleanup() { }
#endif

#endif
