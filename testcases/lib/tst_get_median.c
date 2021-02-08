// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 Oracle and/or its affiliates. All Rights Reserved. */

#include <stdio.h>
#include <stdlib.h>

static int cmp(const void *a, const void *b)
{
   return (*(int *)a - *(int *)b);
}

int main(int argc, const char *argv[])
{
	const size_t size = argc - 1;

	if (!size) {
		fprintf(stderr, "Please provide a numeric list\n");
		return 1;
	}
	if (size == 1) {
		printf("%d", atoi(argv[1]));
		return 0;
	}

	int arr[size];
	size_t i;

	for (i = 0; i < size; ++i)
		arr[i] = atoi(argv[i + 1]);

	qsort(arr, size, sizeof(arr[0]), cmp);

	const size_t size2 = size / 2;
	printf("%d", (size & 1) ? arr[size2] : ((arr[size2 - 1] + arr[size2]) / 2));

	return 0;
}
