/* $Header: /cvsroot/ltp/ltp/lib/get_high_address.c,v 1.3 2002/01/24 04:58:00 iyermanoj Exp $ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 

char *
get_high_address()
{
       return (char *)sbrk(0) + (4 * getpagesize());
}
