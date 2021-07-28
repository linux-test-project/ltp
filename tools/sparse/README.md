# Sparse based linting

This tool checks LTP test and library code for common problems.

## Usage

It is integrated with the LTP build system. Just run `make check` or
`make check-a_test01`, where `a_test01` is an arbitrary test
executable or object file.

## Building

The bad news is you must get and build Sparse[^1]. The good news is
that this only takes a minute and the build system does it for
you. Just try running `make check` as described above.

However if you want to reuse an existing Sparse checkout. Then you can
do the following. Where `$SRC_PATH` is the path to the Sparse
directory.

```sh
$ cd tools/sparse
$ make SPARSE_SRC=$SRC_PATH
```
You can also manually fetch it via the git submodule

```sh
$ cd tools/sparse
$ git submodule update --init
```

### Modifying CFLAGS and -m32

When compiling the LTP with `-m32` it may break building
`sparse-ltp`. We do not pass LTP's `CFLAGS` or `HOST_CFLAGS` to
`libsparse.a`. In the best case it produces a lot of noise, in the
worst it breaks building anyway.

To avoid issues with m32, just pre-build the checker with a non-m32
config. It won't need to be built again unless you are modifying the
tool itself. Similar issues with cross-compiling could be handled in a
similar way. Simply pre-build `sparse-ltp` and `libsparse.a` with a separate
config.

### Clang

Note that while it is possible to build Sparse with Clang. This may
cause some issues. Namely `GCC_BASE` is set to the Clang resource
directory. This contains some headers Sparse can not parse.

[1]: Many distributions have a Sparse package. This only contains some executables. There is no shared library
