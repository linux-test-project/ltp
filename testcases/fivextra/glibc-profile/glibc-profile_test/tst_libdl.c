

#include <stdlib.h>
#include <dlfcn.h>

int test_dl( void)
{
   char buf[8];
   long addr;
   void *handle;

   handle = dlopen(NULL,RTLD_LAZY);   
   addr = (long)dlsym(handle,"system");
   printf("System() is at 0x%x\n",addr);

   return 0;
  
}

int main (void )
 {
  
    return test_dl();
 
  }
