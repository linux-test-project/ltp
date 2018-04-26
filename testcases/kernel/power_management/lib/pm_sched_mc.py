#!/usr/bin/env python3
''' Reusable functions related to sched mc FVT are put together
'''

import os
import sys
import re
from time import time

__author__ = "Vaidyanathan Srinivasan <svaidy@linux.vnet.ibm.com>"
__author__ = "Poornima Nayak <mpnayak@linux.vnet.ibm.com>"


cpu_map = {}
stats_start = {}
stats_stop = {}
stats_percentage = {}
intr_start = []
intr_stop = []
cpu_count = 0
socket_count = 0
cpu1_max_intr = 0
cpu2_max_intr = 0
intr_stat_timer_0 = []
siblings_list = []

def clear_dmesg():
    '''
       Clears dmesg
    '''
    try:
        os.system('dmesg -c >/dev/null')
    except OSError as e:
        print('Clearing dmesg failed', e)
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
    except IOError as e:
        print("Could not get cpu count", e)
        sys.exit(1)

def count_num_sockets():
    ''' Returns number of cpu's in system
    '''
    socket_list = []
    global socket_count
    try:
        for i in range(0, cpu_count):
            phy_pkg_file = '/sys/devices/system/cpu/cpu%s' % i
            phy_pkg_file += '/topology/physical_package_id'
            socket_id = open(phy_pkg_file).read().rstrip()
            if socket_id not in socket_list:
                socket_list.append(socket_id)
                socket_count = socket_count + 1
    except Exception as details:
        print("INFO: Failed to get number of sockets in system", details)
        sys.exit(1)

def is_multi_socket():
    '''Return 1 if the system is multi socket else return 0
    '''
    try:
        if socket_count > 1:
            return 1
        else:
            return 0
    except Exception:
        print("Failed to check if system is multi socket system")
        sys.exit(1)

def is_hyper_threaded():
    '''Return 1 if the system is hyper threaded else return 0
    '''
    try:
        file_cpuinfo = open("/proc/cpuinfo", 'r')
        for line in file_cpuinfo:
            if line.startswith('siblings'):
                siblings = line.split(":")
            if line.startswith('cpu cores'):
                cpu_cores = line.split(":")
                break
        if int( siblings[1] ) / int( cpu_cores[1] )> 1:
            file_cpuinfo.close()
            return 1
        else:
            return 0
    except Exception:
        print("Failed to check if system is hyper-threaded")
        sys.exit(1)

def is_multi_core():
    ''' Return true if system has sockets has multiple cores
    '''

    try:
        file_cpuinfo = open("/proc/cpuinfo", 'r')
        for line in file_cpuinfo:
            if line.startswith('siblings'):
                siblings = line.split(":")
            if line.startswith('cpu cores'):
                cpu_cores = line.split(":")
                break

        if int( siblings[1] ) == int( cpu_cores[1] ):
            if int( cpu_cores[1] ) > 1:
                multi_core = 1
            else:
                multi_core = 0
        else:
            num_of_cpus = int(siblings[1]) / int(cpu_cores[1])
            if num_of_cpus > 1:
                multi_core = 1
            else:
                multi_core = 0
        file_cpuinfo.close()
        return multi_core
    except Exception:
        print("Failed to check if system is multi core system")
        sys.exit(1)

def get_hyper_thread_count():
    ''' Return number of threads in CPU. For eg for x3950 this function
        would return 2. In future if 4 threads are supported in CPU, this
        routine would return 4
    '''
    try:
        file_cpuinfo = open("/proc/cpuinfo", 'r')
        for line in file_cpuinfo:
            if line.startswith('siblings'):
                siblings = line.split(":")
            if line.startswith('cpu cores'):
                cpu_cores = line.split(":")
                break
        return( int( siblings[1] ) / int( cpu_cores[1] ) )
    except Exception:
        print("Failed to check if system is hyper-threaded")
        sys.exit(1)

