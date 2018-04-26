#!/usr/bin/env python3
''' This Python script validates sched domain information in dmesg
    with information in sysfs topology
'''

import os
import sys
from pm_sched_mc import *
from optparse import OptionParser

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    usage = "-w"
    parser = OptionParser(usage)
    parser.add_option("-c", "--mc_level", dest="mc_level", default=-1,
        help="Sched mc power saving value 0/1/2")
    parser.add_option("-t", "--smt_level", dest="smt_level", default=-1,
        help="Sched smt power saving value 0/1/2")
    (options, args) = parser.parse_args()

    try:
        clear_dmesg()
        count_num_cpu()
        map_cpuid_pkgid()

        if is_hyper_threaded() and int(options.smt_level) >= 0:
            set_sched_smt_power(options.smt_level)

        if int(options.mc_level) >= 0:
            set_sched_mc_power(options.mc_level)
        if int(options.smt_level) >= 0 or int(options.mc_level) >= 0:
            status = verify_sched_domain_dmesg(options.mc_level, options.smt_level)
            reset_schedmc()
            if is_hyper_threaded():
                reset_schedsmt()
                return(status)
        else:
            print("INFO: Invalid arguments given")
            return 1
    except Exception as details:
        print("INFO: sched domain test failed: ", details)
        return(1)

# Run test based on the command line arguments
if __name__ == "__main__":
    sys.exit(main())
