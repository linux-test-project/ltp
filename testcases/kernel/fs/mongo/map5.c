/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int fd;
	int block;
	int first_block;
	int last_block;
	int totals_block;
	int fragments;
	int i;
	int n;

	for (n = 1; n < argc; n++) {
		if (argc < 2) {
			printf
			    ("Used to see file maps \nUsage: %s filename1 [[..[filename2]...filename(N-1)] filenameN]\n",
			     argv[0]);
			return 0;
		}
		fd = open(argv[n], O_RDONLY);
		if (fd == -1) {
			perror("open failed");
			continue;
		}
		// printf ("file %s occupies blocks: \n", argv[1]);
		// printf ("START\tEND\tCOUNT\n");
		i = 0;
		block = 0;
		first_block = 0;
		last_block = 0;
		fragments = 0;
		totals_block = 0;

		while (ioctl(fd, FIBMAP, &block) == 0) {
			if (first_block == 0) {
				last_block = block - 1;
				first_block = block;
			}
			if (block != last_block + 1) {
				// printf ("%d\t%d\t%d\n",first_block,last_block,last_block-first_block+1);
				totals_block += last_block - first_block + 1;
				fragments++;
				first_block = block;
				last_block = block;
			} else {
				last_block++;
			}

			if (!block) {
				//printf ("Fragments: %d\tBlocks: %d\n",fragments,totals_block);
				//printf ("%d:%d\t",fragments,totals_block);
				//if (fragments == 1) printf(".",totals_block);
				//else printf("%d_",fragments,totals_block);
				printf("%d\n", fragments);
				break;
			}

			i++;
			block = i;
		}
		if (errno) {
			perror("FIBMAP failed");
		}
		close(fd);
		// printf ("\n");
	}
	return 0;
}
