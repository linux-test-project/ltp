all: bal

install:
	./do_nothing.sh

create:
	./create.sh

bal: create
	$(MAKE) -i -C ballista

clean:
	$(MAKE) -C ballista clean