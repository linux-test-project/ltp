#!/usr/bin/env python3
''' This Python script interprets interrupt values.
    Validates Ideal load balancer runs in same package where workload is running
'''

import os
import sys
from optparse import OptionParser
from pm_sched_mc import *

__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    usage = "-w"
    parser = OptionParser(usage)
    parser.add_option("-c", "--mc_level", dest="mc_level",
        default=0, help="Sched mc power saving value 0/1/2")
    parser.add_option("-t", "--smt_level", dest="smt_level",
        default=0, help="Sched smt power saving value 0/1/2")
    parser.add_option("-w", "--workload", dest="work_ld",
        default="ebizzy", help="Workload can be ebizzy/kernbench")
    (options, args) = parser.parse_args()

    try:
        count_num_cpu()
        count_num_sockets()
        if is_multi_socket():
            set_sched_mc_power(options.mc_level)
        if is_hyper_threaded():
            set_sched_smt_power(options.smt_level)
        map_cpuid_pkgid()
        print("INFO: Created table mapping cpu to package")
        background="no"
        duration=120
        pinned="yes"

        trigger_workld(options.smt_level,options.work_ld, "single_job", duration, background, pinned, "no")
        generate_loc_intr_report()
        status = validate_ilb(options.mc_level, options.smt_level)
        reset_schedmc()
        if is_hyper_threaded():
            reset_schedsmt()
        return(status)

    except Exception as details:
        print("INFO: Idle Load Balancer test failed", details)
        return(1)

if __name__ == "__main__":
    sys.exit(main())
