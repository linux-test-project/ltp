all: k-tree-gen k-tree-trav 

k-tree-trav: k-tree-trav.o 

k-tree-gen: k-tree-gen.o

install: all
	cp fs-metadata.sh k-thread.sh k-tree-gen k-tree-trav ../../bin

clean:
	rm -f k-tree-gen k-tree-trav *.o
