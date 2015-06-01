#include <stdio.h>

extern int d1;

void use_s1(void)
{
	d1 = d1 + d1;
}