def map_cpuid_pkgid():
    ''' Routine to map physical package id to cpu id
    '''
    if is_hyper_threaded():
        core_info = {}
        try:
            for i in range(0, cpu_count):
                phy_pkg_file = '/sys/devices/system/cpu/cpu%s' % i
                phy_pkg_file += '/topology/physical_package_id'
                core_file = '/sys/devices/system/cpu/cpu%s' % i
                core_file += '/topology/core_id'
                core_id = open(core_file).read().rstrip()
                cpu_phy_id = open(phy_pkg_file).read().rstrip()
                if not cpu_phy_id in list(cpu_map.keys()):
                    core_info = {}
                else:
                    core_info = cpu_map[cpu_phy_id]
                if not core_id in list(core_info.keys()):
                    core_info[core_id] = [i]
                else:
                    core_info[core_id].append(i)
                cpu_map[cpu_phy_id] = core_info
        except Exception as details:
            print("Package, core & cpu map table creation failed", e)
            sys.exit(1)
    else:
        for i in range(0, cpu_count):
            try:
                phy_pkg_file = '/sys/devices/system/cpu/cpu%s' %i
                phy_pkg_file += '/topology/physical_package_id'
                cpu_phy_id = open(phy_pkg_file).read().rstrip()
                if not cpu_phy_id in list(cpu_map.keys()):
                    cpu_map[cpu_phy_id] = [i]
                else:
                    cpu_map[cpu_phy_id].append(i)
            except IOError as e:
                print("Mapping of CPU to pkg id failed", e)
                sys.exit(1)


def generate_sibling_list():
    ''' Routine to generate siblings list
    '''
    try:
        for i in range(0, cpu_count):
            siblings_file = '/sys/devices/system/cpu/cpu%s' % i
            siblings_file += '/topology/thread_siblings_list'
            threads_sibs = open(siblings_file).read().rstrip()
            thread_ids = threads_sibs.split("-")

            if not thread_ids in siblings_list:
                siblings_list.append(thread_ids)
    except Exception as details:
        print("Exception in generate_siblings_list", details)
        sys.exit(1)

def get_siblings(cpu_id):
    ''' Return siblings of cpu_id
    '''
    try:
        cpus = ""
        for i in range(0, len(siblings_list)):
            for cpu in siblings_list[i]:
                if cpu_id == cpu:
                    for j in siblings_list[i]:
                        # Exclude cpu_id in the list of siblings
                        if j != cpu_id:
                            cpus += j
                    return cpus
        return cpus
    except Exception as details:
        print("Exception in get_siblings", details)
        sys.exit(1)

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
    except OSError as e:
        print("Could not read statistics", e)
        sys.exit(1)

def get_proc_loc_count(loc_stats):
    ''' Read /proc/interrupts info and store in list
    '''
    try:
        file_procstat = open("/proc/interrupts", 'r')
        for line in file_procstat:
            if line.startswith(' LOC:') or line.startswith('LOC:'):
                data = line.split()
                for i in range(0, cpu_count):
                    # To skip LOC
                    loc_stats.append(data[i+1])
                file_procstat.close()
                return
    except Exception as details:
        print("Could not read interrupt statistics", details)
        sys.exit(1)


def set_sched_mc_power(sched_mc_level):
    ''' Routine to set sched_mc_power_savings to required level
    '''
    try:
        os.system('echo %s > \
            /sys/devices/system/cpu/sched_mc_power_savings 2>/dev/null'
            % sched_mc_level)

        get_proc_data(stats_start)
    except OSError as e:
        print("Could not set sched_mc_power_savings to", sched_mc_level, e)
    sys.exit(1)

def set_sched_smt_power(sched_smt_level):
    ''' Routine to set sched_smt_power_savings to required level
    '''
    try:
        os.system('echo %s > \
            /sys/devices/system/cpu/sched_smt_power_savings 2>/dev/null'
            % sched_smt_level)

        get_proc_data(stats_start)
    except OSError as e:
        print("Could not set sched_smt_power_savings to", sched_smt_level, e)
    sys.exit(1)

def set_timer_migration_interface(value):
    ''' Set value of timer migration interface to a value
        passed as argument
    '''
    try:
        os.system('echo %s > \
            /proc/sys/kernel/timer_migration 2>/dev/null' % value)
    except OSError as e:
        print("Could not set timer_migration to ", value, e)
        sys.exit(1)

def get_job_count(stress, workload, sched_smt):
    ''' Returns number of jobs/threads to be triggered
    '''

    try:
        if stress == "thread":
            threads = get_hyper_thread_count()
        if stress == "partial":
            threads = cpu_count / socket_count
            if is_hyper_threaded():
                if workload == "ebizzy" and int(sched_smt) ==0:
                    threads = threads / get_hyper_thread_count()
                if workload == "kernbench" and int(sched_smt) < 2:
                    threads = threads / get_hyper_thread_count()
        if stress == "full":
            threads = cpu_count
        if stress == "single_job":
            threads = 1
            duration = 180
        return threads
    except Exception as details:
        print("get job count failed ", details)
        sys.exit(1)

