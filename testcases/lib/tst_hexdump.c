// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Convert bytes from standard input to hexadecimal representation.
 *
 * Parameters:
 * -d   Convert hexadecimal values from standard input to binary representation
 *      instead.
 */

#include <stdio.h>
#include <unistd.h>

int decode_hex(void)
{
	int ret;
	unsigned int val;

	while ((ret = scanf("%2x", &val)) == 1)
		putchar(val);

	return ret != EOF || ferror(stdin);
}

int encode_hex(void)
{
	int val;

	for (val = getchar(); val >= 0 && val <= 0xff; val = getchar())
		printf("%02x", val);

	return val != EOF || ferror(stdin);
}

int main(int argc, char **argv)
{
	int ret, decode = 0;

	while ((ret = getopt(argc, argv, "d"))) {
		if (ret < 0)
			break;

		switch (ret) {
		case 'd':
			decode = 1;
			break;
		}
	}

	if (decode)
		return decode_hex();
	else
		return encode_hex();
}
