/*
 * Copyright (c) 2002 Network Associates Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

/*
 * Function to handle SIGIO.
 */
static void sig_io(int signo) {
  exit(0);
}

/*
 * This program waits either for a SIGIO signal, or will time out. The
 * exit status is 'good' on the SIGIO receive, 'bad' on the timeout.
 */
int main(int argc, char **argv) {

  struct sigaction siga;

  /*
   * Set up signal handler for SIGIO.
   */
   siga.sa_handler = sig_io;
   sigemptyset(&siga.sa_mask);
   siga.sa_flags = 0;
   sigaction(SIGIO, &siga, NULL);
   sleep(5);
   exit(1);

}
