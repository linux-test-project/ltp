/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Author: Serge Hallyn <serue@us.ibm.com>
*
* Create shm with key 0xEAEAEA
* clone, clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
* In cloned process, try to get the created shm

***************************************************************************/

#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "test.h"
#include <libclone.h>

char *TCID = "sysvipc_namespace";
int TST_TOTAL=1;
#define TESTKEY 0xEAEAEA

int p1[2];
int p2[2];

int check_shmid(void *vtest)
{
      char buf[3];
      int id;

      close(p1[1]);
      close(p2[0]);

      read(p1[0], buf, 3);
      id = shmget(TESTKEY, 100, 0);
      if (id == -1) {
              write(p2[1], "notfnd", 7);
      } else {
              write(p2[1], "exists", 7);
              shmctl(id, IPC_RMID, NULL);
      }

      tst_exit();
}

#define UNSHARESTR "unshare"
#define CLONESTR "clone"
#define NONESTR "none"
int main(int argc, char *argv[])
{
      int r, use_clone = T_NONE;
      int id;
      char *tsttype = NONESTR;
      char buf[7];

      if (argc != 2) {
              tst_resm(TFAIL, "Usage: %s <clone|unshare|none>\n", argv[0]);
              tst_resm(TFAIL, " where clone, unshare, or fork specifies unshare method.");
              tst_exit();
      }
      if (pipe(p1) == -1) { perror("pipe"); exit(EXIT_FAILURE); }
      if (pipe(p2) == -1) { perror("pipe"); exit(EXIT_FAILURE); }
      tsttype = NONESTR;
      if (strcmp(argv[1], "clone") == 0) {
              use_clone = T_CLONE;
              tsttype = CLONESTR;
      } else if (strcmp(argv[1], "unshare") == 0) {
              use_clone = T_UNSHARE;
              tsttype = UNSHARESTR;
      }

      /* first create the key */
      id = shmget(TESTKEY, 100, IPC_CREAT);
      if (id == -1) {
              perror("shmget");
              tst_resm(TFAIL, "shmget failed\n");
              tst_exit();
      }

      tst_resm(TINFO, "shmid namespaces test : %s\n",tsttype);
      /* fire off the test */
      r = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmid, NULL);
      if (r < 0) {
              tst_resm(TFAIL, "%s failed\n", tsttype);
              tst_exit();
      }

      close(p1[0]);
      close(p2[1]);
      write(p1[1], "go", 3);
      read(p2[0], buf, 7);
      if (strcmp(buf, "exists") == 0) {
              if (use_clone == T_NONE)
                      tst_resm(TPASS, "plain cloned process found shmid\n");
              else
                      tst_resm(TFAIL, "%s: child process found shmid\n",
                              tsttype);
      } else {
              if (use_clone == T_NONE)
                      tst_resm(TFAIL, "plain cloned process didn't find shmid\n");
              else
                      tst_resm(TPASS, "%s: child process didn't find shmid\n",
                              tsttype);
      }

      /* destroy the key */
      shmctl(id, IPC_RMID, NULL);

      tst_exit();
}
