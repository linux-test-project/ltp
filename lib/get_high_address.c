/* $Header: /cvsroot/ltp/ltp/lib/get_high_address.c,v 1.1 2000/08/04 20:48:22 nstraz Exp $ */

/*
 *	(C) COPYRIGHT CRAY RESEARCH, INC.
 *	UNPUBLISHED PROPRIETARY INFORMATION.
 *	ALL RIGHTS RESERVED.
 */

#include <unistd.h> 

char *
get_high_address()
{
       return (char *)sbrk(0) + 16384;
}
