/*
 *	Aurélien Charbon - Bull SA
 *	ACL testing basic program
 *	Purpose: setting an acl on a file a verifies that the accesses are right
 */

#include <sys/param.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "tst_res_flags.h"

#ifdef HAVE_LIBACL

#include <sys/acl.h>

#define OP_READ 0x1
#define OP_WRITE 0x2
#define OP_EXEC 0x4

acl_t testacl;
/* the "typical" acl used for the test */

static char *permtab[] =
    { "---", "r--", "-w-", "rw-", "--x", "r-x", "-wx", "rwx" };

struct statstore {
	/* number of passed tests */
	int ok;
	/* number of failed tests */
	int failed;
} aclstat;

int do_file_op(char *filename)
{
	int exe;
	int result;
	uid_t uid;
	result = 0;
	FILE *fptr;
	char str[256] = "./";

	uid = geteuid();
	strcat(str, filename);

	exe = execl(str, NULL, NULL);
	if (exe == -1 && errno != EACCES)
		result = result + OP_EXEC;

	fptr = fopen(filename, "r");
	if (fptr != NULL) {
		result = result + OP_READ;
		fclose(fptr);
	}

	fptr = fopen(filename, "r+");
	if (fptr != NULL) {
		result = result + OP_WRITE;
		fclose(fptr);
	}

	return result;
}

/*  acl with user entries used for the test */
acl_t test_acl_user_create(void)
{
	char acl_text[] =
	    "u::rwx,u:user1:rwx,u:user2:rw-,u:user3:r--,u:user4:r-x,u:user5:---,g::r-x,o::r-x,m::rwx";
	acl_t acl;
	acl = acl_from_text(acl_text);
	return acl;
}

/*  acl with group entries used for the test */

acl_t test_acl_grp_create(void)
{
	char acl_text[] =
	    "u::rwx,g:grp1:rwx,g:grp2:rw-,g:grp3:r--,g:grp4:r-x,g:grp5:---,g::---,o::r-x,m::rwx";
	acl_t acl;
	acl = acl_from_text(acl_text);
	return acl;
}

acl_t test_acl_default_create(void)
{
	char acl_text[] =
	    "u::rwx,u:user1:rwx,u:user2:rw-,u:user3:r--,u:user4:r-x,u:user5:---,g::r-x,m::rwx,o::r-x";
	acl_t acl;
	acl = acl_from_text(acl_text);
	return acl;
}

static void report(testnum, expected, result, fail)
int testnum;			/* test number */
int expected;			/* expected result */
int result;			/* actual result */
int fail;			/* fail or warning */
{
	char *res;
	if (expected == result) {
		res = "[OK]";
		aclstat.ok++;
	} else {
		res = "[FAILED]";
		aclstat.failed++;
	}
	printf("\ttest #%d - Expected: %s - Obtained: %s - %s\n", testnum,
	       permtab[expected], permtab[result], res);

	fflush(stdout);
}

/*
 * set acl in order the file is only readable for the testuser
 * - try to read
 * - try to write
 */
static void test1(char *file)
{
	int result;
	if (seteuid((uid_t) 601) == 0) {
		result = do_file_op(file);
		/* expected result = OP_READ || OP_WRITE || OP_EXEC */
		report(1, OP_READ + OP_WRITE + OP_EXEC, result);
		seteuid((uid_t) 0);
		setegid((gid_t) 0);
	}
}

/*
 * set acl in order the file is only readable for the testgroup
 * - try to read with test user
 * - try to write with test user
 *
 */

static void test2(char *file)
{
	int result;
	if (seteuid((uid_t) 602) == 0) {
		result = do_file_op(file);
		/* expected result = OP_READ || OP_WRITE */
		report(2, OP_READ + OP_WRITE, result);
		seteuid((uid_t) 0);
	}
}

/*
 * set acl in order the file is only readable for the testuser
 * - try to read
 * - try to write
 */

static void test3(char *file)
{
	int result;
	if (seteuid((uid_t) 603) == 0) {
		result = do_file_op(file);
		/* expected result = OP_READ */
		report(3, OP_READ, result);
		seteuid((uid_t) 0);
	}
}

/*
 * set read-write acl on the file for the testuser
 * - try to read
 * - try to write
 */

static void test4(char *file)
{
	int result;
	if (seteuid((uid_t) 604) == 0) {
		result = do_file_op(file);
		/* expected result = OP_READ || OP_EXEC */
		report(4, OP_READ + OP_EXEC, result);
		seteuid((uid_t) 0);
	}
}

static void test5(char *file)
{
	int result;
	if (seteuid((uid_t) 605) == 0) {
		result = do_file_op(file);
		/* expected result = 0x0 */
		report(5, 0x00, result);
		seteuid((uid_t) 0);
	}
}

