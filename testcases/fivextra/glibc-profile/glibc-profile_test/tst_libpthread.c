
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static void *f1 (void *);
static void *f2 (void *);



int
test_pthread (void)
{
  pthread_attr_t attr;
  pthread_t th1 = 0;
  pthread_t th2 = 0;
  void *res1;
  void *res2;

  pthread_attr_init (&attr);
  if (pthread_attr_setstacksize (&attr, 70*1024) != 0)
    {
      puts ("invalid stack size");
      return 1;
    }

  pthread_create (&th1, NULL, f1, NULL);
  pthread_create (&th2, &attr, f2, NULL);

  pthread_join (th1, &res1);
  pthread_join (th2, &res2);

  printf ("res1 = %p\n", res1);
  printf ("res2 = %p\n", res2);

  return res1 != (void *) 1 || res2 != (void *) 2;
}
int main ( void )
 {
    int ret; 
 
    ret = test_pthread();

    return ret;
  }
 
static void *
f1 (void *parm)
{
  printf ("This is `%s'\n", __FUNCTION__);
  fflush (stdout);

  return (void *) 1;
}

static void *
f2 (void *parm)
{
  printf ("This is `%s'\n", __FUNCTION__);
  fflush (stdout);
  sleep (1);

  return (void *) 2;
}
