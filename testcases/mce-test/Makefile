
all:
	$(MAKE) -C tools
	$(MAKE) -C tsrc

clean:
	$(MAKE) -C tools clean
	$(MAKE) -C tsrc clean
	$(MAKE) reset

distclean:
	$(MAKE) -C tools distclean
	$(MAKE) -C tsrc distclean
	$(MAKE) reset

reset:
	rm -rf work/*
	rm -rf results/*

test:
	$(MAKE) reset
	./drivers/simple/driver.sh simple.conf
	./drivers/kdump/driver.sh kdump.conf

test-simple:
	$(MAKE) reset
	./drivers/simple/driver.sh simple.conf
