/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * The mmap( ) function shall fail if:
 * [EOVERFLOW] The file is a regular file and the value of off 
 * plus len exceeds the offset maximum established in the open 
 * file description associated with fildes.
 *
 * Test Steps:
 * 1. Create a file, and set its size using ftruncate;
 * 2. mmap() the file, set (len + off) > file size;
 *
 * FIXME: Not quite sure how to make "the value of off plus len
 * exceeds the offset maxium established in the open file description
 * associated with files".
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
 
#define TNAME "mmap/31-1.c"

int main()
{
  char tmpfname[256];
  long file_size; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t len;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  long page_size = sysconf(_SC_PAGE_SIZE);

  file_size = 2 * page_size;
  
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_31_1_%d",
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
  
  /* Set size of the file */
  if (ftruncate(fd, file_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  
  flag = MAP_SHARED;
  prot = PROT_READ | PROT_WRITE;
  
  /* len + off > file_size */
  len = file_size;
  
  /* off should be a multiple of page size */
  off = file_size / 2;
  
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EOVERFLOW)
  {
  	printf ("Test Pass: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
    exit(PTS_PASS);
  }
 
  if (pa == MAP_FAILED)
    perror("Test FAIL: expect EOVERFLOW but get other error");
  else  
    printf ("Test FAIL : Expect EOVERFLOW but got no error\n");
  
  close (fd);
  munmap (pa, len);
  return PTS_FAIL;
}
