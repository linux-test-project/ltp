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
#include <stdio.h>
#include <string.h>
#include "dataascii.h"

#define CHARS		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghjiklmnopqrstuvwxyz\n"
#define CHARS_SIZE	sizeof(CHARS)

#ifdef UNIT_TEST
#include <stdlib.h> /* malloc */
#endif

static char Errmsg[80];

int
dataasciigen(listofchars, buffer, bsize, offset)
char *listofchars;	/* a null terminated list of characters */
char *buffer;
int bsize;
int offset;
{
   int cnt;
   int total;
   int ind;	/* index into CHARS array */
   char *chr;
   int chars_size;
   char *charlist;

	chr=buffer;
	total=offset+bsize;

	if ( listofchars == NULL ) {
	    charlist=CHARS;
	    chars_size=CHARS_SIZE;
	}
	else {
	    charlist=listofchars;
	    chars_size=strlen(listofchars);
	}

	for(cnt=offset; cnt<total;  cnt++) {
		ind=cnt%chars_size;
		*chr++=charlist[ind];
	}

	return bsize;

}	/* end of dataasciigen */

int
dataasciichk(listofchars, buffer, bsize, offset, errmsg)
char *listofchars;	/* a null terminated list of characters */
char *buffer;
int bsize;
int offset;
char **errmsg;
{
   int cnt;
   int total;
   int ind;	/* index into CHARS array */
   char *chr;
   int chars_size;
   char *charlist;

	chr=buffer;
	total=offset+bsize;

	if ( listofchars == NULL ) {
	    charlist=CHARS;
	    chars_size=CHARS_SIZE;
	}
	else {
	    charlist=listofchars;
	    chars_size=strlen(listofchars);
	}

	if ( errmsg != NULL ) {
	    *errmsg = Errmsg;
	}

	for(cnt=offset; cnt<total;  chr++, cnt++) {
	    ind=cnt%chars_size;
	    if ( *chr != charlist[ind] ) {
		sprintf(Errmsg,
		    "data mismatch at offset %d, exp:%#o, act:%#o", cnt,
		    charlist[ind], *chr);
		return cnt;
	    }
	}

	sprintf(Errmsg, "all %d bytes match desired pattern", bsize);
	return -1;	/* buffer is ok */

}	/* end of dataasciichk */


#if UNIT_TEST

/***********************************************************************
 * main for doing unit testing
 ***********************************************************************/
int
main(ac, ag)
int ac;
char **ag;
{

int size=1023;
char *buffer;
int ret;
char *errmsg;

    if ((buffer=(char *)malloc(size)) == NULL ) {
        perror("malloc");
        exit(2);
    }

    dataasciigen(NULL, buffer, size, 0);
    printf("dataasciigen(NULL, buffer, %d, 0)\n", size);

    ret=dataasciichk(NULL, buffer, size, 0, &errmsg);
    printf("dataasciichk(NULL, buffer, %d, 0, &errmsg) returned %d %s\n",
        size, ret, errmsg);

    if ( ret == -1 )
        printf("\tPASS return value is -1 as expected\n");
    else
        printf("\tFAIL return value is %d, expected -1\n", ret);

    ret=dataasciichk(NULL, &buffer[1], size-1, 1, &errmsg);
    printf("dataasciichk(NULL, &buffer[1], %d, 1, &errmsg) returned %d %s\n",
        size-1, ret, errmsg);

    if ( ret == -1 )
        printf("\tPASS return value is -1 as expected\n");
    else
        printf("\tFAIL return value is %d, expected -1\n", ret);

    buffer[25]= 0x0;
    printf("changing char 25\n");

    ret=dataasciichk(NULL, &buffer[1], size-1, 1, &errmsg);
    printf("dataasciichk(NULL, &buffer[1], %d, 1, &errmsg) returned %d %s\n",
        size-1, ret, errmsg);

    if ( ret == 25 )
	printf("\tPASS return value is 25 as expected\n");
    else
	printf("\tFAIL return value is %d, expected 25\n", ret);

    dataasciigen("this is a test of the my string" , buffer, size, 0);
    printf("dataasciigen(\"this is a test of the my string\", buffer, %d, 0)\n", size);

    ret=dataasciichk("this is a test of the my string", buffer, size, 0, &errmsg);
    printf("dataasciichk(\"this is a test of the my string\", buffer, %d, 0, &errmsg) returned %d %s\n",
        size, ret, errmsg);

    if ( ret == -1 )
        printf("\tPASS return value is -1 as expected\n");
    else
        printf("\tFAIL return value is %d, expected -1\n", ret);

    ret=dataasciichk("this is a test of the my string", &buffer[1], size-1, 1, &errmsg);
    printf("dataasciichk(\"this is a test of the my string\", &buffer[1], %d, 1, &errmsg) returned %d %s\n",
        size-1, ret, errmsg);

    if ( ret == -1 )
        printf("\tPASS return value is -1 as expected\n");
    else
        printf("\tFAIL return value is %d, expected -1\n", ret);

    buffer[25]= 0x0;
    printf("changing char 25\n");

    ret=dataasciichk("this is a test of the my string", &buffer[1], size-1, 1, &errmsg);
    printf("dataasciichk(\"this is a test of the my string\", &buffer[1], %d, 1, &errmsg) returned %d %s\n",
        size-1, ret, errmsg);

    if ( ret == 25 )
	printf("\tPASS return value is 25 as expected\n");
    else
	printf("\tFAIL return value is %d, expected 25\n", ret);

    exit(0);
}

#endif

