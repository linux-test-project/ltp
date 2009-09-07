
srcdir := ..
include $(srcdir)/config.mk

SUBDIRS = \
	async_handler \
	gtod_latency \
	hrtimer-prio \
	matrix_mult \
	measurement \
	periodic_cpu_load \
	pi-tests \
	pi_perf \
	prio-preempt \
	prio-wake \
	pthread_kill_latency \
	sched_football \
	sched_jitter \
	sched_latency \
	thread_clock \
	rt-migrate

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
