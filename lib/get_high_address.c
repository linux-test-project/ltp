/* $Header: /cvsroot/ltp/ltp/lib/get_high_address.c,v 1.2 2002/01/17 21:56:50 iyermanoj Exp $ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 

char *
get_high_address()
{
       return (char *)sbrk(0) + getpagesize();
}
