/*
 * pcreate.c - test create posix thread API: pthread_create()
 * Created by Helen Pang
 * 06/23/2003
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>

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

void *threadfunc(void *parm)
{
  thread_parm_t *p = (thread_parm_t *)parm;
  printf("%s, parm = %d\n", p->string, p->value);
  free(p);
  return NULL;
}

int main(int argc, char **argv)
{

  pthread_t             thread;
  int                   rc=0;
  pthread_attr_t        pta;
  thread_parm_t        *parm=NULL;

  printf("Enter Testcase - %s\n", argv[0]);

  printf("Create a thread attributes object\n");
  rc = pthread_attr_init(&pta);
  checkResults("pthread_attr_init()\n", rc);

  /* Create 2 threads using default attributes in different ways */
  printf("Create thread using the NULL attributes\n");

  /* Set up multiple parameters to pass to the thread */
  parm = (thread_parm_t*)malloc(sizeof(thread_parm_t)*2);
  parm->value = 5;
  strcpy(parm->string, "Inside secondary thread");
  rc = pthread_create(&thread, NULL, threadfunc, (void *)parm);
  checkResults("pthread_create(NULL)\n", rc);

  printf("Create thread using the default attributes\n");
  /* Set up multiple parameters to pass to the thread */
  parm = (thread_parm_t*)malloc(sizeof(thread_parm_t)*2);
  parm->value = 77;
  strcpy(parm->string, "Inside secondary thread");
  rc = pthread_create(&thread, &pta, threadfunc, (void *)parm);
  checkResults("pthread_create(&pta)\n", rc);

  printf("Destroy thread attributes object\n");
  rc = pthread_attr_destroy(&pta);
  checkResults("pthread_attr_destroy()\n", rc);

  /* sleep() is not a very robust way to wait for the thread */
  sleep(5);

  printf("Main completed\n");
  
  return PASS;
}
