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
 */
#ifndef _FORKER_H_
#define _FORKER_H_

#define FORKER_MAX_PIDS	4098

extern int Forker_pids[FORKER_MAX_PIDS];      /* holds pids of forked processes */
extern int Forker_npids;               /* number of entries in Forker_pids */

/*
 * This function will fork and the parent will exit zero and
 * the child will return.  This will orphan the returning process
 * putting it in the background.
 */
int background( char * );

/*
 * Forker will fork ncopies-1 copies of self. 
 *
 * arg 1: Number of copies of the process to be running after return.
 *        This value minus one is the number of forks performed. 
 * arg 2: mode: 0 - all children are first generation descendents.
 *              1 - each subsequent child is a descendent of another
 *              descendent, resulting in only one direct descendent of the
 *              parent and each other child is a child of another child in
 *              relation to the parent.
 * arg 3: prefix: string to preceed any error messages.  A value of NULL
 *              results in no error messages on failure.
 * returns: returns to parent the number of children forked.
 */
int forker( int , int , char * );

#endif /* _FORKER_H_ */
