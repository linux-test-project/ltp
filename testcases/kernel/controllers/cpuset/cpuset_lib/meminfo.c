#include <stdio.h>
#include <string.h>

#include "common.h"
#include "bitmask.h"
#include "cpuset.h"
#include "meminfo.h"

#define LIST_PRESENT_MEM_FILE	"/sys/devices/system/node/possible"
#define LIST_ONLINE_MEM_FILE	"/sys/devices/system/node/online"

int nmems;
int mems_nbits;

/*
 * get the bitmask of the online nodes
 *
 * return value: 0  - success
 * 		 -1 - failed
 */
int online_memmask(struct bitmask *memmask)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];

	if (memmask == NULL)
		return -1;

	/*
	 * open file /sys/devices/system/node/online and get online
	 * nodelist
	 */
	if ((fp = fopen(LIST_ONLINE_MEM_FILE, "r")) == NULL)
		return -1;

	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return -1;
	}

	fclose(fp);

	/* parse present nodelist to bitmap */
	buf[strlen(buf) - 1] = '\0';
	if (bitmask_parselist(buf, memmask) != 0)
		return -1;

	return 0;
}

/*
 * get the bitmask of the present nodes including offline nodes
 *
 * return value: 0  - success
 * 		 -1 - failed
 */
int present_memmask(struct bitmask *memmask)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];

	if (memmask == NULL)
		return -1;

	/*
	 * open file /sys/devices/system/node/possible and get present
	 * nodelist
	 */
	if ((fp = fopen(LIST_PRESENT_MEM_FILE, "r")) == NULL)
		return -1;

	if (fgets(buf, sizeof(buf), fp) == NULL) {
		fclose(fp);
		return -1;
	}

	fclose(fp);

	/* parse present nodelist to bitmap */
	buf[strlen(buf) - 1] = '\0';
	if (bitmask_parselist(buf, memmask) != 0)
		return -1;

	return 0;
}

/*
 * get the number of the nodes including offline nodes
 */
int get_nmems(void)
{
	struct bitmask *bmp = NULL;
	int n = 0;

	/* get the bitmask's len */
	if (mems_nbits <= 0) {
		mems_nbits = cpuset_mems_nbits();
		if (mems_nbits <= 0)
			return -1;
	}

	/* allocate the space for bitmask */
	bmp = bitmask_alloc(mems_nbits);
	if (bmp == NULL)
		return -1;

	if (present_memmask(bmp)) {
		bitmask_free(bmp);
		return -1;
	}

	/* Numbwe of highest set bit +1 is the number of the nodes */
	if (bitmask_weight(bmp) <= 0) {
		bitmask_free(bmp);
		return -1;
	}

	n = bitmask_last(bmp) + 1;
	bitmask_free(bmp);

	return n;
}