def trigger_ebizzy (sched_smt, stress, duration, background, pinned):
    ''' Triggers ebizzy workload for sched_mc=1
        testing
    '''
    try:
        threads = get_job_count(stress, "ebizzy", sched_smt)
        workload = "ebizzy"
        olddir = os.getcwd()
        path = '%s/testcases/bin' % os.environ['LTPROOT']
        os.chdir(path)
        workload_file = ""
        for file_name in os.listdir('.'):
            if file_name == workload:
                workload_file = file_name
                break
        if workload_file == "":
            print("INFO: ebizzy benchmark not found")
            os.chdir(olddir)
            sys.exit(1)
        get_proc_data(stats_start)
        get_proc_loc_count(intr_start)
        try:
            if background == "yes":
                succ = os.system('./ebizzy -t%s -s4096 -S %s >/dev/null &'
                    % (threads, duration))
            else:
                if pinned == "yes":
                    succ = os.system('taskset -c %s ./ebizzy -t%s -s4096 -S %s >/dev/null'
                        % (cpu_count -1, threads, duration))
                else:
                    succ = os.system('./ebizzy -t%s -s4096 -S %s >/dev/null'
                        % (threads, duration))

            if succ == 0:
                print("INFO: ebizzy workload triggerd")
                os.chdir(olddir)
                #Commented bcoz it doesnt make sense to capture it when workload triggered
                #in background
                #get_proc_loc_count(intr_stop)
                #get_proc_data(stats_stop)
            else:
                print("INFO: ebizzy workload triggerd failed")
                os.chdir(olddir)
                sys.exit(1)
        except Exception as details:
            print("Ebizzy workload trigger failed ", details)
            sys.exit(1)
    except Exception as details:
        print("Ebizzy workload trigger failed ", details)
        sys.exit(1)

def trigger_kernbench (sched_smt, stress, background, pinned, perf_test):
    ''' Trigger load on system like kernbench.
        Copys existing copy of LTP into as LTP2 and then builds it
        with make -j
    '''
    olddir = os.getcwd()
    try:
        threads = get_job_count(stress, "kernbench", sched_smt)

        dst_path = "/root"
        workload = "kernbench"
        olddir = os.getcwd()
        path = '%s/testcases/bin' % os.environ['LTPROOT']
        os.chdir(path)
        workload_file = ""
        for file_name in os.listdir('.'):
            if file_name == workload:
                workload_file = file_name
                break
        if workload_file != "":
            benchmark_path = path
        else:
            print("INFO: kernbench benchmark not found")
            os.chdir(olddir)
            sys.exit(1)

        os.chdir(dst_path)
        linux_source_dir=""
        for file_name in os.listdir('.'):
            if file_name.find("linux-2.6") != -1 and os.path.isdir(file_name):
                linux_source_dir=file_name
                break
        if linux_source_dir != "":
            os.chdir(linux_source_dir)
        else:
            print("INFO: Linux kernel source not found in /root. Workload\
               Kernbench cannot be executed")
            sys.exit(1)

        get_proc_data(stats_start)
        get_proc_loc_count(intr_start)
        if pinned == "yes":
            os.system ( 'taskset -c %s %s/kernbench -o %s -M -H -n 1 \
                >/dev/null 2>&1 &' % (cpu_count-1, benchmark_path, threads))

            # We have to delete import in future
            import time
            time.sleep(240)
            stop_wkld("kernbench")
        else:
            if background == "yes":
                os.system ( '%s/kernbench -o %s -M -H -n 1 >/dev/null 2>&1 &' \
                    % (benchmark_path, threads))
            else:
                if perf_test == "yes":
                    os.system ( '%s/kernbench -o %s -M -H -n 1 >/dev/null 2>&1' \
                        % (benchmark_path, threads))
                else:
                    os.system ( '%s/kernbench -o %s -M -H -n 1 >/dev/null 2>&1 &' \
                        % (benchmark_path, threads))
                    # We have to delete import in future
                    import time
                    time.sleep(240)
                    stop_wkld("kernbench")

        print("INFO: Workload kernbench triggerd")
        os.chdir(olddir)
    except Exception as details:
        print("Workload kernbench trigger failed ", details)
        sys.exit(1)

