ifeq ($(shell perl -v > /dev/null 2>&1 && echo yes),yes)

all: bal

install:
	./do_nothing.sh

else

all install:
	@echo "WARNING: No perl detected; skipping ballista tests"

endif

.configured:
	cd ballista ; ./configure
	touch .configured

bal: .configured
	$(MAKE) -i -C ballista

clean:
	$(MAKE) -C ballista clean
	rm -f .configured
