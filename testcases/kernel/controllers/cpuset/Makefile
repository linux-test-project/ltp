SUBDIRS = cpuset_base_ops_test cpuset_lib cpuset_inherit_test
SUBDIRS += cpuset_exclusive_test cpuset_hierarchy_test cpuset_syscall_test
SUBDIRS += cpuset_hotplug_test cpuset_load_balance_test
SUBDIRS += cpuset_memory_pressure_test cpuset_memory_spread_test
SUBDIRS += cpuset_memory_test
all:
	@set -e; \
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ;done;

install:
	@set -e; \
	for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done;
	ln -f run_cpuset_test.sh ../../../bin/run_cpuset_test.sh;
	chmod +x run_cpuset_test.sh;

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
