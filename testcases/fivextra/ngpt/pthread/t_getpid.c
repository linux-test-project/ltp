/*
 * t_getpid.c - test create posix thread API: getpid()
 * Created by Helen Pang
 * 09/02/2003
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h> 

pid_t getpid(void);


/*
 * return codes
 */
#define PASS        0
#define FAIL        1

#define checkResults(string, val) {if (val) {printf("Failed with %d at %s", val, string); exit(FAIL); }}

typedef struct {
  int   value;
  char  string[128];
} thread_parm_t;

pid_t   pid = 0;
pid_t   pid1 = 0;


void *threadfunc(void *parm)
{
  thread_parm_t *p = (thread_parm_t *)parm;
  printf("%s, parm = %d\n", p->string, p->value);
  pid1 = getpid();
  free(p);
  return NULL;
}


int main(int argc, char **argv)
{
  pthread_t             thread;
  int                   rc=0;
  thread_parm_t        *parm=NULL;

  printf("Enter Testcase:  %s\n", argv[0]);
  
  printf("Get pid from the main thread");
  pid = getpid();

  /* Create  thread using the NULL attributes */
  printf("Create thread using the NULL attributes\n");

  /* Set up multiple parameters to pass to the thread */
  parm = (thread_parm_t*)malloc(sizeof(thread_parm_t)*2);
  parm->value = 5;
  rc = pthread_create(&thread, NULL, threadfunc, (void *)parm);
  checkResults("pthread_create(NULL)\n", rc);

  /* sleep() is not a very robust way to wait for the thread */
  sleep(5);

  if(pid1 != pid)
  {
    printf("Get pid error: pid = %d, pid1 = %d\n\n", pid, pid1);
    return FAIL;
  }

  return PASS;
}
