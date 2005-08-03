all: bal

install:
	./do_nothing.sh

create:
		 cd ballista ; ./configure

bal: create
	$(MAKE) -i -C ballista

clean:
		 $(MAKE) -C ballista clean
