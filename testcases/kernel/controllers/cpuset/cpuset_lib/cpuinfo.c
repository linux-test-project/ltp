#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>

#include "bitmask.h"
#include "cpuset.h"
#include "common.h"
#include "cpuinfo.h"

#if HAVE_LINUX_MEMPOLICY_H

#define CPUINFO_FILE		"/proc/cpuinfo"
#define SCHEDSTAT_FILE		"/proc/schedstat"
#define CGROUPINFO_FILE		"/proc/cgroups"
#define SYS_CPU_DIR		"/sys/devices/system/cpu"
#define LIST_PRESENT_CPU_FILE	"/sys/devices/system/cpu/present"
#define LIST_ONLINE_CPU_FILE	"/sys/devices/system/cpu/online"

struct cpuinfo *cpus;
int ncpus;
int cpus_nbits;

/* get cpu_baseinfo from /proc/cpuinfo */
static int get_cpu_baseinfo(void)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];
	char *istr = NULL, *valstr = NULL, *saveptr = NULL;
	int ci = 0;
	int data = 0;

	/* get the number of cpus including offline cpus */
	if (!ncpus) {
		ncpus = get_ncpus();
		if (ncpus <= 0)
			return -1;
	}

	if (cpus != NULL) {
		free(cpus);
		cpus = NULL;
	}

	/* allocate the memory space for cpus */
	cpus = malloc(sizeof(*cpus) * ncpus);
	if (cpus == NULL)
		return -1;
	memset(cpus, 0, sizeof(*cpus) * ncpus);

	/* open file /proc/cpuinfo */
	if ((fp = fopen(CPUINFO_FILE, "r")) == NULL)
		return -1;

	/* get cpuinfo */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		istr = strtok_r(buf, "\t", &saveptr);
		valstr = strchr(saveptr, ':');
		if (valstr == NULL)
			continue;
		valstr++;
		sscanf(valstr, " %d\n", &data);
		if (!strcmp(istr, "processor")) {
			if (data >= ncpus) {
				warnx("Warn: wrong cpu index");
				fclose(fp);
				return -1;
			}
			ci = data;
			cpus[ci].online = 1;
		}
	}

	fclose(fp);
	return 0;
}

/*
 * get the cpu bitmask of the online processors
 *
 * return value: 0  - success
 *               -1 - failed
 */
int online_cpumask(struct bitmask *cpumask)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];
	int i;

	if (cpumask == NULL)
		return -1;
	/*
	 * open file /sys/devices/system/cpu/online and get online
	 * cpulist.
	 */
	if ((fp = fopen(LIST_ONLINE_CPU_FILE, "r")) == NULL) {
		if (get_cpu_baseinfo() != 0)
			return -1;
		for (i = 0; i < ncpus; i++) {
			if (cpus[i].online)
				bitmask_setbit(cpumask, i);
		}
	} else {
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			fclose(fp);
			return -1;
		}
		fclose(fp);

		/* parse present cpu list to bitmap */
		buf[strlen(buf) - 1] = '\0';
		if (bitmask_parselist(buf, cpumask) != 0)
			return -1;
	}

	return 0;
}

/*
 * get the cpu bitmask of the present processors including offline CPUs
 *
 * return value: 0  - success
 *               -1 - failed
 */
int present_cpumask(struct bitmask *cpumask)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];
	char c_relpath[PATH_MAX];
	int cpu = -1;

	if (cpumask == NULL)
		return -1;
	/*
	 * open file /sys/devices/system/cpu/present and get present
	 * cpulist.
	 */
	if ((fp = fopen(LIST_PRESENT_CPU_FILE, "r")) == NULL) {
		while_each_childdir(SYS_CPU_DIR, "/", c_relpath,
				    sizeof(c_relpath)) {
			if (!strncmp(c_relpath + 1, "cpu", 3)
			    && sscanf(c_relpath + 4, "%d", &cpu) > 0) {
				if (cpu >= 0)
					bitmask_setbit(cpumask, cpu);
			}
		}
	end_while_each_childdir} else {
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			fclose(fp);
			return -1;
		}
		fclose(fp);

		/* parse present cpu list to bitmap */
		buf[strlen(buf) - 1] = '\0';
		if (bitmask_parselist(buf, cpumask) != 0)
			return -1;
	}

	return 0;
}