def trigger_workld(sched_smt, workload, stress, duration, background, pinned, perf_test):
    ''' Triggers workload passed as argument. Number of threads
        triggered is based on stress value.
    '''
    try:
        if workload == "ebizzy":
            trigger_ebizzy (sched_smt, stress, duration, background, pinned)
        if workload == "kernbench":
            trigger_kernbench (sched_smt, stress, background, pinned, perf_test)
    except Exception as details:
        print("INFO: Trigger workload failed", details)
        sys.exit(1)

def generate_report():
    ''' Generate report of CPU utilization
    '''
    cpu_labels = ('cpu', 'user', 'nice', 'system', 'idle', 'iowait', 'irq',
	'softirq', 'x', 'y')
    if (not os.path.exists('/procstat')):
        os.mkdir('/procstat')

    get_proc_data(stats_stop)

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
        print(cpu_labels[i], '\t', end=' ', file=debugfile)
    print(file=debugfile)
    for l in sorted(stats_stop.keys()):
        print(l, '\t', end=' ', file=debugfile)
        for i in range(1, len(stats_stop[l])):
            print(stats_stop[l][i], '\t', end=' ', file=debugfile)
        print(file=debugfile)

    for i in range(0, len(cpu_labels)):
        print(cpu_labels[i], '\t', end=' ', file=reportfile)
    print(file=reportfile)
    for l in sorted(stats_percentage.keys()):
        print(l, '\t', end=' ', file=reportfile)
        for i in range(1, len(stats_percentage[l])):
            print(" %3.4f" % stats_percentage[l][i], end=' ', file=reportfile)
        print(file=reportfile)

    #Now get the package ID information
    try:
        print("cpu_map: ", cpu_map, file=debugfile)
        keyvalfile = open('/procstat/keyval', 'a')
        print("nr_packages=%d" % len(cpu_map), file=keyvalfile)
        print("system-idle=%3.4f" % (stats_percentage['cpu'][4]), file=keyvalfile)
        for pkg in sorted(cpu_map.keys()):
            if is_hyper_threaded():
                for core in sorted(cpu_map[pkg].keys()):
                    total_idle = 0
                    total = 0
                    for cpu in cpu_map[pkg][core]:
                        total_idle += stats_stop["cpu%d" % cpu][4]
                        for i in range(1, len(stats_stop["cpu%d" % cpu])):
                            total += stats_stop["cpu%d" % cpu][i]
            else:
                total_idle = 0
                total = 0
                for cpu in cpu_map[pkg]:
                    total_idle += stats_stop["cpu%d" % cpu][4]
                    for i in range(1, len(stats_stop["cpu%d" % cpu])):
                        total += stats_stop["cpu%d" % cpu][i]
            print("Package: ", pkg, "Idle %3.4f%%" \
	        % (float(total_idle)*100/total), file=reportfile)
            print("package-%s=%3.4f" % \
		(pkg, (float(total_idle)*100/total)), file=keyvalfile)
    except Exception as details:
        print("Generating utilization report failed: ", details)
        sys.exit(1)

    #Add record delimiter '\n' before closing these files
    print(file=debugfile)
    debugfile.close()
    print(file=reportfile)
    reportfile.close()
    print(file=keyvalfile)
    keyvalfile.close()

def generate_loc_intr_report():
    ''' Generate interrupt report of CPU's
    '''
    try:
        if (not os.path.exists('/procstat')):
            os.mkdir('/procstat')

        get_proc_loc_count(intr_stop)

        reportfile = open('/procstat/cpu-loc_interrupts', 'a')
        print("==============================================", file=reportfile)
        print("     Local timer interrupt stats              ", file=reportfile)
        print("==============================================", file=reportfile)

        for i in range(0, cpu_count):
            intr_stop[i] =  int(intr_stop[i]) - int(intr_start[i])
            print("CPU%s: %s" %(i, intr_stop[i]), file=reportfile)
        print(file=reportfile)
        reportfile.close()
    except Exception as details:
        print("Generating interrupt report failed: ", details)
        sys.exit(1)

def record_loc_intr_count():
    ''' Record Interrupt statistics when timer_migration
        was disabled
    '''
    try:
        global intr_start, intr_stop
        for i in range(0, cpu_count):
            intr_stat_timer_0.append(intr_stop[i])
        intr_start = []
        intr_stop = []
    except Exception as details:
        print("INFO: Record interrupt statistics when timer_migration=0",details)

