#!/usr/bin/python
''' Reusable functions related to sched mc FVT are put together
'''

import os
import sys

__author__ = "Vaidyanathan Srinivasan <svaidy@linux.vnet.ibm.com>"
__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"

cpu_map = {}
stats_start = {}
stats_stop = {}
stats_percentage = {}
cpu_count = 0

def get_proc_data(stats_list):
    ''' Read /proc/stat info and store in dictionary
    '''
    try:
        file_procstat = open("/proc/stat", 'r')
        for line in file_procstat:
            if line.startswith('cpu'):
                data = line.split()
                stats_list[data[0]] = data
        file_procstat.close()
    except OSError, e:
        print "Could not read statistics", e
     sys.exit(1)

def set_sched_mc_power(sched_mc_power):
    ''' Routine to set sched_mc_power_savings to required level
    '''
    try:
        os.system('echo %s > \
            /sys/devices/system/cpu/sched_mc_power_savings'
            % sched_mc_power)
    except OSError, e:
        print "Could not set sched_mc_power_savings to %s" % mc_level
        sys.exit(1)

def count_num_cpu():
    ''' Returns number of cpu's in system
    '''
    try:
        cpuinfo = open('/proc/cpuinfo', 'r')
        global cpu_count
        for line in cpuinfo:
            if line.startswith('processor'):
                cpu_count += 1
        cpuinfo.close()
    except:
        print "Could not get cpu count", e
        sys.exit(1)

def map_cpuid_pkgid():
    ''' Routine to map physical package id to cpu id
    '''
    for i in range(0, cpu_count):
        try:
            phy_pkg_file = '/sys/devices/system/cpu/cpu%s' %i
            phy_pkg_file += '/topology/physical_package_id'
            cpu_phy_id = open(phy_pkg_file).read().rstrip()
            try:
                cpu_map[cpu_phy_id].append(i)
            except KeyError:
                cpu_map[cpu_phy_id] = [i]
        except:
            print "Mapping of CPU to pkg id failed"
            sys.exit(1)

def trigger_workld(workload):
    ''' Based on value in argument passed triggers workloads.
        Triggers workload with number of threads same as number
        of cores in package.
    '''
    get_proc_data(stats_start)
    threads = cpu_count / len(cpu_map)
    duration = 60 # let the test run for 1 minute
    path = '%s/utils/benchmark' % os.environ['LTPROOT']
    try:
        olddir = os.getcwd()
        os.chdir(path)
        wklds_avlbl = list()
        workload_dir = ""
        # Use the latest version of simillar workload available
        for file in os.listdir('.'):
            if file.find(workload) != -1:
                wklds_avlbl.append(file)
        wklds_avlbl.sort()
        workload_dir = wklds_avlbl[len(wklds_avlbl)-1]

        if workload_dir != "":
            new_path = os.path.join(path,"%s" % workload_dir)
            os.chdir(new_path)
            if workload == "ebizzy":
                try:
                    os.system('./ebizzy -t%s -s4096 -S %s >/dev/null'
                        % (threads, duration))
                    print "INFO: ebizzy workload triggerd"
                    os.chdir(olddir)
                    get_proc_data(stats_stop)
                except OSError, e:
                    print "Workload trigger failed",e
                    sys.exit(1)
        else:
            print "Benchmark/Workload is missing in LTP utils"
            os.chdir(olddir)
            sys.exit(1)
    except OSError, e:
            print "Benchmark/Workload trigger failed ", e
            sys.exit(1)

def generate_report():
    ''' Generate report of CPU utilization
    '''
    cpu_labels = ('cpu', 'user', 'nice', 'system', 'idle', 'iowait', 'irq',
     'softirq', 'x', 'y')
    if (not os.path.exists('/procstat')):
        os.mkdir('/procstat')

    reportfile = open('/procstat/cpu-utilisation', 'a')
    debugfile = open('/procstat/cpu-utilisation.debug', 'a')
    for l in stats_stop:
        percentage_list = []
        total = 0
        for i in range(1, len(stats_stop[l])):
            stats_stop[l][i] =  int(stats_stop[l][i]) - int(stats_start[l][i])
            total += stats_stop[l][i]
        percentage_list.append(l)
        for i in range(1, len(stats_stop[l])):
            percentage_list.append(float(stats_stop[l][i])*100/total)

        stats_percentage[l] = percentage_list

    for i in range(0, len(cpu_labels)):
        print >> debugfile, cpu_labels[i], '\t',
    print >> debugfile
    for l in sorted(stats_stop.keys()):
        print >> debugfile, l, '\t',
        for i in range(1, len(stats_stop[l])):
            print >> debugfile, stats_stop[l][i], '\t',
        print >> debugfile

    for i in range(0, len(cpu_labels)):
        print >> reportfile, cpu_labels[i], '\t',
    print >> reportfile
    for l in sorted(stats_percentage.keys()):
        print >> reportfile, l, '\t',
        for i in range(1, len(stats_percentage[l])):
            print >> reportfile, " %3.4f" % stats_percentage[l][i],
        print >> reportfile

    #Now get the package ID information
    print >> debugfile, "cpu_map: ", cpu_map
    keyvalfile = open('/procstat/keyval', 'a')
    print >> keyvalfile, "nr_packages=%d" % len(cpu_map)
    print >> keyvalfile, "system-idle=%3.4f" % stats_percentage['cpu'][4])
    for pkg in sorted(cpu_map.keys()):
        total_idle = 0
        total = 0
        for cpu in cpu_map[pkg]:
            total_idle += stats_stop["cpu%d" % cpu][4]
            for i in range(1, len(stats_stop["cpu%d" % cpu])):
                total += stats_stop["cpu%d" % cpu][i]
        print >> reportfile, "Package: ", pkg, "Idle %3.4f%%" \
           % (float(total_idle)*100/total)
        print >> keyvalfile, "package-%s=%3.4f" % \
           (pkg, (float(total_idle)*100/total))

    #Add record delimiter '\n' before closing these files
    print >> debugfile
    debugfile.close()
    print >> reportfile
    reportfile.close()
    print >> keyvalfile
    keyvalfile.close()

def validate_cpu_consolidation():
    ''' Verify if cpu's on which threads executed belong to same
    package
    '''
    cpus_utilized = list()
    for l in sorted(stats_percentage.keys()):
        if stats_percentage[l][1] > 20:
            cpu_id = stats_percentage[l][0].split("cpu")
            if cpu_id[1] != '':
                cpus_utilized.append(int(cpu_id[1]))
    cpus_utilized.sort()
    print "INFO: CPU's utilized %s" %cpus_utilized

    for pkg in sorted(cpu_map.keys()):
        pkg_cpus = cpu_map[pkg]
        if pkg_cpus == cpus_utilized:
            return(0)
    sys.exit(1)

