/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * The mmap( ) function shall add an extra reference to the file
 * associated with the file descriptor fildes which is not removed
 * by a subsequent close( ) on that file descriptor. 
 * This reference shall be removed when there are no more 
 * mappings to the file.
 * 
 * Test Steps:
 * 1. Create a file, while it is open call unlink().
 * 2. mmap the file to memory, then call close(). If mmap() add
 *    extra reference to the file, the file descriptor will not be removed.
 * 3. Try to open the file, open should success;
 * 3. munmap the mapped memory;
 * 4. Try open the file again, should get ENOENT;
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
 
#define TNAME "mmap/12-1.c"

int main()
{
  char tmpfname[256];
  long total_size; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t size;
  int flag;
  int fd, fd2;
  off_t off = 0;
  int prot;

  total_size = 1024;
  size = total_size;
  
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_12_1_%d",
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
  
  /* If there is no reference to the file, 
   * this file will be removed 
   */
  unlink(tmpfname);
  
  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  
  flag = MAP_SHARED;
  prot = PROT_READ | PROT_WRITE;
  
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
  	printf ("Test Fail: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }
  
  close(fd);

  /* File still exists */

  fd2 = open(tmpfname, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1 && errno == ENOENT)
  {  
    printf(TNAME " Error at open(): %s\n", 
           strerror(errno));   
    printf ("Test Fail: " TNAME  " The file is removed. mmap does not "
    		    "add extra reference to the file associated with fd\n"); 
    exit(PTS_FAIL);
  }
  
  munmap (pa, size);

  /* The file should have been removed */
  fd2 = open(tmpfname, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd2 == -1 && errno == ENOENT)
  {  
    printf ("Test PASS: " TNAME  " The file is removed. munmap removed "
    		    "reference to the file associated with fd\n"); 
    exit(PTS_PASS);
  }

  if (fd2 != -1)
  {
  	printf ("Test FAIL: " TNAME 
            " The file is not removed. munmap does not remove "
    	      "reference to the file associated with fd\n"); 
  	return PTS_FAIL;
  }

  printf("PTS_UNRESLOVED " TNAME " Error at open(): %s\n", 
           strerror(errno));   
  return PTS_UNRESOLVED;
}