def expand_range(range_val):
    '''
       Expand the range of value into actual numbers
    '''
    ids_list = list()
    try:
        sep_comma = range_val.split(",")
        for i in range(0, len(sep_comma)):
            hyphen_values = sep_comma[i].split("-")
            if len(hyphen_values) == 1:
                ids_list.append(int(hyphen_values[0]))
            else:
                for j in range(int(hyphen_values[0]), int(hyphen_values[1])+1):
                    ids_list.append(j)
        return(ids_list)
    except Exception as details:
        print("INFO: expand_pkg_grps failed ", details)

def is_quad_core():
    '''
       Read /proc/cpuinfo and check if system is Quad core
    '''
    try:
        cpuinfo = open('/proc/cpuinfo', 'r')
        for line in cpuinfo:
            if line.startswith('cpu cores'):
                cores = line.split("cpu cores")
                num_cores = cores[1].split(":")
                cpuinfo.close()
                if int(num_cores[1]) == 4:
                    return(1)
                else:
                    return(0)
    except IOError as e:
        print("Failed to get cpu core information", e)
        sys.exit(1)

def validate_cpugrp_map(cpu_group, sched_mc_level, sched_smt_level):
    '''
       Verify if cpugrp belong to same package
    '''
    modi_cpu_grp = cpu_group[:]
    try:
        if is_hyper_threaded():
            for pkg in sorted(cpu_map.keys()):
                # if CPU utilized is across package this condition will be true
                if len(modi_cpu_grp) != len(cpu_group):
                    break
                for core in sorted(cpu_map[pkg].keys()):
                    core_cpus = cpu_map[pkg][core]
                    if core_cpus == modi_cpu_grp:
                        return 0
                    else:
                        #if CPUs used across the cores
                        for i in range(0, len(core_cpus)):
                            if core_cpus[i] in modi_cpu_grp:
                                modi_cpu_grp.remove(core_cpus[i])
                                if len(modi_cpu_grp) == 0:
                                    return 0
                            #This code has to be deleted
                            #else:
                                # If sched_smt == 0 then its oky if threads run
                                # in different cores of same package
                                #if sched_smt_level > 0 :
                                    #return 1
        else:
            for pkg in sorted(cpu_map.keys()):
                pkg_cpus = cpu_map[pkg]
                if len(cpu_group) == len(pkg_cpus):
                    if pkg_cpus == cpu_group:
                        return(0)
                else:
                    if int(cpus_utilized[0]) in cpu_map[pkg] or int(cpus_utilized[1]) in cpu_map[pkg]:
                        return(0)

        return(1)

    except Exception as details:
        print("Exception in validate_cpugrp_map: ", details)
        sys.exit(1)


def verify_sched_domain_dmesg(sched_mc_level, sched_smt_level):
    '''
       Read sched domain information from dmesg.
    '''
    cpu_group = list()
    try:
        dmesg_info = os.popen('dmesg').read()
        if dmesg_info != "":
            lines = dmesg_info.split('\n')
            for i in range(0, len(lines)):
                if lines[i].endswith('CPU'):
                    groups = lines[i+1].split("groups:")
                    group_info = groups[1]
                    if group_info.find("(") != -1:
                        openindex=group_info.index("(")
                        closeindex=group_info.index(")")
                        group_info=group_info.replace\
                            (group_info[openindex:closeindex+1],"")

                    subgroup = group_info.split(",")
                    for j in range(0, len(subgroup)):
                        cpu_group = expand_range(subgroup[j])
                        status = validate_cpugrp_map(cpu_group, sched_mc_level,\
                        sched_smt_level)
                        if status == 1:
                            if is_quad_core() == 1:
                                if int(sched_mc_level) == 0:
                                    return(0)
                                else:
                                    return(1)
                            else:
                                return(1)
            return(0)
        else:
            return(1)
    except Exception as details:
        print("Reading dmesg failed", details)
        sys.exit(1)

def get_cpu_utilization(cpu):
    ''' Return cpu utilization of cpu_id
    '''
    try:
        for l in sorted(stats_percentage.keys()):
            if cpu == stats_percentage[l][0]:
                return stats_percentage[l][1]
        return -1
    except Exception as details:
        print("Exception in get_cpu_utilization", details)
        sys.exit(1)

