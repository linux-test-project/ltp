/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * The mmap( ) function shall establish a mapping between a process's 
 * address space and a file,
 *
 * Test Step:
 * 1. Create a tmp file;
 * 2. mmap it to memory using mmap();
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
 
#define TNAME "mmap/1-1.c"

int main()
{
  char tmpfname[256];
  char* data;
  int total_size = 1024; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t len = total_size;
  int prot = PROT_READ | PROT_WRITE;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;

  char * ch;

  data = (char *) malloc(total_size); 
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_1_1_%d",
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
  memset(data, 'a', total_size);
  if (write(fd, data, total_size) != total_size)
  {
    printf(TNAME "Error at write(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  free(data);
  
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }
  
  ch = pa;

  if(*ch != 'a')
  {
    printf ("Test Fail: " TNAME 
            " The file did not mapped to memory\n");    
    exit(PTS_FAIL);
  }

  close(fd);
  munmap(pa, len);
  printf ("Test Pass\n");
  return PTS_PASS;
}
