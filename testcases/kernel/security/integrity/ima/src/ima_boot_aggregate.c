// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2009
 * Copyright (c) 2016-2019 Petr Vorel <pvorel@suse.cz>
 *
 * Authors: Mimi Zohar <zohar@us.ibm.com>
 *
 * Calculate a SHA1 boot aggregate value based on the TPM 1.2
 * binary_bios_measurements.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"

#if HAVE_LIBCRYPTO
#include <openssl/sha.h>

#define MAX_EVENT_SIZE (1024*1024)
#define EVENT_HEADER_SIZE 32
#define MAX_EVENT_DATA_SIZE (MAX_EVENT_SIZE - EVENT_HEADER_SIZE)
#define NUM_PCRS 8		/*  PCR registers 0-7 in boot aggregate */

static char *debug;
static char *file;

static unsigned char boot_aggregate[SHA_DIGEST_LENGTH];

static struct {
	struct {
		u_int32_t pcr;
		u_int32_t type;
		u_int8_t digest[SHA_DIGEST_LENGTH];
		u_int32_t len;
	} header __attribute__ ((packed));
	char *data;
} event;

static struct {
	unsigned char digest[SHA_DIGEST_LENGTH];
} pcr[NUM_PCRS];

static void display_sha1_digest(unsigned char *pcr)
{
	int i;

	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", *(pcr + i) & 0xff);
	printf("\n");
}

static void do_test(void)
{
	FILE *fp;
	SHA_CTX c;
	int i;

	if (!file)
		tst_brk(TBROK, "missing binary_bios_measurement file, specify with -f");

	fp = SAFE_FOPEN(file, "r");

	/* Initialize psuedo PCR registers 0 - 7 */
	for (i = 0; i < NUM_PCRS; i++)
		memset(&pcr[i].digest, 0, SHA_DIGEST_LENGTH);

	event.data = malloc(MAX_EVENT_DATA_SIZE);
	if (!event.data)
		tst_brk(TBROK, "cannot allocate memory");

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
			tst_res(TWARN, "error event too long");
			break;
		}
#endif
		fread(event.data, event.header.len, 1, fp);
	}

	SAFE_FCLOSE(fp);
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

	printf("sha1:");
	display_sha1_digest(boot_aggregate);
	tst_res(TPASS, "found sha1 hash");
}

static struct tst_option options[] = {
	{"d", &debug, "Enable debug"},
	{"f:", &file, "binary_bios_measurement file (required)\n"},
	{NULL, NULL, NULL}
};

static struct tst_test test = {
	.needs_root = 1,
	.test_all = do_test,
	.options = options,
};

#else
TST_TEST_TCONF("libcrypto and openssl development packages required");
#endif
