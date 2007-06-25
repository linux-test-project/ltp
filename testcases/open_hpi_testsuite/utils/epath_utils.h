/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 */

#ifndef __EPATH_UTILS_H
#define __EPATH_UTILS_H

#ifndef __OH_UTILS_H
#warning *** Include oh_utils.h instead of individual utility header files ***
#endif

#include <glib.h>
#include <SaHpi.h>

/* Character for blanking out normalized strings based on entity path */
#define OH_DERIVE_BLANK_CHAR 'x'
#define OH_DERIVE_BLANK_STR "x"

/* Max number of digits an enitity instance has */
#define OH_MAX_LOCATION_DIGITS 6

/* Definitions for describing entity path patterns */
#define OH_MAX_EP_TUPLES SAHPI_MAX_ENTITY_PATH+1

typedef struct {
        SaHpiBoolT is_dot;
        SaHpiEntityTypeT type;
} oh_entity_type_pattern;

typedef struct {
        SaHpiBoolT is_dot;
        SaHpiEntityLocationT location;
} oh_entity_location_pattern;

typedef struct {
        SaHpiBoolT is_splat;
        oh_entity_type_pattern etp;
        oh_entity_location_pattern elp;
} oh_entity_pattern;

typedef struct {
        oh_entity_pattern epattern[OH_MAX_EP_TUPLES];
} oh_entitypath_pattern;

#ifdef __cplusplus
extern "C" {
#endif 

SaHpiBoolT oh_cmp_ep(const SaHpiEntityPathT *ep1,
		     const SaHpiEntityPathT *ep2);
	
SaErrorT oh_concat_ep(SaHpiEntityPathT *dest,
		      const SaHpiEntityPathT *append);

SaErrorT oh_decode_entitypath(const SaHpiEntityPathT *ep,
			      oh_big_textbuffer *bigbuf);

SaErrorT oh_encode_entitypath(const gchar *epstr,
			      SaHpiEntityPathT *ep);

SaErrorT oh_init_ep(SaHpiEntityPathT *ep);

#define oh_print_ep(ep_ptr, offsets) oh_fprint_ep(stdout, ep_ptr, offsets)
SaErrorT oh_fprint_ep(FILE *stream, const SaHpiEntityPathT *ep, int offsets);

SaErrorT oh_set_ep_location(SaHpiEntityPathT *ep,
			    SaHpiEntityTypeT et,
			    SaHpiEntityLocationT ei);

SaHpiBoolT oh_valid_ep(const SaHpiEntityPathT *ep);

gchar * oh_derive_string(SaHpiEntityPathT *ep,
			 SaHpiEntityLocationT offset,
			 int base,
			 const gchar *str);
                     
SaErrorT oh_compile_entitypath_pattern(const char *epp_str,
                                       oh_entitypath_pattern *epp);
SaHpiBoolT oh_match_entitypath_pattern(oh_entitypath_pattern *epp,
                                       SaHpiEntityPathT *ep);

#ifdef __cplusplus
}
#endif

#endif
