/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#define _GNU_SOURCE 3
#include <string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<asm/ioctls.h>

/*
 * Test the sigio operations by creating a child and registering that process
 * for SIGIO signals for the terminal. The main process forces a SIGIO
 * on the terminal by sending a charcter to that device.
 */
int main(int argc, char **argv) {

  int fd;
  int rc;
  int flags;
  pid_t pid;
  char key = '\r';
  char ex_name[255];

  fd = open(ctermid(NULL), O_RDWR, 0);
  
  if(fd == -1) {
    perror("selinux_sigiotask:open");
    exit(2);
  }

 /*
  * Spawn off the child process to handle the information protocol.
  */
  if( (pid = fork()) < 0 ) {
     perror("selinux_sigiotask:fork");
     exit(2);
  }

 /*
  * child process
  */
  if( pid == 0 ) {
    /* Create the path to the executable the child will run */
    sprintf(ex_name, "%s/selinux_wait_io", dirname(strdup(argv[0])));
printf("ex_name is %s\n", ex_name);
    if( execl(ex_name, (char *) 0) < 0 ) {
      perror("selinux_sigiotask:execl");
      exit(2);
    }
  }

  /*
   * parent process
   */
  rc = fcntl(fd, F_SETSIG, 0);
  if( rc == -1 ) {
    perror("selinux_sigiotask:F_SETSIG");
    exit(2);
  }

  rc = fcntl(fd, F_SETOWN, pid);
  if( rc == -1 ) {
    perror("selinux_sigiotask:F_SETOWN");
    exit(2);
  }

  flags = fcntl(fd, F_GETFL, 0);
  if( flags < 0 ) {
    perror("selinux_sigiotask:F_GETFL");
    exit(2);
  }
  flags |= O_ASYNC;
  rc = fcntl(fd, F_SETFL, flags);
  if( rc == -1 ) {
    perror("selinux_sigiotask:F_SETFL");
    exit(2);
  }

  sleep(1);          /* Allow the child time to start up */
  rc = ioctl(fd, TIOCSTI, &key);  /* Send a key to the tty device */
  if( rc == -1 ) {
    perror("selinux_sigiotask:write");
    exit(2);
  }
  close(fd);
  wait(&rc);
  if( WIFEXITED(rc) ) {   /* exit status from child is normal? */
    exit(WEXITSTATUS(rc));
  } else {
    exit(1);
  }

}
