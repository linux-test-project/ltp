// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com> */
/*\
 * [Description]
 *
 * Check that something (e.g. irqbalance daemon) is performing IRQ
 * load balancing.
 *
 * On many systems userland needs to set /proc/irq/$IRQ/smp_affinity
 * to prevent many IRQs being delivered to the same CPU.
 *
 * Note some drivers and IRQ controllers will distribute IRQs
 * evenly. Some systems will have housekeeping CPUs configured. Some
 * IRQs can not be masked etc. So this test is not appropriate for all
 * scenarios.
 *
 * Furthermore, exactly how IRQs should be distributed is a
 * performance and/or security issue. This is only a generic smoke
 * test. It will hopefully detect misconfigured systems and total
 * balancing failures which are often silent errors.
 *
 * Heuristic: Evidence of Change
 *
 * 1. Find IRQs with a non-zero count
 * 2. Check if they are now disallowed
 *
 * There are two sources of information we need to parse:
 *
 * 1. /proc/interrupts
 * 2. /proc/irq/$IRQ/smp_affinity
 *
 * We get the active IRQs and CPUs from /proc/interrupts. It also
 * contains the per-CPU IRQ counts and info we do not care about.
 *
 * We get the IRQ masks from each active IRQ's smp_affinity file. This
 * is a bitmask written out in hexadecimal format. It shows which CPUs
 * an IRQ may be received by.
 */

#include <stdlib.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_file_at.h"

enum affinity {
	ALLOW,
	DENY,
};

static unsigned int *irq_stats;
static enum affinity *irq_affinity;

static unsigned int nr_cpus;
static unsigned int nr_irqs;
static unsigned int *irq_ids;

static void collect_irq_info(void)
{
	char *buf = NULL, *c, *first_row;
	char path[PATH_MAX];
	size_t size = 1024;
	size_t ret, row, col;
	long acc;
	unsigned int cpu_total, mask_pos;
	int fd = SAFE_OPEN("/proc/interrupts", O_RDONLY);

	nr_cpus = 0;
	nr_irqs = 0;

	do {
		size *= 2;
		buf = SAFE_REALLOC(buf, size);
		SAFE_LSEEK(fd, 0, SEEK_SET);
		ret = SAFE_READ(0, fd, buf, size - 1);
	} while (ret >= size - 1);

	SAFE_CLOSE(fd);

	if (ret < 1)
		tst_brk(TBROK, "Empty /proc/interrupts?");

	buf[ret] = '\0';

	/* Count CPUs, header columns are like /CPU[0-9]+/ */
	for (c = buf; *c != '\0' && *c != '\n'; c++) {
		if (!strncmp(c, "CPU", 3))
			nr_cpus++;
	}

	c++;
	first_row = c;
	/* Count IRQs, real IRQs start with /[0-9]+:/ */
	while (*c != '\0') {
		switch (*c) {
		case ' ':
		case '\t':
		case '\n':
		case '0' ... '9':
			c++;
			break;
		case ':':
			nr_irqs++;
			/* fall-through */
		default:
			while (*c != '\n' && *c != '\0')
				c++;
		}
	}

	tst_res(TINFO, "Found %u CPUS, %u IRQs", nr_cpus, nr_irqs);

	irq_ids = SAFE_REALLOC(irq_ids, nr_irqs * sizeof(*irq_ids));
	irq_stats = SAFE_REALLOC(irq_stats, nr_cpus * (nr_irqs + 1) * sizeof(*irq_stats));
	irq_affinity = SAFE_REALLOC(irq_affinity, nr_cpus * nr_irqs * sizeof(*irq_affinity));

	c = first_row;
	acc = -1;
	row = col = 0;
	/* Parse columns containing IRQ counts and IRQ IDs into acc. Ignore everything else. */
	while (*c != '\0') {
		switch (*c) {
		case ' ':
		case '\t':
			if (acc >= 0) {
				irq_stats[row * nr_cpus + col] = acc;
				acc = -1;
				col++;
			}
			break;
		case '\n':
			col = 0;
			row++;
			break;
		case '0' ... '9':
			if (acc == -1)
				acc = 0;

			acc *= 10;
			acc += *c - '0';
			break;
		case ':':
			irq_ids[row] = acc;
			acc = -1;
			break;
		default:
			acc = -1;
			while (*c != '\n' && *c != '\0')
				c++;
			continue;
		}

		c++;
	}

	for (col = 0; col < nr_cpus; col++) {
		cpu_total = 0;

		for (row = 0; row < nr_irqs; row++)
			cpu_total += irq_stats[row * nr_cpus + col];

		irq_stats[row * nr_cpus + col] = cpu_total;
	}

	/* Read the CPU affinity masks for each IRQ. See bitmap_string() in the kernel (%*pb) */
	for (row = 0; row < nr_irqs; row++) {
		sprintf(path, "/proc/irq/%u/smp_affinity", irq_ids[row]);
		ret = SAFE_FILE_READAT(0, path, buf, size);
		if (ret < 1)
			tst_brk(TBROK, "Empty /proc/irq/%u/smp_affinity?", irq_ids[row]);
		c = buf;
		col = 0;

		while (*c != '\0') {
			switch (*c) {
			case '\n':
			case ' ':
			case ',':
				c++;
				continue;
			case '0' ... '9':
				acc = *c - '0';
				break;
			case 'a' ... 'f':
				acc = 10 + *c - 'a';
				break;
			default:
				tst_res(TINFO, "%u/smp_affnity: %s", irq_ids[row], buf);
				tst_brk(TBROK, "Wasn't expecting 0x%02x", *c);
			}

			for (mask_pos = 0; mask_pos < 4; mask_pos++) {
				if (mask_pos + col >= nr_cpus)
					break;

				irq_affinity[row * nr_cpus + (nr_cpus - 1 - col - mask_pos)] =
					(acc & (8 >> mask_pos)) ? ALLOW : DENY;
			}

			col += mask_pos;
			c++;
		}
	}

	free(buf);
}

