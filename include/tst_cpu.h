/*
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TST_CPU_H__
#define TST_CPU_H__

long tst_ncpus(void);
long tst_ncpus_conf(void);
long tst_ncpus_max(void);

#define VIRT_XEN	1	/* xen dom0/domU */
#define VIRT_KVM	2	/* only default virtual CPU */

int tst_is_virt(int virt_type);

#endif	/* TST_CPU_H__ */
