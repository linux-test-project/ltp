#ifndef CPUINFO_H
# define CPUINFO_H

struct cpuinfo {
	int nodeid;	/* which node is this CPU on */
	int online;	/* is this CPU online */

	/* sched domains in each sched level of this CPU */
	struct bitmask *sched_domain;
};

extern struct cpuinfo *cpus;
extern int ncpus;
extern int cpus_nbits;
extern struct bitmask **domains;
extern int ndomains;

extern int online_cpumask(struct bitmask *cpumask);
extern int present_cpumask(struct bitmask *cpumask);
extern int get_ncpus(void);
extern int getcpuinfo(void);
extern int partition_domains(void);

#endif
