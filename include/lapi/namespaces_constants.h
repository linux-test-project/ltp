/* Copyright (c) 2015 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#ifndef __NAMESPACES_CONSTANTS_H__
#define __NAMESPACES_CONSTANTS_H__

#ifndef CLONE_NEWIPC
#  define CLONE_NEWIPC	0x08000000
#endif
#ifndef CLONE_NEWNS
#  define CLONE_NEWNS	0x00020000
#endif
#ifndef CLONE_NEWNET
#  define CLONE_NEWNET	0x40000000
#endif
#ifndef CLONE_NEWPID
#  define CLONE_NEWPID	0x20000000
#endif
#ifndef CLONE_NEWUSER
#  define CLONE_NEWUSER	0x10000000
#endif
#ifndef CLONE_NEWUTS
#  define CLONE_NEWUTS	0x04000000
#endif

#endif /* __NAMESPACES_CONSTANTS_H__ */
