/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * The off argument is constrained to be aligned and sized 
 * according to the value returned by
 * sysconf( ) when passed _SC_PAGESIZE or _SC_PAGE_SIZE. 
 * 
 * The mmap( ) function shall fail if:
 * [EINVAL] The addr argument (if MAP_FIXED was specified) or off is not a multiple of
 * the page size as returned by sysconf( ), or is considered invalid by the
 * implementation.
 *
 * Test step:
 * 1. Set 'off' a value which is not a multiple of page size;
 * 2. Call mmap() and get EINVAL;
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
 
#define TNAME "mmap/11-1.c"

int main()
{
  char tmpfname[256];
  long  page_size;
  long total_size; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  page_size = sysconf(_SC_PAGE_SIZE);
  total_size = 3 * page_size;
  size = page_size;
  
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_11_1_%d",
           getpid());
  unlink(tmpfname);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {  
    printf(TNAME " Error at open(): %s\n", 
           strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);
  
  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
    
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_SHARED;
 	
  /* This offset is considered illegal, not a multiple of page_size, 
   * unless the page_size is 1 byte, which is considered impossible.
   */

  off = page_size + 1;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EINVAL)
  {
    printf("Test Pass: " TNAME " Get "
    	     "EINVAL when 'off' is not a multiple of page size\n");    
    exit(PTS_PASS);
  }
  
  printf ("Test FAIL, Did not get EINVAL"
          " when 'off' is not a multiple of page size, get: %s\n", strerror(errno));
  close (fd);
  munmap (pa, size);
  return PTS_FAIL;
}
