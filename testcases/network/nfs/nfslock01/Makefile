CFLAGS += -w -O
TESTS	= nfs_flock nfs_flock_dgen nfs_flock_frk

all: $(TESTS)

install:
	ln -f nfslock01 ../../../bin

nfs_flock:
	$(CC) $(CFLAGS) $(LDFLAGS) -o nfs_flock nfs_flock.c nfs_flock_func.c

nfs_flock_frk:
	$(CC) $(CFLAGS) $(LDFLAGS) -o nfs_flock_frk nfs_flock_frk.c

clean:
	rm -f $(TESTS)


