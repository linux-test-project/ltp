/*
 * Program for testing file locking. The original data file consists of
 * characters from 'A' to 'Z'. The data file after running this program
 * would consist of lines with 1's in even lines and 0's in odd lines.
 */

#include "nfs_flock.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BYTES 64
#define LINES 16384

int
main(argc, argv)
   int argc;
   char **argv;
{
   int i, fd, mac;
   int offset=0;
   char buf[BUFSIZ];

   if (argc != 3) {
      fprintf(stderr, "Usage: %s <mac num> <file name>\n", argv[0]);
      exit(2);
   }

   if ((fd = open(argv[2], O_RDWR)) < 0) {
      perror("opening a file");
      exit(1);
   }

   mac = atoi(argv[1]);

   /*
    * Replace a line of characters by 1's if it is process one
    * else with 0's. Number of charcters in any line are BYTES-1,
    * the last character being a newline character.
    */
   for (i = 0; i < BYTES-1; i++) {
      if (mac == 1)
         buf[i] = '1';
      else
         buf[i] = '0';
   }
   buf[BYTES-1]='\n';

   for (i = 0; i < LINES; i++) {
      if (mac == 1) {        /* Set the offset to even lines */
         if ((i%2) == 0) {
            if (i == 0)
               offset = 0;
            else
               offset += 2*BYTES;
         } else
            continue;
      } else {               /* Set the offset to odd lines */
         if ((i%2) == 1) {
            if (i == 1)
               offset = BYTES;
            else
               offset += 2*BYTES;
         } else
            continue;
      }

      if (writeb_lock(fd, offset, SEEK_SET, BYTES) < 0)
         printf("failed in writeb_lock, Errno = %d", errno);

      lseek(fd, offset, SEEK_SET);

      /* write to the test file */
      write(fd, buf, BYTES);

      if (unb_lock(fd, offset, SEEK_SET, BYTES) < 0)
         printf("failed in unb_lock, Errno = %d", errno);
   }
   exit(0);
}
