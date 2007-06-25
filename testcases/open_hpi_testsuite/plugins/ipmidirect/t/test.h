#ifndef dTest_h
#define dTest_h


#include <stdio.h>


static int num_ok   = 0;
static int num_fail = 0;


static void
TestFunction( const char *str, const char *file, int line, bool expr )
{
  if ( expr )
       num_ok++;
  else
     {
       printf( "FAIL %s:%d: %s\n", file, line, str );
       num_fail++;
     }
}


#define Test(expr) TestFunction( __STRING(expr), __FILE__, __LINE__, expr )


static int
TestResult()
{
  if ( num_fail )
       return 1;

  return 0;
}


#endif
