/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2005               */
/*                                                                            */
/*   This program is free software;  you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by     */
/*   the Free Software Foundation; either version 2 of the License, or        */
/*   (at your option) any later version.                                      */
/*                                                                            */
/*   This program is distributed in the hope that it will be useful,          */
/*   but WITHOUT ANY WARRANTY;  without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                */
/*   the GNU General Public License for more details.                         */
/*                                                                            */
/*   You should have received a copy of the GNU General Public License        */
/*   along with this program;  if not, write to the Free Software             */
/*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  */
/*                                                                            */
/******************************************************************************/


/*
 * File:
 *	ns-traffic.h
 *
 * Description:
 *	Header file for TCP/UDP traffic utilities
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/


/* 
 * Fixed Values
 */
#define PORTNUMMIN IPPORT_RESERVED + 1
#define PORTNUMMAX 0xFFFF

/*
 * Gloval variables
 */
#ifdef NS_COMMON
#  define EXTERN
#else
#  define EXTERN extern
#endif
EXTERN int debug;		/* If nonzero, output debug information. */


/* 
 * Functions in ns-common.c
 */
void fatal_error(char *errmsg);
void maximize_sockbuf(int sd);
