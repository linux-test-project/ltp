#include <unistd.h>
#include <stdio.h>

int main(void)
{
	printf("%d\n", getpagesize());
	return 0;
}
