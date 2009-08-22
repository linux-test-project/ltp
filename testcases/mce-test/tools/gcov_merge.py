#!/usr/bin/env python
#
# Merge gcov graph from several test cases. This can be used to check
# the coverage of several test cases.
#
# Copyright (C) 2008, Intel Corp.
#   Author: Huang Ying <ying.huang@intel.com>
#
# This file is released under the GPLv2.

import sys

def die(str):
    print str
    sys.exit(-1)

def die_on(cond, str):
    if cond:
        die(str)

class GCovLine(object):
    def __init__(self, l):
        object.__init__(self)
        self.parse(l)
    def parse(self, l):
        sep = l.find(':')
        die_on(sep <= 0, 'input error format error')
        remain = l[sep:]
        scnt = l[:sep]
        if scnt[-1] == '-':
            count = -1
        elif scnt[-1] == '#':
            count = 0
        else:
            count = int(scnt)
        self.sep = sep
        self.count = count
        self.remain = remain
    def merge(self, gcl):
        self.count = self.count + gcl.count
    def write(self, of):
        if self.count < 0:
            of.write("%*s" % (self.sep, '-'))
        elif self.count == 0:
            of.write("%*s" % (self.sep, '#####'))
        else:
            of.write("%*d" % (self.sep, self.count))
        of.write(self.remain)

def parse(f):
    return [GCovLine(l) for l in f]

def merge(gcls1, gcls2):
    for gcl1, gcl2 in zip(gcls1, gcls2):
        gcl1.merge(gcl2)

def gcov_merge(fns, of):
    f = file(fns[0])
    gcls_base = parse(f)

    for fn in fns[1:]:
        f = file(fn)
        gcls = parse(f)
        merge(gcls_base, gcls)

    for gcl in gcls_base:
        gcl.write(of)

def usage():
    print 'Usage: %s <gcov graph files>' % (sys.argv[0])

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        usage()
        exit -1
    gcov_merge(sys.argv[1:], sys.stdout)
