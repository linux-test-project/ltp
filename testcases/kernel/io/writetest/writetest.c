/*
 *
 *   Copyright (c) CHANG Industry, Inc., 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *  FILE        : writetest.c
 *  DESCRIPTION : The purpose of this test is to verify that writes to
 *                disk occur without corruption.  It writes one 1MB
 *                buffer at a time, where each byte in the buffer is
 *                generated from a random number.  Once done
 *                completed, the file is re-opened, the random number
 *                generator is re-seeded, and the file is verified.
 *
 *  HISTORY:
 *   05/12/2004 : Written by Danny Sung <dannys@changind.com> to
 *                verify integrity of disk writes.
 *
 */


#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BLOCKSIZE (1024*1024)
#define FILE_OUT    "fileout"
#define FILE_MODE   0644
#define MAX_FILENAME_LEN 1024

int Verbosity = 0;
int DefaultSeed = 0;
char Filename[MAX_FILENAME_LEN] = FILE_OUT;
off_t NumBlocks = 1;
const char *ProgramName = "writetest";


inline void verbose(int level, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));

inline void verbose(int level, const char *fmt, ...)
{
    va_list ap;
    if( Verbosity >= level ) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        fflush(stdout);
        va_end(ap);
    }
}

void buf_init(void )
{
    static int seed = 0;
    if( seed == 0 )
        seed = DefaultSeed;
    srand(seed);
}

void buf_fill(uint8_t *buf)
{
    int i;
    for(i=0; i<BLOCKSIZE; i++) {
        *buf = (rand() & 0xff);
        buf++;
    }
}

int write_file(off_t num_blocks, const char *filename)
{
    int fd;
    int ret = 0;
    off_t block;
    uint8_t buf[BLOCKSIZE];

    fd = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE, FILE_MODE);
    if( fd < 0 ) {
        perror(ProgramName);
        return(-1);
    }
    for(block=0; block<num_blocks; block++) {
        int rv;
        verbose(3, "Block: %lld/%lld  (%3lld%%)\r", (long long int)block, (long long int)num_blocks, (long long int)(block*100/num_blocks));
        buf_fill(buf);
        rv = write(fd, buf, BLOCKSIZE);
        if( rv != BLOCKSIZE ) {
            ret = -1;
            break;
        }
    }
    verbose(3, "Block: %lld/%lld  (%3lld%%)\r", (long long int)block, (long long int)num_blocks, (long long int)(block*100/num_blocks));
    verbose(3, "\n");
    close(fd);
    return(ret);
}

int verify_file(off_t num_blocks, const char *filename)
{
    int fd;
    int ret = 0;
    off_t block;
    uint8_t buf_actual[BLOCKSIZE];
    char buf_read[BLOCKSIZE];

    fd = open(filename, O_RDONLY);
    if( fd < 0 ) {
        perror(ProgramName);
        return(-1);
    }
    for(block=0; block<num_blocks; block++) {
        int rv;
        int n;
        verbose(3, "Block: %lld/%lld  (%3lld%%)\r", (long long int)block, (long long int)num_blocks, (long long int)(block*100/num_blocks));
        buf_fill(buf_actual);
        rv = read(fd, buf_read, BLOCKSIZE);
        if( rv != BLOCKSIZE ) {
            ret = -1;
            break;
        }
        for(n=0; n<BLOCKSIZE; n++) {
            int ba, br;
            ba = buf_actual[n] & 0xff;
            br = buf_read[n] & 0xff;
            if( ba != br ) {
                verbose(1, "Mismatch: block=%lld +%d bytes offset=%lld read: %02xh actual: %02xh\n",
                    (long long int)block, n, (long long int)((block*BLOCKSIZE)+n), br, ba);
                ret++;
            }
        }
    }
    close(fd);

    verbose(3, "Block: %lld/%lld  (%3lld%%)\r", (long long int)block, (long long int)num_blocks, (long long int)(block*100/num_blocks));
    verbose(3, "\n");
    return(ret);
}

void usage(void)
{
    printf("%s [-v] [-b blocks] [-s seed] [-o filename]\n", ProgramName);
    printf(
        "\n"
        "   -v          - increase verbosity level\n"
        "   blocks      - number of blocks to write\n"
        "   seed        - seed to use (0 to use timestamp)\n"
        "   filename    - name of output file\n"
        );
}

void parse_args(int argc, char **argv)
{
    int c;
    ProgramName = argv[0];
    
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            { "blocks", 1, 0, 'b' },
            { "out", 1, 0, 'o' },
            { "seed", 1, 0, 's' },
            { "verbose", 0, 0, 'v' },
            { "help", 0, 0, 'h' },
            { 0, 0, 0, 0 }
        };
        c = getopt_long(argc, argv, "hvb:o:s:", long_options, &option_index);
        if( c == -1 ) 
            break;
        switch(c) {
            case 'b':
                NumBlocks = strtoul(optarg, NULL, 0);
                break;
            case 'o':
                strncpy(Filename, optarg, MAX_FILENAME_LEN);
                break;
            case 's':
                DefaultSeed = strtoul(optarg, NULL, 0);
                break;
            case 'v':
                Verbosity++;
                break;
            case 'h':
            default:
                usage();
                exit(-1);
        }
    }
}

int main(int argc, char *argv[])
{
    int rv;
    int status = 0;

    DefaultSeed = time(NULL);
    parse_args(argc, argv);
    verbose(2, "Blocks:       %lld\n", (long long int)NumBlocks);
    verbose(2, "Seed:         %d\n", DefaultSeed);
    verbose(2, "Output file: '%s'\n", Filename);

    verbose(1, "Writing %lld blocks of %d bytes to '%s'\n", (long long int)NumBlocks, BLOCKSIZE, Filename);
    buf_init();
    rv = write_file(NumBlocks, Filename);
    if( rv == 0 ) {
        verbose(1, "Write: Success\n");
    } else {
        verbose(1, "Write: Failure\n");
        status = rv;
    }

    verbose(1, "Verifing %lld blocks in '%s'\n", (long long int)NumBlocks, Filename);
    buf_init();
    rv = verify_file(NumBlocks, Filename);
    if( rv == 0 ) {
        verbose(1, "Verify: Success\n");
    } else {
        verbose(1, "Verify: Failure\n");
        status = rv;
    }
    if( rv > 0 ) {
        verbose(1, "Total mismatches: %d bytes\n", rv);
    }

    return status;
}
