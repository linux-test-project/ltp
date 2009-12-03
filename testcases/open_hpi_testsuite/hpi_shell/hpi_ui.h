/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Kouzmich	< Mikhail.V.Kouzmich@intel.com >
 *
 */

#ifndef __HPI_UI_H
#define __HPI_UI_H

#include <SaHpi.h>
#include <oh_utils.h>

#define SHOW_BUF_SZ	1024

/* defines for show_rpt_list function */

#define SHOW_ALL_RPT	0
#define SHOW_ALL_RDR	1
#define SHOW_RPT_RDR	2

#define	SHORT_LSRES	0x0
#define	STATE_LSRES	0x1
#define	PATH_LSRES	0x2

typedef unsigned char	uchar;

typedef union {
	int	i;
	double	d;
	char	*s;
	void	*a;
} union_type_t;

/*	value types	*/

#define	NO_TYPE		0	// value doesn't present
#define	INT_TYPE	1	// interger
#define	FLOAT_TYPE	2	// float
#define	STR_TYPE	3	// string
#define	STRUCT_TYPE	4	// structure
#define	ARRAY_TYPE	5	// array
#define	LOOKUP_TYPE	6	// call lookup function
#define	DECODE_TYPE	7	// call encode function
#define	DECODE1_TYPE	8	// call encode1 function
#define	READING_TYPE	9	// call reading print function
#define	BOOL_TYPE	10	// boolean
#define	HEX_TYPE	11	// hexadecimal
#define	TEXT_BUFF_TYPE	12	// TextBuffer

typedef struct {
	char		*name;		// attribute name
	int		type;		// value type
	int		lunum;		// lookup proc number
	union_type_t	value;		// value
} attr_t;

typedef struct {
	int	n_attrs;
	attr_t	*Attrs;
} Attributes_t;

typedef struct {
	SaHpiRdrT	Record;
	SaHpiEntryIdT	RecordId;
	SaHpiRdrTypeT	RdrType;
	Attributes_t	Attrutes;
} Rdr_t;

typedef struct {
	SaHpiRptEntryT		Table;
	SaHpiEntryIdT		EntryId;
	SaHpiResourceIdT	ResourceId;
	Attributes_t		Attrutes;
} Rpt_t;

typedef struct {
	SaHpiSessionIdT		sessionId;
	SaHpiDomainIdT		domainId;
	int			session_opened;
	int			discovered;
} Domain_t;

/* print function 
 * This parameter is passed to the interface show functions
 * It return HPI_UI_OK on successful completion, otherwise HPI_UI_END on
 * output ending.
 */

typedef enum {
	HPI_UI_OK = 0,
	HPI_UI_END = -1
} Pr_ret_t;		// Print return code

typedef Pr_ret_t (*hpi_ui_print_cb_t)(char *buf);
typedef int (*get_int_param_t)(char *buf, int *val);

extern char *hex_codes;

extern SaErrorT	find_rdr_by_num(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiInstrumentIdT num, SaHpiRdrTypeT type, int as,
			SaHpiRdrT *retrdr);
extern void	free_attrs(Attributes_t *At);
extern SaErrorT	get_rpt_attr(Rpt_t *rpt, char *attr_name, union_type_t *val);
			// get rpt attribute value
extern SaErrorT	get_rpt_attr_as_string(Rpt_t *rpt, char *attr_name, char *val, int len);
			// get rpt attribute value as string (max length: len)
extern SaErrorT	get_rdr_attr(Rdr_t *rdr, char *attr_name, union_type_t *val);
			// get rdr attribute value
extern SaErrorT	get_rdr_attr_as_string(Rdr_t *rdr, char *attr_name, char *val, int len);
			// get rdr attribute value as string (max length: len)
extern void	get_text_buffer_text(char *mes, SaHpiTextBufferT *buf, char *meslast,
			char *outbuf);
