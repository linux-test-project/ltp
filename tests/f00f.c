/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
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
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: f00f.c,v 1.1 2000/09/08 15:04:19 alaffin Exp $ */
/*
 * This is a simple test for handling of the pentium f00f bug.
 * It is an example of a catistrophic test case.  If the system
 * doesn't correctly handle this test, it will likely lockup.
 */
#include <signal.h>
#ifdef __i386__

/*
 * an f00f instruction
 */
char x[5] = { 0xf0, 0x0f, 0xc7, 0xc8 };

void
sigill (int sig)
{
  printf ("SIGILL received from f00f instruction.  Good.\n");
  exit (0);
}

int 
main ()
{
  void (*f) () = (void *) x;

  signal (SIGILL, sigill);
  printf ("Testing for proper f00f instruction handling.\n");

  f ();

  /*
   * we shouldn't get here, the f00f instruction should trigger
   * a SIGILL or lock the system.
   */
  exit (1);
}

#else /* __i386__ */

int
main ()
{
  printf ("f00f bug test only for i386\n");
  exit (0);
}

#endif /* __i386__ */
