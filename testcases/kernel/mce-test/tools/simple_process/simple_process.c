#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    void *p = malloc(128);
    printf("allocating 128 bytes of memory\n"); 
    while (1)
    {
	memset(p, 0, 128);
    }
    return 0;
}



