
 #include <math.h>
 #include <stdio.h>
 #include <stdlib.h>


 int test_math ( void )
 {
    double x;

    x=cos(2);
    printf( "cos(2) is %f .\n", x);

    x=sin(2);
    printf( "sin(2) is %f .\n", x);

    return 0;

 }


 int main ( void )
 {
  
   test_math();

   return 0;
 }
