/*
 * pjoin.c - test pthread_join() API
 * Created by Helen Pang
 * 06/23/2003
 */

#include <pthread.h>
#include <stdio.h>

/*
 * return codes
 */
#define PASS        0
#define FAIL        1

#define checkResults(string, val) {if (val) {printf("Failed with %d at %s", val, string); exit(FAIL); }}

int  seconds    = 2;		// seconds to wait before the child thread exit
int  end_exec = 0;
void *status;

void *threadfunc(void *parm)
{
  printf("Inside secondary thread\n");
  sleep (seconds);
  end_exec=1;
  pthread_exit((void*)PASS);
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t             thread;
  int                   rc=0;

  printf("Enter Testcase - %s\n", argv[0]);

  printf("Create thread using default attributes that allow join\n");
  end_exec = 0;
  rc = pthread_create(&thread, NULL, threadfunc, NULL);
  checkResults("pthread_create()\n", rc);

  printf("Wait for the thread to exit\n");
  rc = pthread_join(thread, &status);
  checkResults("pthread_join()\n", rc);
  if ((int)(status) != PASS) {
    printf("Secondary thread failed\n");
    exit(FAIL);
  }
  if(end_exec == 0)
  {
    printf("Test FAILED: main() did not wait for thread to finish execution.\n");
    return FAIL;
  }

  printf("Got secondary thread status as expected\n");
  printf("Main completed\n");
  return PASS;
}

