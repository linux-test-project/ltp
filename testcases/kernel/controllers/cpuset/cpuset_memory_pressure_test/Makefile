CFLAGS += -Wall -g -Wextra

all: cpuset_memory_pressure

cpuset_memory_pressure: cpuset_memory_pressure.o
	cc cpuset_memory_pressure.o -o cpuset_memory_pressure

clean:
	rm -f cpuset_memory_pressure.o cpuset_memory_pressure

install:
	@set -e;
	ln -f cpuset_memory_pressure ../../../../bin/cpuset_memory_pressure;
	ln -f cpuset_memory_pressure_testset.sh ../../../../bin/cpuset_memory_pressure_testset.sh;
	chmod +x cpuset_memory_pressure;
	chmod +x cpuset_memory_pressure_testset.sh;
