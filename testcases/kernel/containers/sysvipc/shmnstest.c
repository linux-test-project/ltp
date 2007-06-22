/*
 * Copyright 2007 IBM
 * Author: Serge Hallyn <serue@us.ibm.com>
 *
 * Create shm with key 0xEAEAEA
 * clone, clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
 * In cloned process, try to get the created shm
 */

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
#ifndef NO_LTP
#include <test.h>
#include <libclone.h>
#else
#include "../libclone/libclone.h"
#endif

char *TCID = "sysvipc_namespace";
int TST_TOTAL=1;
#define TESTKEY 0xEAEAEA

#ifdef NO_LTP
#define TFAIL "FAILURE: "
#define TPASS "PASS: "
#define TINFO "INFO: "
#define tst_resm(x, format, arg...) printf("%s:" format, x,## arg)
#define tst_exit(ret) exit(ret)
#endif

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
      tst_exit(0);
}

#define UNSHARESTR "unshare"
#define CLONESTR "clone"
#define NONESTR "none"
int main(int argc, char *argv[])
{
      int r, pid, use_clone = T_NONE;
      int id;
      char *tsttype = NONESTR;
      char buf[7];

      if (argc != 2) {
              tst_resm(TFAIL, "Usage: %s <clone|unshare|none>\n", argv[0]);
              tst_resm(TFAIL, " where clone, unshare, or fork specifies unshare method.");
              tst_exit(2);
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
              tst_exit(3);
      }

      tst_resm(TINFO, "shmid namespaces test : %s\n",tsttype);
      /* fire off the test */
      r = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmid, NULL);
      if (r < 0) {
              tst_resm(TFAIL, "%s failed\n", tsttype);
              tst_exit(1);
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
      tst_exit(0);
}
