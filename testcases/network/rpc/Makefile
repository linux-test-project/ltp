SUBDIRS = basic_tests

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i ; done

install:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done
	chmod +x rpc-tirpc-full-test-suite/run_tirpc_tests.sh
	ln -f rpc-tirpc-full-test-suite/run_tirpc_tests.sh ../../bin
	chmod +x rpc-tirpc-full-test-suite/run_rpc_tests.sh
	ln -f rpc-tirpc-full-test-suite/run_rpc_tests.sh ../../bin
	chmod +x rpc-tirpc-full-test-suite/rpc_ts_wizard.sh
	ln -f rpc-tirpc-full-test-suite/rpc_ts_wizard.sh ../../bin

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

