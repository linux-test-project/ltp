#include <stdio.h>

int main(void)
{

	int i=0;

	for( i=0; i<10; i++)
	{
		if ( i %2 )
			printf(" Hello \n ");
		else
			printf("World \n");
	}

	printf(" the end \n");

	return 0;

}
