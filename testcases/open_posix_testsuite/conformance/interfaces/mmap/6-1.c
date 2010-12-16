/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * MPR An implementation may permit accesses other than those specified by prot;
 * however, if the Memory Protection option is supported, the implementation
 * shall not permit a write to succeed where PROT_WRITE has not been set or
 * shall not permit any access where PROT_NONE alone has been set.
 * The implementation shall support at least the following values of prot:
 * PROT_NONE, PROT_READ, PROT_WRITE, and the bitwise-inclusive OR of PROT_READ and
 * PROT_WRITE.
 *
 * Test Step:
 *
 * If Memory Protection option is supported:
 * 1. Spawn a child process.
 * 2. The child process mmap a memory region setting prot as PROT_READ.
 * 3. Try to write the mapped memory.
 * 4. If the writing triger SIGSEGV, the PASS.
 *
 * Please refer to IEEE_1003.1-2001. 2.8.3.3 Memory Protection.
 */
#define _XOPEN_SOURCE 600
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "mmap/6-1.c"

int main(void)
{
#ifdef _POSIX_MEMORY_PROTECTION
  char tmpfname[256];
  int total_size = 1024;

  void *pa = NULL;
  void *addr = NULL;
  size_t size = total_size;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;
  int prot;

  char * ch;

  pid_t child;
  int status;
  int sig_num;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_6_1_%d",
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

  child = fork();

  if (child == 0)
  {
    if (ftruncate(fd, total_size) == -1)
    {
      printf(TNAME "Error at ftruncate(): %s\n",
              strerror(errno));
      exit(PTS_UNRESOLVED);
    }

    prot = PROT_READ;
    pa = mmap(addr, size, prot, flag, fd, off);
    if (pa == MAP_FAILED)
    {
      printf("Test Fail: " TNAME " Error at mmap: %s\n",
            strerror(errno));
      exit(PTS_FAIL);
    }

    ch = pa;

    *ch = 'b';
    exit(0);
  }
  else if (child > 0)
  {
    waitpid(child, &status, WUNTRACED);
    close(fd);
    if (WIFSTOPPED(status))
    {
      sig_num = WSTOPSIG(status);
      printf("Child process stopped by signal %d\n", sig_num);
      if (sig_num == SIGSEGV)
      {
        printf("Test Pass: " TNAME
                " Got SIGSEGV when writing to the mapped memory, "
                "without setting PROT_WRITE\n");
        return PTS_PASS;
      }
    }
    if (WIFSIGNALED(status))
    {
      sig_num = WTERMSIG(status);
      printf("Child process terminated by signal %d\n", sig_num);
      if (sig_num == SIGSEGV)
      {
        printf ("Test Pass: " TNAME
                " Got SIGSEGV when writing to the mapped memory, "
                "without setting PROT_WRITE\n");
        return PTS_PASS;
      }
    }
    if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) == 0)
      {
        printf ("Test FAIL: " TNAME
                " Did not got SIGSEGV when writing to the mapped memory,"
                " without setting PROT_WRITE\n");
        return PTS_FAIL;
      }
    }
    printf ("Test Unresolved\n");
    return PTS_UNRESOLVED;
  }

#else
  printf ("Test Unresolved: " TNAME
          " _POSIX_MEMORY_PROTECTION not defined\n");
  return PTS_UNRESOLVED;
#endif

}