def validate_cpu_consolidation(stress, work_ld, sched_mc_level, sched_smt_level):
    ''' Verify if cpu's on which threads executed belong to same
    package
    '''
    cpus_utilized = list()
    threads = get_job_count(stress, work_ld, sched_smt_level)
    try:
        for l in sorted(stats_percentage.keys()):
            #modify threshold
            cpu_id = stats_percentage[l][0].split("cpu")
            if cpu_id[1] == '':
                continue
            if int(cpu_id[1]) in cpus_utilized:
                continue
            if is_hyper_threaded():
                if work_ld == "kernbench" and sched_smt_level < sched_mc_level:
                    siblings = get_siblings(cpu_id[1])
                    if siblings != "":
                        sib_list = siblings.split()
                        utilization = int(stats_percentage[l][1])
                        for i in range(0, len(sib_list)):
                            utilization += int(get_cpu_utilization("cpu%s" %sib_list[i]))
                    else:
                        utilization = stats_percentage[l][1]
                    if utilization > 40:
                        cpus_utilized.append(int(cpu_id[1]))
                        if siblings != "":
                            for i in range(0, len(sib_list)):
                                cpus_utilized.append(int(sib_list[i]))
                else:
                    # This threshold wuld be modified based on results
                    if stats_percentage[l][1] > 40:
                        cpus_utilized.append(int(cpu_id[1]))
            else:
                if work_ld == "kernbench" :
                    if stats_percentage[l][1] > 50:
                        cpus_utilized.append(int(cpu_id[1]))
                else:
                    if stats_percentage[l][1] > 70:
                        cpus_utilized.append(int(cpu_id[1]))
            cpus_utilized.sort()
        print("INFO: CPU's utilized ", cpus_utilized)

        # If length of CPU's utilized is not = number of jobs exit with 1
        if len(cpus_utilized) < threads:
            return 1

        status = validate_cpugrp_map(cpus_utilized, sched_mc_level, \
            sched_smt_level)
        if status == 1:
            print("INFO: CPUs utilized is not in same package or core")

        return(status)
    except Exception as details:
        print("Exception in validate_cpu_consolidation: ", details)
        sys.exit(1)

def get_cpuid_max_intr_count():
    '''Return the cpu id's of two cpu's with highest number of intr'''
    try:
        highest = 0
        second_highest = 0
        cpus_utilized = []

        #Skipping CPU0 as it is generally high
        for i in range(1, cpu_count):
            if int(intr_stop[i]) > int(highest):
                if highest != 0:
                    second_highest = highest
                    cpu2_max_intr = cpu1_max_intr
                highest = int(intr_stop[i])
                cpu1_max_intr = i
            else:
                if int(intr_stop[i]) > int(second_highest):
                    second_highest = int(intr_stop[i])
                    cpu2_max_intr = i
        cpus_utilized.append(cpu1_max_intr)
        cpus_utilized.append(cpu2_max_intr)

        for i in range(1, cpu_count):
            if i != cpu1_max_intr and i != cpu2_max_intr:
                diff = second_highest - intr_stop[i]
                ''' Threshold of difference has to be manipulated '''
                if diff < 10000:
                    print("INFO: Diff in interrupt count is below threshold")
                    cpus_utilized = []
                    return cpus_utilized
        print("INFO: Interrupt count in other CPU's low as expected")
        return cpus_utilized
    except Exception as details:
        print("Exception in get_cpuid_max_intr_count: ", details)
        sys.exit(1)

def validate_ilb (sched_mc_level, sched_smt_level):
    ''' Validate if ilb is running in same package where work load is running
    '''
    try:
        cpus_utilized = get_cpuid_max_intr_count()
        if not cpus_utilized:
            return 1

        status = validate_cpugrp_map(cpus_utilized, sched_mc_level, sched_smt_level)
        return status
    except Exception as details:
        print("Exception in validate_ilb: ", details)
        sys.exit(1)

def reset_schedmc():
    ''' Routine to reset sched_mc_power_savings to Zero level
    '''
    try:
        os.system('echo 0 > \
            /sys/devices/system/cpu/sched_mc_power_savings 2>/dev/null')
    except OSError as e:
        print("Could not set sched_mc_power_savings to 0", e)
        sys.exit(1)

def reset_schedsmt():
    ''' Routine to reset sched_smt_power_savings to Zero level
    '''
    try:
        os.system('echo 0 > \
            /sys/devices/system/cpu/sched_smt_power_savings 2>/dev/null')
    except OSError as e:
        print("Could not set sched_smt_power_savings to 0", e)
        sys.exit(1)

def stop_wkld(work_ld):
    ''' Kill workload triggered in background
    '''
    try:
        os.system('pkill %s 2>/dev/null' %work_ld)
        if work_ld == "kernbench":
            os.system('pkill make 2>/dev/null')
    except OSError as e:
        print("Exception in stop_wkld", e)
        sys.exit(1)
