#include <unistd.h>
#include <stdio.h>

int main(void)
{
	printf("%d\n", getpagesize());

	tst_exit();
}