/*
 * get the number of the processors including offline CPUs
 * We get this number from /sys/devices/system/cpu/present.
 * By analyzing the present cpu list, we get the number of all cpus
 */
int get_ncpus(void)
{
	struct bitmask *bmp = NULL;
	int n = 0;

	/* get the bitmask's len */
	cpus_nbits = cpuset_cpus_nbits();
	if (cpus_nbits <= 0)
		return -1;

	/* allocate the space for bitmask */
	bmp = bitmask_alloc(cpus_nbits);
	if (bmp == NULL)
		return -1;

	if (present_cpumask(bmp)) {
		bitmask_free(bmp);
		return -1;
	}

	/* Number of highest set bit +1 is the number of the CPUs */
	n = bitmask_last(bmp) + 1;
	bitmask_free(bmp);

	return n;
}

/* get the sched domain's info for each cpu */
static int get_sched_domains(void)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];
	char str1[20], str2[BUFFSIZE];
	int ci = 0;

	/* get the bitmask's len */
	if (!cpus_nbits) {
		cpus_nbits = cpuset_cpus_nbits();
		if (cpus_nbits <= 0) {
			warnx("get cpus nbits failed.");
			return -1;
		}
	}

	/* open file /proc/schedstat */
	if ((fp = fopen(SCHEDSTAT_FILE, "r")) == NULL)
		return -1;

	/* get cpuinfo */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		sscanf(buf, "%s %s", str1, str2);
		if (!strncmp(str1, "cpu", 3)) {
			ci = atoi(str1 + 3);
			if (ci < 0 || ci >= ncpus) {
				fprintf(stderr, "Warn: wrong cpu index");
				fclose(fp);
				return -1;
			}
		} else if (!strncmp(str1, "domain", 6)) {
			if (!cpus[ci].sched_domain) {
				cpus[ci].sched_domain =
				    bitmask_alloc(cpus_nbits);
				if (!cpus[ci].sched_domain) {
					fclose(fp);
					return -1;
				}
			}
			if (bitmask_parsehex(str2, cpus[ci].sched_domain)) {
				fclose(fp);
				return -1;
			}
		}
	}

	fclose(fp);
	return 0;
}

int getcpuinfo(void)
{
	int i;
	int node = -1;

	/* get the number of cpus including offline cpus */
	if (!ncpus) {
		ncpus = get_ncpus();
		if (ncpus <= 0)
			return -1;
	}

	if (cpus == NULL) {
		if (get_cpu_baseinfo() != 0) {
			warn("get base infomation of cpus from /proc/cpuinfo "
			     "failed.");
			return -1;
		}
	}

	/* which node is every cpu belong to? */
	for (i = 0; i < ncpus; i++) {
		node = cpuset_cpu2node(i);
		if (node == -1)
			warnx("cpu2node failed(cpu = %d)", i);
		cpus[i].nodeid = node;
	}

	/* get sched domain's infomation for each cpu */
	if (get_sched_domains()) {
		warnx("get sched domain's info for each cpu failed.");
		return -1;
	}

	return 0;
}

/* get the number of the cpuset groups */
static int get_num_cpusets(void)
{
	FILE *fp = NULL;
	char buf[BUFFSIZE];
	char subsys_name[BUFFSIZE];
	int num_cgroups = 0;
	int hierarchy;
	int enabled;

	/* open file /proc/cgroups and get num cpusets */
	if ((fp = fopen(CGROUPINFO_FILE, "r")) == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (!strncmp(buf, "cpuset", 6)) {
			sscanf(buf, "%s\t%d\t%d\t%d\n", subsys_name,
			       &hierarchy, &num_cgroups, &enabled);
		}
	}

	fclose(fp);

	return num_cgroups;
}

static struct cpuset **cpusets;
static int ncpusets;

