/*
 * pcancel.c - test cancel posix thread API: pthread_cancel()         
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

void *threadfunc(void *parm)
{
  printf("Entered secondary thread\n");
  while (1) {
    printf("Secondary thread is looping\n");
    pthread_testcancel();
    sleep(1);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t             thread;
  int                   rc=0;

  printf("Entering testcase\n");
  
  /* Create a thread using default attributes */
  printf("Create thread using the NULL attributes\n");
  rc = pthread_create(&thread, NULL, threadfunc, NULL);
  checkResults("pthread_create(NULL)\n", rc);
  
  /* sleep() is not a very robust way to wait for the thread */
  sleep(2);

  printf("Cancel the thread\n");
  rc = pthread_cancel(thread);
  checkResults("pthread_cancel()\n", rc);

  /* sleep() is not a very robust way to wait for the thread */
  sleep(3);
  printf("Main completed\n");
  return PASS;
}