extern SaErrorT	get_value(Attributes_t *Attrs, int num, union_type_t *val);
			// get attribute value as string by number (max length: len)
extern SaErrorT	get_value_as_string(Attributes_t *Attrs, int num, char *val, int len);
			// get attribute value as string by number (max length: len)
extern char	*get_attr_name(Attributes_t *Attrs, int num);
			// get attribute name
extern int	get_attr_type(Attributes_t *Attrs, int num);
			// get attribute type
extern void	make_attrs_rdr(Rdr_t *Rdr, SaHpiRdrT *rdr_entry);
extern void	make_attrs_rpt(Rpt_t *Rpt, SaHpiRptEntryT *rptentry);
extern Pr_ret_t	print_text_buffer(char *mes, SaHpiTextBufferT *buf, char *meslast,
			hpi_ui_print_cb_t proc);
extern Pr_ret_t	print_text_buffer_lang(char *mes, SaHpiTextBufferT *buf,
			char *meslast, hpi_ui_print_cb_t proc);
extern Pr_ret_t	print_text_buffer_length(char *mes, SaHpiTextBufferT *buf,
			char *meslast, hpi_ui_print_cb_t proc);
extern Pr_ret_t	print_text_buffer_text(char *mes, SaHpiTextBufferT *buf,
			char *meslast, hpi_ui_print_cb_t proc);
extern Pr_ret_t	print_text_buffer_type(char *mes, SaHpiTextBufferT *buf,
			char *meslast, hpi_ui_print_cb_t proc);
extern Pr_ret_t	print_thres_value(SaHpiSensorReadingT *item, char *mes,
			SaHpiSensorThdDefnT *def, int num, hpi_ui_print_cb_t proc);
extern SaErrorT	show_control(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiCtrlNumT num, hpi_ui_print_cb_t proc);
extern SaErrorT	show_control_state(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiCtrlNumT num, hpi_ui_print_cb_t proc, get_int_param_t);
extern SaErrorT	show_dat(Domain_t *domain, hpi_ui_print_cb_t proc);
extern SaErrorT	show_event_log(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			int show_short, hpi_ui_print_cb_t proc);
extern void	show_inv_area_header(SaHpiIdrAreaHeaderT *Header, int del,
			hpi_ui_print_cb_t proc);
extern void	show_inv_field(SaHpiIdrFieldT *Field, int del, hpi_ui_print_cb_t proc);
extern SaErrorT	show_inventory(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiIdrIdT num, SaHpiEntryIdT areaid, hpi_ui_print_cb_t proc);
extern SaErrorT	show_Rdr(Rdr_t *Rdr, hpi_ui_print_cb_t proc);
extern SaErrorT	show_Rpt(Rpt_t *Rpt, hpi_ui_print_cb_t proc);
extern int	show_rdr_list(Domain_t *D, SaHpiResourceIdT resourceid,
			SaHpiRdrTypeT passed_type, hpi_ui_print_cb_t proc);
extern int	show_rpt_list(Domain_t *domain, int as, SaHpiResourceIdT rptid,
			int addedfields, hpi_ui_print_cb_t proc);
extern Pr_ret_t	show_sensor_list(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			hpi_ui_print_cb_t proc);
extern SaErrorT	show_sensor(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiSensorNumT sensornum, hpi_ui_print_cb_t proc);
extern Pr_ret_t	show_short_event(SaHpiEventT *event, hpi_ui_print_cb_t proc);
extern SaErrorT	show_threshold(SaHpiSessionIdT sessionid, SaHpiResourceIdT resourceid,
			SaHpiSensorNumT sensornum, SaHpiSensorRecT *sen,
			hpi_ui_print_cb_t proc);
extern SaErrorT	sensor_list(SaHpiSessionIdT sessionid, hpi_ui_print_cb_t proc);
extern void	time2str(SaHpiTimeT time, char * str, size_t size);

#endif /* __HPI_UI_H */

 