static int find_domain_cpusets(char *relpath)
{
	struct cpuset *cp = NULL;
	char c_relpath[PATH_MAX];
	int ret = 0;

	if (relpath == NULL) {
		errno = -EFAULT;
		return -1;
	}

	cp = cpuset_alloc();
	if (cp == NULL) {
		errno = -ENOMEM;
		return -1;
	}

	if (cpuset_query(cp, relpath)) {
		cpuset_free(cp);
		return -1;
	}

	if (cpuset_cpus_weight(cp) == 0)
		return 0;

	if (cpuset_cpus_weight(cp) > 0
	    && cpuset_get_iopt(cp, "sched_load_balance") == 1) {
		cpusets[ncpusets] = cp;
		ncpusets++;
		return 0;
	}

	while_each_childdir(cpuset_mountpoint(), relpath, c_relpath,
			    sizeof(c_relpath)) {
		if ((ret = find_domain_cpusets(c_relpath)))
			break;
	}
	end_while_each_childdir;

	return ret;
}

struct bitmask **domains;
int ndomains;

int partition_domains(void)
{
	int num_cpusets = 0;
	int i, j;
	struct bitmask *cpusa = NULL, *cpusb = NULL, *cpusc = NULL;
	int *flg = NULL;
	int ret = 0;

	num_cpusets = get_num_cpusets();
	if (num_cpusets == 0) {
		warnx("cpuset subsystem is't compiled into kernel.");
		return -1;
	}

	if (!cpus_nbits) {
		cpus_nbits = cpuset_cpus_nbits();
		if (!cpus_nbits) {
			warnx("nbits of cpus is wrong.");
			return -1;
		}
	}

	cpusa = bitmask_alloc(cpus_nbits);
	if (cpusa == NULL) {
		warnx("bitmask_alloc for partition domains failed.");
		return -1;
	}

	cpusb = bitmask_alloc(cpus_nbits);
	if (cpusb == NULL) {
		warnx("bitmask_alloc for partition domains failed.");
		ret = -1;
		goto errcpusb;
	}

	cpusc = bitmask_alloc(cpus_nbits);
	if (cpusb == NULL) {
		warnx("bitmask_alloc for partition domains failed.");
		ret = -1;
		goto errcpusc;
	}

	cpusets = malloc(num_cpusets * sizeof(*cpusets));
	if (cpusets == NULL) {
		warnx("alloc cpusets space failed.");
		ret = -1;
		goto errcpusets;
	}

	if ((ret = find_domain_cpusets("/"))) {
		warnx("find domain cpusets failed.");
		goto errfindcpusets;
	}

	flg = malloc(num_cpusets * sizeof(int));
	if (flg == NULL) {
		warnx("alloc flg failed.");
		ret = -1;
		goto errfindcpusets;
	}
	memset(flg, 0, num_cpusets * sizeof(int));

	ndomains = ncpusets;
restart:
	for (i = 0; i < ncpusets; i++) {
		struct cpuset *cpa = cpusets[i];

		if (flg[i])
			continue;

		cpuset_getcpus(cpa, cpusa);

		for (j = i + 1; j < ncpusets; j++) {
			struct cpuset *cpb = cpusets[j];

			if (flg[j])
				continue;

			cpuset_getcpus(cpb, cpusb);
			if (bitmask_intersects(cpusa, cpusb)) {
				bitmask_or(cpusc, cpusa, cpusb);
				cpuset_setcpus(cpa, cpusc);
				flg[j] = 1;
				ndomains--;
				goto restart;
			}
		}
	}

	domains = malloc(ndomains * sizeof(*domains));
	if (domains == NULL) {
		warnx("alloc domains space failed.");
		ret = -1;
		goto errdomains;
	}

	for (i = 0, j = 0; i < ncpusets; i++) {
		if (flg[i])
			continue;
		domains[j] = bitmask_alloc(cpus_nbits);
		if (cpuset_getcpus(cpusets[i], domains[j])) {
			warnx("cpuset getcpus failed.");
			ret = -1;
			goto errgetdomains;
		}
		j++;
	}
	goto errdomains;

errgetdomains:
	for (i = 0; i < j; i++)
		bitmask_free(domains[i]);
	free(domains);
	domains = NULL;
errdomains:
	free(flg);
errfindcpusets:
	for (i = 0; i < ncpusets; i++)
		cpuset_free(cpusets[i]);
	free(cpusets);
	cpusets = NULL;
	ncpusets = 0;
errcpusets:
	bitmask_free(cpusc);
errcpusc:
	bitmask_free(cpusb);
errcpusb:
	bitmask_free(cpusa);
	return ret;
}

#endif
