/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * The mmap( ) function shall fail if:
 * [ENOTSUP] MAP_FIXED or MAP_PRIVATE was specified 
 * in the flags argument and the
 * implementation does not support this functionality.
 * The implementation does not support the combination 
 * of accesses requested in the prot argument.
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
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"
 
#define TNAME "mmap/27-1.c"

int main()
{
  char tmpfname[256];
  char* data;
  int total_size = 1024; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t len = total_size;
  int prot;
  int flag;
  int fd;
  off_t off = 0;
#ifdef MAP_FIXED
  printf("Test Untested: MAP_FIXED defined\n");
  exit(PTS_UNTESTED);
#endif

#ifdef MAP_SHARED
  printf("Test Untested: MAP_SHARED defined\n");
  exit(PTS_UNTESTED);
#endif
  
#ifdef PROT_WRITE
  printf("Test Untested: PROT_WRITE defined\n");
  exit(PTS_UNTESTED);
#endif

#ifdef PROT_READ
  printf("Test Untested: PROT_READ defined\n");
  exit(PTS_UNTESTED);
#endif

  data = (char *) malloc(total_size); 
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_27_1_%d",
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
  
  /* Make sure the file is removed when it is closed */
  unlink(tmpfname);

  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_FIXED | MAP_SHARED;  
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa != MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Did not get error\n");    
    exit(PTS_FAIL);
  }
  else if (errno != ENOTSUP)
  { 
    printf("Test Fail: " TNAME " Get error: %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
    
  }

  printf ("Test Pass\n");
  return PTS_PASS;
}
