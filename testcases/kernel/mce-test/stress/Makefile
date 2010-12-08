CFLAGS := -g -Wall
LDFLAGS := -g

export CFLAGS
export LDFLAGS

all:
	mkdir -p bin
	$(MAKE) -C tools install

clean:
	$(MAKE) -C tools clean
	rm -rf bin log result runtest

distclean: clean

test: all stress-soft stress-hard

stress-soft:
	@echo "run soft stress tester for 60 seconds"
	./hwpoison.sh -T -C 1 -t 60 -S

stress-hard:
	@echo "run hard stress tester for 60 seconds"
	./hwpoison.sh -T -C 20 -t 60 -M
