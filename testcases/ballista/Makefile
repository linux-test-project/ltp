all: bal

install:
	./do_nothing.sh

.configured:
	cd ballista ; ./configure
	touch .configured

bal: .configured
	$(MAKE) -i -C ballista

clean:
	$(MAKE) -C ballista clean
	rm -f create
