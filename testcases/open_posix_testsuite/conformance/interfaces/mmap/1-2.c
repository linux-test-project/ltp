/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * The mmap( ) function shall establish a mapping 
 * between a process's address space 
 * and a shared memory object.
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
 
#define TNAME "mmap/1-2.c"

int main()
{
  char tmpfname[256];
  int shm_fd;

  void *pa = NULL; 
  void *addr = NULL;
  size_t size = 1024 * 4 * 1024;
  int prot = PROT_READ | PROT_WRITE;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;
 
  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_1_2_%d",
           getpid());
	
  /* Create shared object */
	shm_unlink(tmpfname);
	shm_fd = shm_open(tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if(shm_fd == -1)
	{
		printf(TNAME " Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
  shm_unlink(tmpfname);   
  if(ftruncate(shm_fd, size) == -1) {
    printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
    return PTS_UNRESOLVED;
  }

  fd = shm_fd;	
  pa = mmap (addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf ("Test Fail: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }

  close(fd);
  munmap(pa, size);
  printf ("Test Pass\n");
  return PTS_PASS;
}
