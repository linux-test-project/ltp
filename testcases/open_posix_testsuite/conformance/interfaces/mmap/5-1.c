/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * The prot shall be either PROT_NONE or the bitwise-inclusive OR of 
 * one or more of the other flags in the following table, defined in the
 * <sys/mman.h> header.
 * PROT_READ Data can be read.
 * PROT_WRITE Data can be written.
 * PROT_EXEC Data can be executed.
 * PROT_NONE Data cannot be accessed. 
 * If an implementation cannot support the combination of access types
 * specified by prot, the call to mmap( ) shall fail.
 *
 * Test Step:
 * 1. mmap(), setting 'prot' as PROT_NONE;
 * 2. mmap(), setting 'prot' as PROT_READ | PROT_WRITE | PROT_EXEC
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
 
#define TNAME "mmap/5-1.c"

int main()
{
  char tmpfname[256];
  int total_size = 1024; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t size = total_size;
  int flag = MAP_SHARED;
  int fd;
  int prot;
  off_t off = 0;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_5_1_%d",
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
 
#ifndef PROT_READ
    printf("Test Fail: " TNAME " PROT_READ not defined\n");    
    exit(PTS_FAIL);
#endif   
#ifndef PROT_WRITE
    printf("Test Fail: " TNAME " PROT_WRITE not defined\n");    
    exit(PTS_FAIL);
#endif   
#ifndef PROT_EXEC
    printf("Test Fail: " TNAME " PROT_EXEC not defined\n");    
    exit(PTS_FAIL);
#endif   
#ifndef PROT_NONE
    printf("Test Fail: " TNAME " PROT_READ not defined\n");    
    exit(PTS_FAIL);
#endif   
  
  prot = PROT_NONE;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Error at mmap, with PROT_NONE  %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }
  munmap(pa, size);
  
  prot = PROT_READ | PROT_WRITE | PROT_EXEC;
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test Fail: " TNAME " Error at mmap," 
            "with PROT_READ | PROT_WRITE | PROT_EXEC  %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }
  munmap(pa, size);
  
  close(fd);
  printf ("Test Pass\n");
  return PTS_PASS;
}
