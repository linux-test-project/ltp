
#include <arpa/inet.h>
#include <netinet/in.h>
#define __USE_GNU
#include <netdb.h>
#undef __USE_GNU
#include <stdio.h>
#include <stdlib.h>


 int cancel_it( struct gaicb * pgaicb )
 {
    int ret;
    
    ret = gai_cancel ( pgaicb );
 
     return ret;
  }

 int main (void)
 {

   struct gaicb temp;

    temp.ar_name = NULL; 
    temp.ar_service = NULL;
    temp.ar_request = NULL;
    temp.ar_result = NULL;


   cancel_it (&temp);

    return 0;
  }
