/*
* Copyright (c) International Business Machines  Corp., 2009
*
* Authors:
* Mimi Zohar <zohar@us.ibm.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation, version 2 of the
* License.
*
* File: ima_boot_aggregate.c
*
* Calculate a SHA1 boot aggregate value based on the TPM
* binary_bios_measurements.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"
#include "test.h"

char *TCID = "ima_boot_aggregate";

#if HAVE_LIBCRYPTO
#include <openssl/sha.h>

#define MAX_EVENT_SIZE (1024*1024)
#define EVENT_HEADER_SIZE 32
#define MAX_EVENT_DATA_SIZE (MAX_EVENT_SIZE - EVENT_HEADER_SIZE)
#define NUM_PCRS 8		/*  PCR registers 0-7 in boot aggregate */

int TST_TOTAL = 1;

static void display_sha1_digest(unsigned char *pcr)
{
	int i;

	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", *(pcr + i) & 0xff);
	printf("\n");
}

int main(int argc, char *argv[])
{
	unsigned char boot_aggregate[SHA_DIGEST_LENGTH];
	struct {
		struct {
			u_int32_t pcr;
			u_int32_t type;
			u_int8_t digest[SHA_DIGEST_LENGTH];
			u_int32_t len;
		} header __attribute__ ((packed));
		char *data;
	} event;
	struct {
		unsigned char digest[SHA_DIGEST_LENGTH];
	} pcr[NUM_PCRS];
	FILE *fp;
	int i;
	int debug = 0;
	SHA_CTX c;

	if (argc != 2) {
		printf("format: %s binary_bios_measurement file\n", argv[0]);
		return 1;
	}
	fp = fopen(argv[1], "r");
	if (!fp) {
		perror("unable to open pcr file\n");
		return 1;
	}

	/* Initialize psuedo PCR registers 0 - 7 */
	for (i = 0; i < NUM_PCRS; i++)
		memset(&pcr[i].digest, 0, SHA_DIGEST_LENGTH);

	event.data = malloc(MAX_EVENT_DATA_SIZE);
	if (!event.data) {
		printf("Cannot allocate memory\n");
		return 1;
	}

	/* Extend the pseudo PCRs with the event digest */
	while (fread(&event, sizeof(event.header), 1, fp)) {
		if (debug) {
			printf("%03u ", event.header.pcr);
			display_sha1_digest(event.header.digest);
		}

		if (event.header.pcr < NUM_PCRS) {
			SHA1_Init(&c);
			SHA1_Update(&c, pcr[event.header.pcr].digest,
				    SHA_DIGEST_LENGTH);
			SHA1_Update(&c, event.header.digest,
				    SHA_DIGEST_LENGTH);
			SHA1_Final(pcr[event.header.pcr].digest, &c);
		}

#if MAX_EVENT_DATA_SIZE < USHRT_MAX
		if (event.header.len > MAX_EVENT_DATA_SIZE) {
			printf("Error event too long\n");
			break;
		}
#endif
		fread(event.data, event.header.len, 1, fp);
	}
	fclose(fp);
	free(event.data);

	/* Extend the boot aggregate with the pseudo PCR digest values */
	memset(&boot_aggregate, 0, SHA_DIGEST_LENGTH);
	SHA1_Init(&c);
	for (i = 0; i < NUM_PCRS; i++) {
		if (debug) {
			printf("PCR-%2.2x: ", i);
			display_sha1_digest(pcr[i].digest);
		}
		SHA1_Update(&c, pcr[i].digest, SHA_DIGEST_LENGTH);
	}
	SHA1_Final(boot_aggregate, &c);

	printf("boot_aggregate:");
	display_sha1_digest(boot_aggregate);
	tst_exit();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "test requires libcrypto and openssl development packages");
}
#endif
