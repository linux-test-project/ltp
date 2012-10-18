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
*
* Requires openssl; compile with -lcrypto option
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "test.h"
#if HAVE_OPENSSL_SHA_H
#include <openssl/sha.h>
#endif

#define MAX_EVENT_SIZE 500
#define EVENT_HEADER_SIZE 32
#define MAX_EVENT_DATA_SIZE (MAX_EVENT_SIZE - EVENT_HEADER_SIZE)
#define NUM_PCRS 8	/*  PCR registers 0-7 in boot aggregate */

char *TCID = "ima_boot_aggregate";
int TST_TOTAL = 1;

static void display_sha1_digest(unsigned char *pcr)
{
	int i;

	for (i = 0; i < 20; i++)
		printf("%02x", *(pcr + i) & 0xff);
	printf("\n");
}

int
main(int argc, char *argv[])
{
#if HAVE_OPENSSL_SHA_H
	unsigned char boot_aggregate[SHA_DIGEST_LENGTH];
	struct {
		struct {
			u_int32_t pcr;
			int type;
			unsigned char digest[SHA_DIGEST_LENGTH];
			u_int16_t len;
		} header;
		unsigned char data[MAX_EVENT_DATA_SIZE];
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

	/* Extend the pseudo PCRs with the event digest */
	while (fread(&event, sizeof(event.header), 1, fp)) {
		if (debug) {
			printf("%03u ", event.header.pcr);
			display_sha1_digest(event.header.digest);
		}
		SHA1_Init(&c);
		SHA1_Update(&c, pcr[event.header.pcr].digest, 20);
		SHA1_Update(&c, event.header.digest, 20);
		SHA1_Final(pcr[event.header.pcr].digest, &c);
		if (event.header.len > MAX_EVENT_DATA_SIZE) {
			printf("Error event too long");
			break;
		}
		fread(event.data, event.header.len, 1, fp);
	}
	fclose(fp);

	/* Extend the boot aggregate with the pseudo PCR digest values */
	memset(&boot_aggregate, 0, SHA_DIGEST_LENGTH);
	SHA1_Init(&c);
	for (i = 0; i < NUM_PCRS; i++) {
		if (debug) {
			printf("PCR-%2.2x: ", i);
			display_sha1_digest(pcr[i].digest);
		}
		SHA1_Update(&c, pcr[i].digest, 20);
	}
	SHA1_Final(boot_aggregate, &c);

	printf("boot_aggregate:");
	display_sha1_digest(boot_aggregate);
#else
	tst_resm(TCONF, "System doesn't have openssl/sha.h");
#endif
	tst_exit();
}