static void testgrp1(char *file)
{
	int result;
	if (setegid((gid_t) 601) == 0) {
		if (seteuid((uid_t) 601) == 0) {
			result = do_file_op(file);
			/* expected result = OP_READ || OP_WRITE || OP_EXEC */
			report(1, OP_READ + OP_WRITE + OP_EXEC, result);
			seteuid((uid_t) 0);
			setegid((gid_t) 0);
		}
	}
}

/*
 * set acl in order the file is only readable for the testgroup
 * - try to read with test user
 * - try to write with test user
 *
 */

static void testgrp2(char *file)
{
	int result;
	if ((setegid((gid_t) 602) == 0) && (seteuid((uid_t) 602) == 0)) {
		result = do_file_op(file);
		/* expected result = OP_READ || OP_WRITE */
		report(2, OP_READ + OP_WRITE, result);
		seteuid((uid_t) 0);
		setegid((gid_t) 0);
	}
}

/*
 * set acl in order the file is only readable for the testuser
 * - try to read
 * - try to write
 */

static void testgrp3(char *file)
{
	int result;
	if ((setegid((gid_t) 603) == 0) && (seteuid((uid_t) 603) == 0)) {
		result = do_file_op(file);
		/* expected result = OP_READ */
		report(3, OP_READ, result);
		seteuid((uid_t) 0);
		setegid((gid_t) 0);
	}
}

/*
 * set read-write acl on the file for the testuser
 * - try to read
 * - try to write
 */

static void testgrp4(char *file)
{
	int result;
	if (setegid((gid_t) 604) == 0) {
		if (seteuid((uid_t) 604) == 0)
			result = do_file_op(file);
		/* expected result = OP_READ || OP_EXEC */
		report(4, OP_READ + OP_EXEC, result);
		seteuid((uid_t) 0);
		setegid((gid_t) 0);
	}
}

static void testgrp5(char *file)
{
	int result;
	if (setegid((gid_t) 605) == 0) {
		if (seteuid((uid_t) 605) == 0)
			result = do_file_op(file);
		/* expected result = 0x0 */
		report(5, 0x00, result);
		seteuid((uid_t) 0);
		setegid((gid_t) 0);
	}
}

/* testing default acl */
void test_acl_default(char *dir, acl_t acl)
{
	/* set default acl on directory */
	/* create a file in this directory */
	/* compare the file's acl and the parent directory's one */
	int res;
	acl_t acl1, acl2;

	res = acl_set_file(dir, ACL_TYPE_DEFAULT, acl);
	acl1 = acl_get_file(dir, ACL_TYPE_DEFAULT);
	if (res == -1)
		printf("path = %s **** errno = %d", dir, errno);
	char *path = strcat(dir, "/testfile");
	fopen(path, "w+");
	char *cmd = malloc(256);

	strcpy(cmd, "chmod 7777 ");
	printf(cmd, NULL);
	strcat(cmd, dir);
	system(cmd);
	acl2 = acl_get_file(path, ACL_TYPE_ACCESS);

	test1(path);
	test2(path);
	test3(path);
	test4(path);
	test5(path);
}

static void showstats(void)
{
	printf("\nACL TESTS RESULTS: %d passed, %d failed\n\n", aclstat.ok,
	       aclstat.failed);
}

int main(int argc, char *argv[])
{
	int result;
	aclstat.ok = 0;
	aclstat.failed = 0;
	acl_t testacl;
	printf("Test acl with entries on users\n");
	testacl = test_acl_user_create();

	/* set the right acl for the test */
	result = acl_set_file(argv[1], ACL_TYPE_ACCESS, testacl);
	if (result == -1) {
		printf("setting acl on file %s failed\nBad NFS configuration",
		       argv[1]);
		exit(1);
	}
	test1(argv[1]);
	test2(argv[1]);
	test3(argv[1]);
	test4(argv[1]);
	test5(argv[1]);
	acl_free(testacl);
	printf("\nTest of default acl:\n");

	testacl = test_acl_default_create();
	test_acl_default(argv[2], testacl);

	printf("\nTest acl with entries concerning groups\n");
	testacl = test_acl_grp_create();
	result = acl_set_file(argv[1], ACL_TYPE_ACCESS, testacl);
	if (result == -1)
		printf("setting acl on file %s failed\n", argv[1]);

	testgrp1(argv[1]);
	testgrp2(argv[1]);
	testgrp3(argv[1]);
	testgrp4(argv[1]);
	testgrp5(argv[1]);

	acl_free(testacl);

	showstats();
	return 1;
}
#else
int main(void)
{
	printf("The acl library was missing upon compilation.\n");
	return TCONF;
}
#endif /* HAVE_LIBACL */