static void print_irq_info(void)
{
	size_t row, col;
	unsigned int count;
	enum affinity aff;

	for (row = 0; row < nr_irqs; row++) {
		tst_printf("%5u:", irq_ids[row]);

		for (col = 0; col < nr_cpus; col++) {
			count = irq_stats[row * nr_cpus + col];
			aff = irq_affinity[row * nr_cpus + col] == ALLOW ? '+' : '-';

			tst_printf("%10u%c", count, aff);
		}

		tst_printf("\n");
	}

	tst_printf("Total:");

	for (col = 0; col < nr_cpus; col++)
		tst_printf("%11u", irq_stats[row * nr_cpus + col]);

	tst_printf("\n");
}

static void evidence_of_change(void)
{
	size_t row, col, changed = 0;

	for (row = 0; row < nr_irqs; row++) {
		for (col = 0; col < nr_cpus; col++) {
			if (!irq_stats[row * nr_cpus + col])
				continue;

			if (irq_affinity[row * nr_cpus + col] == ALLOW)
				continue;

			changed++;
		}
	}

	tst_res(changed ? TPASS : TFAIL,
		"Heuristic: Detected %zu irq-cpu pairs have been dissallowed",
		changed);
}

static void setup(void)
{
	collect_irq_info();
	print_irq_info();

	if (nr_cpus < 1)
		tst_brk(TBROK, "No CPUs found in /proc/interrupts?");

	if (nr_irqs < 1)
		tst_brk(TBROK, "No IRQs found in /proc/interrupts?");
}

static void run(void)
{
	collect_irq_info();

	evidence_of_change();
}

static void cleanup(void)
{
	if (irq_ids)
		free(irq_ids);
	if (irq_stats)
		free(irq_stats);
	if (irq_affinity)
		free(irq_affinity);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_cpus = 2,
};
