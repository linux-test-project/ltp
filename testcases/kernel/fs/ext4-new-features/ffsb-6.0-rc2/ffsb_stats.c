#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "ffsb_stats.h"
#include "util.h"

char *syscall_names[] = {
	"open",
	"read",
	"write",
	"create",
	"lseek",
	"unlink",
	"close",
	"stat",
};

/* yuck, just for the parser anyway.. */
int ffsb_stats_str2syscall(char *str, syscall_t * sys)
{
	int i;
	int ret;
	for (i = 0; i < FFSB_NUM_SYSCALLS; i++) {
		ret = strncasecmp(syscall_names[i], str,
				  strlen(syscall_names[i]));
		/* printf("%s = syscall_names[%d] vs %str ret = %d\n",
		 * syscall_names[i],i,str,ret);
		 */
		if (0 == ret) {
			*sys = (syscall_t) i;	/* ewww */
			/* printf("matched syscall %s\n",syscall_names[i]); */
			return 1;
		}
	}
	printf("warning: failed to get match for syscall %s\n", str);
	return 0;
}

void ffsb_statsc_init(ffsb_statsc_t * fsc)
{
	fsc->num_buckets = 0;
	fsc->buckets = NULL;
	fsc->ignore_stats = 0;
}

void ffsb_statsc_addbucket(ffsb_statsc_t * fsc, uint32_t min, uint32_t max)
{
	struct stat_bucket *temp;
	fsc->num_buckets++;

	/* printf("ffsb_realloc(): fsc_buckets = %d\n",fsc->num_buckets); */
	temp = ffsb_realloc(fsc->buckets, sizeof(struct stat_bucket) *
			    fsc->num_buckets);

	fsc->buckets = temp;

	/* Convert to micro-secs from milli-secs */
	fsc->buckets[fsc->num_buckets - 1].min = min;
	fsc->buckets[fsc->num_buckets - 1].max = max;
}

void ffsb_statsc_destroy(ffsb_statsc_t * fsc)
{
	free(fsc->buckets);
}

void ffsb_statsc_ignore_sys(ffsb_statsc_t * fsc, syscall_t s)
{
	/* printf("fsis: oring 0x%x with 0x%x\n",
	 *      fsc->ignore_stats,
	 *      (1 << s ) );
	 */
	fsc->ignore_stats |= (1 << s);
}

int fsc_ignore_sys(ffsb_statsc_t * fsc, syscall_t s)
{
	return fsc->ignore_stats & (1 << s);
}

void ffsb_statsd_init(ffsb_statsd_t * fsd, ffsb_statsc_t * fsc)
{
	int i;
	memset(fsd, 0, sizeof(*fsd));

	for (i = 0; i < FFSB_NUM_SYSCALLS; i++) {
		fsd->totals[i] = 0;
		fsd->mins[i] = UINT_MAX;
		fsd->maxs[i] = 0;
		fsd->buckets[i] = ffsb_malloc(sizeof(uint32_t) *
					      fsc->num_buckets);
		assert(fsd->buckets[i] != NULL);

		memset(fsd->buckets[i], 0, sizeof(uint32_t) * fsc->num_buckets);
	}
	fsd->config = fsc;
}

void ffsb_statsd_destroy(ffsb_statsd_t * fsd)
{
	int i;
	for (i = 0; i < FFSB_NUM_SYSCALLS; i++)
		free(fsd->buckets[i]);
}

void ffsb_add_data(ffsb_statsd_t * fsd, syscall_t s, uint32_t value)
{
	unsigned num_buckets, i;
	struct stat_bucket *bucket_defs;

	if (!fsd || fsc_ignore_sys(fsd->config, s))
		return;

	if (value < fsd->mins[s])
		fsd->mins[s] = value;
	if (value > fsd->maxs[s])
		fsd->maxs[s] = value;

	fsd->counts[s]++;
	fsd->totals[s] += value;

	if (fsd->config->num_buckets == 0)
		return;

	num_buckets = fsd->config->num_buckets;
	bucket_defs = fsd->config->buckets;

	for (i = 0; i < num_buckets; i++) {
		struct stat_bucket *b = &bucket_defs[i];

		if (value <= b->max && value >= b->min) {
			fsd->buckets[s][i]++;
			break;
		}
	}
}

