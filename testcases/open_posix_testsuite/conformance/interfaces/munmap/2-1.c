/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * If there are no
 * mappings in the specified address range, then munmap( ) has no effect.
 *
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"
 
#define TNAME "munmap/2-1.c"

int main()
{
  int rc;

  int page_size;
  void *buffer = NULL, *new_addr = NULL;
  
  page_size = sysconf(_SC_PAGE_SIZE);
  buffer =  malloc(page_size * 2);
  if (buffer == NULL)
  {
  	printf("Error at malloc\n");
    exit(PTS_UNRESOLVED);
  } 
 
  /* Make new_addr is a multiple of page_size, while
   * [new_addr, new_addr + page_size] is a valid memory range
   */
  new_addr = buffer + (page_size - (unsigned long)buffer % page_size);
 
  rc = munmap(new_addr, page_size);
  if (rc == -1)
  {
  	printf ("Test FAILED " TNAME " Error at munmap(): %s\n",
             strerror(errno));
  	free(buffer);
  	exit(PTS_FAIL);
  }
  
  free(buffer);
  printf ("Test PASSED\n");
  return PTS_PASS;
}
