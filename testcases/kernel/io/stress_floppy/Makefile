all: 

install: generate
	ln -f stress_floppy ../../../bin
	ln -f *_file ../../../bin
	ln -fs ../kernel/io/stress_floppy/dumpdir ../../../bin/dumpdir


generate:
	./generate.sh

clean:
	rm -rf 10*K_file dumpdir
