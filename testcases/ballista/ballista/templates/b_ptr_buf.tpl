// b_ptr_buf.tpl : Ballista Datatype Template for a buffer pointer
// Copyright (C) 1998-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

name void* b_ptr_buf;

parent b_ptr_void;

includes
[
{
#include <stdlib.h>
#include <values.h>
#include <limits.h>
#include "bTypes.h"
#include "b_ptr_void.h"
}
]

global_defines
[
{
#define bm_PAGESIZE 4096
#define fillbuf(buf,len)   for (int i=0; i<len; i++) buf[i] = 'a'
static char *save_loc_buf = NULL;	

// a generic function to eat up clock cycles, and give us something
//  to point to
int fib(int i) 
{
   if (i <= 1) 
   {
     return (1);
   } 
   else 
   {
     return (fib(i-1) + fib(i-2));
   }
}

}
]

dials
[
  enum_dial BUF_SIZE : BUF_SMALL,BUF_MED,BUF_LARGE,BUF_XLARGE,BUF_HUGE,BUFMAX,BUF_64k,BUF_END_MED,BUF_FAR_PAST,BUF_ODD,BUF_FREED,BUF_CODE,BUF_LOW;
]

access
[
{
   char *buf_ptr;
   const int buf_SMALL = 1;             /* size of small buf */
   const int buf_MED = bm_PAGESIZE;     /* size of medium buf */

   // Setup log file for template information

   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      fclose(logFile);
      exit(99);
   }
}

 BUF_SMALL
 {
    save_loc_buf = buf_ptr = (char *) malloc (buf_SMALL);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_SMALL, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, buf_SMALL);
    _theVariable = buf_ptr;
 }

 BUF_MED
 {
    save_loc_buf = buf_ptr = (char *)malloc (buf_MED);  /* try to put this on a page boundary */
    if(buf_ptr == NULL)
    {
        fprintf(logFile, "malloc failed in b_ptr_buf - BUF_MED, function not tested\n");
	fclose(logFile);
        exit(99);
    }
    fillbuf(buf_ptr, buf_MED);
    _theVariable = buf_ptr;
 }

 BUF_LARGE
 {
    save_loc_buf = buf_ptr = (char *)malloc ((1 << 29) + 1);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_LARGE, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    /* don't fill this one because it takes too long */
    _theVariable = buf_ptr;
 }

 BUF_XLARGE
 {
   save_loc_buf = buf_ptr = (char *)malloc ((1 << 30) + 1);
   if(buf_ptr == NULL)
   {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_XLARGE, function not tested\n");
	fclose(logFile);
	exit(99);
   }
   /* don't fill this one because it takes too long */
   _theVariable= buf_ptr;

 }	

 BUF_HUGE
 {
    save_loc_buf = buf_ptr =(char *) malloc (( 1 << 31) + 1);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_HUGE, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    /* don't fill this one because it takes too long */
    _theVariable= buf_ptr;
 }

 BUFMAX
 {
    save_loc_buf = buf_ptr =(char *) malloc (ULONG_MAX);
    if(buf_ptr == NULL)
    {
        fprintf(logFile, "malloc failed in b_ptr_buf - BUFMAX, function not tested\n");
	fclose(logFile);
        exit(99);
    }
    /* don't fill this one because it takes too long */
    _theVariable= buf_ptr;
 }


 BUF_64k
 {
    save_loc_buf = buf_ptr = (char *)malloc ((1 << 16) + 1);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_64K, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, (1 << 16) + 1);
    _theVariable = buf_ptr;
 } 

 BUF_END_MED
 {
    save_loc_buf = buf_ptr = (char *)malloc (buf_MED);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_END_MED, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, buf_MED);
    _theVariable = (void *) (((unsigned long) buf_ptr) + buf_MED - 1);
 }

 
 BUF_FAR_PAST
 { 
    save_loc_buf = buf_ptr = (char *)malloc (buf_MED);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_FAR_PAST, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, buf_MED);
    _theVariable = (void *) (((unsigned long) buf_ptr) + (bm_PAGESIZE * 1000));
 }
 
 BUF_ODD
 {
    save_loc_buf = buf_ptr =  (char *)malloc (buf_MED);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_ODD, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, buf_MED);
    _theVariable = (void *) (((unsigned long) buf_ptr) + 1);
 }

 BUF_FREED
 {
    save_loc_buf = buf_ptr =  (char *)malloc (buf_MED);
    if(buf_ptr == NULL)
    {
	fprintf(logFile, "malloc failed in b_ptr_buf - BUF_FREED, function not tested\n");
	fclose(logFile);
	exit(99);
    }
    fillbuf(buf_ptr, buf_MED);
 } 

 BUF_CODE
 {
  _theVariable = (void *) &fib;
 }

 BUF_LOW
 {
    _theVariable = (void *) 16;
 }

{
   fclose(logFile);
}
]

commit
[
 BUF_FREED
 {
    free (save_loc_buf);
    _theVariable = save_loc_buf;
 }
]

cleanup
[
{
   if (save_loc_buf != NULL) 
   {
      free(save_loc_buf);
   }

}
]
