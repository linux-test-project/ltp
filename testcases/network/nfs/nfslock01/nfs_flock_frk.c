/*
 * This program starts processes one and two simultaneously.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main (int argc, char **argv)
{
   pid_t pid;
   char *Prog;

   if (argc != 3) {
      fprintf(stderr, "Usage: %s <process> <datafile>\n", argv[0]);
      exit(2);
   }

   Prog = strrchr(argv[1], '/');
   Prog++;
 
   if ((pid = fork()) < 0) {
      printf("Failed in forking, Errno = %d", errno);
      exit(2);
   } else if (pid == 0) {               /* child */
      execl(argv[1], Prog, "0", argv[2], (char *) 0);
   } else {                             /* parent */
      execl(argv[1], Prog, "1", argv[2], (char *) 0);
   }

   /*if (waitpid(pid, NULL, 0) != pid)
      printf("Failed in waitpid, Errno = %d", errno);
*/
   exit(0);
}
