#include <stdio.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
 
int test_nsl (void) {
 XDR stream;
 int ret; 

 xdrstdio_create(& stream, stdin, XDR_DECODE);

 printf("Creat stream.\n");

 xdr_destroy(& stream);
 
 return 0;
}

int main (void)
 {

   return test_nsl();
 }
