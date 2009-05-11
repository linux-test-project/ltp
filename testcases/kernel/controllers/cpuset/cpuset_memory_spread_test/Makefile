CFLAGS += -Wall -g -Wextra

all: cpuset_mem_hog

cpuset_mem_hog: cpuset_mem_hog.o
	cc cpuset_mem_hog.o -o cpuset_mem_hog

clean:
	rm -f cpuset_mem_hog.o cpuset_mem_hog

install:
	@set -e;
	ln -f cpuset_mem_hog ../../../../bin/cpuset_mem_hog;
	ln -f cpuset_memory_spread_testset.sh ../../../../bin/cpuset_memory_spread_testset.sh;
	chmod +x cpuset_mem_hog;
	chmod +x cpuset_memory_spread_testset.sh;
