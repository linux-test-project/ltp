/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Portions Copyright (c) 2000 Ulrich Drepper
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
 */
/* $Id: fork05.c,v 1.1 2000/09/08 15:48:17 nstraz Exp $ */
/**********************************************************
 *
 *    Linux Test Project - Silicon Graphics, Inc.
 *    
 *    TEST IDENTIFIER	: fork05
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Make sure LDT is propagated correctly
 *
 *    TEST CASE TOTAL	: 1
 *
 *    CPU TYPES		: i386
 *
 *    AUTHORS		: Ulrich Drepper
 *			  Nate Straz
 *
 *On Sat, Aug 12, 2000 at 12:47:31PM -0700, Ulrich Drepper wrote:
 *> Ever since the %gs handling was fixed in the 2.3.99 series the
 *> appended test program worked.  Now with 2.4.0-test6 it's not working
 *> again.  Looking briefly over the patch from test5 to test6 I haven't
 *> seen an immediate candidate for the breakage.  It could be missing
 *> propagation of the LDT to the new process (and therefore an invalid
 *> segment descriptor) or simply clearing %gs.
 *> 
 *> Anyway, this is what you should see and what you get with test5:
 *> 
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> a = 42
 *> %gs = 0x0007
 *> %gs = 0x0007
 *> a = 99
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> 
 *> This is what you get with test6:
 *> 
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> a = 42
 *> %gs = 0x0007
 *> %gs = 0x0000
 *> <SEGFAULT>
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *> 
 *> If somebody is actually creating a test suite for the kernel, please
 *> add this program.  It's mostly self-contained.  The correct handling
 *> of %gs is really important since glibc 2.2 will make heavy use of it.
 *> 
 *> - -- 
 *> - ---------------.                          ,-.   1325 Chesapeake Terrace
 *> Ulrich Drepper  \    ,-------------------'   \  Sunnyvale, CA 94089 USA
 *> Red Hat          `--' drepper at redhat.com   `------------------------
 *> 
 *> ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *
 *********************************************************/
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

struct modify_ldt_ldt_s
{
  unsigned int entry_number;
  unsigned long int base_addr;
  unsigned int limit;
  unsigned int seg_32bit:1;
  unsigned int contents:2;
  unsigned int read_exec_only:1;
  unsigned int limit_in_pages:1;
  unsigned int seg_not_present:1;
  unsigned int useable:1;
  unsigned int empty:25;
};

asm("	.type modify_ldt,@function
modify_ldt:
	push   %ebx
	mov    0x10(%esp,1),%edx
	mov    0xc(%esp,1),%ecx
	mov    0x8(%esp,1),%ebx
	mov    $0x7b,%eax
	int    $0x80
	pop    %ebx
	ret");

int a = 42;

int
main ()
{
  struct modify_ldt_ldt_s ldt0;
  int lo;
  pid_t pid;
  int res;

  ldt0.entry_number = 0;
  ldt0.base_addr = (long) &a;
  ldt0.limit = 4;
  ldt0.seg_32bit = 1;
  ldt0.contents = 0;
  ldt0.read_exec_only = 0;
  ldt0.limit_in_pages = 0;
  ldt0.seg_not_present = 0;
  ldt0.useable = 1;
  ldt0.empty = 0;

  modify_ldt (1, &ldt0, sizeof (ldt0));

  asm ("movw %w0, %%gs" : : "q" (7));

  asm ("movl %%gs:0, %0" : "=r" (lo));
  printf ("a = %d\n", lo);

  asm ("pushl %%gs; popl %0" : "=q" (lo));
  printf ("%%gs = %#06hx\n", lo);

  asm ("movl %0, %%gs:0" : : "r" (99));

  pid = fork ();

  if (pid == 0) {
      asm ("pushl %%gs; popl %0" : "=q" (lo));
      printf ("%%gs = %#06hx\n", lo);

      asm ("movl %%gs:0, %0" : "=r" (lo));
      printf ("a = %d\n", lo);

      exit (lo != 99);
  } else {
      waitpid (pid, &res);
  }

  return res;
}
