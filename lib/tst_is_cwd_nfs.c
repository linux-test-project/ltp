/******************************************************************************/
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/******************************************************************************/
/*
 *    AUTHOR
 *     Kumar Gala <galak@kernel.crashing.org>, 2007-11-14
 *     based on tst_is_cwd_tmpfs()
 *
 *    DESCRIPTION
 *     Check if current directory is on a nfs filesystem
 *     If current directory is nfs, return 1
 *     If current directory is NOT nfs, return 0
 *
 *
 */
/******************************************************************************/

#include <sys/vfs.h>

#define NFS_MAGIC 0x6969 /* man 2 statfs */

int
tst_is_cwd_nfs()
{
       struct statfs sf;
       statfs(".", &sf);

       /* Verify that the file is not on a nfs filesystem */
       return sf.f_type == NFS_MAGIC?1:0;
}
