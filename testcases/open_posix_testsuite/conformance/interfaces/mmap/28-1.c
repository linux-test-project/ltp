/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * The mmap( ) function shall fail if:
 * [ENXIO] Addresses in the range [off,off+len) are invalid 
 * for the object specified by fildes.
 *
 * Test Step:
 * 1. mmap a shared memory object into memory;
 * 2. (off + len) will beyond the the shared memory object;
 *
 * FIXME: Not sure what does "invalid" mean here
 *
 * This mmap might trigger SIGBUS on some system.
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
#include <signal.h>
#include "posixtest.h"
 
#define TNAME "mmap/28-1.c"

void sigbus_handler (int signum)
{
  printf("Test UNRESOLVED: Triger SIGBUS\n");
  exit(PTS_UNRESOLVED);
}

int main()
{
  char tmpfname[256];
  int shm_fd;
  long shm_size;

  void *pa = NULL; 
  void *addr = NULL;
  size_t len = 0;
  int prot = PROT_READ | PROT_WRITE;
  int flag;
  int fd;
  off_t off = 0;
  
  long page_size = sysconf(_SC_PAGE_SIZE);
  shm_size = 2 * page_size;

  if (signal(SIGBUS, sigbus_handler) == SIG_ERR)
  {
		printf(TNAME " Error at signal(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
  }

  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_28_1_%d",
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
  if(ftruncate(shm_fd, shm_size) == -1) {
    printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
    return PTS_UNRESOLVED;
  }

  /* (off + len) will go outside the object */
  fd = shm_fd;
  len = 2 * shm_size;
  flag = MAP_SHARED;
  off = page_size;
  pa = mmap (addr, len, prot, flag, fd, off);
  if (pa != MAP_FAILED )
  {
    printf ("Test Fail: " TNAME " Got no error at mmap()\n");    
    close(fd);
    munmap(pa, len);
    exit(PTS_FAIL);
  }
  else if (errno != ENXIO)
  {
    printf ("Test Fail: " TNAME " Did not get ENXIO,"
            " get other error: %s\n", strerror(errno));    
    exit(PTS_FAIL);
  }  
  
  printf ("Test Pass\n");
  return PTS_PASS;
}
