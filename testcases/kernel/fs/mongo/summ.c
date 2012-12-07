/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

#include <stdio.h>
#include <stdlib.h>
char str[100];

int main(int argc, char **argv)
{
	char c, *p;
	int sum = 0, n = 0;

	p = str;
	while ((c = getchar()) != EOF) {
		if (c != '\n') {
			*p++ = c;
		} else {
			*p = '\0';
			n = atol(str);
			sum += n;
			printf("%i\n", sum);
			p = str;
			*p = '\0';
		}
	}
}
