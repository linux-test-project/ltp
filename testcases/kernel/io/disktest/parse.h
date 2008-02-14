/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
* $Id: parse.h,v 1.5 2008/02/14 08:22:23 subrata_modak Exp $
*
*/

#ifndef _PARSE_H
#define _PARSE_H

#include <sys/stat.h>

#ifdef WINDOWS
#include "getopt.h"
#define IS_FILE(x)	(_S_IFREG & x)
#define IS_BLK(x)	(_S_IFCHR & x)

#define L_ASSUME	"(-L %I64d) "
#define N_ASSUME	"(-N %I64d) "
#define MAXTRSIZ	"Max transfer size is %lu and Volume size is %I64d\n"
#define VSIZETS		"Volume size, %I64d, is to small for transfer size, %lu.\n"
#define TSEEK		"Total seeks of %I64d, is invalid.\n"
#define SBRSB		"Stop Block of range, %I64d, must be greater the Start Block, %I64d.\n"
#define SLBARSLBA	"Stop LBA of range, %I64d, must be greater the Start LBA, %I64d.\n"
#define SLBAGTLBA	"Stop LBA, %I64d, greater then total volume LBAs, %I64d.\n"
#define STLBAGTLBA	"Start LBA, %I64d, greater then total volume LBAs, %I64d.\n"
#define SBGTTLBA	"Stop Block, %I64d, greater then total volume LBAs, %I64d.\n"
#define STBGTTLBA	"Start Block, %I64d, greater then total volume LBAs, %I64d.\n"
#define LBAOFFGSLBA	"LBA offset of %lu, is greater then stop LBA of %I64d\n"
#define LBAOTSGSLBA	"LBA offset of %lu and transfer size of %lu, is greater then stop LBA of %I64d\n"
#else
#define IS_FILE(x)	S_ISREG(x)
#define IS_BLK(x)	S_ISBLK(x)

#define L_ASSUME	"(-L %lld) "
#define N_ASSUME	"(-N %lld) "
#define MAXTRSIZ	"Max transfer size is %lu and Volume size is %lld\n"
#define VSIZETS		"Volume size, %lld, is to small for transfer size, %lu.\n"
#define TSEEK		"Total seeks of %lld, is invalid.\n"
#define SBRSB		"Stop Block of range, %lld, must be greater the Start Block, %lld.\n"
#define SLBARSLBA	"Stop LBA of range, %lld, must be greater the Start LBA, %lld.\n"
#define SLBAGTLBA	"Stop LBA, %lld, greater then total volume LBAs, %lld.\n"
#define STLBAGTLBA	"Start LBA, %lld, greater then total volume LBAs, %lld.\n"
#define SBGTTLBA	"Stop Block, %lld, greater then total volume LBAs, %lld.\n"
#define STBGTTLBA	"Start Block, %lld, greater then total volume LBAs, %lld.\n"
#define LBAOFFGSLBA	"LBA offset of %lu, is greater then stop LBA of %lld\n"
#define LBAOTSGSLBA	"LBA offset of %lu and transfer size of %lu, is greater then stop LBA of %lld\n"
#endif

#include "main.h"
#include "defs.h"

int fill_cld_args(int, char **, child_args_t *);
int make_assumptions(child_args_t *);
int check_conclusions(child_args_t *);

#endif
