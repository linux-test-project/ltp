  #include <stdio.h>
  #include <pty.h>        /* forkpty() */
  #include <sys/time.h>
  #include <sys/types.h>
  #include <unistd.h>
  #include <fcntl.h>
  
  int test_util( void )
  {
    int           pid, 
                 mast, 
                    n;
    char ptyname[255]; 
  
    pid = forkpty (&mast, ptyname, NULL, NULL);
  
    switch (pid)
      {
      case -1: 
        {
          perror ("forkpty()");
          return 1;
        }
  
      case 0:  
          printf ("Slave process.\n");
  
      default: 
          printf ("Master process , ptyname is  %s, pid is %d.\n ", ptyname, pid );
      }

      return 0;

   }

  int main ( void )
   {

     return test_util ();

   }