void ffsb_statsc_copy(ffsb_statsc_t * dest, ffsb_statsc_t * src)
{
	memcpy(dest, src, sizeof(*src));
}

void ffsb_statsd_add(ffsb_statsd_t * dest, ffsb_statsd_t * src)
{
	int i, j;
	unsigned num_buckets;
	if (dest->config != src->config)
		printf("ffsb_statsd_add: warning configs do not"
		       "match for data being collected\n");

	num_buckets = dest->config->num_buckets;

	for (i = 0; i < FFSB_NUM_SYSCALLS; i++) {
		dest->counts[i] += src->counts[i];
		dest->totals[i] += src->totals[i];

		if (src->mins[i] < dest->mins[i])
			dest->mins[i] = src->mins[i];
		if (src->maxs[i] > dest->maxs[i])
			dest->maxs[i] = src->maxs[i];

		for (j = 0; j < num_buckets; j++)
			dest->buckets[i][j] += src->buckets[i][j];
	}
}

static void print_buckets_helper(ffsb_statsc_t * fsc, uint32_t * buckets)
{
	int i;
	if (fsc->num_buckets == 0) {
		printf("   -\n");
		return;
	}
	for (i = 0; i < fsc->num_buckets; i++) {
		struct stat_bucket *sb = &fsc->buckets[i];
		printf("\t\t msec_range[%d]\t%f - %f : %8u\n",
		       i, (double)sb->min / 1000.0f, (double)sb->max / 1000.0f,
		       buckets[i]);
	}
	printf("\n");
}

void ffsb_statsd_print(ffsb_statsd_t * fsd)
{
	int i;
	printf("\nSystem Call Latency statistics in millisecs\n" "=====\n");
	printf("\t\tMin\t\tAvg\t\tMax\t\tTotal Calls\n");
	printf("\t\t========\t========\t========\t============\n");
	for (i = 0; i < FFSB_NUM_SYSCALLS; i++)
		if (fsd->counts[i]) {
			printf("[%7s]\t%05f\t%05lf\t%05f\t%12u\n",
			       syscall_names[i], (float)fsd->mins[i] / 1000.0f,
			       (fsd->totals[i] / (1000.0f *
						  (double)fsd->counts[i])),
			       (float)fsd->maxs[i] / 1000.0f, fsd->counts[i]);
			print_buckets_helper(fsd->config, fsd->buckets[i]);
		}
}

#if 0				/* Testing */

void *ffsb_malloc(size_t s)
{
	void *p = malloc(s);
	assert(p != NULL);
	return p;
}

int main(int arc, char *argv[])
{
	ffsb_statsc_t fsc;
	ffsb_statsd_t fsd;
	int i;

	printf("init\n");

	ffsb_statsc_init(&fsc, 10);
	ffsb_statsc_setbucket(&fsc, 0, 0.0f, 50.0f);
	ffsb_statsc_setbucket(&fsc, 1, 50.0f, 10000.0f);
	ffsb_statsc_setbucket(&fsc, 2, 0.1f, 0.2f);
	ffsb_statsc_setbucket(&fsc, 3, 0.0f, 50.0f);
	ffsb_statsc_setbucket(&fsc, 4, 50.0f, 10000.0f);
	ffsb_statsc_setbucket(&fsc, 5, 0.1f, 0.2f);
	ffsb_statsc_setbucket(&fsc, 6, 0.0f, 50.0f);
	ffsb_statsc_setbucket(&fsc, 7, 50.0f, 10000.0f);
	ffsb_statsc_setbucket(&fsc, 8, 0.1f, 0.2f);
	ffsb_statsc_setbucket(&fsc, 9, 50.0f, 10000.0f);
	ffsb_statsd_init(&fsd, &fsc);

	printf("test\n");
	for (i = 0; i < 50000000; i++)
		ffsb_add_data(&fsd, SYS_READ, (float)i);

	printf("cleanup\n");
	ffsb_statsd_destroy(&fsd);
	ffsb_statsc_destroy(&fsc);
	return 0;
}

#endif /* Testing */
