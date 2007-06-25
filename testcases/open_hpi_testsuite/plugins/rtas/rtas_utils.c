/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *        Renier Morales <renier@openhpi.org>
 *        Daniel de Araujo <ddearauj@us.ibm.com>
 */


#include <rtas_utils.h>

/**
 * librtas_error
 * @brief check for a librtas specific return code
 *
 * @param error librtas return code
 * @param buf buffer to write librtas error message to
 * @param size size of "buffer"
 */
void decode_rtas_error (int error, char *buf, size_t size, int token, int index) 
{
	switch (error) {
		case -1:
			snprintf(buf, size, "Hardware error retrieving a sensor: token %04d, "
				            "index %d\n", token, index);
			break;
		case -3:
			snprintf(buf, size,"The sensor at token %04d, index %d is not "
						    "implemented.\n", token, index);
			break;	
		case RTAS_KERNEL_INT:
			snprintf(buf, size, "No kernel interface to firmware");
			break;
		case RTAS_KERNEL_IMP:
			snprintf(buf, size, "No kernel implementation of function");
			break;
		case RTAS_PERM:
			snprintf(buf, size, "Non-root caller");
			break;
		case RTAS_NO_MEM:
			snprintf(buf, size, "Out of heap memory");
			break;
		case RTAS_NO_LOWMEM:
			snprintf(buf, size, "Kernel out of low memory");
			break;
		case RTAS_FREE_ERR:
			snprintf(buf, size, "Attempt to free nonexistant RMO buffer");
			break;
		case RTAS_TIMEOUT:
			snprintf(buf, size, "RTAS delay exceeded specified timeout");
			break;
		case RTAS_IO_ASSERT:
			snprintf(buf, size, "Unexpected I/O error");
			break;
		case RTAS_UNKNOWN_OP:
			snprintf(buf, size, "No firmware implementation of function");
			break;
		default:
			snprintf(buf, size, "Unknown librtas error %d", error);
	}

}
