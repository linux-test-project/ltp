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

/* $Id: mc_getopt.h,v 1.2 2000/07/30 19:34:14 alaffin Exp $ */
#ifndef __MC_GETOPT_H__
#define __MC_GETOPT_H__ 1

/*
 * Possible return value of mc_getopt
 */

#define MC_UNKNOWN_OPTION       "UNKNOWN_OPTION"
#define MC_AMBIGUOUS_OPTION     "AMBIGUOUS_OPTION"
#define MC_MISSING_OPTARG       "MISSING_OPTARG"
#define MC_DONE                 NULL

/*
 * Mc_getopt flags argument bits
 */
#define MC_FULL_TOKEN_MATCH     01
#define MC_CASE_INSENSITIVE     02

extern int   mc_optind;
extern char *mc_optarg;
extern char *mc_optopt;

/*
 * Prototypes
 */

extern char *mc_getopt(int argc, char * const argv[], int flags, const char *optstring);
extern char *mc_getoptv(int argc, char * const argv[], int flags, int nopts, char * const opt_arr[]);

#endif  /* __MC_GETOPT_H__ */

