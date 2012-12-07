/*
 * Copyright (c) International Business Machines  Corp., 2008
 *
 * Authors:
 * Reiner Sailer <sailer@watson.ibm.com>
 * Mimi Zohar <zohar@us.ibm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * File: ima_measure.c
 *
 * Calculate the SHA1 aggregate-pcr value based on the IMA runtime
 * binary measurements.
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

#define TCG_EVENT_NAME_LEN_MAX	255

char *TCID = "ima_measure";
int TST_TOTAL = 1;

static int verbose;

#define print_info(format, arg...) \
	if (verbose) \
		printf(format, ##arg)

#if HAVE_OPENSSL_SHA_H

static u_int8_t zero[SHA_DIGEST_LENGTH];
static u_int8_t fox[SHA_DIGEST_LENGTH];

struct event {
	struct {
		u_int32_t pcr;
		u_int8_t digest[SHA_DIGEST_LENGTH];
		u_int32_t name_len;
	} header;
	char name[TCG_EVENT_NAME_LEN_MAX + 1];
	struct {
		u_int8_t digest[SHA_DIGEST_LENGTH];
		char filename[TCG_EVENT_NAME_LEN_MAX + 1];
	} ima_data;
	int filename_len;
};

static void display_sha1_digest(u_int8_t * digest)
{
	int i;

	for (i = 0; i < 20; i++)
		print_info(" %02X", (*(digest + i) & 0xff));
}

/*
 * Calculate the sha1 hash of data
 */
static void calc_digest(u_int8_t * digest, int len, void *data)
{
	SHA_CTX c;

	/* Calc template hash for an ima entry */
	memset(digest, 0, sizeof *digest);
	SHA1_Init(&c);
	SHA1_Update(&c, data, len);
	SHA1_Final(digest, &c);
}

static int verify_template_hash(struct event *template)
{
	int rc;

	rc = memcmp(fox, template->header.digest, sizeof fox);
	if (rc != 0) {
		u_int8_t digest[SHA_DIGEST_LENGTH];

		memset(digest, 0, sizeof digest);
		calc_digest(digest, sizeof template->ima_data,
			    &template->ima_data);
		rc = memcmp(digest, template->header.digest, sizeof digest);
		return rc != 0 ? 1 : 0;
	}
	return 0;
}

#endif

/*
 * ima_measurements.c - calculate the SHA1 aggregate-pcr value based
 * on the IMA runtime binary measurements.
 *
 * format: ima_measurement [--validate] [--verify] [--verbose]
 *
 * --validate: forces validation of the aggregrate pcr value
 * 	     for an invalidated PCR. Replace all entries in the
 * 	     runtime binary measurement list with 0x00 hash values,
 * 	     which indicate the PCR was invalidated, either for
 * 	     "a time of measure, time of use"(ToMToU) error, or a
 *	     file open for read was already open for write, with
 * 	     0xFF's hash value, when calculating the aggregate
 *	     pcr value.
 *
 * --verify: for all IMA template entries in the runtime binary
 * 	     measurement list, calculate the template hash value
 * 	     and compare it with the actual template hash value.
 *	     Return the number of incorrect hash measurements.
 *
 * --verbose: For all entries in the runtime binary measurement
 *	     list, display the template information.
 *
 * template info:  list #, PCR-register #, template hash, template name
 *	IMA info:  IMA hash, filename hint
 *
 * Ouput: displays the aggregate-pcr value
 * Return code: if verification enabled, returns number of verification
 * 		errors.
 */
int main(int argc, char *argv[])
{

#if HAVE_OPENSSL_SHA_H
	FILE *fp;
	struct event template;
	u_int8_t pcr[SHA_DIGEST_LENGTH];
	int i, count = 0;

	int validate = 0;
	int verify = 0;

	if (argc < 2) {
		printf("format: %s binary_runtime_measurements"
		       " [--validate] [--verbose] [--verify]\n", argv[0]);
		return 1;
	}

	for (i = 2; i < argc; i++) {
		if (strncmp(argv[i], "--validate", 8) == 0)
			validate = 1;
		if (strncmp(argv[i], "--verbose", 7) == 0)
			verbose = 1;
		if (strncmp(argv[i], "--verify", 6) == 0)
			verify = 1;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		printf("fn: %s\n", argv[1]);
		perror("Unable to open file\n");
		return 1;
	}
	memset(pcr, 0, SHA_DIGEST_LENGTH);	/* initial PCR content 0..0 */
	memset(zero, 0, SHA_DIGEST_LENGTH);
	memset(fox, 0xff, SHA_DIGEST_LENGTH);

	print_info("### PCR HASH                                  "
		   "TEMPLATE-NAME\n");
	while (fread(&template.header, sizeof template.header, 1, fp)) {
		SHA_CTX c;

		/* Extend simulated PCR with new template digest */
		SHA1_Init(&c);
		SHA1_Update(&c, pcr, SHA_DIGEST_LENGTH);
		if (validate) {
			if (memcmp(template.header.digest, zero, 20) == 0)
				memset(template.header.digest, 0xFF, 20);
		}
		SHA1_Update(&c, template.header.digest, 20);
		SHA1_Final(pcr, &c);

		print_info("%3d %03u ", count++, template.header.pcr);
		display_sha1_digest(template.header.digest);
		if (template.header.name_len > TCG_EVENT_NAME_LEN_MAX) {
			printf("%d ERROR: event name too long!\n",
			       template.header.name_len);
			exit(1);
		}
		memset(template.name, 0, sizeof template.name);
		fread(template.name, template.header.name_len, 1, fp);
		print_info(" %s ", template.name);

		memset(&template.ima_data, 0, sizeof template.ima_data);
		fread(&template.ima_data.digest,
		      sizeof template.ima_data.digest, 1, fp);
		display_sha1_digest(template.ima_data.digest);

		fread(&template.filename_len,
		      sizeof template.filename_len, 1, fp);
		fread(template.ima_data.filename, template.filename_len, 1, fp);
		print_info(" %s\n", template.ima_data.filename);

		if (verify)
			if (verify_template_hash(&template) != 0) {
				tst_resm(TFAIL, "Hash failed");
			}
	}
	fclose(fp);

	verbose = 1;
	print_info("PCRAggr (re-calculated):");
	display_sha1_digest(pcr);
#else
	tst_resm(TCONF, "System doesn't have openssl/sha.h");
#endif
	tst_exit();
}
