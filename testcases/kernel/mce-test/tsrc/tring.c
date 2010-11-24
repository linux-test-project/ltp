/* Unit tester for ring buffer code in mce.c */
#define DEFINE_PER_CPU(a,b) a b
#define __get_cpu_var(x) x
#define barrier() asm volatile("" ::: "memory")
#define rmb() barrier()
#define wmb() barrier()

/*
 * Simple lockless ring to communicate PFNs from the exception handler with the
 * process context work function. This is vastly simplified because there's
 * only a single reader and a single writer.
 */
#define MCE_RING_SIZE 16	/* we use one entry less */

struct mce_ring {
	unsigned short start;
	unsigned short end;
	unsigned long ring[MCE_RING_SIZE];
};
static DEFINE_PER_CPU(struct mce_ring, mce_ring);

static int mce_ring_empty(void)
{
	struct mce_ring *r = &__get_cpu_var(mce_ring);

	return r->start == r->end;
}

static int mce_ring_get(unsigned long *pfn)
{
	struct mce_ring *r = &__get_cpu_var(mce_ring);

	if (r->start == r->end)
		return 0;
	*pfn = r->ring[r->start];
	r->start = (r->start + 1) % MCE_RING_SIZE;
	return 1;
}

static int mce_ring_add(unsigned long pfn)
{
	struct mce_ring *r = &__get_cpu_var(mce_ring);
	unsigned next;

	next = (r->end + 1) % MCE_RING_SIZE;
	if (next == r->start)
		return -1;
	r->ring[r->end] = pfn;
	wmb();
	r->end = next;
	return 0;
}

#include <stdio.h>
#include <assert.h>
#include <pthread.h>

void *thread(void *arg)
{
	long i = 0;
	for (;;) { 
		if (mce_ring_add(i) >= 0)
			i++;
	}
}

int main(void)
{
	long k;
	
	pthread_t thr;
	pthread_create(&thr, NULL, thread, NULL);

	k = 0;
	for (;;) { 	
		while (!mce_ring_empty()) { 
			unsigned long pfn;
			int r = mce_ring_get(&pfn);
			assert(r != 0);
			if (pfn != k) 
				printf("got %lu expected %lu delta %ld\n", pfn, k, k-pfn);
			k++;
		}
	}

	return 0;
}
