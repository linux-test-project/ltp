/*******************************************************************************
**
** FILE:
**   SaHpi.h
**
** DESCRIPTION:
**   This file provides the C language binding for the Service
**   Availability(TM) Forum Platform Interface. It contains all of
**   the prototypes and type definitions. Note, this file was
**   generated from the Platform Interface specification document.
**
** SPECIFICATION VERSION:
**   SAI-HPI-B.03.01
**
** DATE:
**   Wed  Oct  08  2008  18:33
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS.
**   The Specification and all worldwide copyrights therein are
**   the exclusive property of Licensor.  You may not remove, obscure, or
**   alter any copyright or other proprietary rights notices that are in or
**   on the copy of the Specification you download.  You must reproduce all
**   such notices on all copies of the Specification you make.  Licensor
**   may make changes to the Specification, or to items referenced therein,
**   at any time without notice.  Licensor is not obligated to support or
**   update the Specification.
**
**   Copyright(c) 2004, 2008, Service Availability(TM) Forum. All rights
**   reserved.
**
**   Permission to use, copy, modify, and distribute this software for any
**   purpose without fee is hereby granted, provided that this entire notice
**   is included in all copies of any software which is or includes a copy
**   or modification of this software and in all copies of the supporting
**   documentation for such software.
**
**   THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
**   WARRANTY.  IN PARTICULAR, THE SERVICE AVAILABILITY FORUM DOES NOT MAKE ANY
**   REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
**   OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
**
*******************************************************************************/

#ifndef __SAHPI_H
#define __SAHPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                 Basic Data Types and Values                **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* General Types - need to be specified correctly for the host architecture */

/*
** It is recommended that these types be defined such that the data sizes
** and alignment of each data type are as indicated. The only requirement
** for source compatibility is that the types be defined to be able to
** contain at least the required data (e.g., at least signed 8-bit values
** must be contained in the data type defined as SaHpiInt8T, etc.)
** Following the full recommendations for data size and alignment, however,
** may promote more binary compatibility.
*/

/* The following definitions produce the recommended sizes and alignments
** using the gcc compiler for the i386 (IA-32) platform.
*/
/* unsigned 8-bit data, 1-byte alignment   */
typedef unsigned char     SaHpiUint8T;

/* unsigned 16-bit data, 2-byte alignment  */
typedef unsigned short    SaHpiUint16T;

/* unsigned 32-bit data, 4-byte alignment  */
typedef unsigned int      SaHpiUint32T;

/* unsigned 64-bit data, 8-byte alignment  */
typedef unsigned long long int  SaHpiUint64T __attribute__((__aligned__(8)));

/* signed 8-bit data, 1-byte alignment     */
typedef signed char       SaHpiInt8T;

/* signed 16-bit data, 2-byte alignment    */
typedef signed short      SaHpiInt16T;

/* signed 32-bit data, 4-byte alignment    */
typedef signed int        SaHpiInt32T;

/* signed 64-bit data, 8-byte alignment    */
typedef signed long long int SaHpiInt64T __attribute__((__aligned__(8)));

/* 64-bit floating point, 8-byte alignment */
typedef double            SaHpiFloat64T __attribute__((__aligned__(8)));

/* enum types are recommended to be 32-bit values, with 4-byte alignment  */


typedef SaHpiUint8T     SaHpiBoolT;
#define SAHPI_TRUE      1        /* While SAHPI_TRUE = 1, any non-zero
                                    value is also considered to be True
                                    and HPI Users/Implementers of this
                                    specification should not test for
                                    equality against SAHPI_TRUE. */

#define SAHPI_FALSE     0

/* Platform, O/S, or Vendor dependent */
#ifndef SAHPI_API
#define SAHPI_API
#endif

#ifndef SAHPI_IN
#define SAHPI_IN
#endif

#ifndef SAHPI_OUT
#define SAHPI_OUT
#endif

#ifndef SAHPI_INOUT
#define SAHPI_INOUT
#endif

/*
** Identifier for the manufacturer
**
** This is the IANA-assigned private enterprise number for the
** manufacturer of the resource or FRU, or of the manufacturer
** defining an OEM Control or event type.  A list of current
** IANA-assigned private enterprise numbers may be obtained at
**
**     http://www.iana.org/assignments/enterprise-numbers
**
** If a manufacturer does not currently have an assigned number, one
** may be obtained by following the instructions located at
**
**     http://www.iana.org/cgi-bin/enterprise.pl
*/
typedef SaHpiUint32T SaHpiManufacturerIdT;
#define SAHPI_MANUFACTURER_ID_UNSPECIFIED (SaHpiManufacturerIdT)0

/* Version Types */
typedef SaHpiUint32T SaHpiVersionT;

/*
** Interface Version
**
** The interface version is the version of the actual interface and not the
** version of the implementation. It is a 24 bit value where
** the most significant 8 bits represent the compatibility level
** (with letters represented as the corresponding numbers);
** the next 8 bits represent the major version number; and
** the least significant 8 bits represent the minor version number.
*/
#define SAHPI_INTERFACE_VERSION (SaHpiVersionT)0x020301 /* B.03.01 */

/*
** Return Codes
**
** SaErrorT is defined in the HPI specification.  In the future a
** common SAF types definition may be created to contain this type.  At
** that time, this typedef should be removed.  Each of the return codes
** is defined in Section 4.1 of the specification.
**
** Future version of the specification may add new return codes.  User
** programs should accept unknown return codes as indicating an unknown
** error.
**
*/
typedef SaHpiInt32T SaErrorT; /* Return code */

/*
** SA_OK:
*/
#define SA_OK                          (SaErrorT)0x0000

/* This value is the base for all HPI-specific error codes. */
#define SA_HPI_ERR_BASE                -1000

#define SA_ERR_HPI_ERROR               (SaErrorT)(SA_HPI_ERR_BASE - 1)
#define SA_ERR_HPI_UNSUPPORTED_API     (SaErrorT)(SA_HPI_ERR_BASE - 2)
#define SA_ERR_HPI_BUSY                (SaErrorT)(SA_HPI_ERR_BASE - 3)
#define SA_ERR_HPI_INTERNAL_ERROR      (SaErrorT)(SA_HPI_ERR_BASE - 4)
#define SA_ERR_HPI_INVALID_CMD         (SaErrorT)(SA_HPI_ERR_BASE - 5)
#define SA_ERR_HPI_TIMEOUT             (SaErrorT)(SA_HPI_ERR_BASE - 6)
#define SA_ERR_HPI_OUT_OF_SPACE        (SaErrorT)(SA_HPI_ERR_BASE - 7)
#define SA_ERR_HPI_OUT_OF_MEMORY       (SaErrorT)(SA_HPI_ERR_BASE - 8)
#define SA_ERR_HPI_INVALID_PARAMS      (SaErrorT)(SA_HPI_ERR_BASE - 9)
#define SA_ERR_HPI_INVALID_DATA        (SaErrorT)(SA_HPI_ERR_BASE - 10)
#define SA_ERR_HPI_NOT_PRESENT         (SaErrorT)(SA_HPI_ERR_BASE - 11)
#define SA_ERR_HPI_NO_RESPONSE         (SaErrorT)(SA_HPI_ERR_BASE - 12)
#define SA_ERR_HPI_DUPLICATE           (SaErrorT)(SA_HPI_ERR_BASE - 13)
#define SA_ERR_HPI_INVALID_SESSION     (SaErrorT)(SA_HPI_ERR_BASE - 14)
#define SA_ERR_HPI_INVALID_DOMAIN      (SaErrorT)(SA_HPI_ERR_BASE - 15)
#define SA_ERR_HPI_INVALID_RESOURCE    (SaErrorT)(SA_HPI_ERR_BASE - 16)
#define SA_ERR_HPI_INVALID_REQUEST     (SaErrorT)(SA_HPI_ERR_BASE - 17)
#define SA_ERR_HPI_ENTITY_NOT_PRESENT  (SaErrorT)(SA_HPI_ERR_BASE - 18)
#define SA_ERR_HPI_READ_ONLY           (SaErrorT)(SA_HPI_ERR_BASE - 19)
#define SA_ERR_HPI_CAPABILITY          (SaErrorT)(SA_HPI_ERR_BASE - 20)
#define SA_ERR_HPI_UNKNOWN             (SaErrorT)(SA_HPI_ERR_BASE - 21)
#define SA_ERR_HPI_INVALID_STATE       (SaErrorT)(SA_HPI_ERR_BASE - 22)
#define SA_ERR_HPI_UNSUPPORTED_PARAMS  (SaErrorT)(SA_HPI_ERR_BASE - 23)



/*
** Domain, Session and Resource Type Definitions
*/

/* Domain ID. */
typedef SaHpiUint32T SaHpiDomainIdT;

/* The SAHPI_UNSPECIFIED_DOMAIN_ID value is used to specify the default
** domain.
*/
#define SAHPI_UNSPECIFIED_DOMAIN_ID (SaHpiDomainIdT) 0xFFFFFFFF

/* Session ID. */
typedef SaHpiUint32T SaHpiSessionIdT;

/* Resource identifier. */
typedef SaHpiUint32T SaHpiResourceIdT;

/* The SAHPI_UNSPECIFIED_RESOURCE_ID value is used to specify the Domain
** Event Log and to specify that there is no resource for such things as HPI
** User events/alarms.
*/
#define SAHPI_UNSPECIFIED_RESOURCE_ID (SaHpiResourceIdT) 0xFFFFFFFF

/* Table Related Type Definitions  */
typedef SaHpiUint32T SaHpiEntryIdT;
#define SAHPI_FIRST_ENTRY (SaHpiEntryIdT)0x00000000
#define SAHPI_LAST_ENTRY  (SaHpiEntryIdT)0xFFFFFFFF
#define SAHPI_ENTRY_UNSPECIFIED SAHPI_FIRST_ENTRY

/*
** Time Related Type Definitions
**
** An HPI time value represents time as either the number of nanoseconds
** since startup (called "relative time") or as the number of
** nanoseconds since 00:00:00, January 1, 1970 (called "absolute time").
** Any time value less than or equal to 0x0C00000000000000 is
** interpreted as "relative time".  Any time value greater than this
** value is interpreted as "absolute time".
**
** When reporting a relative time, the specific meaning of "startup"
** is implementation dependent.  It may mean, for example, system boot,
** startup of the HPI implementation, startup of a particular resource, etc.
**
** With the exception of event log entry timestamps, it is implementation-
** dependent whether absolute or relative time is used for each time value
** passed to an HPI User.  For event log entry timestamps, the default
** representation is implementation-specific.  However, an HPI User can
** change the representation used on subsequent event log entries by calling
** saHpiEventLogTimeSet().
**
** HPI time values can represent relative times in a range of 0 to 27 years
** and absolute times from 1997 through 2262.  Specific HPI implementations
** may not be able to support these entire ranges.  However, all HPI
** implementations must be able to report appropriate time values during the
** life of the system.
**
** For event log timestamps, all HPI implementations must support relative
** times in the range of 0 to the longest time since "startup" that is ever
** expected to be encountered and absolute times representing current time
** throughout the expected life of the system.
**
** It should be noted that although nano-second resolution is supported
** in the data type, the actual resolution provided by an implementation
** may be more limited than this.
**
** The value -2**63, which is 0x8000000000000000, is used to indicate
** "unknown/unspecified time".
**
** Conversion to/from POSIX and other common time representations is
** relatively straightforward. The following code framgment converts
** between SaHpiTimeT and time_t:
**
**     time_t tt1, tt2;
**     SaHpiTimeT saHpiTime;
**
**     time(&tt1);
**     saHpiTime = (SaHpiTimeT) tt1 * 1000000000;
**     tt2 = saHpiTime / 1000000000;
**
** The following fragment converts between SaHpiTimeT and a struct timeval:
**
**     struct timeval tv1, tv2;
**     SaHpiTimeT saHpiTime;
**
**     gettimeofday(&tv1, NULL);
**     saHpiTime = (SaHpiTimeT) tv1.tv_sec * 1000000000 + tv1.tv_usec * 1000;
**     tv2.tv_sec = saHpiTime / 1000000000;
**     tv2.tv_usec = saHpiTime % 1000000000 / 1000;
**
** The following fragment converts between SaHpiTimeT and a struct timespec:
**
**     struct timespec ts1, ts2;
**     SaHpiTimeT saHpiTime;
**
**     clock_gettime(CLOCK_REALTIME, &ts1);
**     saHpiTime = (SaHpiTimeT) ts1.tv_sec * 1000000000 + ts1.tv_nsec;
**     ts2.tv_sec = saHpiTime / 1000000000;
**     ts2.tv_nsec = saHpiTime % 1000000000;
**
** Note, however, that since time_t is (effectively) universally 32 bits,
** all of these conversions will cease to work on January 18, 2038.
** In any of these conversions, if the SaHpiTimeT value is less than or
** equal to 0x0C00000000000000, then the POSIX time structure represents
** time since startup.  If the SaHpiTimeT value is greater than this
** value, then the POSIX time structure represents absolute time of day.
**
*/
typedef SaHpiInt64T SaHpiTimeT;    /* Time in nanoseconds */

/* Unspecified or unknown time */
#define SAHPI_TIME_UNSPECIFIED     (SaHpiTimeT) 0x8000000000000000LL

/* Maximum time that can be specified as relative */
#define SAHPI_TIME_MAX_RELATIVE    (SaHpiTimeT) 0x0C00000000000000LL
typedef SaHpiInt64T SaHpiTimeoutT; /* Timeout in nanoseconds */

/* Non-blocking call */
#define SAHPI_TIMEOUT_IMMEDIATE    (SaHpiTimeoutT) 0x0000000000000000LL

/* Blocking call, wait indefinitely for call to complete */
#define SAHPI_TIMEOUT_BLOCK        (SaHpiTimeoutT) -1LL

/*
** Language
**
** This enumeration lists all of the languages that can be associated with text.
**
** SAHPI_LANG_UNDEF indicates that the language is unspecified or
** unknown.
**
** This enumerated list may grow in future versions of this specification
** as more languages are added. Legacy HPI Users should consider these new
** languages "valid but unknown".
*/
typedef enum {
    SAHPI_LANG_UNDEF = 0, SAHPI_LANG_AFAR, SAHPI_LANG_ABKHAZIAN,
    SAHPI_LANG_AFRIKAANS, SAHPI_LANG_AMHARIC, SAHPI_LANG_ARABIC,
    SAHPI_LANG_ASSAMESE, SAHPI_LANG_AYMARA, SAHPI_LANG_AZERBAIJANI,
    SAHPI_LANG_BASHKIR, SAHPI_LANG_BYELORUSSIAN, SAHPI_LANG_BULGARIAN,
    SAHPI_LANG_BIHARI, SAHPI_LANG_BISLAMA, SAHPI_LANG_BENGALI,
    SAHPI_LANG_TIBETAN, SAHPI_LANG_BRETON, SAHPI_LANG_CATALAN,
    SAHPI_LANG_CORSICAN, SAHPI_LANG_CZECH, SAHPI_LANG_WELSH,
    SAHPI_LANG_DANISH, SAHPI_LANG_GERMAN, SAHPI_LANG_BHUTANI,
    SAHPI_LANG_GREEK, SAHPI_LANG_ENGLISH, SAHPI_LANG_ESPERANTO,
    SAHPI_LANG_SPANISH, SAHPI_LANG_ESTONIAN, SAHPI_LANG_BASQUE,
    SAHPI_LANG_PERSIAN, SAHPI_LANG_FINNISH, SAHPI_LANG_FIJI,
    SAHPI_LANG_FAEROESE, SAHPI_LANG_FRENCH, SAHPI_LANG_FRISIAN,
    SAHPI_LANG_IRISH, SAHPI_LANG_SCOTSGAELIC, SAHPI_LANG_GALICIAN,
    SAHPI_LANG_GUARANI, SAHPI_LANG_GUJARATI, SAHPI_LANG_HAUSA,
    SAHPI_LANG_HINDI, SAHPI_LANG_CROATIAN, SAHPI_LANG_HUNGARIAN,
    SAHPI_LANG_ARMENIAN, SAHPI_LANG_INTERLINGUA, SAHPI_LANG_INTERLINGUE,
    SAHPI_LANG_INUPIAK, SAHPI_LANG_INDONESIAN, SAHPI_LANG_ICELANDIC,
    SAHPI_LANG_ITALIAN, SAHPI_LANG_HEBREW, SAHPI_LANG_JAPANESE,
    SAHPI_LANG_YIDDISH, SAHPI_LANG_JAVANESE, SAHPI_LANG_GEORGIAN,
    SAHPI_LANG_KAZAKH, SAHPI_LANG_GREENLANDIC, SAHPI_LANG_CAMBODIAN,
    SAHPI_LANG_KANNADA, SAHPI_LANG_KOREAN, SAHPI_LANG_KASHMIRI,
    SAHPI_LANG_KURDISH, SAHPI_LANG_KIRGHIZ, SAHPI_LANG_LATIN,
    SAHPI_LANG_LINGALA, SAHPI_LANG_LAOTHIAN, SAHPI_LANG_LITHUANIAN,
    SAHPI_LANG_LATVIANLETTISH, SAHPI_LANG_MALAGASY, SAHPI_LANG_MAORI,
    SAHPI_LANG_MACEDONIAN, SAHPI_LANG_MALAYALAM, SAHPI_LANG_MONGOLIAN,
    SAHPI_LANG_MOLDAVIAN, SAHPI_LANG_MARATHI, SAHPI_LANG_MALAY,
    SAHPI_LANG_MALTESE, SAHPI_LANG_BURMESE, SAHPI_LANG_NAURU,
    SAHPI_LANG_NEPALI, SAHPI_LANG_DUTCH, SAHPI_LANG_NORWEGIAN,
    SAHPI_LANG_OCCITAN, SAHPI_LANG_AFANOROMO, SAHPI_LANG_ORIYA,
    SAHPI_LANG_PUNJABI, SAHPI_LANG_POLISH, SAHPI_LANG_PASHTOPUSHTO,
    SAHPI_LANG_PORTUGUESE, SAHPI_LANG_QUECHUA, SAHPI_LANG_RHAETOROMANCE,
    SAHPI_LANG_KIRUNDI, SAHPI_LANG_ROMANIAN, SAHPI_LANG_RUSSIAN,
    SAHPI_LANG_KINYARWANDA, SAHPI_LANG_SANSKRIT, SAHPI_LANG_SINDHI,
    SAHPI_LANG_SANGRO, SAHPI_LANG_SERBOCROATIAN, SAHPI_LANG_SINGHALESE,
    SAHPI_LANG_SLOVAK, SAHPI_LANG_SLOVENIAN, SAHPI_LANG_SAMOAN,
    SAHPI_LANG_SHONA, SAHPI_LANG_SOMALI, SAHPI_LANG_ALBANIAN,
    SAHPI_LANG_SERBIAN, SAHPI_LANG_SISWATI, SAHPI_LANG_SESOTHO,
    SAHPI_LANG_SUDANESE, SAHPI_LANG_SWEDISH, SAHPI_LANG_SWAHILI,
    SAHPI_LANG_TAMIL, SAHPI_LANG_TELUGU, SAHPI_LANG_TAJIK,
    SAHPI_LANG_THAI, SAHPI_LANG_TIGRINYA, SAHPI_LANG_TURKMEN,
    SAHPI_LANG_TAGALOG, SAHPI_LANG_SETSWANA, SAHPI_LANG_TONGA,
    SAHPI_LANG_TURKISH, SAHPI_LANG_TSONGA, SAHPI_LANG_TATAR,
    SAHPI_LANG_TWI, SAHPI_LANG_UKRAINIAN, SAHPI_LANG_URDU,
    SAHPI_LANG_UZBEK, SAHPI_LANG_VIETNAMESE, SAHPI_LANG_VOLAPUK,
    SAHPI_LANG_WOLOF, SAHPI_LANG_XHOSA, SAHPI_LANG_YORUBA,
    SAHPI_LANG_CHINESE, SAHPI_LANG_ZULU,
    SAHPI_LANG_MAX_VALID = SAHPI_LANG_ZULU
} SaHpiLanguageT;

/*
** Text Buffers
**
** These structures are used for defining the type of data in the text buffer
** and the length of the buffer. Text buffers are used in many places for
** variable length strings of data.
**
** The encoding of the Data field in the SaHpiTextBufferT structure is defined
** by the value of the DataType field in the buffer.  The following table
** describes the various encodings:
**
**    DataType                     Encoding
**    --------                     --------
**
**   SAHPI_TL_TYPE_UNICODE         16-bit Unicode.  See Note 3 below.
**
**   SAHPI_TL_TYPE_BCDPLUS         8-bit ASCII, "0"-"9" or space, dash, period,
**                                 colon, comma, or underscore only.
**                                 See Note 2 below.
**
**   SAHPI_TL_TYPE_ASCII6          8-bit ASCII, reduced set, 0x20=0x5f only.
**                                 See Note 2 below.
**
**   SAHPI_TL_TYPE_TEXT            8-bit ASCII+Latin 1.  See Note 1 below.
**
**   SAHPI_TL_TYPE_BINARY          8-bit bytes, any values legal
**
** Note 1: "ASCII+Latin 1" is derived from the first 256 characters of
**        Unicode 2.0. The first 256 codes of Unicode follow ISO 646 (ASCII)
**        and ISO 8859/1 (Latin 1). The Unicode "C0 Controls and Basic Latin"
**        set defines the first 128 8-bit characters (00h-7Fh) and the
**        "C1 Controls and Latin 1 Supplement" defines the second 128 (80h-FFh).
**
** Note 2: The SAHPI_TL_TYPE_BCDPLUS and SAHPI_TL_TYPE_ASCII6 encodings
**        use normal ASCII character encodings, but restrict the allowed
**        characters to a subset of the entire ASCII character set. These
**        encodings are used when the target device contains restrictions
**        on which characters it can store or display.  SAHPI_TL_TYPE_BCDPLUS
**        data may be stored externally as 4-bit values, and
**        SAHPI_TL_TYPE_ASCII6 may be stored externally as 6-bit values.
**        But, regardless of how the data is stored externally, it is
**        encoded as 8-bit ASCII in the SaHpiTextBufferT structure passed
**        across the HPI.
**
** Note 3: Unicode data is encoded according to the UTF-16LE encoding scheme
**       as defined in the UNICODE 4.0 standard.  This encoding scheme stores
**       21-bit UNICODE code points in a series of 16-bit values stored
**       least-siginficant-byte first in the SaHpiTextBufferT Data field.
**       The SaHpiTextBufferT DataLength field contains the number of bytes,
**       so must always be an even number (maximum 0xFE) when the DataType is
**       SAHPI_TL_TYPE_UNICODE.  In addition to containing an even number of
**       bytes, the text buffer must also contain a well-formed UTF-16LE
**       sequence (meaning no "unmatched surrogates" may be present), and
**       must not include encodings of any UNICODE "Non-Characters" in order
**       to be considered valid.  More details can be found in the Unicode
**       specification available at www.unicode.org.
*/

#define SAHPI_MAX_TEXT_BUFFER_LENGTH  255

typedef enum {
    SAHPI_TL_TYPE_UNICODE = 0,     /* 2-byte UNICODE characters; DataLength
                                     must be even. */
    SAHPI_TL_TYPE_BCDPLUS,        /* String of ASCII characters, "0"-"9", space,
                                     dash, period, colon, comma or underscore
                                     ONLY */
    SAHPI_TL_TYPE_ASCII6,         /* Reduced ASCII character set: 0x20-0x5F
                                     ONLY */
    SAHPI_TL_TYPE_TEXT,           /* ASCII+Latin 1 */
    SAHPI_TL_TYPE_BINARY,         /* Binary data, any values legal */
    SAHPI_TL_TYPE_MAX_VALID = SAHPI_TL_TYPE_BINARY
} SaHpiTextTypeT;

typedef struct {
    SaHpiTextTypeT DataType;
    SaHpiLanguageT Language;      /* Language the text is in. */
    SaHpiUint8T    DataLength;    /* Bytes used in Data buffer  */
    SaHpiUint8T    Data[SAHPI_MAX_TEXT_BUFFER_LENGTH];  /* Data buffer */
} SaHpiTextBufferT;

/*
** Instrument Id
**
** The following data type is used for all management instrument identifiers -
** Sensor numbers, Control numbers, Watchdog Timer numbers, etc.
**
*/

typedef SaHpiUint32T SaHpiInstrumentIdT;


/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Entities                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/*
** Entity Types
**
** An Entity is a physical hardware component of the system.  Entities are
** defined with an entity type enumeration, and an entity location number
** (to identify the physical location of a particular type of entity).
**
** Entities are uniquely identified in a system with an ordered series of
** Entity Type / Entity Location pairs called an "Entity Path".  Each subsequent
** Entity Type/Entity Location in the path is the next higher "containing"
** entity. The "root" of the Entity Path (the outermost level of containment)
** is designated with an Entity Type of SAHPI_ENT_ROOT if the entire Entity Path
** is fewer than SAHPI_MAX_ENTITY_PATH entries in length.
**
** Enumerated Entity Types include those types enumerated by the IPMI Consortium
** for IPMI-managed entities, as well as additional types defined by the
** HPI specification.
** Future versions of this specification may add new Entity Types to this
** enumerated type. Room is left in the enumeration for the inclusion of Entity
** Types taken from other lists, if needed in the future.
** Legacy HPI Users should consider these new entity types "valid but unknown".
*/
/* Base values for entity types from various sources. */
#define SAHPI_ENT_IPMI_GROUP 0
#define SAHPI_ENT_SAFHPI_GROUP 0x10000
#define SAHPI_ENT_ROOT_VALUE 0xFFFF
typedef enum {
    SAHPI_ENT_UNSPECIFIED = SAHPI_ENT_IPMI_GROUP,
    SAHPI_ENT_OTHER,
    SAHPI_ENT_UNKNOWN,
    SAHPI_ENT_PROCESSOR,
    SAHPI_ENT_DISK_BAY,            /* Disk or disk bay  */
    SAHPI_ENT_PERIPHERAL_BAY,
    SAHPI_ENT_SYS_MGMNT_MODULE,    /* System management module  */
    SAHPI_ENT_SYSTEM_BOARD,        /* Main system board, may also be
                                     processor board and/or internal
                                     expansion board */
    SAHPI_ENT_MEMORY_MODULE,       /* Board holding memory devices */
    SAHPI_ENT_PROCESSOR_MODULE,    /* Holds processors, use this
                                     designation when processors are not
                                     mounted on system board */
    SAHPI_ENT_POWER_SUPPLY,        /* Main power supply (supplies) for the
                                     system */
    SAHPI_ENT_ADD_IN_CARD,
    SAHPI_ENT_FRONT_PANEL_BOARD,   /* Control panel  */
    SAHPI_ENT_BACK_PANEL_BOARD,
    SAHPI_ENT_POWER_SYSTEM_BOARD,
    SAHPI_ENT_DRIVE_BACKPLANE,
    SAHPI_ENT_SYS_EXPANSION_BOARD, /* System internal expansion board
                                     (contains expansion slots). */
    SAHPI_ENT_OTHER_SYSTEM_BOARD,  /* Part of board set          */
    SAHPI_ENT_PROCESSOR_BOARD,     /* Holds 1 or more processors. Includes
                                     boards that hold SECC modules */
    SAHPI_ENT_POWER_UNIT,          /* Power unit / power domain (typically
                                     used as a pre-defined logical entity
                                     for grouping power supplies)*/
    SAHPI_ENT_POWER_MODULE,        /* Power module / DC-to-DC converter.
                                     Use this value for internal
                                     converters. Note: You should use
                                     entity ID (power supply) for the
                                     main power supply even if the main
                                     supply is a DC-to-DC converter */
    SAHPI_ENT_POWER_MGMNT,         /* Power management/distribution
                                     board */
    SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD,
    SAHPI_ENT_SYSTEM_CHASSIS,
    SAHPI_ENT_SUB_CHASSIS,
    SAHPI_ENT_OTHER_CHASSIS_BOARD,
    SAHPI_ENT_DISK_DRIVE_BAY,
    SAHPI_ENT_PERIPHERAL_BAY_2,
    SAHPI_ENT_DEVICE_BAY,
    SAHPI_ENT_COOLING_DEVICE,      /* Fan/cooling device */
    SAHPI_ENT_COOLING_UNIT,        /* Can be used as a pre-defined logical
                                     entity for grouping fans or other
                                     cooling devices. */
    SAHPI_ENT_INTERCONNECT,        /* Cable / interconnect */
    SAHPI_ENT_MEMORY_DEVICE,       /* This Entity ID should be used for
                                     replaceable memory devices, e.g.
                                     DIMM/SIMM. It is recommended that
                                     Entity IDs not be used for
                                     individual non-replaceable memory
                                     devices. Rather, monitoring and
                                     error reporting should be associated
                                     with the FRU [e.g. memory card]
                                     holding the memory. */
    SAHPI_ENT_SYS_MGMNT_SOFTWARE, /* System Management Software  */
    SAHPI_ENT_BIOS,
    SAHPI_ENT_OPERATING_SYSTEM,
    SAHPI_ENT_SYSTEM_BUS,
    SAHPI_ENT_GROUP,              /* This is a logical entity for use with
                                     Entity Association records. It is
                                     provided to allow a Sensor data
                                     record to point to an entity-
                                     association record when there is no
                                     appropriate pre-defined logical
                                     entity for the entity grouping.
                                     This Entity should not be used as a
                                     physical entity. */
    SAHPI_ENT_REMOTE,             /* Out of band management communication
                                     device */
    SAHPI_ENT_EXTERNAL_ENVIRONMENT,
    SAHPI_ENT_BATTERY,
    SAHPI_ENT_RESERVED_1,
    SAHPI_ENT_RESERVED_2,
    SAHPI_ENT_RESERVED_3,
    SAHPI_ENT_RESERVED_4,
    SAHPI_ENT_RESERVED_5,
    SAHPI_ENT_MC_FIRMWARE   ,    /* Management Controller Firmware,
                        represents firmware or software
                        running on a management controller */
    SAHPI_ENT_IPMI_CHANNEL,  /* This Entity ID enables associating
                        Sensors with the IPMI communication
                        channels - for example a Redundancy
                        Sensor could be used to report
                        redundancy status for a channel that
                        is composed of multiple physical
                        links.  By convention, the Entity
                        Instance corresponds to the channel
                        number. */

     SAHPI_ENT_PCI_BUS,
     SAHPI_ENT_PCI_EXPRESS_BUS,
     SAHPI_ENT_SCSI_BUS,
     SAHPI_ENT_SATA_BUS,
     SAHPI_ENT_PROC_FSB,         /* Processor, front side bus */
     SAHPI_ENT_CLOCK,        /* e.g. Real Time Clock (RTC) */
     SAHPI_ENT_SYSTEM_FIRMWARE,  /* e.g. BIOS/ EFI */
                     /* The range from
                                    SAHPI_ENT_SYSTEM_FIRMWARE + 1 to
                                    SAHPI_ENT_CHASSIS_SPECIFIC - 1 is
                                    Reserved for future use by this
                                    specification */
    SAHPI_ENT_CHASSIS_SPECIFIC    = SAHPI_ENT_IPMI_GROUP + 0x90,
    SAHPI_ENT_BOARD_SET_SPECIFIC  = SAHPI_ENT_IPMI_GROUP + 0xB0,
    SAHPI_ENT_OEM_SYSINT_SPECIFIC = SAHPI_ENT_IPMI_GROUP + 0xD0,
    SAHPI_ENT_ROOT = SAHPI_ENT_ROOT_VALUE,
    SAHPI_ENT_RACK = SAHPI_ENT_SAFHPI_GROUP,
    SAHPI_ENT_SUBRACK,
    SAHPI_ENT_COMPACTPCI_CHASSIS,
    SAHPI_ENT_ADVANCEDTCA_CHASSIS,
    SAHPI_ENT_RACK_MOUNTED_SERVER,
    SAHPI_ENT_SYSTEM_BLADE,
    SAHPI_ENT_SWITCH,                    /* Network switch, such as a
                                            rack-mounted ethernet or fabric
                                            switch. */
    SAHPI_ENT_SWITCH_BLADE,              /* Network switch, as above, but in
                                            a bladed system. */
    SAHPI_ENT_SBC_BLADE,
    SAHPI_ENT_IO_BLADE,
    SAHPI_ENT_DISK_BLADE,
    SAHPI_ENT_DISK_DRIVE,
    SAHPI_ENT_FAN,
    SAHPI_ENT_POWER_DISTRIBUTION_UNIT,
    SAHPI_ENT_SPEC_PROC_BLADE,           /* Special Processing Blade,
                                            including DSP */
    SAHPI_ENT_IO_SUBBOARD,               /* I/O Sub-Board, including
                                            PMC I/O board */
    SAHPI_ENT_SBC_SUBBOARD,              /* SBC Sub-Board, including PMC
                                            SBC board */
    SAHPI_ENT_ALARM_MANAGER,             /* Chassis alarm manager board */
    SAHPI_ENT_SHELF_MANAGER,             /* Blade-based shelf manager */
    SAHPI_ENT_DISPLAY_PANEL,         /* Display panel, such as an
                                            alarm display panel. */
    SAHPI_ENT_SUBBOARD_CARRIER_BLADE,    /* Includes PMC Carrier Blade --
                                            Use only if "carrier" is only
                                            function of blade.  Else use
                                            primary function (SBC_BLADE,
                                            SPEC_PROC_BLADE, etc.). */
    SAHPI_ENT_PHYSICAL_SLOT,             /* Indicates the physical slot into
                                            which a FRU is inserted. */
    SAHPI_ENT_PICMG_FRONT_BLADE,         /* Any blade conforming to a PICMG
                            Standard.  E.g. AdvancedTCA */

    SAHPI_ENT_SYSTEM_INVENTORY_DEVICE,   /* Inventory storage device for
                                storing system definitions */
    SAHPI_ENT_FILTRATION_UNIT,       /* E.g. a fan filter */
    SAHPI_ENT_AMC,           /* Advanced Mezzannine Card */
                         /* The range from
                            SAHPI_ENT_AMC + 0x01 to
                            SAHPI_ENT_SAFHPI_GROUP + 0x2F is
                            reserved for future use by this
                            specification */
    SAHPI_ENT_BMC = SAHPI_ENT_SAFHPI_GROUP + 0x30, /* Baseboard Management
                            Controller */
    SAHPI_ENT_IPMC,                    /* IPM controller */
    SAHPI_ENT_MMC,                     /* Module Management controller */
    SAHPI_ENT_SHMC,                    /* Shelf Mangement Controller */
    SAHPI_ENT_CPLD,                    /* Complex Programmable Logic Device */
    SAHPI_ENT_EPLD,                    /* Electrically Programmable
                                          Logic Device */
    SAHPI_ENT_FPGA,                    /* Field Prorammable Gate Array */
    SAHPI_ENT_DASD,                    /* Direct Access Storage Device */
    SAHPI_ENT_NIC,                     /* Network Interface Card */
    SAHPI_ENT_DSP,                     /* Digital Signal Processor */
    SAHPI_ENT_UCODE,                   /* Microcode */
    SAHPI_ENT_NPU,                     /* Network Processor */
    SAHPI_ENT_OEM,                     /* Proprietary device */
    SAHPI_ENT_MAX_VALID = SAHPI_ENT_OEM
                        /* The range from
                                           SAHPI_ENT_OEM + 0x01 to
                                           SAHPI_ENT_SAFHPI_GROUP + 0xFF is
                                           reserved for future use by this
                                           specification */
} SaHpiEntityTypeT;

typedef SaHpiUint32T SaHpiEntityLocationT;

typedef struct {
    SaHpiEntityTypeT     EntityType;
    SaHpiEntityLocationT EntityLocation;
} SaHpiEntityT;

#define SAHPI_MAX_ENTITY_PATH 16

typedef struct {
    SaHpiEntityT  Entry[SAHPI_MAX_ENTITY_PATH];
} SaHpiEntityPathT;


/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                     Sensor Event States                    **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/*
** Category
**
** Sensor events contain an event category and event state.  Depending on the
** event category, the event states take on different meanings for events
** generated by specific Sensors.
**
** It is recommended that implementations map their Sensor specific
** event categories into the set of categories listed here.  When such a mapping
** is impractical or impossible, the SAHPI_EC_SENSOR_SPECIFIC category should
** be used.
**
** The SAHPI_EC_GENERIC category can be used for discrete Sensors which have
** state meanings other than those identified with other event categories.
**
** Future versions of this specification may add new event categories to this
** list. Legacy HPI Users should consider these new event categories as being
** equivalent to SAHPI_EC_GENERIC.
*/
typedef SaHpiUint8T SaHpiEventCategoryT;

#define SAHPI_EC_UNSPECIFIED     (SaHpiEventCategoryT)0x00 /* Unspecified */
#define SAHPI_EC_THRESHOLD       (SaHpiEventCategoryT)0x01 /* Threshold
                                                              events */
#define SAHPI_EC_USAGE           (SaHpiEventCategoryT)0x02 /* Usage state
                                                              events */
#define SAHPI_EC_STATE           (SaHpiEventCategoryT)0x03 /* Generic state
                                                              events */
#define SAHPI_EC_PRED_FAIL       (SaHpiEventCategoryT)0x04 /* Predictive fail
                                                              events */
#define SAHPI_EC_LIMIT           (SaHpiEventCategoryT)0x05 /* Limit events */
#define SAHPI_EC_PERFORMANCE     (SaHpiEventCategoryT)0x06 /* Performance
                                                              events    */
#define SAHPI_EC_SEVERITY        (SaHpiEventCategoryT)0x07 /* Severity
                                                              indicating
                                                              events */
#define SAHPI_EC_PRESENCE        (SaHpiEventCategoryT)0x08 /* Device presence
                                                              events */
#define SAHPI_EC_ENABLE          (SaHpiEventCategoryT)0x09 /* Device enabled
                                                              events */
#define SAHPI_EC_AVAILABILITY    (SaHpiEventCategoryT)0x0A /* Availability
                                                              state events */
#define SAHPI_EC_REDUNDANCY      (SaHpiEventCategoryT)0x0B /* Redundancy
                                                              state events */
#define SAHPI_EC_SENSOR_SPECIFIC (SaHpiEventCategoryT)0x7E /* Sensor-
                                                              specific events */
#define SAHPI_EC_GENERIC         (SaHpiEventCategoryT)0x7F /* OEM defined
                                                              events */

/*
** Event States
**
** The following event states are specified relative to the categories listed
** above. The event types are only valid for their given category.  Each set of
** events is labeled as to which category it belongs to.
** Each event has only one event state associated with it. When retrieving
** the event status or event enabled status a bit mask of all applicable event
** states is used. Similarly, when setting the event enabled status a bit mask
** of all applicable event states is used.
*/
typedef SaHpiUint16T SaHpiEventStateT;

/*
** SaHpiEventCategoryT == <any>
*/
#define SAHPI_ES_UNSPECIFIED (SaHpiEventStateT)0x0000

/*
** SaHpiEventCategoryT == SAHPI_EC_THRESHOLD
** When using these event states, the event state should match
** the event severity (for example SAHPI_ES_LOWER_MINOR should have an
** event severity of SAHPI_MINOR).
*/
#define SAHPI_ES_LOWER_MINOR (SaHpiEventStateT)0x0001
#define SAHPI_ES_LOWER_MAJOR (SaHpiEventStateT)0x0002
#define SAHPI_ES_LOWER_CRIT  (SaHpiEventStateT)0x0004
#define SAHPI_ES_UPPER_MINOR (SaHpiEventStateT)0x0008
#define SAHPI_ES_UPPER_MAJOR (SaHpiEventStateT)0x0010
#define SAHPI_ES_UPPER_CRIT  (SaHpiEventStateT)0x0020

/* SaHpiEventCategoryT == SAHPI_EC_USAGE */
#define SAHPI_ES_IDLE   (SaHpiEventStateT)0x0001
#define SAHPI_ES_ACTIVE (SaHpiEventStateT)0x0002
#define SAHPI_ES_BUSY   (SaHpiEventStateT)0x0004

/* SaHpiEventCategoryT == SAHPI_EC_STATE */
#define SAHPI_ES_STATE_DEASSERTED (SaHpiEventStateT)0x0001
#define SAHPI_ES_STATE_ASSERTED   (SaHpiEventStateT)0x0002

/* SaHpiEventCategoryT == SAHPI_EC_PRED_FAIL */
#define SAHPI_ES_PRED_FAILURE_DEASSERT (SaHpiEventStateT)0x0001
#define SAHPI_ES_PRED_FAILURE_ASSERT   (SaHpiEventStateT)0x0002

/* SaHpiEventCategoryT == SAHPI_EC_LIMIT */
#define SAHPI_ES_LIMIT_NOT_EXCEEDED (SaHpiEventStateT)0x0001
#define SAHPI_ES_LIMIT_EXCEEDED     (SaHpiEventStateT)0x0002

/* SaHpiEventCategoryT == SAHPI_EC_PERFORMANCE */
#define SAHPI_ES_PERFORMANCE_MET   (SaHpiEventStateT)0x0001
#define SAHPI_ES_PERFORMANCE_LAGS  (SaHpiEventStateT)0x0002

/*
** SaHpiEventCategoryT == SAHPI_EC_SEVERITY
** When using these event states, the event state should match
** the event severity
*/
#define SAHPI_ES_OK                  (SaHpiEventStateT)0x0001
#define SAHPI_ES_MINOR_FROM_OK       (SaHpiEventStateT)0x0002
#define SAHPI_ES_MAJOR_FROM_LESS     (SaHpiEventStateT)0x0004
#define SAHPI_ES_CRITICAL_FROM_LESS  (SaHpiEventStateT)0x0008
#define SAHPI_ES_MINOR_FROM_MORE     (SaHpiEventStateT)0x0010
#define SAHPI_ES_MAJOR_FROM_CRITICAL (SaHpiEventStateT)0x0020
#define SAHPI_ES_CRITICAL            (SaHpiEventStateT)0x0040
#define SAHPI_ES_MONITOR             (SaHpiEventStateT)0x0080
#define SAHPI_ES_INFORMATIONAL       (SaHpiEventStateT)0x0100

/* SaHpiEventCategoryT == SAHPI_EC_PRESENCE */
#define SAHPI_ES_ABSENT  (SaHpiEventStateT)0x0001
#define SAHPI_ES_PRESENT (SaHpiEventStateT)0x0002

/* SaHpiEventCategoryT == SAHPI_EC_ENABLE */
#define SAHPI_ES_DISABLED (SaHpiEventStateT)0x0001
#define SAHPI_ES_ENABLED  (SaHpiEventStateT)0x0002

/* SaHpiEventCategoryT == SAHPI_EC_AVAILABILITY */
#define SAHPI_ES_RUNNING       (SaHpiEventStateT)0x0001
#define SAHPI_ES_TEST          (SaHpiEventStateT)0x0002
#define SAHPI_ES_POWER_OFF     (SaHpiEventStateT)0x0004
#define SAHPI_ES_ON_LINE       (SaHpiEventStateT)0x0008
#define SAHPI_ES_OFF_LINE      (SaHpiEventStateT)0x0010
#define SAHPI_ES_OFF_DUTY      (SaHpiEventStateT)0x0020
#define SAHPI_ES_DEGRADED      (SaHpiEventStateT)0x0040
#define SAHPI_ES_POWER_SAVE    (SaHpiEventStateT)0x0080
#define SAHPI_ES_INSTALL_ERROR (SaHpiEventStateT)0x0100

/* SaHpiEventCategoryT == SAHPI_EC_REDUNDANCY */
#define SAHPI_ES_FULLY_REDUNDANT                  (SaHpiEventStateT)0x0001
#define SAHPI_ES_REDUNDANCY_LOST                  (SaHpiEventStateT)0x0002
#define SAHPI_ES_REDUNDANCY_DEGRADED              (SaHpiEventStateT)0x0004
#define SAHPI_ES_REDUNDANCY_LOST_SUFFICIENT_RESOURCES \
                                                  (SaHpiEventStateT)0x0008
#define SAHPI_ES_NON_REDUNDANT_SUFFICIENT_RESOURCES \
                                                  (SaHpiEventStateT)0x0010
#define SAHPI_ES_NON_REDUNDANT_INSUFFICIENT_RESOURCES \
                                                  (SaHpiEventStateT)0x0020
#define SAHPI_ES_REDUNDANCY_DEGRADED_FROM_FULL    (SaHpiEventStateT)0x0040
#define SAHPI_ES_REDUNDANCY_DEGRADED_FROM_NON     (SaHpiEventStateT)0x0080

/*
** SaHpiEventCategoryT == SAHPI_EC_GENERIC || SAHPI_EC_SENSOR_SPECIFIC
** These event states are implementation-specific.
*/
#define SAHPI_ES_STATE_00  (SaHpiEventStateT)0x0001
#define SAHPI_ES_STATE_01  (SaHpiEventStateT)0x0002
#define SAHPI_ES_STATE_02  (SaHpiEventStateT)0x0004
#define SAHPI_ES_STATE_03  (SaHpiEventStateT)0x0008
#define SAHPI_ES_STATE_04  (SaHpiEventStateT)0x0010
#define SAHPI_ES_STATE_05  (SaHpiEventStateT)0x0020
#define SAHPI_ES_STATE_06  (SaHpiEventStateT)0x0040
#define SAHPI_ES_STATE_07  (SaHpiEventStateT)0x0080
#define SAHPI_ES_STATE_08  (SaHpiEventStateT)0x0100
#define SAHPI_ES_STATE_09  (SaHpiEventStateT)0x0200
#define SAHPI_ES_STATE_10  (SaHpiEventStateT)0x0400
#define SAHPI_ES_STATE_11  (SaHpiEventStateT)0x0800
#define SAHPI_ES_STATE_12  (SaHpiEventStateT)0x1000
#define SAHPI_ES_STATE_13  (SaHpiEventStateT)0x2000
#define SAHPI_ES_STATE_14  (SaHpiEventStateT)0x4000

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Sensors                          **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Sensor Number */
typedef SaHpiInstrumentIdT SaHpiSensorNumT;
/*
** The following specifies the named range for Sensor numbers reserved
** by the HPI specification.
** HPI Implementations shall not assign Sensor numbers within this
** number range, except for the specific Sensors identified in the
** specification.  For example, currently, three aggregate Sensors are named in
** this range. Other Sensors exposed by the HPI Implementation (e.g., a
** temperature Sensor) must be assigned a number outside of this range.
**/
#define SAHPI_STANDARD_SENSOR_MIN   (SaHpiSensorNumT)0x00000100
#define SAHPI_STANDARD_SENSOR_MAX   (SaHpiSensorNumT)0x000001FF

#define SAHPI_SENSOR_TYPE_SAFHPI_GROUP 0x10000

/* Type of Sensor
** Future versions of this specification may add new Sensor types as required.
** Legacy HPI Users should consider these new types "valid but unknown".
*/
typedef enum {
    SAHPI_TEMPERATURE = 0x01,
    SAHPI_VOLTAGE,
    SAHPI_CURRENT,
    SAHPI_FAN,
    SAHPI_PHYSICAL_SECURITY,
    SAHPI_PLATFORM_VIOLATION,
    SAHPI_PROCESSOR,
    SAHPI_POWER_SUPPLY,
    SAHPI_POWER_UNIT,
    SAHPI_COOLING_DEVICE,
    SAHPI_OTHER_UNITS_BASED_SENSOR,
    SAHPI_MEMORY,
    SAHPI_DRIVE_SLOT,
    SAHPI_POST_MEMORY_RESIZE,
    SAHPI_SYSTEM_FW_PROGRESS,
    SAHPI_EVENT_LOGGING_DISABLED,
    SAHPI_RESERVED1,
    SAHPI_SYSTEM_EVENT,
    SAHPI_CRITICAL_INTERRUPT,
    SAHPI_BUTTON,
    SAHPI_MODULE_BOARD,
    SAHPI_MICROCONTROLLER_COPROCESSOR,
    SAHPI_ADDIN_CARD,
    SAHPI_CHASSIS,
    SAHPI_CHIP_SET,
    SAHPI_OTHER_FRU,
    SAHPI_CABLE_INTERCONNECT,
    SAHPI_TERMINATOR,
    SAHPI_SYSTEM_BOOT_INITIATED,
    SAHPI_BOOT_ERROR,
    SAHPI_OS_BOOT,
    SAHPI_OS_CRITICAL_STOP,
    SAHPI_SLOT_CONNECTOR,
    SAHPI_SYSTEM_ACPI_POWER_STATE,
    SAHPI_RESERVED2,
    SAHPI_PLATFORM_ALERT,
    SAHPI_ENTITY_PRESENCE,
    SAHPI_MONITOR_ASIC_IC,
    SAHPI_LAN,
    SAHPI_MANAGEMENT_SUBSYSTEM_HEALTH,
    SAHPI_BATTERY,
    SAHPI_SESSION_AUDIT,
    SAHPI_VERSION_CHANGE,
    SAHPI_OPERATIONAL = 0xA0,
    SAHPI_OEM_SENSOR=0xC0,
    SAHPI_COMM_CHANNEL_LINK_STATE = SAHPI_SENSOR_TYPE_SAFHPI_GROUP + 0x1,
    SAHPI_MANAGEMENT_BUS_STATE,
    SAHPI_SENSOR_TYPE_MAX_VALID = SAHPI_MANAGEMENT_BUS_STATE
}  SaHpiSensorTypeT;

/*
** Sensor Reading Type
**
** These definitions list the available data types that can be
** used for Sensor readings.
**
*/

#define SAHPI_SENSOR_BUFFER_LENGTH 32

typedef enum {
      SAHPI_SENSOR_READING_TYPE_INT64,
      SAHPI_SENSOR_READING_TYPE_UINT64,
      SAHPI_SENSOR_READING_TYPE_FLOAT64,
      SAHPI_SENSOR_READING_TYPE_BUFFER,   /* 32 byte array. The format of
                                             the buffer is implementation-
                                             specific. Sensors that use
                                             this reading type may not have
                                             thresholds that are settable
                                             or readable. */
      SAHPI_SENSOR_READING_TYPE_MAX_VALID = SAHPI_SENSOR_READING_TYPE_BUFFER
} SaHpiSensorReadingTypeT;

typedef union {
    SaHpiInt64T          SensorInt64;
    SaHpiUint64T         SensorUint64;
    SaHpiFloat64T        SensorFloat64;
    SaHpiUint8T          SensorBuffer[SAHPI_SENSOR_BUFFER_LENGTH];
} SaHpiSensorReadingUnionT;

/*
** Sensor Reading
**
** The Sensor reading data structure is returned from a call to get
** Sensor reading. The structure is also used when setting and getting Sensor
** threshold values and reporting Sensor ranges.
**
** IsSupported is set to True if a Sensor reading/threshold value is available.
** Otherwise, if no reading or threshold is supported, this flag is set to
** False.
**
*/

typedef struct {
      SaHpiBoolT                  IsSupported;
      SaHpiSensorReadingTypeT     Type;
      SaHpiSensorReadingUnionT     Value;
} SaHpiSensorReadingT;


/* Sensor Event Mask Actions - used with saHpiSensorEventMasksSet() */

typedef enum {
    SAHPI_SENS_ADD_EVENTS_TO_MASKS,
    SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS,
    SAHPI_SENS_EVENT_MASK_ACTION_MAX_VALID = SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS
} SaHpiSensorEventMaskActionT;

/* Value to use for AssertEvents or DeassertEvents parameter
   in saHpiSensorEventMasksSet() to set or clear all supported
   event states for a Sensor in the mask */

#define SAHPI_ALL_EVENT_STATES (SaHpiEventStateT)0xFFFF

/*
** Threshold Values
** This structure encompasses all of the thresholds that can be set.
** These are set and read with the same units as Sensors report in
** saHpiSensorReadingGet().  When hysteresis is not constant over the
** range of Sensor values, it is calculated at the nominal Sensor reading,
** as given in the Range field of the Sensor RDR.
**
** Thresholds are required to be set in-order (such that the setting for
** UpCritical is greater than or equal to the setting for UpMajor, etc.).
**
** A high-going threshold asserts an event state when the reading is
** greater-than or equal-to the threshold value.  And a low-going threshold
** asserts an event state when the reading is less-than or equal-to the
** threshold value.
**
** The PosThdHysteresis and NegThdHysteresis factor into when the deassertion
** events occur.  A high-going threshold must have the reading drop to a
** value that is less than PosThdHysteresis below the threshold value in
** order for the deassertion event to occur.  A low-going threshold must have
** the reading rise to a value that is greater than NegThdHysteresis above the
** threshold to become deasserted.
**
** Note that a zero hysteresis value still leads to a difference between where
** the deassertion events occur.  An event cannot be in the asserted and
** deasserted states simultaneously.  Thus, for zero hysteresis a high-going
** threshold event becomes asserted when the reading is greater-than or
** equal-to the threshold, and becomes deasserted when the reading goes
** less-than the threshold.  A low-going threshold event becomes asserted when
** the reading goes less-than or equal-to the threshold, and becomes deasserted
** when the reading goes greater-than the threshold.*/

typedef struct {
    SaHpiSensorReadingT LowCritical;      /* Lower Critical Threshold */
    SaHpiSensorReadingT LowMajor;         /* Lower Major Threshold */
    SaHpiSensorReadingT LowMinor;         /* Lower Minor Threshold */
    SaHpiSensorReadingT UpCritical;       /* Upper critical Threshold */
    SaHpiSensorReadingT UpMajor;          /* Upper major Threshold */
    SaHpiSensorReadingT UpMinor;          /* Upper minor Threshold */
    SaHpiSensorReadingT PosThdHysteresis; /* Positive Threshold Hysteresis */
    SaHpiSensorReadingT NegThdHysteresis; /* Negative Threshold Hysteresis */
} SaHpiSensorThresholdsT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                  Sensor Resource Data Records              **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
**  Sensor Range
** Sensor range values can include minimum, maximum, normal minimum, normal
** maximum, and nominal values.
**
** Sensor thresholds cannot be set outside of the range defined by SAHPI_SRF_MIN
** through SAHPI_SRF_MAX, if these limits are present (as indicated by the
** SaHpiSensorRangeFlagsT).  If the MIN limit is not present, no lower bound
** is enforced on Sensor thresholds.  If the MAX limit is not present, no
** upper bound is enforced on Sensor thresholds.
*/
typedef SaHpiUint8T SaHpiSensorRangeFlagsT;
#define SAHPI_SRF_MIN        (SaHpiSensorRangeFlagsT)0x10
#define SAHPI_SRF_MAX        (SaHpiSensorRangeFlagsT)0x08
#define SAHPI_SRF_NORMAL_MIN (SaHpiSensorRangeFlagsT)0x04
#define SAHPI_SRF_NORMAL_MAX (SaHpiSensorRangeFlagsT)0x02
#define SAHPI_SRF_NOMINAL    (SaHpiSensorRangeFlagsT)0x01

typedef struct {
    SaHpiSensorRangeFlagsT  Flags;
    SaHpiSensorReadingT     Max;
    SaHpiSensorReadingT     Min;
    SaHpiSensorReadingT     Nominal;
    SaHpiSensorReadingT     NormalMax;
    SaHpiSensorReadingT     NormalMin;
} SaHpiSensorRangeT;

/*
** Sensor Units
** This is a list of all the Sensor units supported by HPI.
** Future versions of this specification may add new Sensor units as required.
** Legacy HPI Users should consider these new units "valid but unknown".
*/
typedef enum {
    SAHPI_SU_UNSPECIFIED = 0, SAHPI_SU_DEGREES_C, SAHPI_SU_DEGREES_F,
    SAHPI_SU_DEGREES_K, SAHPI_SU_VOLTS, SAHPI_SU_AMPS,
    SAHPI_SU_WATTS, SAHPI_SU_JOULES, SAHPI_SU_COULOMBS,
    SAHPI_SU_VA, SAHPI_SU_NITS, SAHPI_SU_LUMEN,
    SAHPI_SU_LUX, SAHPI_SU_CANDELA, SAHPI_SU_KPA,
    SAHPI_SU_PSI, SAHPI_SU_NEWTON, SAHPI_SU_CFM,
    SAHPI_SU_RPM, SAHPI_SU_HZ, SAHPI_SU_MICROSECOND,
    SAHPI_SU_MILLISECOND, SAHPI_SU_SECOND, SAHPI_SU_MINUTE,
    SAHPI_SU_HOUR, SAHPI_SU_DAY, SAHPI_SU_WEEK,
    SAHPI_SU_MIL, SAHPI_SU_INCHES, SAHPI_SU_FEET,
    SAHPI_SU_CU_IN, SAHPI_SU_CU_FEET, SAHPI_SU_MM,
    SAHPI_SU_CM, SAHPI_SU_M, SAHPI_SU_CU_CM,
    SAHPI_SU_CU_M, SAHPI_SU_LITERS, SAHPI_SU_FLUID_OUNCE,
    SAHPI_SU_RADIANS, SAHPI_SU_STERADIANS, SAHPI_SU_REVOLUTIONS,
    SAHPI_SU_CYCLES, SAHPI_SU_GRAVITIES, SAHPI_SU_OUNCE,
    SAHPI_SU_POUND, SAHPI_SU_FT_LB, SAHPI_SU_OZ_IN,
    SAHPI_SU_GAUSS, SAHPI_SU_GILBERTS, SAHPI_SU_HENRY,
    SAHPI_SU_MILLIHENRY, SAHPI_SU_FARAD, SAHPI_SU_MICROFARAD,
    SAHPI_SU_OHMS, SAHPI_SU_SIEMENS, SAHPI_SU_MOLE,
    SAHPI_SU_BECQUEREL, SAHPI_SU_PPM, SAHPI_SU_RESERVED,
    SAHPI_SU_DECIBELS, SAHPI_SU_DBA, SAHPI_SU_DBC,
    SAHPI_SU_GRAY, SAHPI_SU_SIEVERT, SAHPI_SU_COLOR_TEMP_DEG_K,
    SAHPI_SU_BIT, SAHPI_SU_KILOBIT, SAHPI_SU_MEGABIT,
    SAHPI_SU_GIGABIT, SAHPI_SU_BYTE, SAHPI_SU_KILOBYTE,
    SAHPI_SU_MEGABYTE, SAHPI_SU_GIGABYTE, SAHPI_SU_WORD,
    SAHPI_SU_DWORD, SAHPI_SU_QWORD, SAHPI_SU_LINE,
    SAHPI_SU_HIT, SAHPI_SU_MISS, SAHPI_SU_RETRY,
    SAHPI_SU_RESET, SAHPI_SU_OVERRUN, SAHPI_SU_UNDERRUN,
    SAHPI_SU_COLLISION, SAHPI_SU_PACKETS, SAHPI_SU_MESSAGES,
    SAHPI_SU_CHARACTERS, SAHPI_SU_ERRORS, SAHPI_SU_CORRECTABLE_ERRORS,
    SAHPI_SU_UNCORRECTABLE_ERRORS,
    SAHPI_SU_MAX_VALID = SAHPI_SU_UNCORRECTABLE_ERRORS
} SaHpiSensorUnitsT;

/*
** Modifier Unit Use
** This type defines how the modifier unit is used.  For example: base unit ==
** meter, modifier unit == seconds, and modifier unit use ==
** SAHPI_SMUU_BASIC_OVER_MODIFIER. The resulting unit would be meters per
** second.
*/
typedef enum {
    SAHPI_SMUU_NONE = 0,
    SAHPI_SMUU_BASIC_OVER_MODIFIER,  /* Basic Unit / Modifier Unit */
    SAHPI_SMUU_BASIC_TIMES_MODIFIER,  /* Basic Unit * Modifier Unit */
    SAHPI_SMUU_MAX_VALID = SAHPI_SMUU_BASIC_TIMES_MODIFIER
} SaHpiSensorModUnitUseT;

/*
** Data Format
** When IsSupported is False, the Sensor does not support data readings
** (it only supports event states).  A False setting for this flag
** indicates that the rest of the structure is not meaningful.
**
** This structure encapsulates all of the various types that make up the
** definition of Sensor data.  For reading type of
** SAHPI_SENSOR_READING_TYPE_BUFFER, the rest of the structure
** (beyond ReadingType) is not meaningful.
**
** The Accuracy Factor is expressed as a floating point percentage
** (e.g. 0.05 = 5%) and represents statistically how close the measured
** reading is to the actual value. It is an interpreted value that
** figures in all Sensor accuracies, resolutions, and tolerances.
*/

typedef struct {
    SaHpiBoolT                 IsSupported;    /* Indicates if Sensor data
                                                   readings are supported.*/
    SaHpiSensorReadingTypeT    ReadingType;    /* Type of value for Sensor
                                                   reading. */
    SaHpiSensorUnitsT          BaseUnits;      /* Base units (meters, etc.)  */
    SaHpiSensorUnitsT          ModifierUnits;  /* Modifier unit (second, etc.)*/
    SaHpiSensorModUnitUseT     ModifierUse;    /* Modifier use(m/sec, etc.)  */
    SaHpiBoolT                 Percentage;     /* Is value a percentage */
    SaHpiSensorRangeT          Range;          /* Valid range of Sensor */
    SaHpiFloat64T              AccuracyFactor; /* Accuracy */
} SaHpiSensorDataFormatT;

/*
** Threshold Support
**
** These types define what threshold values are readable and writable.
** Thresholds are read/written in the same ReadingType as is used for Sensor
** readings.
*/
typedef SaHpiUint8T SaHpiSensorThdMaskT;
#define SAHPI_STM_LOW_MINOR      (SaHpiSensorThdMaskT)0x01
#define SAHPI_STM_LOW_MAJOR      (SaHpiSensorThdMaskT)0x02
#define SAHPI_STM_LOW_CRIT       (SaHpiSensorThdMaskT)0x04
#define SAHPI_STM_UP_MINOR       (SaHpiSensorThdMaskT)0x08
#define SAHPI_STM_UP_MAJOR       (SaHpiSensorThdMaskT)0x10
#define SAHPI_STM_UP_CRIT        (SaHpiSensorThdMaskT)0x20
#define SAHPI_STM_UP_HYSTERESIS  (SaHpiSensorThdMaskT)0x40
#define SAHPI_STM_LOW_HYSTERESIS (SaHpiSensorThdMaskT)0x80

typedef struct {
    SaHpiBoolT            IsAccessible; /* True if the Sensor
                                           supports readable or writable
                                           thresholds. If False,
                                           rest of structure is not
                                           meaningful. Sensors that have the
                                           IsAccessible flag set to True must
                                           also support the threshold event
                                           category.
                                           A Sensor of reading type SAHPI_
                                           SENSOR_READING_TYPE_BUFFER cannot
                                           have accessible thresholds.*/
    SaHpiSensorThdMaskT   ReadThold;    /* Readable thresholds */
    SaHpiSensorThdMaskT   WriteThold;   /* Writable thresholds */
    SaHpiBoolT            Nonlinear;    /* If this flag is set to True,
                                           hysteresis values are calculated
                                           at the nominal Sensor value. */
} SaHpiSensorThdDefnT;

/*
** Event Control
**
** This type defines how Sensor event messages can be controlled (can be turned
** off and on for each type of event, etc.).
*/
typedef enum {
    SAHPI_SEC_PER_EVENT = 0,    /* Event message control per event,
                                   or by entire Sensor; Sensor event enable
                                   status can be changed, and assert/deassert
                                   masks can be changed */
    SAHPI_SEC_READ_ONLY_MASKS,  /* Control for entire Sensor only; Sensor
                                   event enable status can be changed, but
                                   assert/deassert masks cannot be changed */
    SAHPI_SEC_READ_ONLY,        /* Event control not supported; Sensor event
                                   enable status cannot be changed and
                                   assert/deassert masks cannot be changed */
    SAHPI_SEC_MAX_VALID = SAHPI_SEC_READ_ONLY
} SaHpiSensorEventCtrlT;

/*
** Record
**
** This is the Sensor resource data record which describes all of the static
** data associated with a Sensor.
*/
typedef struct {
    SaHpiSensorNumT         Num;           /* Sensor Number/Index */
    SaHpiSensorTypeT        Type;          /* General Sensor Type */
    SaHpiEventCategoryT     Category;      /* Event category */
    SaHpiBoolT              EnableCtrl;    /* True if HPI User can enable
                                              or disable Sensor via
                                              saHpiSensorEnableSet() */
    SaHpiSensorEventCtrlT   EventCtrl;     /* How events can be controlled */
    SaHpiEventStateT        Events;        /* Bit mask of event states
                                              supported */
    SaHpiSensorDataFormatT  DataFormat;    /* Format of the data */
    SaHpiSensorThdDefnT     ThresholdDefn; /* Threshold Definition */
    SaHpiUint32T            Oem;           /* Reserved for OEM use */
} SaHpiSensorRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Aggregate Status                      **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* These are the default Sensor numbers for aggregate status. */
#define SAHPI_DEFAGSENS_OPER (SaHpiSensorNumT)0x00000100
#define SAHPI_DEFAGSENS_PWR  (SaHpiSensorNumT)0x00000101
#define SAHPI_DEFAGSENS_TEMP (SaHpiSensorNumT)0x00000102

/* The following specifies the named range for aggregate status. */
#define SAHPI_DEFAGSENS_MIN   (SaHpiSensorNumT)0x00000100
#define SAHPI_DEFAGSENS_MAX   (SaHpiSensorNumT)0x0000010F

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Controls                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Control Number  */
typedef SaHpiInstrumentIdT SaHpiCtrlNumT;

/*
** Type of Control
**
** This enumerated type defines the different types of generic Controls.
** Future versions of this specification may add new Control types to this
** enumerated type. Legacy HPI Users should consider these new Control types
** "valid but unknown" and should not attempt to interpret the resource data
** records or Control states of such Controls.
*/
typedef enum {
    SAHPI_CTRL_TYPE_DIGITAL = 0x00,
    SAHPI_CTRL_TYPE_DISCRETE,
    SAHPI_CTRL_TYPE_ANALOG,
    SAHPI_CTRL_TYPE_STREAM,
    SAHPI_CTRL_TYPE_TEXT,
    SAHPI_CTRL_TYPE_OEM = 0xC0,
    SAHPI_CTRL_TYPE_MAX_VALID = SAHPI_CTRL_TYPE_OEM
} SaHpiCtrlTypeT;

/*
** Control State Type Definitions
*/

/*
** Digital Control State Definition
**
** Defines the types of digital Control states.
** Any of the four states may be set using saHpiControlSet().
** Only ON or OFF are appropriate returns from saHpiControlGet().
** (PULSE_ON and PULSE_OFF are transitory and end in OFF and ON states,
** respectively.)
** OFF - the Control is off
** ON  - the Control is on
** PULSE_OFF - the Control is briefly turned off, and then turned back on
** PULSE_ON  - the Control is briefly turned on, and then turned back off
**
*/
typedef enum {
    SAHPI_CTRL_STATE_OFF = 0,
    SAHPI_CTRL_STATE_ON,
    SAHPI_CTRL_STATE_PULSE_OFF,
    SAHPI_CTRL_STATE_PULSE_ON,
    SAHPI_CTRL_STATE_MAX_VALID = SAHPI_CTRL_STATE_PULSE_ON
} SaHpiCtrlStateDigitalT;

/*
** Discrete Control State Definition
*/
typedef SaHpiUint32T SaHpiCtrlStateDiscreteT;

/*
** Analog Control State Definition
*/
typedef SaHpiInt32T  SaHpiCtrlStateAnalogT;

/*
** Stream Control State Definition
*/
#define SAHPI_CTRL_MAX_STREAM_LENGTH 4
typedef struct {
    SaHpiBoolT   Repeat;       /* Repeat flag */
    SaHpiUint32T StreamLength; /* Length of the data, in bytes,
                                 stored in the stream. */
    SaHpiUint8T  Stream[SAHPI_CTRL_MAX_STREAM_LENGTH];
} SaHpiCtrlStateStreamT;

/*
** Text Control State Definition
*/
typedef SaHpiUint8T SaHpiTxtLineNumT;

/* Reserved number for sending output to all lines */
#define SAHPI_TLN_ALL_LINES (SaHpiTxtLineNumT)0x00

typedef struct {
    SaHpiTxtLineNumT    Line; /* Operate on line # */
    SaHpiTextBufferT    Text; /* Text to display */
} SaHpiCtrlStateTextT;

/*
** OEM Control State Definition
*/
#define SAHPI_CTRL_MAX_OEM_BODY_LENGTH 255
typedef struct {
    SaHpiManufacturerIdT MId;
    SaHpiUint8T BodyLength;
    SaHpiUint8T Body[SAHPI_CTRL_MAX_OEM_BODY_LENGTH]; /* OEM Specific */
} SaHpiCtrlStateOemT;

typedef union {
    SaHpiCtrlStateDigitalT  Digital;
    SaHpiCtrlStateDiscreteT Discrete;
    SaHpiCtrlStateAnalogT   Analog;
    SaHpiCtrlStateStreamT   Stream;
    SaHpiCtrlStateTextT     Text;
    SaHpiCtrlStateOemT      Oem;
} SaHpiCtrlStateUnionT;

typedef struct {
    SaHpiCtrlTypeT          Type;       /* Type of Control */
    SaHpiCtrlStateUnionT    StateUnion; /* Data for Control type */
} SaHpiCtrlStateT;
/*
** Control Mode Type Definition
**
** Controls may be in either AUTO mode or MANUAL mode.
**
*/
typedef enum {
    SAHPI_CTRL_MODE_AUTO,
    SAHPI_CTRL_MODE_MANUAL,
    SAHPI_CTRL_MODE_MAX_VALID = SAHPI_CTRL_MODE_MANUAL
} SaHpiCtrlModeT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                 Control Resource Data Records              **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Output Type
**
** This enumeration defines the Control's output.
** Future versions of this specification may add new Control output types to
** this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_CTRL_GENERIC = 0,
    SAHPI_CTRL_LED,
    SAHPI_CTRL_FAN_SPEED,
    SAHPI_CTRL_DRY_CONTACT_CLOSURE,
    SAHPI_CTRL_POWER_SUPPLY_INHIBIT,
    SAHPI_CTRL_AUDIBLE,
    SAHPI_CTRL_FRONT_PANEL_LOCKOUT,
    SAHPI_CTRL_POWER_INTERLOCK,
    SAHPI_CTRL_POWER_STATE,
    SAHPI_CTRL_LCD_DISPLAY,
    SAHPI_CTRL_OEM,
    SAHPI_CTRL_OUTPUT_TYPE_MAX_VALID = SAHPI_CTRL_OEM
} SaHpiCtrlOutputTypeT;

/*
** Specific Record Types
** These types represent the specific types of Control resource data records.
*/
typedef struct {
    SaHpiCtrlStateDigitalT Default;
} SaHpiCtrlRecDigitalT;

typedef struct {
    SaHpiCtrlStateDiscreteT Default;
} SaHpiCtrlRecDiscreteT;

typedef struct {
    SaHpiCtrlStateAnalogT  Min;    /* Minimum Value */
    SaHpiCtrlStateAnalogT  Max;    /* Maximum Value */
    SaHpiCtrlStateAnalogT  Default;
} SaHpiCtrlRecAnalogT;

typedef struct {
   SaHpiCtrlStateStreamT  Default;
} SaHpiCtrlRecStreamT;

typedef struct {
    SaHpiUint8T             MaxChars; /* Maximum chars per line.
                                         If the Control DataType is
                                         SAHPI_TL_TYPE_UNICODE, there are
                                         two bytes required for each
                                         character.  This field reports the
                                         number of characters per line- not the
                                         number of bytes.  MaxChars must not be
                                         larger than the number of characters
                                         that can be placed in a single
                                         SaHpiTextBufferT structure. */
    SaHpiUint8T             MaxLines; /* Maximum # of lines */
    SaHpiLanguageT          Language; /* Language Code */
    SaHpiTextTypeT          DataType; /* Permitted Data */
    SaHpiCtrlStateTextT     Default;
} SaHpiCtrlRecTextT;

#define SAHPI_CTRL_OEM_CONFIG_LENGTH 10
typedef struct {
    SaHpiManufacturerIdT    MId;
    SaHpiUint8T             ConfigData[SAHPI_CTRL_OEM_CONFIG_LENGTH];
    SaHpiCtrlStateOemT      Default;
} SaHpiCtrlRecOemT;

typedef union {
    SaHpiCtrlRecDigitalT  Digital;
    SaHpiCtrlRecDiscreteT Discrete;
    SaHpiCtrlRecAnalogT   Analog;
    SaHpiCtrlRecStreamT   Stream;
    SaHpiCtrlRecTextT     Text;
    SaHpiCtrlRecOemT      Oem;
} SaHpiCtrlRecUnionT;

/*
** Default Control Mode Structure
** This structure tells an HPI User if the Control comes up in Auto mode or
** in Manual mode, by default.  It also indicates if the mode can be
** changed (using saHpiControlSet()).  When ReadOnly is False, the mode
** can be changed from its default setting; otherwise attempting to
** change the mode results in an error.
*/
typedef struct {
    SaHpiCtrlModeT          Mode; /* Auto or Manual */
    SaHpiBoolT              ReadOnly; /* Indicates if mode is read-only */
} SaHpiCtrlDefaultModeT;

/*
** Record Definition
** Definition of the Control resource data record.
*/
typedef struct {
    SaHpiCtrlNumT         Num;         /* Control Number/Index */
    SaHpiCtrlOutputTypeT  OutputType;
    SaHpiCtrlTypeT        Type;        /* Type of Control */
    SaHpiCtrlRecUnionT    TypeUnion;   /* Specific Control record */
    SaHpiCtrlDefaultModeT DefaultMode; /*Indicates if the Control comes up
                                         in Auto or Manual mode. */
    SaHpiBoolT            WriteOnly;   /* Indicates if the Control is
                                          write-only. */
    SaHpiUint32T          Oem;         /* Reserved for OEM use */
} SaHpiCtrlRecT;


/*******************************************************************************
********************************************************************************
**********                                                            **********
**********               Inventory Data Repositories                  **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/*
** These structures are used to read and write inventory data to entity
** Inventory Data Repositories within a resource.
*/
/*
** Inventory Data Repository ID
** Identifier for an Inventory Data Repository.
*/
typedef SaHpiInstrumentIdT SaHpiIdrIdT;
#define SAHPI_DEFAULT_INVENTORY_ID (SaHpiIdrIdT)0x00000000

/* Inventory Data Area type definitions
** Future versions of this specification may add new area types to
** this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_IDR_AREATYPE_INTERNAL_USE = 0xB0,
    SAHPI_IDR_AREATYPE_CHASSIS_INFO,
    SAHPI_IDR_AREATYPE_BOARD_INFO,
    SAHPI_IDR_AREATYPE_PRODUCT_INFO,
    SAHPI_IDR_AREATYPE_OEM = 0xC0,
    SAHPI_IDR_AREATYPE_UNSPECIFIED = 0xFF,
    SAHPI_IDR_AREATYPE_MAX_VALID = SAHPI_IDR_AREATYPE_UNSPECIFIED
} SaHpiIdrAreaTypeT;

/* Inventory Data Field type definitions
** Future versions of this specification may add new field types to
** this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_IDR_FIELDTYPE_CHASSIS_TYPE,
    SAHPI_IDR_FIELDTYPE_MFG_DATETIME,
    SAHPI_IDR_FIELDTYPE_MANUFACTURER,
    SAHPI_IDR_FIELDTYPE_PRODUCT_NAME,
    SAHPI_IDR_FIELDTYPE_PRODUCT_VERSION,
    SAHPI_IDR_FIELDTYPE_SERIAL_NUMBER,
    SAHPI_IDR_FIELDTYPE_PART_NUMBER,
    SAHPI_IDR_FIELDTYPE_FILE_ID,
    SAHPI_IDR_FIELDTYPE_ASSET_TAG,
    SAHPI_IDR_FIELDTYPE_CUSTOM,
    SAHPI_IDR_FIELDTYPE_UNSPECIFIED = 0xFF,
    SAHPI_IDR_FIELDTYPE_MAX_VALID = SAHPI_IDR_FIELDTYPE_UNSPECIFIED
} SaHpiIdrFieldTypeT;

/* Inventory Data Field structure definition */
typedef struct {
    SaHpiEntryIdT        AreaId;    /* AreaId for the IDA to which */
                                    /* the Field belongs */
    SaHpiEntryIdT        FieldId;   /* Field Identifier */
    SaHpiIdrFieldTypeT   Type;      /* Field Type */
    SaHpiBoolT           ReadOnly;  /* Describes if a field is read-only. */
    SaHpiTextBufferT     Field;     /* Field Data */
} SaHpiIdrFieldT;

/* Inventory Data Area header structure definition */
typedef struct {
    SaHpiEntryIdT      AreaId;      /* Area Identifier */
    SaHpiIdrAreaTypeT  Type;        /* Type of area */
    SaHpiBoolT         ReadOnly;    /* Describes if an area is read-only. */
    SaHpiUint32T       NumFields;   /* Number of Fields contained in Area */
} SaHpiIdrAreaHeaderT;

/* Inventory Data Repository Information structure definition */
typedef struct {
    SaHpiIdrIdT        IdrId;       /* Repository Identifier */
    SaHpiUint32T       UpdateCount; /* The count is incremented any time the */
                                    /* IDR is changed.  It rolls over to zero */
                                    /* when the maximum value is reached */
    SaHpiBoolT         ReadOnly;    /* Describes if the IDR is read-only. */
    SaHpiUint32T       NumAreas;    /* Number of Areas contained in IDR */
} SaHpiIdrInfoT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********     Inventory Data Repository Resource Data Records        **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** All inventory data contained in an Inventory Data Repository
** must be represented in the RDR repository
** with an SaHpiInventoryRecT.
*/
typedef struct {
    SaHpiIdrIdT   IdrId;
    SaHpiBoolT    Persistent;  /* True indicates that updates to IDR are
                                  automatically and immediately persisted.
                                  False indicates that updates are not
                                  immediately persisted; but optionally may be
                                  persisted via saHpiParmControl() function, as
                                  defined in implementation documentation.*/
    SaHpiUint32T  Oem;
} SaHpiInventoryRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                          Watchdogs                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** This section defines all of the data types associated with Watchdog Timers.
*/

/* Watchdog Number - Identifier for a Watchdog Timer. */
typedef SaHpiInstrumentIdT SaHpiWatchdogNumT;
#define SAHPI_DEFAULT_WATCHDOG_NUM (SaHpiWatchdogNumT)0x00000000

/*
** Watchdog Timer Action
**
** These enumerations represent the possible actions to be taken upon Watchdog
** Timer timeout and the events that are generated for Watchdog actions.
*/
typedef enum {
    SAHPI_WA_NO_ACTION = 0,
    SAHPI_WA_RESET,
    SAHPI_WA_POWER_DOWN,
    SAHPI_WA_POWER_CYCLE,
    SAHPI_WA_MAX_VALID = SAHPI_WA_POWER_CYCLE
} SaHpiWatchdogActionT;

typedef enum {
    SAHPI_WAE_NO_ACTION = 0,
    SAHPI_WAE_RESET,
    SAHPI_WAE_POWER_DOWN,
    SAHPI_WAE_POWER_CYCLE,
    SAHPI_WAE_TIMER_INT=0x08,   /* Used if Timer Preinterrupt only */
    SAHPI_WAE_MAX_VALID = SAHPI_WAE_TIMER_INT
} SaHpiWatchdogActionEventT;

/*
** Watchdog Pre-timer Interrupt
**
** These enumerations represent the possible types of interrupts that may be
** triggered by a Watchdog pre-timer event. The actual meaning of these
** operations may differ depending on the hardware architecture.
*/
typedef enum {
    SAHPI_WPI_NONE = 0,
    SAHPI_WPI_SMI,
    SAHPI_WPI_NMI,
    SAHPI_WPI_MESSAGE_INTERRUPT,
    SAHPI_WPI_OEM = 0x0F,
    SAHPI_WPI_MAX_VALID = SAHPI_WPI_OEM
} SaHpiWatchdogPretimerInterruptT;

/*
** Watchdog Timer Use
**
** These enumerations represent the possible Watchdog users that may have caused
** the Watchdog to expire.  For instance, if a Watchdog is used during power
** on self test (POST), and it expires, the SAHPI_WTU_BIOS_POST expiration type
** is set. Most specific uses for Watchdog timer by users of HPI should
** indicate SAHPI_WTU_SMS_OS if the use is to provide an OS-healthy heartbeat,
** or SAHPI_WTU_OEM if it is used for some other purpose.
*/
typedef enum {
    SAHPI_WTU_NONE = 0,
    SAHPI_WTU_BIOS_FRB2,
    SAHPI_WTU_BIOS_POST,
    SAHPI_WTU_OS_LOAD,
    SAHPI_WTU_SMS_OS,            /* System Management System providing
                                   heartbeat for OS */
    SAHPI_WTU_OEM,
    SAHPI_WTU_UNSPECIFIED = 0x0F,
    SAHPI_WTU_MAX_VALID = SAHPI_WTU_UNSPECIFIED
} SaHpiWatchdogTimerUseT;

/*
** Timer Use Expiration Flags
** These values are used for the Watchdog Timer Use Expiration flags in the
** SaHpiWatchdogT structure.
*/
typedef SaHpiUint8T SaHpiWatchdogExpFlagsT;
#define SAHPI_WATCHDOG_EXP_BIOS_FRB2   (SaHpiWatchdogExpFlagsT)0x02
#define SAHPI_WATCHDOG_EXP_BIOS_POST   (SaHpiWatchdogExpFlagsT)0x04
#define SAHPI_WATCHDOG_EXP_OS_LOAD     (SaHpiWatchdogExpFlagsT)0x08
#define SAHPI_WATCHDOG_EXP_SMS_OS      (SaHpiWatchdogExpFlagsT)0x10
#define SAHPI_WATCHDOG_EXP_OEM         (SaHpiWatchdogExpFlagsT)0x20

/*
** Watchdog Structure
**
** This structure is used by the saHpiWatchdogTimerGet() and
** saHpiWatchdogTimerSet() functions. The use of the structure varies slightly
** by each function.
**
** For saHpiWatchdogTimerGet() :
**
**   Log -                indicates whether the Watchdog is configured to
**                        issue events. True=events are generated.
**   Running -            indicates whether the Watchdog is currently
**                        running or stopped. True=Watchdog is running.
**   TimerUse -           indicates the current use of the timer; one of the
**                        enumerated preset uses which was included on the last
**                        saHpiWatchdogTimerSet() function call, or through some
**                        other implementation-dependent means to start the
**                        Watchdog timer.
**   TimerAction -        indicates what action will be taken when the Watchdog
**                        times out.
**   PretimerInterrupt -  indicates which action will be taken
**                        "PreTimeoutInterval" milliseconds prior to Watchdog
**                        timer expiration.
**   PreTimeoutInterval - indicates how many milliseconds prior to timer time
**                        out the PretimerInterrupt action will be taken. If
**                        "PreTimeoutInterval" = 0, the PretimerInterrupt action
**                        occurs concurrently with "TimerAction". HPI
**                        implementations may not be able to support millisecond
**                        resolution, and because of this may have rounded the
**                        set value to whatever resolution could be supported.
**                        The HPI implementation returns this rounded value.
**   TimerUseExpFlags -   set of five bit flags which indicate that a Watchdog
**                        timer timeout has occurred while the corresponding
**                        TimerUse value was set. Once set, these flags stay
**                        set until specifically cleared with a
**                        saHpiWatchdogTimerSet() call, or by some other
**                        implementation-dependent means.
**   InitialCount -       The time, in milliseconds, before the timer will time
**                        out after the Watchdog is started/restarted, or some
**                        other implementation-dependent strobe is
**                        sent to the Watchdog. HPI implementations may not be
**                        able to support millisecond resolution, and because
**                        of this may have rounded the set value to whatever
**                        resolution could be supported.  The HPI implementation
**                        returns this rounded value.
**   PresentCount -       The remaining time in milliseconds before the timer
**                        times out unless an saHpiWatchdogTimerReset()
**                        function call is made, or some other implementation-
**                        dependent strobe is sent to the Watchdog.
**                        HPI implementations may not be able to support
**                        millisecond resolution on Watchdog Timers, but
**                        returns the number of clock ticks remaining times the
**                        number of milliseconds between each tick.
**
** For saHpiWatchdogTimerSet():
**
**   Log -                indicates whether the Watchdog should issue
**                        events. True=events are generated.
**   Running -            indicates whether the Watchdog should be
**                        stopped before updating.
**                        True =  Watchdog is not stopped. If it is
**                                already stopped, it remains stopped,
**                                but if it is running, it continues
**                                to run, with the countown timer reset
**                                to the new InitialCount.  Note that
**                                there is a race condition possible
**                                with this setting, so it should be used
**                                with care.
**                        False = Watchdog is stopped.  After
**                                saHpiWatchdogTimerSet() is called, a
**                                subsequent call to
**                                saHpiWatchdogTimerReset() is required
**                                to start the timer.
**   TimerUse -           indicates the current use of the timer.  Controls
**                        which TimerUseExpFlag will be set if the timer
**                        expires.
**   TimerAction -        indicates what action will be taken when the Watchdog
**                        times out.
**   PretimerInterrupt -  indicates which action will be taken
**                        "PreTimeoutInterval" milliseconds prior to Watchdog
**                        timer expiration.
**   PreTimeoutInterval - indicates how many milliseconds prior to timer time
**                        out the PretimerInterrupt action will be taken. If
**                        "PreTimeoutInterval" = 0, the PretimerInterrupt action
**                        occurs concurrently with "TimerAction". HPI
**                        implementations may not be able to support millisecond
**                        resolution and may have a maximum value restriction.
**                        These restrictions should be documented by the
**                        provider of the HPI interface.
**   TimerUseExpFlags -   Set of five bit flags corresponding to the five
**                        TimerUse values.  For each bit set to "1", the
**                        corresponding Timer Use Expiration Flag is cleared,
**                        that is, set to "0". Generally, a program should only
**                        clear the Timer Use Expiration Flag corresponding to
**                        its own TimerUse, so that other software, which may
**                        have used the timer for another purpose in the past
**                        can still read its TimerUseExpFlag to determine
**                        whether the timer expired during that use.
**   InitialCount -       The time, in milliseconds, before the timer will time
**                        out after an saHpiWatchdogTimerReset() function call is
**                        made, or some other implementation-dependent strobe is
**                        sent to the Watchdog. HPI implementations may not be
**                        able to support millisecond resolution and may have a
**                        maximum value restriction. These restrictions should
**                        be documented by the provider of the HPI interface.
**   PresentCount -       Not used on saHpiWatchdogTimerSet() function. Ignored.
**
*/

typedef struct {
    SaHpiBoolT                        Log;
    SaHpiBoolT                        Running;
    SaHpiWatchdogTimerUseT            TimerUse;
    SaHpiWatchdogActionT              TimerAction;
    SaHpiWatchdogPretimerInterruptT   PretimerInterrupt;
    SaHpiUint32T                      PreTimeoutInterval;
    SaHpiWatchdogExpFlagsT            TimerUseExpFlags;
    SaHpiUint32T                      InitialCount;
    SaHpiUint32T                      PresentCount;
} SaHpiWatchdogT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                  Watchdog Resource Data Records            **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** When the "Watchdog" capability is set in a resource, a Watchdog with an
** identifier of SAHPI_DEFAULT_WATCHDOG_NUM is required.  All Watchdogs must be
** represented in the RDR repository with an SaHpiWatchdogRecT, including the
** Watchdog with an identifier of SAHPI_DEFAULT_WATCHDOG_NUM.
*/
typedef struct {
    SaHpiWatchdogNumT  WatchdogNum;
    SaHpiUint32T       Oem;
} SaHpiWatchdogRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                              DIMI                          **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/*
** This section defines all of the data types associated with a DIMI
*/

/* DIMI Number - numeric identifier for a DIMI */
typedef SaHpiInstrumentIdT SaHpiDimiNumT;

/*
** Test Service Impact Levels
*/

typedef enum {
    SAHPI_DIMITEST_NONDEGRADING,
    SAHPI_DIMITEST_DEGRADING,
    SAHPI_DIMITEST_VENDOR_DEFINED_LEVEL, /* VENDOR_DEFINED_LEVEL can be used
                                          to indicate severity of degrading
                                          test */
    SAHPI_DIMITEST_SERVICE_IMPACT_MAX_VALID = SAHPI_DIMITEST_VENDOR_DEFINED_LEVEL
} SaHpiDimiTestServiceImpactT;


/* A test may affect the entity corresponding to DIMI it runs on. The same test
** may also affect other entities as well. This stuct defines the entity path &
** the service impact (as defined by SaHpiDimiTestServiceImpactT) on that
** affected entity.
*/

typedef struct {
    SaHpiEntityPathT        EntityImpacted; /* Entity path of impacted
                                                    entity */
    SaHpiDimiTestServiceImpactT     ServiceImpact;   /* Service Impact on affected
                                                    entity */
} SaHpiDimiTestAffectedEntityT;

typedef enum {
    SAHPI_DIMITEST_STATUS_NOT_RUN,   /* Only returned if test has never
                            been executed on the DIMI */
    SAHPI_DIMITEST_STATUS_FINISHED_NO_ERRORS, /* Test is not running. Last run
                            finished without any errors */
    SAHPI_DIMITEST_STATUS_FINISHED_ERRORS, /* Test is not running.  But the last
                             run finished with error */
    SAHPI_DIMITEST_STATUS_CANCELED,        /* This is returned when a test has
                                             canceled using the
                                             saHpiDimiTestCancel function */
    SAHPI_DIMITEST_STATUS_RUNNING,         /* This is returned when a test is
                                             in progress */
    SAHPI_DIMITEST_STATUS_MAX_VALID = SAHPI_DIMITEST_STATUS_RUNNING
} SaHpiDimiTestRunStatusT;


/*  Error codes that can be generated by a test
** Future versions of this specification may add new error codes to
** this enumerated type. Legacy HPI Users should consider these new values
** undefined errors.  */

typedef enum {
    SAHPI_DIMITEST_STATUSERR_NOERR = 0, /* No Error was generated */
    SAHPI_DIMITEST_STATUSERR_RUNERR,    /* Run time error was generated */
    SAHPI_DIMITEST_STATUSERR_UNDEF, /* Undefined Error*/
    SAHPI_DIMITEST_STATUSERR_MAX_VALID = SAHPI_DIMITEST_STATUSERR_UNDEF
} SaHpiDimiTestErrCodeT;


/*  Test results from last run of test */

typedef struct {
SaHpiTimeT      ResultTimeStamp; /* TimeStamp when the results are
                             generated. When test ends,
                             ResultTimeStamp captures the time
                             test ended */
SaHpiTimeoutT       RunDuration;      /* Implementation provides the
                         duration from the start of last run
                             until the time results were
                             generated */
SaHpiDimiTestRunStatusT LastRunStatus;
SaHpiDimiTestErrCodeT   TestErrorCode;
SaHpiTextBufferT    TestResultString; /* String contains either in line
                                              Test result or URI to the file name
                                              containing results from last run */
SaHpiBoolT      TestResultStringIsURI;  /* True = URI to file name,
                                                   False = in-line test result,
                                                   If True, the DataType of the
                                                   TestResultString text buffer
                                                   must be SAHPI_TL_TYPE_TEXT. */
} SaHpiDimiTestResultsT;


/* SaHpiDimiTestParamsDefinitionT struct defines test parameters. The test
** parameter definition consists of Parameter Name, human readable text for
** parameters information, parameters type and value (min, max, default).
** HPI User can use APIs to obtain the parameters definition along with test
** information.  Based on test parameters definition a proper parameter can
** be defined and passed together with the test invocation.
*/

/* Possible types of parameters for a test. */

typedef enum {
    SAHPI_DIMITEST_PARAM_TYPE_BOOLEAN,  /* HPI type SaHpiBoolT */
    SAHPI_DIMITEST_PARAM_TYPE_INT32,    /* HPI type SaHpiUint32T */
    SAHPI_DIMITEST_PARAM_TYPE_FLOAT64,  /* HPI type SaHpiFloat64T */
    SAHPI_DIMITEST_PARAM_TYPE_TEXT, /* HPI type SaHpiTextBufferT */
    SAHPI_DIMITEST_PARAM_TYPE_MAX_VALID = SAHPI_DIMITEST_PARAM_TYPE_TEXT
} SaHpiDimiTestParamTypeT;


/* This union is defining the values for the test parameter*/

typedef union {
    SaHpiInt32T     paramint;
    SaHpiBoolT      parambool;
    SaHpiFloat64T       paramfloat;
    SaHpiTextBufferT    paramtext;  /* Must be DataType = SAHPI_TL_TYPE_TEXT */
} SaHpiDimiTestParamValueT;


/* maximum number of possible parameters for any DIMI test */
#define SAHPI_DIMITEST_MAX_PARAMETERS   10

/* Test parameter name length */
#define SAHPI_DIMITEST_PARAM_NAME_LEN   20

typedef union {
   SaHpiInt32T      IntValue;
   SaHpiFloat64T         FloatValue;
} SaHpiDimiTestParameterValueUnionT;


typedef struct {
  SaHpiUint8T ParamName[SAHPI_DIMITEST_PARAM_NAME_LEN]; /* Name of the
                           parameter, case sensitive */
  SaHpiTextBufferT      ParamInfo;  /* This is a human readable text */
  SaHpiDimiTestParamTypeT       ParamType;
  SaHpiDimiTestParameterValueUnionT MinValue;   /* Only valid for integer and float
                               parameters */
  SaHpiDimiTestParameterValueUnionT MaxValue;   /* Only valid for integer and float
                               parameters */
  SaHpiDimiTestParamValueT DefaultParam;     /* Default value */

} SaHpiDimiTestParamsDefinitionT;

/* The following defines the standard capabilities available for DIMI tests.
** Each test in a DIMI may support any number of capabilities using the bit
** mask. If a test supports one of these capabilities then the corresponding
** bit is set in the bit mask.
** There is a close relationship between capabilities and parameters of a
** test.  A test may support some standard parameters if specific capability
** bits are set. Some capabilities might not have test parameters.
**
** Future versions of the HPI specification may define additional DIMI test
** capabilities and associate them with currently undefined bit positions in
** the SaHpiDimiTestCapabilitiesT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the TestCapabilities field of the SaHpiDimiTestT structure.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
** SAHPI_DIMITEST_CAPABILITY_NO_CAPABILITY
** Test does not support any of the standard capabilities
**
** SAHPI_DIMITEST_CAPABILITY_LOOPCOUNT
** Test supports looping for a specified count of times. When this capability
** is set for a test the standard parameter SAHPI_DIMITEST_LOOP_COUNT_PARAM
** is also supported by the test. Loop count parameter can be configured
** loop the test n number of times. Minimum and default is 1 time so that
** test is executed at least one time.
**
** SAHPI_DIMITEST_CAPABILITY_LOOPTIME
** Test supports looping for a specified amount of time (in seconds). When
** this capability is set for a test the standard parameter
** SAHPI_DIMITEST_LOOP_TIME_PARAM is also supported by the test.
**
** SAHPI_DIMITEST_CAPABILITY_SERVICEMODE
** Test supports a basic as well as extended mode. When this capability is
** set for a test the standard parameter SAHPI_DIMITEST_SERVICE_MODE_PARAM
** is also supported by the test.
**
** SAHPI_DIMITEST_CAPABILITY_LOGGING
** Test supports logging capability. When capability is set for a test the
** standard parameter SAHPI_DIMITEST_LOGGING_PARAM is also supported
** by the test.
**
** SAHPI_DIMITEST_CAPABILITY_RESULTSOUTPUT
** Test supports FINALONLY, ONDEMAND, ASYNC results output capablilities.
** When capability is set for a test the standard parameter
** SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM is also supported by the test.
**
** SAHPI_DIMITEST_CAPABILITY_TESTCANCEL
** Test supports TESTCANCEL capability. This capability has no corresponding
** test parameter.
*/

typedef SaHpiUint32T SaHpiDimiTestCapabilityT;

#define SAHPI_DIMITEST_CAPABILITY_NO_CAPABILITY \
                        (SaHpiDimiTestCapabilityT)0x00000000
#define SAHPI_DIMITEST_CAPABILITY_RESULTSOUTPUT \
                        (SaHpiDimiTestCapabilityT)0x00000001
#define SAHPI_DIMITEST_CAPABILITY_SERVICEMODE   \
                        (SaHpiDimiTestCapabilityT)0x00000002
#define SAHPI_DIMITEST_CAPABILITY_LOOPCOUNT \
                        (SaHpiDimiTestCapabilityT)0x00000004
#define SAHPI_DIMITEST_CAPABILITY_LOOPTIME  \
                        (SaHpiDimiTestCapabilityT)0x00000008
#define SAHPI_DIMITEST_CAPABILITY_LOGGING   \
                        (SaHpiDimiTestCapabilityT)0x00000010
#define SAHPI_DIMITEST_CAPABILITY_TESTCANCEL    \
                        (SaHpiDimiTestCapabilityT)0x00000020

/* The following are the standard test parameters available for use with DIMI
** tests. These parameters are applicable only if the corresponding test
** capability is supported by a DIMI test. HPI User can check the capabilities
** through the bit-stream defined through SaHpiDimiTestCapabilityT. If a test
** supports certain capability, corresponding test parameter is defined in
** standard format. The parameters are defined here as macros.  For tests
** supporting these parameters they are returned as type
** SaHpiDimiTestParamsDefinitionT with the SaHpiDimiTestT structure.
*/

#ifndef SAHPI_DIMITEST_LOOP_COUNT_PARAM
#define SAHPI_DIMITEST_LOOP_COUNT_PARAM_NAME    "Loop Count"
#define SAHPI_DIMITEST_LOOP_COUNT_PARAM                 \
                {                       \
                    SAHPI_DIMITEST_LOOP_COUNT_PARAM_NAME,   \
                    {                       \
                    SAHPI_TL_TYPE_TEXT,         \
                    SAHPI_LANG_ENGLISH,         \
                    15,                 \
                    "Test Loop Count"           \
                    },                      \
                    SAHPI_DIMITEST_PARAM_TYPE_INT32,        \
                    1,                      \
                    0xFFFFFFFF,             \
                    { 1 }                   \
                }
#endif // SAHPI_DIMITEST_LOOP_COUNT_PARAM

#ifndef SAHPI_DIMITEST_LOOP_TIME_PARAM
#define SAHPI_DIMITEST_LOOP_TIME_PARAM_NAME "Loop Time"
#define SAHPI_DIMITEST_LOOP_TIME_PARAM                  \
                {                       \
                    SAHPI_DIMITEST_LOOP_TIME_PARAM_NAME,    \
                           {                        \
                                SAHPI_TL_TYPE_TEXT,         \
                                SAHPI_LANG_ENGLISH,             \
                                14,                 \
                                "Test Loop Time"            \
                               },                       \
                              SAHPI_DIMITEST_PARAM_TYPE_INT32,      \
                              0,                        \
                              0xFFFFFFFF,               \
                              { 1 }                 \
                          }
#endif // SAHPI_DIMITEST_LOOP_TIME_PARAM

#ifndef SAHPI_DIMITEST_SERVICE_MODE_PARAM
#define SAHPI_DIMITEST_SERVICE_MODE_PARAM_NAME  "Service Mode"
#define SAHPI_DIMITEST_SERVICE_MODE_PARAM  {                \
                {                       \
                              SAHPI_DIMITEST_SERVICE_MODE_PARAM_NAME,   \
                              {                     \
                                SAHPI_TL_TYPE_TEXT,         \
                                SAHPI_LANG_ENGLISH,             \
                                14,                 \
                                "Operating Mode"            \
                              },                        \
                              SAHPI_DIMITEST_PARAM_TYPE_INT32,      \
                              0, /* basic mode */           \
                              1, /* extended mode */            \
                              { 0 } /* default is basic mode */     \
                         }
#endif // SAHPI_DIMITEST_SERVICE_MODE_PARAM

#ifndef SAHPI_DIMITEST_LOGGING_PARAM
#define SAHPI_DIMITEST_LOGGING_PARAM_NAME   "Logging"
#define SAHPI_DIMITEST_LOGGING_PARAM                    \
                         {                      \
                               SAHPI_DIMITEST_LOGGING_PARAM_NAME,   \
                              {                     \
                            SAHPI_TL_TYPE_TEXT,         \
                            SAHPI_LANG_ENGLISH,             \
                            18,                 \
                                 "Logging Capability"           \
                              },                        \
                              SAHPI_DIMITEST_PARAM_TYPE_INT32,      \
                              0,    /* No Logging */            \
                              5,    /* Verbose Logging*/            \
                              { 0 }                 \
                         }
#endif  // SAHPI_DIMITEST_LOGGING_PARAM

/* ResultsOutputParam:  Standard parameter describing the capability of a
** DIMI to output the results.
** DIMI can generate results in FINALONLY, ONDEMAND and ASYNC enumerated
** types.
*/

#ifndef SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM

#define SAHPI_DIMITEST_CAPAB_RES_FINALONLY 0
#define SAHPI_DIMITEST_CAPAB_RES_ONDEMAND  1
#define SAHPI_DIMITEST_CAPAB_RES_ASYNC     2


#define SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM_NAME    "Results Output"
#define SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM   {             \
                {                       \
                              SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM_NAME,\
                              {                     \
                                SAHPI_TL_TYPE_TEXT,             \
                                 SAHPI_LANG_ENGLISH,            \
                                 25,                    \
                                 "Results Output Capability"        \
                              },                        \
                              SAHPI_DIMITEST_PARAM_TYPE_INT32,      \
                              0,                    \
                              2,                    \
                              { 0 }                     \
                         }
#endif // SAHPI_DIMITEST_RESULTS_OUTPUT_PARAM


/*
**  SaHpiDimiTestT
*/

/* DIMI test number */
typedef SaHpiUint32T SaHpiDimiTestNumT;

/* Maximum number entities that can be affected by a test */
#define SAHPI_DIMITEST_MAX_ENTITIESIMPACTED 5

typedef struct {
    SaHpiTextBufferT        TestName;
    SaHpiDimiTestServiceImpactT ServiceImpact; /* Service Impact
                               on DIMI itself */
    SaHpiDimiTestAffectedEntityT EntitiesImpacted[SAHPI_DIMITEST_MAX_ENTITIESIMPACTED];
            /* Entities affected by the Test. If entity contains
               other entities then contained entities are considered
               affected as well. If Entities affected by Test are more
               than Max number, its recommended to associate test with
               higher level entity DIMI. */
    SaHpiBoolT          NeedServiceOS;  /* True if a special Service
                                                   OS is needed for this test */
    SaHpiTextBufferT        ServiceOS;  /* If a special Service OS is
                               needed to be run for the test,
                               the HPI User is required to
                               load the particular Service
                               OS before running the tests,
                               else DIMI returns an error
                               code.*/
    SaHpiTimeT          ExpectedRunDuration; /* Expected run duration
                                  for a test with default
                                  parameters */
    SaHpiDimiTestCapabilityT    TestCapabilities;
    SaHpiDimiTestParamsDefinitionT TestParameters[SAHPI_DIMITEST_MAX_PARAMETERS];
} SaHpiDimiTestT;

/* This struct defines the format of parameter which is passed
** to the test on invocation
*/
typedef struct {
    SaHpiUint8T ParamName[SAHPI_DIMITEST_PARAM_NAME_LEN];
                        /* Must exactly match the one returned
                           by the ParamsDefinition */
    SaHpiDimiTestParamTypeT ParamType; /* Must exactly match the one
                          returned by the ParamsDefinition */
    SaHpiDimiTestParamValueT Value;
} SaHpiDimiTestVariableParamsT;

/* Percentage of test completed.  Based on DIMI test capability,
   Value range is 0 - 100,
   0xFF returned if capability not available.
*/
typedef SaHpiUint8T SaHpiDimiTestPercentCompletedT;

typedef enum {
    SAHPI_DIMI_READY,       /* DIMI is in ready state to run a particular test */
    SAHPI_DIMI_WRONG_STATE, /* DIMI is in the wrong state to run a
                              particular test.  For example, need to load a
                              correct ServiceOS before this test can run */
   SAHPI_DIMI_BUSY,        /* DIMI cannot start a particular test at this
                              time.  User can try again later.  */
   SAHPI_DIMI_READY_MAX_VALID = SAHPI_DIMI_BUSY
} SaHpiDimiReadyT;

typedef struct {
 SaHpiUint32T   NumberOfTests; /* It is recommended that the DIMI
                                           advertise all available tests
                                           regardless of ServiceImpact or
                           Service OS */
SaHpiUint32T        TestNumUpdateCounter;   /* If number of tests change for
                       DIMI this counter is incremented and
                           Event is generated*/

} SaHpiDimiInfoT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                       DIMI Resource Data Records           **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** All DIMIs must be represented in the
** RDR repository with an SaHpiDimiRecT
*/
typedef struct {
    SaHpiDimiNumT       DimiNum;
    SaHpiUint32T        Oem;    /* Reserved for OEM use */
} SaHpiDimiRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           FUMI                             **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* FUMI Number - identifier for a FUMI  */
typedef SaHpiInstrumentIdT SaHpiFumiNumT;

/* Bank number on FUMI */
typedef SaHpiUint8T SaHpiBankNumT;

/*
** Spec framework underlying a FUMI implementation
**
** This information can be presented in two ways:
**
** 1) If SAF recognizes underlying spec framework:
**    a well-known assigned Spec Identifier and Spec
**    revision (SaHpiFumiSafDefinedSpecInfoT).
**
** 2) Otherwise, an OEM defined identifier (SaHpiFumiOemDefinedSpecInfoT)
*/

/*
** Underlying spec framework information type
*/
typedef enum {
    SAHPI_FUMI_SPEC_INFO_NONE,
    SAHPI_FUMI_SPEC_INFO_SAF_DEFINED,
    SAHPI_FUMI_SPEC_INFO_OEM_DEFINED,
    SAHPI_FUMI_SPEC_INFO_MAX_VALID = SAHPI_FUMI_SPEC_INFO_OEM_DEFINED
} SaHpiFumiSpecInfoTypeT;

/*
** SAF-recognized underlying spec framework information
*/
typedef enum {
    SAHPI_FUMI_SPEC_HPM1 = 0,
    /* future revisions of HPI specification will define more constants */
    SAHPI_FUMI_SPEC_MAX_VALID = SAHPI_FUMI_SPEC_HPM1
} SaHpiFumiSafDefinedSpecIdT;

typedef struct {
    SaHpiFumiSafDefinedSpecIdT SpecID;
    SaHpiUint32T    RevisionID;
} SaHpiFumiSafDefinedSpecInfoT;

/*
** OEM-defined underlying spec framework information
*/
#define SAHPI_FUMI_MAX_OEM_BODY_LENGTH 255
typedef struct {
    SaHpiManufacturerIdT    Mid;
    SaHpiUint8T BodyLength;
    SaHpiUint8T Body[SAHPI_FUMI_MAX_OEM_BODY_LENGTH];
} SaHpiFumiOemDefinedSpecInfoT;

/*
** Underlying spec framework information
*/
typedef union {
    SaHpiFumiSafDefinedSpecInfoT    SafDefined;
    SaHpiFumiOemDefinedSpecInfoT    OemDefined;
} SaHpiFumiSpecInfoTypeUnionT;

typedef struct {
    SaHpiFumiSpecInfoTypeT SpecInfoType;
    SaHpiFumiSpecInfoTypeUnionT SpecInfoTypeUnion;
} SaHpiFumiSpecInfoT;


/*
** Firmware instance info.
** InstancePresent indicates whether the firmware instance is present or not.
** Identifier contains file name.
** Description contains the description string for the firmware instance.
** DateTime contains the firmware build date-time
** Version further identifies the firmware instance.
** Text buffers may have zero length if data is not available.
** Version field values may be 0 if version data is not available.
*/
typedef  struct {
    SaHpiBoolT      InstancePresent;
    SaHpiTextBufferT    Identifier;
    SaHpiTextBufferT    Description;
    SaHpiTextBufferT    DateTime;
    SaHpiUint32T        MajorVersion;
    SaHpiUint32T        MinorVersion;
    SaHpiUint32T        AuxVersion;
} SaHpiFumiFirmwareInstanceInfoT;


/*
** FUMI Service Impact Levels
*/
typedef enum {
    SAHPI_FUMI_PROCESS_NONDEGRADING,
    SAHPI_FUMI_PROCESS_DEGRADING,
    SAHPI_FUMI_PROCESS_VENDOR_DEFINED_IMPACT_LEVEL,
    SAHPI_FUMI_PROCESS_IMPACT_MAX_VALID = SAHPI_FUMI_PROCESS_VENDOR_DEFINED_IMPACT_LEVEL
} SaHpiFumiServiceImpactT;

/*
** An upgrade process on FUMI may affect the entity corresponding to FUMI
** resource. It also may affect other entities as well.
** This struct defines the entity path and the service impact for an affected
** entity.
*/
typedef struct {
    SaHpiEntityPathT        ImpactedEntity;
    SaHpiFumiServiceImpactT ServiceImpact;
} SaHpiFumiImpactedEntityT;

/*
** Maximum number of entities that can be affected by an upgrade process
** on a FUMI
*/
#define SAHPI_FUMI_MAX_ENTITIES_IMPACTED 5

/*
** This structure lists the entities affected by an upgrade process on a FUMI.
** If an entity is in the list, all its contained entities are implicitly
** considered affected as well (and with the same impact description).
** If both container and contained entity are in list, impact description for
** contained entity is explicitly taken from the corresponding list element.
*/
typedef struct {
    SaHpiUint32T NumEntities;
    SaHpiFumiImpactedEntityT ImpactedEntities[SAHPI_FUMI_MAX_ENTITIES_IMPACTED];
} SaHpiFumiServiceImpactDataT;


/*
** FUMI Source Validation Status codes
*/
typedef enum {
    SAHPI_FUMI_SRC_VALID = 0,
    SAHPI_FUMI_SRC_PROTOCOL_NOT_SUPPORTED,  /* The source URI specifies a
                                               protocol that is not supported
                                               by the FUMI */
    SAHPI_FUMI_SRC_UNREACHABLE,  /* Destination designated by source URI is
                                    not reachable, or does not respond */
    SAHPI_FUMI_SRC_VALIDATION_NOT_STARTED, /* Validation process has not
                                              been started, e.g., no call
                                              to saHpiFumiSourceInfoValidate(). */
    SAHPI_FUMI_SRC_VALIDATION_INITIATED, /* Validation process is in process */
    SAHPI_FUMI_SRC_VALIDATION_FAIL, /* Validation process failed, unable to
                                       determine if source image is valid */
    SAHPI_FUMI_SRC_TYPE_MISMATCH, /* If the source image does not
                                     match the target */
    SAHPI_FUMI_SRC_INVALID,       /* If the source image fails validation such
                                     as failed checksum, corrupted etc. */
    SAHPI_FUMI_SRC_VALIDITY_UNKNOWN, /* If the source image validity cannot be
                                        determined */
    SAHPI_FUMI_SRC_STATUS_MAX_VALID = SAHPI_FUMI_SRC_VALIDITY_UNKNOWN
} SaHpiFumiSourceStatusT;

/*
** FUMI Bank States
*/
typedef enum {
    SAHPI_FUMI_BANK_VALID = 0x00,
    SAHPI_FUMI_BANK_UPGRADE_IN_PROGRESS,
    SAHPI_FUMI_BANK_CORRUPTED,
    SAHPI_FUMI_BANK_ACTIVE,     /* Bank has become active bank */
    SAHPI_FUMI_BANK_BUSY,        /* FUMI operations targeting bank
                                    are not currently possible */
    SAHPI_FUMI_BANK_UNKNOWN,
    SAHPI_FUMI_BANK_STATE_MAX_VALID = SAHPI_FUMI_BANK_UNKNOWN
} SaHpiFumiBankStateT;


/*
** FUMI Upgrade Status
*/
typedef enum {
    SAHPI_FUMI_OPERATION_NOTSTARTED = 0x00, /* Only returned in response to
                                               saHpiFumiUpgradeStatusGet() */
    SAHPI_FUMI_SOURCE_VALIDATION_INITIATED,
    SAHPI_FUMI_SOURCE_VALIDATION_FAILED,
    SAHPI_FUMI_SOURCE_VALIDATION_DONE,
    SAHPI_FUMI_SOURCE_VALIDATION_CANCELLED,
    SAHPI_FUMI_INSTALL_INITIATED,
    SAHPI_FUMI_INSTALL_FAILED_ROLLBACK_NEEDED,
    SAHPI_FUMI_INSTALL_FAILED_ROLLBACK_INITIATED,
    SAHPI_FUMI_INSTALL_FAILED_ROLLBACK_NOT_POSSIBLE,
    SAHPI_FUMI_INSTALL_DONE,
    SAHPI_FUMI_INSTALL_CANCELLED, /* User cancelled operation */
    SAHPI_FUMI_ROLLBACK_INITIATED,
    SAHPI_FUMI_ROLLBACK_FAILED,
    SAHPI_FUMI_ROLLBACK_DONE,
    SAHPI_FUMI_ROLLBACK_CANCELLED,
    SAHPI_FUMI_BACKUP_INITIATED,
    SAHPI_FUMI_BACKUP_FAILED,
    SAHPI_FUMI_BACKUP_DONE,
    SAHPI_FUMI_BACKUP_CANCELLED,
    SAHPI_FUMI_BANK_COPY_INITIATED,
    SAHPI_FUMI_BANK_COPY_FAILED,
    SAHPI_FUMI_BANK_COPY_DONE,
    SAHPI_FUMI_BANK_COPY_CANCELLED,
    SAHPI_FUMI_TARGET_VERIFY_INITIATED,
    SAHPI_FUMI_TARGET_VERIFY_FAILED,
    SAHPI_FUMI_TARGET_VERIFY_DONE,
    SAHPI_FUMI_TARGET_VERIFY_CANCELLED,
    SAHPI_FUMI_ACTIVATE_INITIATED,
    SAHPI_FUMI_ACTIVATE_FAILED_ROLLBACK_NEEDED,
    SAHPI_FUMI_ACTIVATE_FAILED_ROLLBACK_INITIATED,
    SAHPI_FUMI_ACTIVATE_FAILED_ROLLBACK_NOT_POSSIBLE,
    SAHPI_FUMI_ACTIVATE_DONE,
    SAHPI_FUMI_ACTIVATE_CANCELLED,
    SAHPI_FUMI_UPGRADE_STATUS_MAX_VALID = SAHPI_FUMI_ACTIVATE_CANCELLED
} SaHpiFumiUpgradeStatusT;


/*
** FUMI Source Information -
** Source name contains the file in the identifier section, manufacturer and
** build date-time in description section and version number to correctly
** identify the firmware.  Text buffers may have zero length if data is not
** available.  Version values may be 0 if version data is not available.
*/
typedef struct {
    SaHpiTextBufferT      SourceUri;       /* Value set for the source information
                                              with saHpiFumiSourceSet(). */
    SaHpiFumiSourceStatusT   SourceStatus; /* Result of validation */
    SaHpiTextBufferT      Identifier;      /* File name */
    SaHpiTextBufferT      Description;     /* Description */
    SaHpiTextBufferT      DateTime;        /* Build Date-Time */
    SaHpiUint32T          MajorVersion;    /* Major Version */
    SaHpiUint32T          MinorVersion;    /* Minor Version */
    SaHpiUint32T          AuxVersion;      /* Auxiliary Version */

} SaHpiFumiSourceInfoT;


/*
** FUMI subsidiary component information
*/
typedef struct {
    SaHpiEntryIdT   EntryId;
    SaHpiUint32T    ComponentId;
    SaHpiFumiFirmwareInstanceInfoT MainFwInstance;
    SaHpiUint32T ComponentFlags; /* Reserved for future use */
} SaHpiFumiComponentInfoT;


/*
** FUMI Bank Information -
**  Text buffers may have zero length if data is not
** available.  Version values may be 0 if version data is not available.
*/
typedef struct {
    SaHpiUint8T             BankId;         /* Identifier for bank */
    SaHpiUint32T            BankSize;       /* Bank Size in KBytes */
    SaHpiUint32T            Position;       /* Bank Position in boot order.
                                    If only one bank, will always
                                    be set to "1" */
    SaHpiFumiBankStateT     BankState;       /* Validity state of bank */
    SaHpiTextBufferT        Identifier;      /* File name */
    SaHpiTextBufferT        Description;     /* Description */
    SaHpiTextBufferT        DateTime;        /* Build Date-Time */
    SaHpiUint32T            MajorVersion;    /* Major Version */
    SaHpiUint32T            MinorVersion;    /* Minor Version */
    SaHpiUint32T            AuxVersion;      /* Auxiliary Version */
} SaHpiFumiBankInfoT;

/*
** FUMI Logical Bank State Flags
** Currently only one state flag is defined. It indicates that the currently executing main
** firmware instance does not have a persistent copy.
** Additional state flags may be defined in future versions of this specification.
*/
typedef SaHpiUint32T SaHpiFumiLogicalBankStateFlagsT;
#define SAHPI_FUMI_NO_MAIN_PERSISTENT_COPY  (SaHpiFumiLogicalBankStatesT)0x00000001


/*
** FUMI Logical Bank Information
*/
typedef struct
{
    /* The max number of persistent firmware instances that this */
    /* FUMI logical bank can have at the same time */
    SaHpiUint8T FirmwarePersistentLocationCount;
    SaHpiFumiLogicalBankStateFlagsT BankStateFlags;
    SaHpiFumiFirmwareInstanceInfoT PendingFwInstance;
    SaHpiFumiFirmwareInstanceInfoT RollbackFwInstance;
} SaHpiFumiLogicalBankInfoT;


/*
** Logical FUMI Bank Component Information
*/
typedef struct {
    SaHpiEntryIdT   EntryId;
    SaHpiUint32T    ComponentId;
    SaHpiFumiFirmwareInstanceInfoT PendingFwInstance;
    SaHpiFumiFirmwareInstanceInfoT RollbackFwInstance;
    SaHpiUint32T ComponentFlags; /* Reserved for future use */
} SaHpiFumiLogicalComponentInfoT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                 FUMI Resource Data Records                 **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Supported Access Protocols are stored as a bit map for all the different
** access mechanisms that the FUMI implementation supports for accessing the
** firmware binary from the source repository.
**
** Future versions of the HPI specification may define additional access
** protocols and associate them with currently undefined bit positions in
** the SaHpiFumiProtocolT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the AccessProt field of the FUMI Resource Data Record.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**

*/

typedef SaHpiUint32T SaHpiFumiProtocolT;

#define SAHPI_FUMI_PROT_TFTP    (SaHpiFumiProtocolT)0x00000001
#define SAHPI_FUMI_PROT_FTP     (SaHpiFumiProtocolT)0x00000002
#define SAHPI_FUMI_PROT_HTTP    (SaHpiFumiProtocolT)0x00000004
#define SAHPI_FUMI_PROT_LDAP    (SaHpiFumiProtocolT)0x00000008
#define SAHPI_FUMI_PROT_LOCAL   (SaHpiFumiProtocolT)0x00000010 /* Local Copy */
#define SAHPI_FUMI_PROT_NFS     (SaHpiFumiProtocolT)0x00000020
#define SAHPI_FUMI_PROT_DBACCESS (SaHpiFumiProtocolT)0x00000040



/*
** Capability flag of FUMI is a bitmap of supported capabilities
**
** Future versions of the HPI specification may define additional FUMI
** capabilities and associate them with currently undefined bit positions in
** the SaHpiFumiCapabilityT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the Capability field of the FUMI Resource Data Record.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
*/

typedef SaHpiUint32T SaHpiFumiCapabilityT;

#define SAHPI_FUMI_CAP_ROLLBACK (SaHpiFumiCapabilityT)0x00000001
#define SAHPI_FUMI_CAP_BANKCOPY (SaHpiFumiCapabilityT)0x00000002
#define SAHPI_FUMI_CAP_BANKREORDER (SaHpiFumiCapabilityT)0x00000004
#define SAHPI_FUMI_CAP_BACKUP (SaHpiFumiCapabilityT)0x00000008
#define SAHPI_FUMI_CAP_TARGET_VERIFY (SaHpiFumiCapabilityT)0x00000010
#define SAHPI_FUMI_CAP_TARGET_VERIFY_MAIN (SaHpiFumiCapabilityT)0x00000020
#define SAHPI_FUMI_CAP_COMPONENTS (SaHpiFumiCapabilityT)0x00000040
#define SAHPI_FUMI_CAP_AUTOROLLBACK (SaHpiFumiCapabilityT)0x00000080
#define SAHPI_FUMI_CAP_AUTOROLLBACK_CAN_BE_DISABLED \
    (SaHpiFumiCapabilityT)0x00000100
#define SAHPI_FUMI_CAP_MAIN_NOT_PERSISTENT \
    (SaHpiFumiCapabilityT)0x00000200

/*
** Record Definition
** Definition of the FUMI resource data record.
*/
typedef struct {
    SaHpiFumiNumT         Num;         /* FUMI Number/Index */
    SaHpiFumiProtocolT    AccessProt;  /* Supported protocols for
                          repository access */
    SaHpiFumiCapabilityT  Capability;  /* Optional capbilities supported
                          by FUMI */
    SaHpiUint8T           NumBanks;    /* Number of explicit banks supported.
                          Set to "0" if no explicit banks
                                          are supported */
    SaHpiUint32T          Oem;         /* Reserved for OEM use */

} SaHpiFumiRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Hot Swap                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Hot Swap Indicator State */
typedef enum {
    SAHPI_HS_INDICATOR_OFF = 0,
    SAHPI_HS_INDICATOR_ON,
    SAHPI_HS_INDICATOR_STATE_MAX_VALID = SAHPI_HS_INDICATOR_ON

} SaHpiHsIndicatorStateT;

/* Hot Swap Action  */
typedef enum {
    SAHPI_HS_ACTION_INSERTION = 0,
    SAHPI_HS_ACTION_EXTRACTION,
    SAHPI_HS_ACTION_MAX_VALID = SAHPI_HS_ACTION_EXTRACTION
} SaHpiHsActionT;

/* Hot Swap State */
typedef enum {
    SAHPI_HS_STATE_INACTIVE = 0,
    SAHPI_HS_STATE_INSERTION_PENDING,
    SAHPI_HS_STATE_ACTIVE,
    SAHPI_HS_STATE_EXTRACTION_PENDING,
    SAHPI_HS_STATE_NOT_PRESENT,
    SAHPI_HS_STATE_MAX_VALID = SAHPI_HS_STATE_NOT_PRESENT
} SaHpiHsStateT;

/* Cause of Hot Swap State Change */

typedef enum {
    SAHPI_HS_CAUSE_AUTO_POLICY = 0,
    SAHPI_HS_CAUSE_EXT_SOFTWARE,
    SAHPI_HS_CAUSE_OPERATOR_INIT,
    SAHPI_HS_CAUSE_USER_UPDATE,
    SAHPI_HS_CAUSE_UNEXPECTED_DEACTIVATION,
    SAHPI_HS_CAUSE_SURPRISE_EXTRACTION,
    SAHPI_HS_CAUSE_EXTRACTION_UPDATE,
    SAHPI_HS_CAUSE_HARDWARE_FAULT,
    SAHPI_HS_CAUSE_CONTAINING_FRU,
    SAHPI_HS_CAUSE_UNKNOWN = 0xFFFF,
    SAHPI_HS_CAUSE_MAX_VALID = SAHPI_HS_CAUSE_UNKNOWN
} SaHpiHsCauseOfStateChangeT;


/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                            Events                          **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Event Data Structures */

typedef enum {
    SAHPI_CRITICAL = 0,
    SAHPI_MAJOR,
    SAHPI_MINOR,
    SAHPI_INFORMATIONAL,
    SAHPI_OK,
    SAHPI_DEBUG = 0xF0,
    SAHPI_ALL_SEVERITIES = 0xFF, /* Only used with some DAT and      */
                                 /* Annunciator functions.  This is  */
                                 /* not a valid severity for events, */
                                 /* alarms, or announcements         */
    SAHPI_SEVERITY_MAX_VALID = SAHPI_DEBUG
} SaHpiSeverityT;

/*
** Resource Event Types
**
** Future versions of this specification may add new types of resource events
** to this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_RESE_RESOURCE_FAILURE,
    SAHPI_RESE_RESOURCE_RESTORED,
    SAHPI_RESE_RESOURCE_ADDED,
    SAHPI_RESE_RESOURCE_REMOVED,
    SAHPI_RESE_RESOURCE_INACCESSIBLE,
    SAHPI_RESE_RESOURCE_UPDATED,
    SAHPI_RESE_TYPE_MAX_VALID = SAHPI_RESE_RESOURCE_UPDATED
} SaHpiResourceEventTypeT;


typedef struct {
    SaHpiResourceEventTypeT  ResourceEventType;
} SaHpiResourceEventT;


/*
** Domain events are used to announce the addition of domain references
** and the removal of domain references to the DRT.
**
**
** Future versions of this specification may add new types of domain events
** to this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_DOMAIN_REF_ADDED,
    SAHPI_DOMAIN_REF_REMOVED,
    SAHPI_DOMAIN_EVENT_TYPE_MAX_VALID = SAHPI_DOMAIN_REF_REMOVED
} SaHpiDomainEventTypeT;

typedef struct {
    SaHpiDomainEventTypeT  Type;        /* Type of domain event */
    SaHpiDomainIdT         DomainId;    /* Domain Id of domain added
                                           to or removed from DRT. */
} SaHpiDomainEventT;


/*
** Sensor events and Sensor enable change events are issued to report
** changes in status of Sensors.  These events are associated with
** specific Sensor management instruments, identified by the SensorNum
** field in the event.
*/

/*
** Sensor Optional Data
**
** Sensor events may contain optional data items passed and stored with the
** event. If these optional data items are present, they are included with
** the event data returned in response to an saHpiEventGet() or
** saHpiEventLogEntryGet() function call.  Also, the optional data items may be
** included with the event data passed to the saHpiEventLogEntryAdd() function.
**
** Specific implementations of HPI may have restrictions on how much data may
** be passed to saHpiEventLogEntryAdd(). These restrictions should be documented
** by the provider of the HPI interface.
**
** Future versions of the HPI specification may define additional Sensor
** optional data items and associate them with currently undefined bit positions
** in the SaHpiSensorOptionalDataT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the OptionalDataPresent field of Sensor events.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
*/
typedef SaHpiUint8T SaHpiSensorOptionalDataT;

#define SAHPI_SOD_TRIGGER_READING   (SaHpiSensorOptionalDataT)0x01
#define SAHPI_SOD_TRIGGER_THRESHOLD (SaHpiSensorOptionalDataT)0x02
#define SAHPI_SOD_OEM               (SaHpiSensorOptionalDataT)0x04
#define SAHPI_SOD_PREVIOUS_STATE    (SaHpiSensorOptionalDataT)0x08
#define SAHPI_SOD_CURRENT_STATE     (SaHpiSensorOptionalDataT)0x10
#define SAHPI_SOD_SENSOR_SPECIFIC   (SaHpiSensorOptionalDataT)0x20
typedef struct {
    SaHpiSensorNumT           SensorNum;
    SaHpiSensorTypeT          SensorType;
    SaHpiEventCategoryT       EventCategory;
    SaHpiBoolT                Assertion;        /* True = Event State
                                                   asserted
                                                   False = deasserted */
    SaHpiEventStateT          EventState;       /* single state being asserted
                                                   or deasserted*/
    SaHpiSensorOptionalDataT  OptionalDataPresent;
    /* the following fields are only valid if the corresponding flag is set
       in the OptionalDataPresent field */
    SaHpiSensorReadingT       TriggerReading;   /* Reading that triggered
                                                   the event */
    SaHpiSensorReadingT       TriggerThreshold; /* Value of the threshold
                                                   that was crossed.  Is not
                                                   present if threshold is
                                                   not readable. */
    SaHpiEventStateT          PreviousState;    /* Previous set of asserted
                                                   event states.  If multiple
                                                   event states change at once,
                                                   multiple events may be
                                                   generated for each changing
                                                   event state.  This field
                                                   should indicate the status of
                                                   the Sensor event states prior
                                                   to any of the simultaneous
                                                   changes.

                                                   Thus, it is the same in
                                                   each event generated due to
                                                   multiple simultaneous event
                                                   state changes. */

    SaHpiEventStateT          CurrentState;     /* Current set of asserted
                                                   event states. */
    SaHpiUint32T              Oem;
    SaHpiUint32T              SensorSpecific;
} SaHpiSensorEventT;

/*
** Sensor Enable Change Optional Data
**
** Sensor enable change events may contain optional data items passed and
** stored with the event.
**
** Future versions of the HPI specification may define additional Sensor enable
** change optional data items and associate them with currently undefined bit
** positions in the SaHpiSensorEnableOptDataT bit mask.  Implementations
** conforming to this version of the HPI specification should set all undefined
** bit values to zero in the OptionalDataPresent field of Sensor Enable Change
** events. However, HPI Users should not assume undefined bit values are zero,
** to remain compatible with HPI implementations that implement future versions
** of the specification.
**
*/

typedef SaHpiUint8T SaHpiSensorEnableOptDataT;

#define SAHPI_SEOD_CURRENT_STATE        (SaHpiSensorEnableOptDataT)0x10
#define SAHPI_SEOD_ALARM_STATES         (SaHpiSensorEnableOptDataT)0x40

typedef struct {
    SaHpiSensorNumT           SensorNum;
    SaHpiSensorTypeT          SensorType;
    SaHpiEventCategoryT       EventCategory;
    SaHpiBoolT                SensorEnable;  /* current Sensor enable status  */
    SaHpiBoolT                SensorEventEnable; /* current evt enable status */
    SaHpiEventStateT          AssertEventMask;   /* current assert event mask */
    SaHpiEventStateT          DeassertEventMask; /* current deassert evt mask */
    SaHpiSensorEnableOptDataT OptionalDataPresent;
    /*  The CurrentState field is valid only if the SAHPI_SEOD_CURRENT_STATE
             flag is set in the OptionalDataPresent field.  */
    SaHpiEventStateT          CurrentState;      /* Current set of asserted
                                                    Event states. */


    /*  The CriticalAlarms, MajorAlarms and MinorAlarms fields are valid only
        if the SAHPI_SEOD_ALARM_STATES flag is set in the OptionalDataPresent
        field.  These fields indicate which of the currently asserted event
        states are critical, major, or minor alarms, respectively.  An
        asserted event state is an alarm only if the Sensor is enabled, Sensor
        events are enabled, the corresponding AssertEventMask bit is set, and
        the event generated for that event state assertion has a severity of
        SAHPI_CRITICAL, SAHPI_MAJOR, or SAHPI_MINOR.  Thus, if the event is
        reporting that the Sensor is disabled or Sensor events are disabled,
        then these three fields, if present, must all be equal to 0000.
        Similarly, there should never be a bit set in any of these three
        fields that is not also set in the AssertEventMask field, and, if the
        SAHPI_SEOD_CURRENT_STATE flag is Set, in the CurrentState field. */
    SaHpiEventStateT        CriticalAlarms;  /* current set of asserted
                    Event states that are critical alarms.*/
    SaHpiEventStateT        MajorAlarms;  /* current set of asserted
                              Event states that are
                              major alarms.*/
    SaHpiEventStateT        MinorAlarms;  /* current set of asserted
                              Event states that are
                              minor alarms.*/

} SaHpiSensorEnableChangeEventT;



typedef struct {
    SaHpiHsStateT       HotSwapState;
    SaHpiHsStateT       PreviousHotSwapState;
     SaHpiHsCauseOfStateChangeT CauseOfStateChange;
} SaHpiHotSwapEventT;


/*
** Watchdog events are issued to report changes in status of Sensors.  These
** events are associated with specific Watchdog Timer management instruments,
** identified by the WatchdogNum field in the event.
*/

typedef struct {
    SaHpiWatchdogNumT               WatchdogNum;
    SaHpiWatchdogActionEventT       WatchdogAction;
    SaHpiWatchdogPretimerInterruptT WatchdogPreTimerAction;
    SaHpiWatchdogTimerUseT          WatchdogUse;
} SaHpiWatchdogEventT;

/*
** The following type defines the types of events that can be reported
** by the HPI software implementation.
**
** Audit events report a discrepancy in the audit process.  Audits are typically
** performed by HA software to detect problems.  Audits may look for such things
** as corrupted data stores, inconsistent RPT information, or improperly managed
** queues.
**
** Startup events report a failure to start-up properly, or inconsistencies in
** persisted data.
**
**
** Future versions of this specification may add new types of software events
** to this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_HPIE_AUDIT,
    SAHPI_HPIE_STARTUP,
    SAHPI_HPIE_OTHER,
    SAHPI_HPIE_TYPE_MAX_VALID = SAHPI_HPIE_OTHER
} SaHpiSwEventTypeT;

typedef struct {
    SaHpiManufacturerIdT MId;
    SaHpiSwEventTypeT   Type;
    SaHpiTextBufferT     EventData;
} SaHpiHpiSwEventT;

typedef struct {
    SaHpiManufacturerIdT MId;
    SaHpiTextBufferT     OemEventData;
} SaHpiOemEventT;

/*
** User events may be used for storing custom events created by an HPI User
** when injecting events into the Event Log using saHpiEventLogEntryAdd().
*/
typedef struct {
    SaHpiTextBufferT     UserEventData;
} SaHpiUserEventT;

/*
** DIMI events and DIMI Update events are issued to report changes in status
** of DIMIs or tests initiated by DIMIs.  These events are associated with
** specific Diagnotic Initiator management instruments, identified by the
** DimiNum field in the event.
*/

/*
** DIMI events are generated by DIMI implementation asynchronously when
** status of a test changes (For example when a test starts, ends, gets
** canceled or generated for % completed). The test run status is enumerated
** in SaHpiDimiTestRunStatusT
*/
typedef struct {
    SaHpiDimiNumT           DimiNum;
     SaHpiDimiTestNumT      TestNum;
    SaHpiDimiTestRunStatusT     DimiTestRunStatus;
    SaHpiDimiTestPercentCompletedT DimiTestPercentCompleted;
                    /* Percentage of test completed.  Based on
                       implementation capability,
                       value 0 - 100,
                       0xFF returned if capability not available */
} SaHpiDimiEventT;

/*
** DIMI update events are generated when the set of tests for a DIMI changes.
** For example, when new tests are added or deleted from DIMI. When this event
** is generated, HPI User is expected to retrieve updated test information
** using DimiInfoGet function with the DIMI number provided in the event.
*/

typedef struct {
    SaHpiDimiNumT         DimiNum;
} SaHpiDimiUpdateEventT;


/*
** FUMI events are issued to report changes in status of firmware upgrade
** operations.  These events are associated with specific Firmware Upgrade
** management instruments, identified by the FumiNum field in the event.
*/

/*
** FUMI event definition
**
*/
typedef struct {
    SaHpiFumiNumT           FumiNum;       /* FUMI Number */
    SaHpiUint8T             BankNum;       /* Bank Number */
    SaHpiFumiUpgradeStatusT     UpgradeStatus; /* Upgrade status of bank */
} SaHpiFumiEventT;


/* HPI Event Types
**
** Future versions of this specification may add new event types to this
** enumerated type. Legacy HPI Users should consider these new event types
** "valid but unknown" and should not attempt to interpret such events.
** In general, if an event of unknown type is found in an event log or
** received via an event subscription it should be ignored by the HPI User.
*/

typedef enum {
    SAHPI_ET_RESOURCE,
    SAHPI_ET_DOMAIN,
    SAHPI_ET_SENSOR,
    SAHPI_ET_SENSOR_ENABLE_CHANGE,
    SAHPI_ET_HOTSWAP,
    SAHPI_ET_WATCHDOG,
    SAHPI_ET_HPI_SW,
    SAHPI_ET_OEM,
    SAHPI_ET_USER,
    SAHPI_ET_DIMI,
    SAHPI_ET_DIMI_UPDATE,
    SAHPI_ET_FUMI,
    SAHPI_ET_MAX_VALID = SAHPI_ET_FUMI
} SaHpiEventTypeT;

typedef union {
    SaHpiResourceEventT           ResourceEvent;
    SaHpiDomainEventT             DomainEvent;
    SaHpiSensorEventT             SensorEvent;
    SaHpiSensorEnableChangeEventT SensorEnableChangeEvent;
    SaHpiHotSwapEventT           HotSwapEvent;
    SaHpiWatchdogEventT          WatchdogEvent;
    SaHpiHpiSwEventT              HpiSwEvent;
    SaHpiOemEventT           OemEvent;
    SaHpiUserEventT              UserEvent;
    SaHpiDimiEventT      DimiEvent;
    SaHpiDimiUpdateEventT        DimiUpdateEvent;
    SaHpiFumiEventT      FumiEvent;
} SaHpiEventUnionT;

typedef struct {
    SaHpiResourceIdT  Source;
    SaHpiEventTypeT   EventType;
    SaHpiTimeT        Timestamp; /*Equal to SAHPI_TIME_UNSPECIFIED if time is
                                   not available; Absolute time if greater
                                   than SAHPI_TIME_MAX_RELATIVE, Relative
                                   time if less than or equal to
                                   SAHPI_TIME_MAX_RELATIVE */
    SaHpiSeverityT    Severity;
    SaHpiEventUnionT  EventDataUnion;
} SaHpiEventT;

/*
**  Event Queue Status
**
**      This status word is returned to HPI Users that request it
**      when saHpiEventGet() is called.
**
*/

typedef SaHpiUint32T    SaHpiEvtQueueStatusT;

#define SAHPI_EVT_QUEUE_OVERFLOW        (SaHpiEvtQueueStatusT)0x0001

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                        Annunciators                        **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Annunciator Number
** Identifier for an Annunciator management instrument.
*/
typedef SaHpiInstrumentIdT SaHpiAnnunciatorNumT;

/*
** The following data type is equivalent to the AIS data type SaNameT.
** it is defined in this header file, so that inclusion of the AIS
** header file is not required.  This data type is based on version 1.0
** of the AIS specification
*/
#define SA_HPI_MAX_NAME_LENGTH 256

typedef struct {
    SaHpiUint16T  Length;
    unsigned char Value[SA_HPI_MAX_NAME_LENGTH];
} SaHpiNameT;

/*
** Enumeration of Announcement Types
**
** Future versions of this specification may add new types of announcements
** to this enumerated type. Legacy HPI Users should consider these new types
** "valid but unknown".
*/
typedef enum {
    SAHPI_STATUS_COND_TYPE_SENSOR,
    SAHPI_STATUS_COND_TYPE_RESOURCE,
    SAHPI_STATUS_COND_TYPE_OEM,
    SAHPI_STATUS_COND_TYPE_USER,
    SAHPI_STATUS_COND_TYPE_MAX_VALID = SAHPI_STATUS_COND_TYPE_USER
} SaHpiStatusCondTypeT;


/* Condition structure definition */
typedef struct {

    SaHpiStatusCondTypeT Type;         /* Status Condition Type */
    SaHpiEntityPathT     Entity;       /* Entity assoc. with status condition */
    SaHpiDomainIdT       DomainId;     /* Domain associated with status.
                                          May be SAHPI_UNSPECIFIED_DOMAIN_ID
                                          meaning current domain, or domain
                                          not meaningful for status condition*/
    SaHpiResourceIdT     ResourceId;   /* Resource associated with status.
                                          May be SAHPI_UNSPECIFIED_RESOURCE_ID
                                          if Type is SAHPI_STATUS_COND_USER.
                                          Must be set to valid ResourceId in
                                          domain specified by DomainId,
                                          or in current domain, if DomainId
                                          is SAHPI_UNSPECIFIED_DOMAIN_ID */
    SaHpiSensorNumT      SensorNum;    /* Sensor associated with status
                                          Only valid if Type is
                                          SAHPI_STATUS_COND_TYPE_SENSOR */
    SaHpiEventStateT     EventState;   /* Sensor event state.
                                          Only valid if Type is
                                          SAHPI_STATUS_COND_TYPE_SENSOR. */
    SaHpiNameT           Name;         /* AIS compatible identifier associated
                                          with Status condition */
    SaHpiManufacturerIdT Mid;          /* Manufacturer Id associated with
                                          status condition, required when type
                                          is SAHPI_STATUS_COND_TYPE_OEM. */
    SaHpiTextBufferT     Data;         /* Optional Data associated with
                                          Status condition */
} SaHpiConditionT;


/* Announcement structure definition */
typedef struct {
    SaHpiEntryIdT        EntryId;      /* Announcment Entry Id */
    SaHpiTimeT           Timestamp;    /* Time when announcement added to set */
    SaHpiBoolT           AddedByUser;  /* True if added to set by HPI User,
                                          False if added automatically by
                                          HPI implementation */
    SaHpiSeverityT       Severity;     /* Severity of announcement */
    SaHpiBoolT           Acknowledged; /* Acknowledged flag */
    SaHpiConditionT      StatusCond;   /* Detailed status condition */
} SaHpiAnnouncementT;


/* Annunciator Mode - defines who may add or delete entries in set.  */

typedef enum {
    SAHPI_ANNUNCIATOR_MODE_AUTO,
    SAHPI_ANNUNCIATOR_MODE_USER,
    SAHPI_ANNUNCIATOR_MODE_SHARED,
    SAHPI_ANNUNCIATOR_MODE_MAX_VALID = SAHPI_ANNUNCIATOR_MODE_SHARED
} SaHpiAnnunciatorModeT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********              Annunciator Resource Data Records             **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** The following enumerated type defines the possible output types
** which can be associated with an Annunciator Management Instrument
**
** Future versions of this specification may add new types of announcement
** output types to this enumerated type. Legacy HPI Users should consider
** these new types "valid but unknown".

*/
typedef enum {
    SAHPI_ANNUNCIATOR_TYPE_LED = 0,
    SAHPI_ANNUNCIATOR_TYPE_DRY_CONTACT_CLOSURE,
    SAHPI_ANNUNCIATOR_TYPE_AUDIBLE,
    SAHPI_ANNUNCIATOR_TYPE_LCD_DISPLAY,
    SAHPI_ANNUNCIATOR_TYPE_MESSAGE,
    SAHPI_ANNUNCIATOR_TYPE_COMPOSITE,
    SAHPI_ANNUNCIATOR_TYPE_OEM,
    SAHPI_ANNUNCIATOR_TYPE_MAX_VALID = SAHPI_ANNUNCIATOR_TYPE_OEM
} SaHpiAnnunciatorTypeT;


/*
** All Annunciator management instruments
** must be represented in the RDR repository
** with an SaHpiAnnunciatorRecT.
*/
typedef struct {
    SaHpiAnnunciatorNumT      AnnunciatorNum;
    SaHpiAnnunciatorTypeT     AnnunciatorType; /* Annunciator Output Type */
    SaHpiBoolT                ModeReadOnly;    /* if True, Mode may
                                                  not be changed by HPI User */
    SaHpiUint32T              MaxConditions;   /* maximum number of conditions
                                                  that can be held in current
                                                  set.  0 means no fixed
                                                  limit.  */
    SaHpiUint32T              Oem;
} SaHpiAnnunciatorRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                     Resource Data Record                   **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** The following describes the different types of records that exist within a
** RDR repository and the RDR super-structure to all of the specific RDR types
** (Sensor, Inventory Data, Watchdog, etc.).
**
** Future versions of this specification may add new resource data record
** types to this enumerated type. Legacy HPI Users should consider these new
** RDR types "valid but unknown" and should not attempt to interpret such
** records. In general, if an RDR of unknown type is found within a RDR
** repository the HPI User should ignore it.
*/
typedef enum {
    SAHPI_NO_RECORD,
    SAHPI_CTRL_RDR,
    SAHPI_SENSOR_RDR,
    SAHPI_INVENTORY_RDR,
    SAHPI_WATCHDOG_RDR,
    SAHPI_ANNUNCIATOR_RDR,
    SAHPI_DIMI_RDR,
    SAHPI_FUMI_RDR,
    SAHPI_RDR_TYPE_MAX_VALID = SAHPI_FUMI_RDR
} SaHpiRdrTypeT;

typedef union {
    SaHpiCtrlRecT        CtrlRec;
    SaHpiSensorRecT      SensorRec;
    SaHpiInventoryRecT   InventoryRec;
    SaHpiWatchdogRecT    WatchdogRec;
    SaHpiAnnunciatorRecT AnnunciatorRec;
    SaHpiDimiRecT        DimiRec;
    SaHpiFumiRecT        FumiRec;
} SaHpiRdrTypeUnionT;

typedef struct {
    SaHpiEntryIdT        RecordId;
    SaHpiRdrTypeT        RdrType;
    SaHpiEntityPathT     Entity;        /* Entity to which this RDR relates. */
    SaHpiBoolT           IsFru;         /* Entity is a FRU.  This field is
                                           Only valid if the Entity path given
                                           in the "Entity" field is different
                                           from the Entity path in the RPT
                                           entry for the resource. */
    SaHpiRdrTypeUnionT   RdrTypeUnion;
    SaHpiTextBufferT     IdString;
} SaHpiRdrT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Parameter Control                     **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

typedef enum {
    SAHPI_DEFAULT_PARM = 0,
    SAHPI_SAVE_PARM,
    SAHPI_RESTORE_PARM,
    SAHPI_PARM_ACTION_MAX_VALID = SAHPI_RESTORE_PARM
} SaHpiParmActionT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Reset                            **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

typedef enum {
    SAHPI_COLD_RESET = 0,
    SAHPI_WARM_RESET,
    SAHPI_RESET_ASSERT,
    SAHPI_RESET_DEASSERT,
    SAHPI_RESET_MAX_VALID = SAHPI_RESET_DEASSERT
} SaHpiResetActionT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Power                                 **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

typedef enum {
    SAHPI_POWER_OFF = 0,
    SAHPI_POWER_ON,
    SAHPI_POWER_CYCLE,
    SAHPI_POWER_STATE_MAX_VALID = SAHPI_POWER_CYCLE
} SaHpiPowerStateT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Load ID                               **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

typedef SaHpiUint32T SaHpiLoadNumberT;
#define SAHPI_LOAD_ID_DEFAULT (SaHpiLoadNumberT)0
#define SAHPI_LOAD_ID_BYNAME (SaHpiLoadNumberT)0xffffffff

typedef struct {
    SaHpiLoadNumberT LoadNumber;
    SaHpiTextBufferT LoadName;
} SaHpiLoadIdT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                    Resource Presence Table                 **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*  This section defines the types associated with the RPT. */

/*
** GUID - Globally Unique Identifier
**
** The format of the ID follows that specified by the Wired for Management
** Baseline, Version 2.0 specification.  HPI uses version 1 of the GUID
** format, with a 3-bit variant field of 10x (where x indicates "don't care")
*/
typedef SaHpiUint8T    SaHpiGuidT[16];

/*
** Resource Info Type Definitions
**
**
** SaHpiResourceInfoT contains static configuration data concerning the
** management controller associated with the resource, or the resource itself.
** Note this information is used to describe the resource; that is, the piece of
** infrastructure which manages an entity (or multiple entities) - NOT the
** entities for which the resource provides management. The purpose of the
** SaHpiResourceInfoT structure is to provide information that an HPI User may
** need to interact correctly with the resource (e.g., recognize a
** specific management controller which may have defined OEM fields in Sensors,
** OEM Controls, etc.).
**
** The fields of the SaHpiResourceInfoT structure are intended to uniquely
** identify an implementation of the functionality modeled by an HPI resource.
** Multiple instances of a resource implementation may be present in an HPI
** implementation with different configurations, so it is not required that
** this data be unique in each resource present in an HPI implemenation.
**
** The GUID is used to uniquely identify a Resource.  A GUID value of zero
** indicates that the Resource does not have an associated GUID.
**
** All of the fields in the following structure may or may not be used by a
** given resource.
*/
typedef struct {
    SaHpiUint8T            ResourceRev;
    SaHpiUint8T            SpecificVer;
    SaHpiUint8T            DeviceSupport;
    SaHpiManufacturerIdT   ManufacturerId;
    SaHpiUint16T           ProductId;
    SaHpiUint8T            FirmwareMajorRev;
    SaHpiUint8T            FirmwareMinorRev;
    SaHpiUint8T            AuxFirmwareRev;
    SaHpiGuidT             Guid;
} SaHpiResourceInfoT;

/*
** Resource Capabilities
**
** This definition defines the capabilities of a given resource. One resource
** may support any number of capabilities using the bit mask.  Because each entry
** in an RPT has the SAHPI_CAPABILITY_RESOURCE bit set, zero is not a
** valid value for the capability flag, and is thus used to indicate "no RPT
** entry present" in some function calls.
**
** Future versions of the HPI specification may define additional resource
** capabilities and associate them with currently undefined bit positions in
** the SaHpiCapabilitiesT bit mask.  Implementations conforming to this
** version of the HPI specification should set all undefined bit values to
** zero in the ResourceCapabilities field of all Rpt entries.  However,
** HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
** SAHPI_CAPABILITY_RESOURCE
** SAHPI_CAPABILITY_EVT_DEASSERTS
**   Indicates that all Sensors on the resource have the property that their
**   Assertion and Deassertion event enable flags are the same. That is,
**   for all event states whose assertion triggers an event, it is
**   guaranteed that the deassertion of that event state also
**   triggers an event. Thus, an HPI User may track the state of Sensors on the
**   resource by monitoring events rather than polling for event state changes.
** SAHPI_CAPABILITY_AGGREGATE_STATUS
** SAHPI_CAPABILITY_CONFIGURATION
** SAHPI_CAPABILITY_MANAGED_HOTSWAP
**   Indicates that the resource supports managed hot swap.
**   Since hot swap only makes sense for field-replaceable units, the
**    SAHPI_CAPABILITY_FRU capability bit must also be set for this resource.
** SAHPI_CAPABILITY_WATCHDOG
** SAHPI_CAPABILITY_CONTROL
** SAHPI_CAPABILITY_FRU
**   Indicates that the resource is associated with an entity that is a
**   field-replaceable unit; i.e., it is capable of being removed and
**   replaced in a live system, and the resource reports hot swap events
**   for the FRU.
** SAHPI_CAPABILITY_LOAD_ID
** SAHPI_CAPABILITY_DIMI
** SAHPI_CAPABILITY_FUMI
** SAHPI_CAPABILITY_ANNUNCIATOR
** SAHPI_CAPABILITY_POWER
** SAHPI_CAPABILITY_RESET
** SAHPI_CAPABILITY_INVENTORY_DATA
** SAHPI_CAPABILITY_EVENT_LOG
** SAHPI_CAPABILITY_RDR
**   Indicates that a resource data record (RDR) repository is supplied
**   by the resource. Since the existence of an RDR is mandatory for all
**   management instruments, this
**   capability must be asserted if the resource
**   contains any Sensors, Controls, Watchdog Timers, Annunciators,
**   Inventory Data Repositories, FUMIs or DIMIs.
** SAHPI_CAPABILITY_SENSOR
*/

typedef SaHpiUint32T SaHpiCapabilitiesT;
#define SAHPI_CAPABILITY_RESOURCE         (SaHpiCapabilitiesT)0x40000000
#define SAHPI_CAPABILITY_FUMI             (SaHpiCapabilitiesT)0x00010000
#define SAHPI_CAPABILITY_EVT_DEASSERTS    (SaHpiCapabilitiesT)0x00008000
#define SAHPI_CAPABILITY_DIMI             (SaHpiCapabilitiesT)0x00004000
#define SAHPI_CAPABILITY_AGGREGATE_STATUS (SaHpiCapabilitiesT)0x00002000
#define SAHPI_CAPABILITY_CONFIGURATION    (SaHpiCapabilitiesT)0x00001000
#define SAHPI_CAPABILITY_MANAGED_HOTSWAP  (SaHpiCapabilitiesT)0x00000800
#define SAHPI_CAPABILITY_WATCHDOG         (SaHpiCapabilitiesT)0x00000400
#define SAHPI_CAPABILITY_CONTROL          (SaHpiCapabilitiesT)0x00000200
#define SAHPI_CAPABILITY_FRU              (SaHpiCapabilitiesT)0x00000100
#define SAHPI_CAPABILITY_LOAD_ID          (SaHpiCapabilitiesT)0x00000080
#define SAHPI_CAPABILITY_ANNUNCIATOR      (SaHpiCapabilitiesT)0x00000040
#define SAHPI_CAPABILITY_POWER            (SaHpiCapabilitiesT)0x00000020
#define SAHPI_CAPABILITY_RESET            (SaHpiCapabilitiesT)0x00000010
#define SAHPI_CAPABILITY_INVENTORY_DATA   (SaHpiCapabilitiesT)0x00000008
#define SAHPI_CAPABILITY_EVENT_LOG        (SaHpiCapabilitiesT)0x00000004
#define SAHPI_CAPABILITY_RDR              (SaHpiCapabilitiesT)0x00000002
#define SAHPI_CAPABILITY_SENSOR           (SaHpiCapabilitiesT)0x00000001

/*
** Resource Managed Hot Swap Capabilities
**
** This definition defines the managed hot swap capabilities of a given
** resource.
**
** Future versions of the HPI specification may define additional hot swap
** capabilities and associate them with currently undefined bit positions
** in the SaHpiHsCapabilitiesT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the HotSwapCapabilities field of all RPT entries.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
** SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY
** This capability indicates if the hot swap autoextract timer is read-only.

** SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED
** Indicates whether the resource has a hot swap indicator.
** SAHPI_HS_CAPABILITY_AUTOINSERT_IMMEDIATE
** This capability indicates that the resource ignores the auto insertion
** timeout value(s) configured for the domain(s) of which it is a member,
** but instead begins its auto insertion policy immediately upon insertion.
*/

typedef SaHpiUint32T SaHpiHsCapabilitiesT;
#define SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY \
   (SaHpiHsCapabilitiesT)0x80000000
#define SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED \
   (SaHpiHsCapabilitiesT)0X40000000
#define SAHPI_HS_CAPABILITY_AUTOINSERT_IMMEDIATE\
   (SaHpiHsCapabilitiesT)0x20000000
/*
** RPT Entry
**
** This structure is used to store the RPT entry information.
**
** The ResourceCapabilities field defines the capabilities of the resource.
** This field must be non-zero for all valid resources.
**
** The HotSwapCapabilities field denotes the capabilities of the resource,
** specifically related to hot swap.  This field is only valid if the
** resource supports managed hot swap, as indicated by the
** SAHPI_CAPABILITY_MANAGED_HOT_SWAP resource capability.
**
** The ResourceTag is a data field within an RPT entry available to the HPI
** User for associating application specific data with a resource.  The HPI
** User supplied data is purely informational and is not used by the HPI
** implementation, domain, or associated resource.  For example, an HPI User
** can set the resource tag to a "descriptive" value, which can be used to
** identify the resource in messages to a human operator.
*/
typedef struct {
    SaHpiEntryIdT        EntryId;
    SaHpiResourceIdT     ResourceId;
    SaHpiResourceInfoT   ResourceInfo;
    SaHpiEntityPathT     ResourceEntity;      /* If resource manages a FRU, entity
                                             path of the FRU */
                                          /* If resource manages a single
                                             entity, entity path of that
                                             entity. */
                                          /* If resource manages multiple
                                             entities, the entity path of the
                                             "primary" entity managed by the
                                             resource */
                                          /* Must be set to the same value in
                                             every domain which contains this
                                             resource */
    SaHpiCapabilitiesT   ResourceCapabilities;  /* Must be non-0. */
    SaHpiHsCapabilitiesT HotSwapCapabilities; /* Indicates the hot swap
                                                 capabilities of the resource */
    SaHpiSeverityT       ResourceSeverity; /* Indicates the criticality that
                                              should be raised when the resource
                                              is not responding   */
    SaHpiBoolT           ResourceFailed;  /* Indicates that the resource is not
                                             currently accessible */
    SaHpiTextBufferT     ResourceTag;
} SaHpiRptEntryT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Domain Information                    **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*  This section defines the types associated with the domain controller. */

/*
** Domain Capabilities
**
** This definition defines the capabilities of a given domain.  A domain
** may support any number of capabilities using the bit mask.
**
** Future versions of the HPI specification may define additional domain
** capabilities and associate them with currently undefined bit positions
** in the SaHpiDomainCapabilitiesT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the DomainCapabilities field of the SaHpiDomainInfoT structure.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
** SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY
**   Indicates that the domain auto insert timeout value is read-only
**   and may not be modified using the saHpiHotSwapAutoInsertTimeoutSet()
**   function.
*/

typedef SaHpiUint32T SaHpiDomainCapabilitiesT;
#define SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY \
   (SaHpiDomainCapabilitiesT)0X00000001

/*
** Domain Info
**
** This structure is used to store the information regarding the domain
** including information regarding the domain reference table (DRT) and
** the resource presence table (RPT).
**
** The DomainTag field is an informational value that supplies an HPI User
** with naming information for the domain.
**
** NOTE: Regarding timestamps - If the implementation cannot supply an absolute
** timestamp, then it may supply a timestamp relative to some system-defined
** epoch, such as system boot. The value SAHPI_TIME_UNSPECIFIED indicates that
** the time of the update cannot be determined. Otherwise, If the value is less
** than or equal to SAHPI_TIME_MAX_RELATIVE, then it is relative; if it is
** greater than SAHPI_TIME_MAX_RELATIVE, then it is absolute.
**
** The GUID is used to uniquely identify a domain.  A GUID value of zero is not
** valid and indicates that the domain does not have an associated GUID.
*/

typedef struct {
    SaHpiDomainIdT    DomainId;        /* Unique Domain Id associated with
                                          domain */
    SaHpiDomainCapabilitiesT DomainCapabilities;    /* Domain Capabilities */
    SaHpiBoolT        IsPeer;          /* Indicates that this domain
                                          participates in a peer
                                          relationship. */
    SaHpiTextBufferT  DomainTag;       /* Information tag associated with
                                          domain */
    SaHpiUint32T      DrtUpdateCount;  /* This count is incremented any time the
                                          table is changed. It rolls over to
                                          zero when the maximum value is
                                          reached  */
    SaHpiTimeT        DrtUpdateTimestamp; /* This timestamp is set any time the
                                             DRT table is changed. */
    SaHpiUint32T      RptUpdateCount;   /* This count is incremented any time
                                           the RPT is changed. It rolls over
                                           to zero when the maximum value is
                                           reached  */
    SaHpiTimeT        RptUpdateTimestamp; /* This timestamp is set any time the
                                             RPT table is changed. */
    SaHpiUint32T       DatUpdateCount;  /* This count is incremented any time
                                           the DAT is changed. It rolls over to
                                           zero when the maximum value is
                                           reached */
    SaHpiTimeT         DatUpdateTimestamp; /* This timestamp is set any time the
                                              DAT is changed. */
    SaHpiUint32T       ActiveAlarms;     /* Count of active alarms in the DAT */
    SaHpiUint32T       CriticalAlarms;   /* Count of active critical alarms in
                                            the DAT */
    SaHpiUint32T       MajorAlarms;      /* Count of active major alarms in the
                                            DAT */
    SaHpiUint32T       MinorAlarms;      /* Count of active minor alarms in the
                                            DAT */
    SaHpiUint32T       DatUserAlarmLimit; /* Maximum User Alarms that can be
                                             added to DAT.  0=no fixed limit */
    SaHpiBoolT         DatOverflow;       /* Set to True if there are one
                                             or more non-User Alarms that
                                             are missing from the DAT because
                                             of space limitations */
    SaHpiGuidT        Guid;              /* GUID associated with domain.*/
} SaHpiDomainInfoT;

/*
** DRT Entry
**
** This structure is used to store the DRT entry information.
**
*/
typedef struct {
    SaHpiEntryIdT        EntryId;
    SaHpiDomainIdT       DomainId;  /* The Domain ID referenced by this entry */
    SaHpiBoolT           IsPeer;    /* Indicates if this domain reference
                                       is a peer. */
} SaHpiDrtEntryT;


/*
** DAT Entry
**
** This structure is used to store alarm information in the DAT
**
*/


typedef SaHpiEntryIdT SaHpiAlarmIdT;

typedef struct {
    SaHpiAlarmIdT        AlarmId;      /* Alarm Id */
    SaHpiTimeT           Timestamp;    /* Time when alarm added to DAT */
    SaHpiSeverityT       Severity;     /* Severity of alarm */
    SaHpiBoolT           Acknowledged; /* Acknowledged flag */
    SaHpiConditionT     AlarmCond;   /* Detailed alarm condition */
} SaHpiAlarmT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                       Event Log                            **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* This section defines the types associated with the Event Log. */
/*
** Event Log Information
**
** The Entries entry denotes the number of active entries contained in the Event
**   Log.
** The Size entry denotes the total number of entries the Event Log is able to
**   hold.
** The UserEventMaxSize entry indicates the maximum size of the text buffer
**   data field in an HPI User event that is supported by the Event Log
**   implementation.  If the implementation does not enforce a more restrictive
**   data length, it should be set to SAHPI_MAX_TEXT_BUFFER_LENGTH.
** The UpdateTimestamp entry denotes the time of the last update to the Event
**   Log.
** The CurrentTime entry denotes the Event Log's idea of the current time; i.e
**   the timestamp that would be placed on an entry if it was added now.
** The Enabled entry indicates whether the Event Log is enabled. If the Event
**   Log is "disabled" no events generated within the HPI implementation are
**   added to the Event Log.  Events may still be added to the Event Log with
**   the saHpiEventLogEntryAdd() function. When the Event Log is "enabled"
**   events may be automatically added to the Event Log as they are generated
**   in a resource or a domain, however, it is implementation-specific which
**   events are automatically added to any Event Log.
** The OverflowFlag entry indicates the Event Log has overflowed.  Events have
**   been dropped or overwritten due to a table overflow.
** The OverflowAction entry indicates the behavior of the Event Log when an
**   overflow occurs.
** The OverflowResetable entry indicates if the overflow flag can be
**   cleared by an HPI User with the saHpiEventLogOverflowReset() function.
*/
typedef enum {
    SAHPI_EL_OVERFLOW_DROP,       /* New entries are dropped when Event Log is
                                    full*/
    SAHPI_EL_OVERFLOW_OVERWRITE,  /* Event Log overwrites existing entries
                                    when Event Log is full */
    SAHPI_EL_OVERFLOW_ACTION_MAX_TYPE = SAHPI_EL_OVERFLOW_OVERWRITE
} SaHpiEventLogOverflowActionT;

typedef struct {
    SaHpiUint32T                   Entries;
    SaHpiUint32T                   Size;
    SaHpiUint32T                   UserEventMaxSize;
    SaHpiTimeT                     UpdateTimestamp;
    SaHpiTimeT                     CurrentTime;
    SaHpiBoolT                     Enabled;
    SaHpiBoolT                     OverflowFlag;
    SaHpiBoolT                     OverflowResetable;
    SaHpiEventLogOverflowActionT   OverflowAction;
} SaHpiEventLogInfoT;


/*
** Event Log Capabilities
**
** A value of the following type, SaHpiEventLogCapabilitiesT, is returned by
** the saHpiEventLogCapabilitiesGet() function.  It is defined as a 32-bit
** value with individual bits representing various optional capabilities
** provided by specific event logs in an HPI implementation.  A separate
** value is returned for each event log maintained by an HPI implementation,
** so different event logs may have different capabilities.
**
** For each bit value defined below, if the event log supports the
** corresponding capability, the bit is set to "1" in the value returned
** by saHpiEventLogCapabilitiesGet().  If the event log does not support the
** corresponding capability, the bit is set to "0".
**
** Future versions of the HPI specification may define additional event log
** capabilities and associate them with currently undefined bit positions
** in the SaHpiEventLogCapabilitiesT bit mask.  Implementations conforming to
** this version of the HPI specification should set all undefined bit values
** to zero in the SaHpiEentLogCapabilitiesT value for all event logs.
** However, HPI Users should not assume undefined bit values are zero, to
** remain compatible with HPI implementations that implement future versions
** of the specification.
**
**
** The optional event log capabilities, and the corresponding bit values are:
**
**  SAHPI_EVTLOG_CAPABILITY_ENTRY_ADD
**     The saHpiEventLogEntryAdd() function may be used to add events to the
**     event log.  Note that this capability only addresses whether
**     users can add entries directly to the event log via the
**     saHpiEventLogEntryAdd() function.  Even without this capability, user
**     events added to a domain via the saHpiEventAdd() function may be added
**     to an event log by the HPI implementation.
**  SAHPI_EVTLOG_CAPABILITY_CLEAR
**     The saHpiEventLogClear() function may be used to clear the event log.
**  SAHPI_EVTLOG_CAPABILITY_TIME_SET
**     The saHpiEventLogTimeSet() function may be used to set the event log
**     clock.
**  SAHPI_EVTLOG_CAPABILITY_STATE_SET
**     The saHpiEventLogStateSet() function may be used to enable or disable
**     automatic logging of events in the event log.
**  SAHPI_EVTLOG_CAPABILITY_OVERFLOW_RESET
**     The saHpiEventLogOverflowReset() function may be used to clear the
**     OverflowFlag in the event log info record.
**
*/

typedef SaHpiUint32T SaHpiEventLogCapabilitiesT;
#define SAHPI_EVTLOG_CAPABILITY_ENTRY_ADD (SaHpiEventLogCapabilitiesT)0x00000001
#define SAHPI_EVTLOG_CAPABILITY_CLEAR     (SaHpiEventLogCapabilitiesT)0x00000002
#define SAHPI_EVTLOG_CAPABILITY_TIME_SET  (SaHpiEventLogCapabilitiesT)0x00000004
#define SAHPI_EVTLOG_CAPABILITY_STATE_SET (SaHpiEventLogCapabilitiesT)0x00000008
#define SAHPI_EVTLOG_CAPABILITY_OVERFLOW_RESET  \
                                          (SaHpiEventLogCapabilitiesT)0x00000010



/*
** Event Log Entry
** These types define the Event Log entry.
*/
typedef SaHpiUint32T SaHpiEventLogEntryIdT;
/* Reserved values for Event Log entry IDs */
#define SAHPI_OLDEST_ENTRY    (SaHpiEventLogEntryIdT)0x00000000
#define SAHPI_NEWEST_ENTRY    (SaHpiEventLogEntryIdT)0xFFFFFFFF
#define SAHPI_NO_MORE_ENTRIES (SaHpiEventLogEntryIdT)0xFFFFFFFE

typedef struct {
    SaHpiEventLogEntryIdT EntryId;   /* Entry ID for record */
    SaHpiTimeT            Timestamp; /* Time at which the event was placed
                                   in the Event Log. If less than or equal to
                                   SAHPI_TIME_MAX_RELATIVE, then it is
                                   relative; if it is greater than SAHPI_TIME_
                                   MAX_RELATIVE, then it is absolute. */
    SaHpiEventT           Event;     /* Logged Event */
} SaHpiEventLogEntryT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********               Initialization and Finalization              **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Options are passed in using the following structure.  The OptionId is set
** to the particular option, and IntVal and PointerVal are set based on the
** requirements of the particular option.  See the specific options for details.
*/
typedef struct {
    SaHpiUint32T OptionId;
    union {
        SaHpiInt32T IntVal;
        void *PointerVal;
    } u;
} SaHpiInitOptionT;

/*
** An implementation may define OEM options.  For an OEM option, OptionId
** must be greater than or equal to the following value:
*/
#define SA_HPI_INITOPTION_FIRST_OEM 0x40000000U

/*
** Provide a function to create threads for the library.  A library
** implementation may create threads for internal use; programs may need to
** monitor these threads or set their priority.  This option allows the user
** to pass a function of type SaHpiCreateThreadFuncT with the PointerVal field;
** if this option is specified, the library will use only this function to create
** threads.
**
** When the function of type SaHpiCreateThreadFuncT is called,
** it should create a new thread, and from that thread, call the function
** specified by the StartFunction parameter, and providing in this call
** the FunctionData parameter.
** The return value of StartFunction is not used; it is provided only for
** compatibility with some operating systems.
**
** All threads will return when SaHpiShutdown() is called.  Threads may return at
** any other time; this is not an error.
**
** This option cannot fail, so it has no error codes.
*/
#define SA_HPI_INITOPTION_HANDLE_CREATE_THREAD    1

typedef SaErrorT (*SaHpiCreateThreadFuncT)(void *(*StartFunction)(void *),
                                           void *FunctionData);

/*******************************************************************************
**
** Name: saHpiVersionGet()
**
** Description:
**   This function returns the version identifier of the SaHpi specification
**   version supported by the HPI implementation.
**
** Parameters:
**   None.
**
** Return Value:
**   The interface version identifier, of type SaHpiVersionT is returned.
**
** Remarks:
**   This function differs from all other interface functions in that it returns
**   the version identifier rather than a standard return code. This is because
**   the version itself is necessary in order for an HPI User to properly
**   interpret subsequent API return codes.  Thus, the saHpiVersionGet()
**   function returns the interface version identifier unconditionally.
**
**   This function returns the value of the header file symbol
**   SAHPI_INTERFACE_VERSION in the SaHpi.h header file used when the library
**   was compiled.  An HPI User may compare the returned value to the
**   SAHPI_INTERFACE_VERSION symbol in the SaHpi.h header file used by the
**   calling program to determine if the accessed library is compatible with the
**   calling program.
**
*******************************************************************************/
SaHpiVersionT SAHPI_API saHpiVersionGet ( void );

/*******************************************************************************
**
** Name: saHpiInitialize()
**
** Description:
**   This function allows an HPI User to initialize an HPI library.  
**   The usage of this function is not mandatory; however, if it is used, 
**   it must be invoked when the library is in an initial state, that is, before 
**   any other HPI function has been called or after saHpiFinalize() has been 
**   called. This function allows aspects of the HPI library to be controlled.  
**   The options themselves are defined in Section 8.29.
**
** Parameters:
**   RequestedVersion – [in] The version of the specification to which the 
**      HPI User complies.
**   NumOptions – [in] The number of options in the Options array.
**   Options – [in/out] An array of option information.  
**      The user passes option-specified information in this field if the option 
**      requires it. If information is returned to the user, it is also done 
**      through this field. If no options are required or supplied, this 
**      parameter may be NULL.
**   FailedOption – [out] If this function returns SA_ERR_HPI_INVALID_DATA, 
**      this parameter will be set to the index of the first invalid option. 
**      Passing in NULL is valid. In this case, the function will operate 
**      normally, but if the function returns SA_ERR_HPI_INVALID_DATA, the 
**      user will not know which option failed.
**   OptionError – [out] If this function returns SA_ERR_HPI_INVALID_DATA, this 
**      parameter will be set to an option-specific error code.  Passing in NULL 
**      is valid. In this case; the function will operate normally, but if the 
**      function returns SA_ERR_HPI_INVALID_DATA, the user will not know the 
**      specific reason for the failure.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is 
**      returned.
**   SA_ERR_HPI_UNSUPPORTED_API is returned if the HPI Library does not 
**      support the RequestedVersion.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Options array is passed in as 
**      NULL and NumOptions is not zero.
**   SA_ERR_HPI_INVALID_DATA is returned if an option has an invalid OptionID, 
**      or if a parameter in an option is not valid.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the HPI library is not in initial
**      state.
**
** Remarks:
**   It is not mandatory for the HPI User to call this function before using the 
**   HPI library. If the function is not called, the library will make reasonable 
**   assumptions for the options available.
**
**   If this function returns an error, the library’s state will not be modified.
**
**   The FailedOption and OptionError parameters will be updated if the function 
**   returns SA_ERR_HPI_INVALID_PARAMS; otherwise, the library will not modify
**   FailedOption or OptionError. This overrides the rule described in Section 
**   4.3.
**
**   The RequestedVersion parameter does not have to match the version returned 
**   by saHpiVersionGet(). Because all versions of the specification with the 
**   same compatibility level identifier are backward compatible, the 
**   RequestedVersion may be any earlier version of the specification with the 
**   same compatibility level, and the library shall accept the version. For 
**   example, if the library supports B.03.01, it shall accept users specifying
**   a RequestedVersion of B.01.01 or B.02.01.
**
**   An HPI implementation may accept a RequestedVersion that is not backward 
**   compatible with the version returned by saHpiVersionGet(). If the library 
**   does this, all API functions shall work as specified in the 
**   RequestedVersion, including semantics and data structure layout. Using API
**   functions, constants, or semantics defined in versions later than the 
**   RequestedVersion may result in undefined behavior.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiInitialize (
	SAHPI_IN    SaHpiVersionT        RequestedVersion,
	SAHPI_IN    SaHpiUint32T         NumOptions,
	SAHPI_INOUT SaHpiInitOptionT     *Options,
	SAHPI_OUT   SaHpiUint32T         *FailedOption,
	SAHPI_OUT   SaErrorT             *OptionError
);

/*******************************************************************************
**
** Name: saHpiFinalize()
**
** Description:
**   This function allows an HPI User to return the library to its initial state.
**
** Parameters:
**   None.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is 
**     returned.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the HPI library is already in initial 
**     state.
**
** Remarks:
**   Calling this function is not mandatory. The HPI User can just shut down without 
**   calling it.
**
**   This function will return the library to the initial state at startup.  
**   All sessions will be closed by this function.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFinalize ( void );

/*******************************************************************************
**
** Name: saHpiSessionOpen()
**
** Description:
**   This function opens an HPI session for a given domain and set of security
**   characteristics (future).
**
** Parameters:
**   DomainId - [in] Domain identifier of the domain to be accessed by the HPI
**      User. A domain identifier of SAHPI_UNSPECIFIED_DOMAIN_ID requests that a
**      session be opened to a default domain.
**   SessionId - [out] Pointer to a location to store an identifier for the
**      newly opened session. This identifier is used for subsequent access to
**      domain resources and events.
**   SecurityParams - [in] Pointer to security and permissions data structure.
**      This parameter is reserved for future use, and must be set to NULL.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_DOMAIN is returned if no domain matching the specified
**      domain identifier exists.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * A non-null SecurityParams pointer is passed.
**      * The SessionId pointer is passed in as NULL.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if no more sessions can be opened.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSessionOpen (
    SAHPI_IN  SaHpiDomainIdT      DomainId,
    SAHPI_OUT SaHpiSessionIdT     *SessionId,
    SAHPI_IN  void                *SecurityParams
);

/*******************************************************************************
**
** Name: saHpiSessionClose()
**
** Description:
**   This function closes an HPI session. After closing a session, the SessionId
**   is no longer valid.
**
** Parameters:
**   SessionId - [in] Session identifier previously obtained using
**      saHpiSessionOpen().
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSessionClose (
    SAHPI_IN SaHpiSessionIdT SessionId
);

/*******************************************************************************
**
** Name: saHpiDiscover()
**
** Description:
**   This function requests the HPI implementation service to update information
**   about resources and domains that have recently been added to the system or
**   removed from the system. This function also updates RPT entries for 
**   resources that have changed 'failed' status. 
**
**   An HPI implementation may exhibit latency between when
**   hardware changes occur and when the domain DRT and RPT are updated.  To
**   overcome this latency, the saHpiDiscover() function may be called.  When
**   this function returns, the DRT and RPT should be updated to reflect the
**   current system configuration and status.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**
** Remarks:
**   The saHpiDiscover function is only meant to overcome latencies between
**   actual hardware changes and the time it takes for them to get reflected
**   into the DRT and RPT, and it should be used as such.
**
**   It should be noted that the RPT is not dependent on the saHpiDiscover
**   function to get populated and it is not required to call this function to
**   discover Resources in a Domain after opening a Session. The HPI User is
**   expected follow the procedure described in Section 3.5 - Discovery to
**   gather information about the Domain after a Session has been opened.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDiscover (
    SAHPI_IN SaHpiSessionIdT SessionId  
);

/*******************************************************************************
**
** Name: saHpiDomainInfoGet()
**
** Description:
**   This function is used for requesting information about the domain, the
**   Domain Reference Table (DRT), the Resource Presence Table (RPT), and the
**   Domain Alarm Table (DAT), such as table update counters and timestamps, and
**   the unique domain identifier associated with the domain.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   DomainInfo - [out] Pointer to the information describing the domain and
**      DRT.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the DomainInfo pointer is passed
**      in as NULL.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDomainInfoGet (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_OUT SaHpiDomainInfoT     *DomainInfo
);

/*******************************************************************************
**
** Name: saHpiDrtEntryGet()
**
** Description:
**   This function retrieves domain information for the specified entry of the
**   DRT. This function allows an HPI User to read the DRT entry-by-entry.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   EntryId - [in] Identifier of the DRT entry to retrieve. Reserved EntryId
**      values:
**         * SAHPI_FIRST_ENTRY  Get first entry
**         * SAHPI_LAST_ENTRY  	Reserved as delimiter for end of list. Not a
**            valid entry identifier.
**   NextEntryId - [out] Pointer to location to store the EntryId of next entry
**      in DRT.
**   DrtEntry - [out] Pointer to the structure to hold the returned DRT entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * Entry identified by EntryId is not present.
**      * EntryId is SAHPI_FIRST_ENTRY and the DRT is empty.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * DrtEntry pointer is passed in as NULL.
**      * NextEntryId pointer is passed in as NULL.
**      * EntryId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**
** Remarks:
**   If the EntryId parameter is set to SAHPI_FIRST_ENTRY, the first entry in
**   the DRT is returned. When an entry is successfully retrieved, NextEntryId
**   is set to the identifier of the next valid entry; however, when the last
**   entry has been retrieved, NextEntryId is set to SAHPI_LAST_ENTRY. To
**   retrieve an entire list of entries, call this function first with an
**   EntryId of SAHPI_FIRST_ENTRY and then use the returned NextEntryId in the
**   next call. Proceed until the NextEntryId returned is SAHPI_LAST_ENTRY.
**
**   If an HPI User has not subscribed to receive events and a DRT entry is
**   added while the DRT is being read, that new entry may be missed.  The
**   update counter provides a means for insuring that no domains are missed
**   when stepping through the DRT. To use this feature, an HPI User should call
**   saHpiDomainInfoGet() to get the update counter value before retrieving the
**   first DRT entry. After reading the last entry, the HPI User should again
**   call saHpiDomainInfoGet() to get the update counter value. If the update
**   counter has not been incremented, no new entries have been added.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDrtEntryGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiEntryIdT       EntryId,
    SAHPI_OUT SaHpiEntryIdT       *NextEntryId,
    SAHPI_OUT SaHpiDrtEntryT      *DrtEntry
);

/*******************************************************************************
**
** Name: saHpiDomainTagSet()
**
** Description:
**   This function allows an HPI User to set a descriptive tag for a particular
**   domain. The domain tag is an informational value that supplies an HPI User
**   with naming information for the domain.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   DomainTag - [in] Pointer to SaHpiTextBufferT containing the domain tag.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SaHpiTextBufferT structure
**      passed as DomainTag is not valid.  This would occur when:
**         * The DataType is not one of the enumerated values for that type, or
**         * The data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the DomainTag pointer is passed in
**      as NULL.
**
** Remarks:
**   The HPI implementation provides an appropriate default value for the domain
**   tag; this function is provided so that an HPI User can override the
**   default, if desired. The value of the domain tag may be retrieved from the
**   domain's information structure.
**
**   The domain tag is not necessarily persistent, and may return to its default
**   value if the domain controller function for the domain restarts.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDomainTagSet (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_IN  SaHpiTextBufferT     *DomainTag
);

/*******************************************************************************
**
** Name: saHpiRptEntryGet()
**
** Description:
**   This function retrieves resource information for the specified entry of the
**   resource presence table. This function allows an HPI User to read the RPT
**   entry-by-entry.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   EntryId - [in] Identifier of the RPT entry to retrieve. Reserved EntryId
**      values:
**         * SAHPI_FIRST_ENTRY 	Get first entry.
**         * SAHPI_LAST_ENTRY  	Reserved as delimiter for end of list. Not a
**            valid entry identifier.
**   NextEntryId - [out] Pointer to location to store the EntryId of next entry
**      in RPT.
**   RptEntry - [out] Pointer to the structure to hold the returned RPT entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_NOT_PRESENT is returned when the:
**      * Entry identified by EntryId is not present.
**      * EntryId is SAHPI_FIRST_ENTRY and the RPT is empty.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * RptEntry pointer is passed in as NULL.
**      * NextEntryId pointer is passed in as NULL.
**      * EntryId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**
** Remarks:
**   If the EntryId parameter is set to SAHPI_FIRST_ENTRY, the first entry in
**   the RPT is returned. When an entry is successfully retrieved, NextEntryId
**   is set to the identifier of the next valid entry; however, when the last
**   entry has been retrieved, NextEntryId is set to SAHPI_LAST_ENTRY. To
**   retrieve an entire list of entries, call this function first with an
**   EntryId of SAHPI_FIRST_ENTRY and then use the returned NextEntryId in the
**   next call. Proceed until the NextEntryId returned is SAHPI_LAST_ENTRY.
**
**   At initialization, an HPI User may not wish to receive HPI events, since
**   the context of the events, as provided by the RPT, is not known. In this
**   instance, if a FRU is inserted into the system while the RPT is being read
**   entry by entry, the resource associated with that FRU may be missed. (Keep
**   in mind that there is no specified ordering for the RPT entries.)  The
**   update counter provides a means for insuring that no resources are missed
**   when stepping through the RPT. To use this feature, an HPI User should
**   invoke saHpiDomainInfoGet(), and get the update counter value before
**   retrieving the first RPT entry. After reading the last entry, an HPI User
**   should again invoke the saHpiDomainInfoGet(). If the update counter has not
**   changed, no new records have been added.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRptEntryGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiEntryIdT       EntryId,
    SAHPI_OUT SaHpiEntryIdT       *NextEntryId,
    SAHPI_OUT SaHpiRptEntryT      *RptEntry
);

/*******************************************************************************
**
** Name: saHpiRptEntryGetByResourceId()
**
** Description:
**   This function retrieves resource information from the resource presence
**   table for the specified resource using its ResourceId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   RptEntry  - [out] Pointer to structure to hold the returned RPT entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the RptEntry pointer is passed in
**      as NULL.
**
** Remarks:
**   Typically at start-up, the RPT is read entry-by-entry, using
**   saHpiRptEntryGet(). From this, an HPI User can establish the set of
**   ResourceIds to use for future calls to the HPI functions.
**
**   However, there may be other ways of learning ResourceIds without first
**   reading the RPT. For example, resources may be added to the domain while
**   the system is running in response to a hot swap action. When a resource is
**   added, the application receives a hot swap event containing the ResourceId
**   of the new resource. The application may then want to search the RPT for
**   more detailed information on the newly added resource. In this case, the
**   ResourceId can be used to locate the applicable RPT entry information.
**
**   Note that saHpiRptEntryGetByResourceId() is valid in any hot swap state,
**   except for SAHPI_HS_STATE_NOT_PRESENT.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRptEntryGetByResourceId (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_IN  SaHpiResourceIdT     ResourceId,
    SAHPI_OUT SaHpiRptEntryT       *RptEntry
);

/*******************************************************************************
**
** Name: saHpiResourceSeveritySet()
**
** Description:
**   This function allows an HPI User to set the severity level applied to an
**   event issued if a resource unexpectedly becomes unavailable to the HPI. A
**   resource may become unavailable for several reasons including:
**   The FRU associated with the resource is no longer present in the system (a
**   surprise extraction has occurred.)
**   A catastrophic failure has occurred.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   Severity - [in] Severity level of event issued when the resource
**      unexpectedly becomes unavailable to the HPI.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the value for Severity is not
**      one of the valid enumerated values for this type, or it is equal to
**      SAHPI_ALL_SEVERITIES.
**
** Remarks:
**   Since the resource severity is contained within an RPT entry, its scope is
**   limited to a single domain.  A resource that exists in more than one domain
**   has independent resource severity values within each domain.
**
**   The HPI implementation provides a default value for the resource severity,
**   which may vary by resource, when an RPT entry is added to a domain; an HPI
**   User can override this default value by use of this function.
**
**   If a resource is removed from, then re-added to the RPT (e.g., because of a
**   hot swap action), the HPI implementation may reset the value of the
**   resource severity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceSeveritySet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_IN  SaHpiSeverityT      Severity
);

/*******************************************************************************
**
** Name: saHpiResourceTagSet()
**
** Description:
**   This function allows an HPI User to set the resource tag of an RPT entry
**   for a particular resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   ResourceTag - [in] Pointer to SaHpiTextBufferT containing the resource tag.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SaHpiTextBufferT structure
**      passed as ResourceTag is not valid.  This would occur when:
**      * The DataType is not one of the enumerated values for that type, or 
**      * The data field contains characters that are not legal according to the
**         value of DataType, or
**      * The Language is not one of the enumerated values for that type when
**         the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the ResourceTag pointer is passed
**      in as NULL.
**
** Remarks:
**   The resource tag is a data field within an RPT entry available to an HPI
**   User for associating application specific data with a resource.  HPI User
**   supplied data is purely informational and is not used by the HPI
**   implementation, domain, or associated resource.  For example, an HPI User
**   can set the resource tag to a "descriptive" value, which can be used to
**   identify the resource in messages to a human operator.
**
**   Since the resource tag is contained within an RPT entry, its scope is
**   limited to a single domain.  A resource that exists in more than one domain
**   has independent resource tags within each domain.
**
**   The HPI implementation provides a default value for the resource tag when
**   an RPT entry is added to a domain; this function is provided so that an HPI
**   User can override the default, if desired. The value of the resource tag
**   may be retrieved from the resource's RPT entry.
**
**   If a resource is removed from, then re-added to the RPT (e.g., because of a
**   hot swap action), the HPI implementation may reset the value of the
**   resource tag.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceTagSet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_IN  SaHpiTextBufferT    *ResourceTag
);

/*******************************************************************************
**
** Name: saHpiMyEntityPathGet()
**
** Description:
**   This function returns the EntityPath of the entity upon which the HPI User 
**   is running.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   EntityPath – [out] Pointer to location to hold the returned EntityPath. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the EntityPath pointer is passed
**      in as NULL.
**   SA_ERR_HPI_UNKNOWN is returned if the appropriate EntityPath to return 
**      cannot be determined.
**
** Remarks:
**   To support user authentication, an HPI User must have an open session to 
**   call this function, but the entity path returned will be the same 
**   regardless of which domain is associated with the session.  Thus, it is 
**   possible that the entity path returned may not be manageable through the 
**   domain associated with the passed SessionId.  When there are multiple 
**   domains, an HPI User may call saHpiMyEntityPathGet() using a session open 
**   to any domain to get the entity path, then call saHpiGetIdByEntityPath() on
**   all available domains to find management capabilities for that entity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiMyEntityPathGet (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_OUT SaHpiEntityPathT     *EntityPath
);

/*******************************************************************************
**
** Name: saHpiResourceIdGet()
**
** Note: 
**   This function should not be used in new HPI User programs. 
**   The function saHpiMyEntityPathGet() should be used instead.
**
** Description:
**   This function returns the ResourceId of the resource associated with the
**   entity upon which the HPI User is running.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [out] Pointer to location to hold the returned ResourceId.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the ResourceId pointer is passed
**      in as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the entity the HPI User is running on
**      is not manageable in the addressed domain.
**   SA_ERR_HPI_UNKNOWN is returned if the domain controller cannot determine an
**      appropriate response. That is, there may be an appropriate ResourceId in
**      the domain to return, but it cannot be determined.
**
** Remarks:
**   This function must be issued within a session to a domain that includes a
**   resource associated with the entity upon which the HPI User is running, or
**   the SA_ERR_HPI_NOT_PRESENT return is issued.
**
**   Since entities are contained within other entities, there may be multiple
**   possible resources that could be returned to this call. For example, if
**   there is a ResourceId associated with a particular compute blade upon which
**   the HPI User is running, and another associated with the chassis which
**   contains the compute blade, either could logically be returned as an
**   indication of a resource associated with the entity upon which the HPI User
**   was running. The function should return the ResourceId of the "smallest"
**   resource that is associated with the HPI User. So, in the example above,
**   the function should return the ResourceId of the compute blade.
**
**   If there are multiple resources that could be returned, each with the same
**   entity path, it is implementation specific which one is selected.  However,
**   in making the selection, the implementation should consider what management
**   capabilities are associated with each resource.  Preference may be given,
**   for example, to a resource that provides a Watchdog Timer or managed hot
**   swap capability for the entity, as these are capabilities that are likely
**   to be of particular interest to HPI Users running on the entity itself.
**
**   Once the function has returned the ResourceId, the HPI User may issue
**   further HPI calls using that ResourceId to learn the type of resource that
**   been identified.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceIdGet (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_OUT SaHpiResourceIdT     *ResourceId
);

/*******************************************************************************
**
** Name: saHpiGetIdByEntityPath()
**
** Description:
**   This function, given an entity path, retrieves the ResourceId of a Resource
**   that provides management access to the indicated entity. It can also be
**   used to find a management instrument (Sensor, Control, DIMI, FUMI, etc.)
**   associated with the entity if the instrument type is provided.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   EntityPath - [in] EntityPath of entity for which resources or management
**      instruments are being searched.
**   InstrumentType - [in] Type of management instrument to find. The instrument
**      type is designated by setting this value to the corresponding RDR type.
**      May be set to SAHPI_NO_RECORD, to search for resources that provide any
**      management access to the entity.  See Remarks for more information.
**   InstanceId - [in/out] Pointer to the instance number of the resource or
**      management instrument associated with the entity path to be returned. On
**      return, this value is updated to the instance number of the next
**      resource or management instrument associated with the entity path, or to
**      SAHPI_LAST_ENTRY if there are no more.  Reserved InstanceId values:
**   SAHPI_FIRST_ENTRY 	Get first match that is associated with EntityPath and
**      InstrumentType.
**   SAHPI_LAST_ENTRY  	Reserved as delimiter for end of list. Not a valid entry
**      identifier.
**   ResourceId - [out] Pointer to a location to store the ResourceId of a
**      resource found that provides management access to the designated entity.
**      If InstrumentType is not SAHPI_NO_RECORD then a resource is selected
**      only if it contains a management instrument of the appropriate type for
**      the designated entity.
**   InstrumentId - [out] Pointer to a location to store the Instrument Id
**      (Sensor number, Control number, etc.) of a management instrument of the
**      selected type that provides management access to the designated entity.
**      Ignored if InstrumentType is SAHPI_NO_RECORD.
**   RptUpdateCount - [out] Pointer to a location to store the current value of
**      the RptUpdateCount field from the domain info data for the domain.  This
**      value is stored when the function returns either SA_OK or
**      SA_ERR_HPI_NOT_PRESENT.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * InstanceId pointer is passed in as NULL.
**      * ResourceId pointer is passed in as NULL.
**      * InstanceId points to an invalid reserved value such as
**         SAHPI_LAST_ENTRY.
**      * InstrumentId pointer is passed in as NULL and InstrumentType is not
**         SAHPI_NO_RECORD.
**      * RptUpdateCount pointer is passed in as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if there is not an appropriate
**      ResourceId or InstrumentId that can be returned based on the passed
**      input parameters.  See Remarks for more information.
**
** Remarks:
**   This function can be used two ways. 
**   If InstrumentType is set to SAHPI_NO_RECORD, then it returns the ResourceId
**   of a resource that provides any management access to the requested entity.
**   A resource provides management access to an entity if any management
**   instrument contained in that resource is associated with the entity, or if
**   the resource's other (non-management-instrument) capabilities are
**   associated with the entity. In other words, given an entity path, a
**   resource is found to provide management access to the entity if the entity
**   path in the RPT entry for the resource matches, or if the entity path in
**   any RDR contained in the resource matches.  Entity paths must match
**   exactly.
**
**   If InstrumentType is set to indicate a particular type of management
**   instrument, then the function returns both the ResourceId and InstrumentId
**   (e.g., Sensor number, Control number, DIMI number, FUMI number, etc.) for a
**   management instrument of the requested type that provides management access
**   to the requested entity.  Entity paths must match exactly.  In this case,
**   it only returns the ResourceId of a resource that has a management
**   instrument of the requested type associated with the requested entity, and
**   it may return the same ResourceId several times, each time with a different
**   InstrumentId.
**
**   For any entity, it is possible that there are multiple resources that
**   provide management access, or that there are multiple management
**   instruments of the requested type in one or more resources providing
**   management access.  To retrieve all appropriate ResourceId values or all
**   appropriate ResourceId/InstrumentId pairs, multiple calls to
**   saHpiGetIdByEntityPath() must be made.  The first call should set
**   InstanceId  to SAHPI_FIRST_ENTRY. On subsequent calls, the InstanceId value
**   set upon return can be passed as input on a subsequent call to get the next
**   appropriate ResourceId or ResourceId/InstrumentId pair.  When the last
**   appropriate ResourceId or ResourceId/InstrumentId pair is returned, the
**   returned InstanceId value is SAHPI_LAST_ENTRY.  The order in which
**   ResourceId or ResourceId/InstrumentId pairs are returned is implementation
**   specific.
**
**   It is possible that resources may be added to or removed from a domain
**   between successive calls to this function. As a result, the InstanceId
**   returned in a previous call to this function may become invalid. The
**   RptUpdateCount is provided to notify the HPI User of this condition. The
**   returned RptUpdateCount value should be checked against the last returned
**   value every time this function is called with InstanceId other than
**   SAHPI_FIRST_ENTRY. If it differs, then the RPT has been updated and the
**   search should be restarted with InstanceId set to SAHPI_FIRST_ENTRY. The
**   HPI User should also check RptUpdateCount when SA_ERR_HPI_NOT_PRESENT is
**   returned while calling this function with InstanceId set to a value other
**   than SAHPI_FIRST_ENTRY.  If it has changed from the previous call, the
**   error return is probably due to the change in the RPT, and the search
**   process should be restarted.
**
**   The values returned by this function for ResourceId, InstrumentId, and
**   InstanceId are only valid if:
**   a)the function was called with InstanceId equal to SAHPI_FIRST_ENTRY, or 
**   b)the function was called with InstanceId equal to the value returned by a
**   previous call to the function on the same session, passing the same values
**   for EntityPath and InstrumentType, and with the same value for
**   RptUpdateCount returned on both calls.
**
**   In all other cases, even if the function returns SA_OK, the values returned
**   for ResourceId, InstrumentId, and InstanceId are not valid, and should be
**   ignored.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiGetIdByEntityPath (
    SAHPI_IN  	SaHpiSessionIdT     	SessionId,
	SAHPI_IN	SaHpiEntityPathT	EntityPath,
	SAHPI_IN	SaHpiRdrTypeT		InstrumentType,
	SAHPI_INOUT	SaHpiUint32T		*InstanceId,
	SAHPI_OUT	SaHpiResourceIdT	*ResourceId,
	SAHPI_OUT	SaHpiInstrumentIdT	*InstrumentId,
	SAHPI_OUT	SaHpiUint32T		*RptUpdateCount
);

/*******************************************************************************
**
** Name: saHpiGetChildEntityPath()
**
** Description:
**   This function retrieves entities that are contained within a given entity.
**   The entity path of a containing entity ("parent") is passed to the function
**   and the entity path of a contained entity ("child"), if available, is
**   returned.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ParentEntityPath - [in] EntityPath of parent entity. 
**   InstanceId - [in/out] Pointer to the instance number of the child entity
**      path to be returned. On return, this value is updated to the instance
**      number of the next child entity path for the passed parent entity path,
**      or to SAHPI_LAST_ENTRY if there are no more. Reserved InstanceId values:
**   SAHPI_FIRST_ENTRY 	Get first child entity for entity described by
**      ParentEntityPath.
**   SAHPI_LAST_ENTRY  	Reserved as delimiter for end of list. Not a valid entry
**      identifier.
**   ChildEntityPath - [out] Pointer to the entity path of a child entity.
**   RptUpdateCount - [out] Pointer to a location to store the current value of
**      the RptUpdateCount field from the domain info data for the domain.  This
**      value is stored when the function returns SA_OK,
**      SA_ERR_HPI_INVALID_DATA, or SA_ERR_HPI_NOT_PRESENT.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * InstanceId pointer is passed in as NULL.
**      * InstanceId points to an invalid reserved value such as
**         SAHPI_LAST_ENTRY.
**      * RptUpdateCount pointer is passed in as NULL.
**   SA_ERR_HPI_INVALID_DATA is returned if the entity represented by
**      ParentEntityPath is not present in the current Domain Entity Tree.
**   SA_ERR_HPI_NOT_PRESENT is returned when:
**      * InstanceId is SAHPI_FIRST_ENTRY and no child entity for entity
**         described by ParentEntityPath is present.
**      * InstanceId is not SAHPI_FIRST_ENTRY and no child entity
**         corresponding to InstanceId for entity described by
**         ParentEntityPath is present.
**
** Remarks:
**   This function provides access to the Domain Entity Tree for the domain
**   associated with the open SessionId.  Passed an entity path that identifies
**   a node in the Domain Entity Tree, the function returns entity paths that
**   identify the immediate children of that node; that is, the entity paths for
**   entities in the Domain Entity Tree that are immediately contained by the
**   entity whose entity path was passed as ParentEntityPath.
**
**   For any entity path, it is possible that there are multiple immediate
**   children in the Domain Entity Tree.  To retrieve the entity paths for all
**   children or a particular ParentEntityPath, multiple calls to
**   saHpiGetChildEntityPath() must be made.  On each call, the InstanceId value
**   set upon return can be passed as input on a subsequent call to get the
**   entity path for the next child.  When the entity path for the last child of
**   the passed ParentEntityPath is returned, then InstanceId is set to
**   SAHPI_LAST_ENTRY.  The order in which ChildEntityPath values are returned
**   is implementation specific.
**
**   To retrieve the entire Domain Entity Tree, an HPI User can start by calling
**   this function with an entity path containing only { { SAHPI_ROOT, 0 } } to
**   retrieve children of the root node, then calling the function recursively
**   for each of those children, and so forth.
**
**   It is possible that resources may be added to or removed from a domain
**   between successive calls to this function. As a result, the Domain Entity
**   Tree may change, invalidating data previously returned. The RptUpdateCount
**   is provided to notify the HPI User of this condition. The returned
**   RptUpdateCount value should be checked against the last returned value
**   every time this function is called with InstanceId other than
**   SAHPI_FIRST_ENTRY. If it differs, then the RPT has been updated and the
**   entire sequence of calls should be restarted. The HPI User should also
**   check RptUpdateCount when SA_ERR_HPI_INVALID_DATA or SA_ERR_HPI_NOT_PRESENT
**   is returned while calling this function with InstanceId set to a value
**   other than SAHPI_FIRST_ENTRY.  If it has changed from the previous call,
**   the error return is probably due to the change in the RPT, and the search
**   entire sequence of calls should be restarted.
**
**   The values returned by this function for ChildEntityPath, and InstanceId
**   are only valid if:
**   a)the function was called with InstanceId equal to SAHPI_FIRST_ENTRY, or 
**   b)the function was called with InstanceId equal to the value returned by a
**   previous call to the function on the same session, passing the same value
**   for ParentEntityPath, and with the same value for RptUpdateCount returned
**   on both calls.
**   In all other cases, even if the function returns SA_OK, the values returned
**   for ChildEntityPath and InstanceId are not valid, and should be ignored.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiGetChildEntityPath (
    SAHPI_IN  	SaHpiSessionIdT     	SessionId,
    SAHPI_IN	SaHpiEntityPathT	ParentEntityPath,
    SAHPI_INOUT	SaHpiUint32T		*InstanceId,
    SAHPI_OUT	SaHpiEntityPathT	*ChildEntityPath,
    SAHPI_OUT	SaHpiUint32T		*RptUpdateCount
);

/*******************************************************************************
**
** Name: saHpiResourceFailedRemove()
**
** Description:
**   This function allows an HPI User to remove an RPT entry for a failed
**   resource from the domain containing it. It can be used to remove the RPT
**   entry for a failed resource associated with a FRU that is not functional or
**   has lost management access. If the HPI User acquires knowledge that the FRU
**   associated with such a resource has been physically removed, then this
**   function provides the means to update the RPT so that it no longer contains
**   an entry for that resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**
** Return value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the ResourceId pointer is passed
**      in as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if an entry for the resource identified
**      by ResourceId is not present in the RPT.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the ResourceFailed flag in the
**      resource's RPT entry is not set to True.
**   SA_ERR_HPI_INVALID_CMD is returned if the resource is not associated with a
**      FRU, as indicated by SAHPI_CAPABILITY_FRU in the ResourceCapabilities
**      field of the resource's RPT entry.
**
** Remarks:
**   This function can be used only on resources marked as inaccessible in the 
**   RPT. Resources that do not have the ResourceFailed flag in their RPT entry
**   set to True cannot be removed by this function.  For more information on 
**   failed resources, see Section 3.8.
**
**   When the HPI User calls this function for a failed resource, the HPI 
**   implementation removes the RPT entry for the failed resource and issues an 
**   event.  The event issued depends on whether the resource has the FRU 
**   capability set indicating that it provides hot swap management.  If it is a 
**   FRU resource, a “user update” hot swap event is issued for transition of 
**   the resource to the SAHPI_HS_STATE_NOT_PRESENT state from its last known 
**   state.  If it is not a FRU resource, a “Resource Removed” event is issued.
**
**   If a FRU resource is removed as the result of an HPI User calling this 
**   function, any nested resources are also removed from the RPT, and 
**   appropriate events are issued for those nested resources.  For more 
**   information on nested resources, see Section 3.3.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceFailedRemove (
    SAHPI_IN    SaHpiSessionIdT        SessionId,
    SAHPI_IN    SaHpiResourceIdT       ResourceId
);

/*******************************************************************************
**
** Name: saHpiEventLogInfoGet()
**
** Description:
**   This function retrieves the current number of entries in the Event Log,
**   total size of the Event Log, the time of the most recent update to the
**   Event Log, the current value of the Event Log's clock (i.e., timestamp that
**   would be placed on an entry at this moment), the enabled/disabled status of
**   the Event Log (see Section 6.4.9), the overflow flag, and the action taken 
**   by the Event Log if an overflow occurs.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   Info - [out] Pointer to the returned Event Log information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Info pointer is passed in as
**      NULL.
**
** Remarks:
**   The size field in the returned Event Log information indicates the maximum
**   number of entries that can be held in the Event Log.  This number should be
**   constant for a particular Event Log.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogInfoGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_OUT SaHpiEventLogInfoT  *Info
);

/*******************************************************************************
**
** Name: saHpiEventLogCapabilitiesGet()
**
** Description:
**   This function retrieves a set of flags that indicate various optional
**   capabilities that may be supported by the event log.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   EventLogCapabilities - [out] Pointer to a location to store the Event Log
**      Capabilities value.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the EventLogCapabilities pointer
**      is passed in as NULL.
**
** Remarks:
**   The SaHpiEventLogCapabilitiesT type is defined as an unsigned 32-bit value.
**   Specific capabilities are defined as individual bits within this value.
**   For each defined bit value that is set to a "1", the event log supports the
**   corresponding capability.  For each defined bit value that is set to a "0",
**   the event log does not support the corresponding capability.  Undefined
**   bits in the capability value are reserved, and may be assigned to indicate
**   support for other capabilities in future versions of the specification.
**   See Section 8.28 for a definition of the event log capabilities and the
**   corresponding bits in the returned EventLogCapabilities value that indicate
**   support for each capability.
**
**   For backwards compatibility with the B.01.01 specification, the "Overflow
**   Resetable" capability, represented in the returned  EventLogCapabilities
**   value as SAHPI_EVTLOG_CAPABILITY_OVERFLOW_RESETABLE is also represented in
**   the Event Log Info record returned by saHpiEventLogInfoGet() function, in
**   the OverflowResetable Boolean field.  These two items should agree for a
**   particular event log.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogCapabilitiesGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_OUT SaHpiEventLogCapabilitiesT  *EventLogCapabilities
);

/*******************************************************************************
**
** Name: saHpiEventLogEntryGet()
**
** Description:
**   This function retrieves an Event Log entry.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   EntryId - [in] Identifier of event log entry to retrieve. Reserved values:
**      * SAHPI_OLDEST_ENTRY	Oldest entry in the Event Log.
**      * SAHPI_NEWEST_ENTRY	Newest entry in the Event Log.
**      * SAHPI_NO_MORE_ENTRIES	Not valid for this parameter. Used only when
**         retrieving the next and previous EntryIds.
**   PrevEntryId - [out] Event Log entry identifier for the previous (older
**      adjacent) entry.  Reserved values:
**      * SAHPI_OLDEST_ENTRY	Not valid for this parameter. Used only for the
**         EntryId parameter.
**      * SAHPI_NEWEST_ENTRY	Not valid for this parameter. Used only for the
**         EntryId parameter.
**      * SAHPI_NO_MORE_ENTRIES	No more entries in the Event Log before the one
**         referenced by the EntryId parameter.
**   NextEntryId - [out] Event Log entry identifier for the next (newer
**      adjacent) entry. Reserved values:
**      * SAHPI_OLDEST_ENTRY	Not valid for this parameter. Used only for the
**         EntryId parameter.
**      * SAHPI_NEWEST_ENTRY	Not valid for this parameter. Used only for the
**         EntryId parameter.
**      * SAHPI_NO_MORE_ENTRIES	No more entries in the Event Log after the one
**         referenced by the EntryId parameter.
**   EventLogEntry - [out] Pointer to retrieved Event Log entry.
**   Rdr - [in/out] Pointer to structure to receive resource data record
**      associated with the Event Log entry, if available. If NULL, no RDR data
**      is returned.
**   RptEntry - [in/out] Pointer to structure to receive RPT entry associated
**      with the Event Log entry, if available. If NULL, no RPT entry data is
**      returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_NOT_PRESENT is returned when:
**      * The Event Log has no entries.
**      * The entry identified by EntryId is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned when:
**      Any of PrevEntryId, NextEntryId and EventLogEntry pointers are passed in
**      as NULL.
**   SAHPI_NO_MORE_ENTRIES is passed in to EntryId.
**
** Remarks:
**   The special EntryIds SAHPI_OLDEST_ENTRY and SAHPI_NEWEST_ENTRY are used to
**   select the oldest and newest entries, respectively, in the Event Log being
**   read. A returned NextEntryId of SAHPI_NO_MORE_ENTRIES indicates that the
**   newest entry has been returned; there are no more entries going forward
**   (time-wise) in the Event Log. A returned PrevEntryId of
**   SAHPI_NO_MORE_ENTRIES indicates that the oldest entry has been returned.
**
**   To retrieve an entire list of entries going forward (oldest entry to newest
**   entry) in the Event Log, call this function first with an EntryId of
**   SAHPI_OLDEST_ENTRY and then use the returned NextEntryId as the EntryId in
**   the next call. Proceed until the NextEntryId returned is
**   SAHPI_NO_MORE_ENTRIES.
**
**   To retrieve an entire list of entries going backward (newest entry to
**   oldest entry) in the Event Log, call this function first with an EntryId of
**   SAHPI_NEWEST_ENTRY and then use the returned PrevEntryId as the EntryId in
**   the next call. Proceed until the PrevEntryId returned is
**   SAHPI_NO_MORE_ENTRIES.
**
**   Event Logs may include RPT entries and resource data records associated
**   with the resource and Sensor issuing an event along with the basic event
**   data in the Event Log. Because the system may be reconfigured after the
**   event was entered in the Event Log, this stored information may be
**   important to interpret the event. If the Event Log includes logged RPT
**   entries and/or RDRs, and if an HPI User provides a pointer to a structure
**   to receive this information, it is returned along with the Event Log entry.
**
**   If an HPI User provides a pointer for an RPT entry, but the Event Log does
**   not include a logged RPT entry for the Event Log entry being returned,
**   RptEntry->ResourceCapabilities is set to zero. No valid RptEntry has a zero
**   Capabilities field value.
**
**   If an HPI User provides a pointer for an RDR, but the Event Log does not
**   include a logged RDR for the Event Log entry being returned, Rdr->RdrType
**   is set to SAHPI_NO_RECORD.
**
**   The EntryIds returned via the PrevEntryId and NextEntryId parameters may
**   not be in sequential order, but will reflect the previous and next entries
**   in a chronological ordering of the Event Log, respectively.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogEntryGet (
    SAHPI_IN    SaHpiSessionIdT          SessionId,
    SAHPI_IN    SaHpiResourceIdT         ResourceId,
    SAHPI_IN    SaHpiEventLogEntryIdT    EntryId,
    SAHPI_OUT   SaHpiEventLogEntryIdT    *PrevEntryId,
    SAHPI_OUT   SaHpiEventLogEntryIdT    *NextEntryId,
    SAHPI_OUT   SaHpiEventLogEntryT      *EventLogEntry,
    SAHPI_INOUT SaHpiRdrT                *Rdr,
    SAHPI_INOUT SaHpiRptEntryT           *RptEntry
);

/*******************************************************************************
**
** Name: saHpiEventLogEntryAdd()
**
** Description:
**   This function enables an HPI user to add entries to the Event Log.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   EvtEntry - [in] Pointer to event data to write to the Event Log. The Event
**      field must be of type SAHPI_ET_USER, and the Source field must be
**      SAHPI_UNSPECIFIED_RESOURCE_ID.
**
** Return Value:
**   SA_OK is returned if the event is successfully written in the Event Log;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_CMD is returned if the addressed event log does not
**      support this function.
**   SA_ERR_HPI_INVALID_DATA is returned if the event DataLength is larger than
**      that supported by the implementation and reported in the Event Log info
**      record.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * EvtEntry pointer is passed in as NULL.
**      * Event structure passed via the EvtEntry parameter is not an event of
**         type SAHPI_ET_USER with the Source field set to
**         SAHPI_UNSPECIFIED_RESOURCE_ID.
**      * Severity field within the EvtEntry parameter is not one of the valid
**         enumerated values for this type, or it is equal to
**         SAHPI_ALL_SEVERITIES.
**      * SaHpiTextBufferT structure passed as part of the User Event structure
**         is not valid.  This would occur when:
**            * The DataType is not one of the enumerated values
**               for that type, or 
**            * The data field contains characters that are not legal according
**               to the value of DataType, or
**            * The Language is not one of the enumerated values for that type
**               when the DataType is SAHPI_TL_TYPE_UNICODE or
**               SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the event cannot be written to the
**      Event Log because the Event Log is full, and the Event Log
**      OverflowAction is SAHPI_EL_OVERFLOW_DROP.
**
** Remarks:
**   This function writes an event in the addressed Event Log. Nothing else is
**   done with the event.
**
**   If the Event Log is full, overflow processing occurs as defined by the
**   Event Log's OverflowAction setting, reported in the Event Log info record.
**   If, due to an overflow condition, the event is not written, or if existing
**   events are overwritten, then the OverflowFlag in the Event Log info record
**   is set to True, just as it would be if an internally generated event caused
**   an overflow condition.  If the Event Log's OverflowAction is
**   SAHPI_EL_OVERFLOW_DROP, then an error is returned (SA_ERR_HPI_OUT_OF_SPACE)
**   indicating that the saHpiEventLogEntryAdd() function did not add the event
**   to the Event Log.  If the Event Log's OverflowAction is
**   SAHPI_EL_OVERFLOW_OVERWRITE, then the saHpiEventLogEntryAdd() function
**   returns SA_OK, indicating that the event was added to the Event Log, even
**   though an overflow occurred as a side-effect of this operation.  The
**   overflow may be detected by checking the OverflowFlag in the Event Log info
**   record.
**
**   Specific implementations of HPI may have restrictions on how much data may
**   be passed to the saHpiEventLogEntryAdd() function.  The Event Log info
**   record reports the maximum DataLength that is supported by the Event Log
**   for User Events.  If saHpiEventLogEntryAdd() is called with a User Event
**   that has a larger DataLength than is supported, the event is  not added to
**   the Event Log, and an error is returned.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogEntryAdd (
    SAHPI_IN SaHpiSessionIdT     SessionId,
    SAHPI_IN SaHpiResourceIdT    ResourceId,
    SAHPI_IN SaHpiEventT         *EvtEntry
);

/*******************************************************************************
**
** Name: saHpiEventLogClear()
**
** Description:
**   This function erases the contents of the specified Event Log.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_CMD is returned if the addressed event log does not
**      support this function.
**
** Remarks:
**   The OverflowFlag field in the Event Log info record is reset when the event
**   log is cleared.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogClear (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId
);

/*******************************************************************************
**
** Name: saHpiEventLogTimeGet()
**
** Description:
**   This function retrieves the current time from the Event Log's clock. This
**   clock is used to timestamp entries written into the Event Log.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   Time - [out] Pointer to the returned current Event Log time.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Time pointer is passed in as
**      NULL.
**
** Remarks:
**   If the implementation cannot supply an absolute time value, then it may
**   supply a time relative to some system-defined epoch, such as system
**   startup. If the time value is less than or equal to
**   SAHPI_TIME_MAX_RELATIVE, then it is relative; if it is greater than
**   SAHPI_TIME_MAX_RELATIVE, then it is absolute. The HPI implementation must
**   provide valid timestamps for Event Log entries, using a default time base
**   if no time has been set.  Thus, the value SAHPI_TIME_UNSPECIFIED is never
**   returned.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogTimeGet (
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_IN  SaHpiResourceIdT     ResourceId,
    SAHPI_OUT SaHpiTimeT           *Time
);

/*******************************************************************************
**
** Name: saHpiEventLogTimeSet()
**
** Description:
**   This function sets the Event Log's clock, which is used to timestamp events
**   written into the Event Log.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   Time - [in] Time to which the Event Log clock should be set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_CMD is returned if the addressed event log does not
**      support this function.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Time parameter is set to
**      SAHPI_TIME_UNSPECIFIED.
**   For situations when the underlying implementation cannot represent a time
**      value that is specified in Time, SA_ERR_HPI_INVALID_DATA is returned.
**
** Remarks:
**   If the Time parameter value is less than or equal to
**   SAHPI_TIME_MAX_RELATIVE, but not SAHPI_TIME_UNSPECIFIED, then it is
**   relative; if it is greater than SAHPI_TIME_MAX_RELATIVE, then it is
**   absolute.  Setting this parameter to the value SAHPI_TIME_UNSPECIFIED is
**   invalid and results in an error return code of SA_ERR_HPI_INVALID_PARAMS.
**
**   Entries placed in the Event Log after this function is called will have
**   Event Log timestamps (i.e., the Timestamp field in the SaHpiEventLogEntryT
**   structure) based on the new time.  Setting the clock does not affect
**   existing Event Log entries.  If the time is set to a relative time,
**   subsequent entries placed in the Event Log will have an Event Log timestamp
**   expressed as a relative time; if the time is set to an absolute time,
**   subsequent entries will have an Event Log timestamp expressed as an
**   absolute time.
**
**   This function only sets the Event Log time clock and does not have any
**   direct bearing on the timestamps placed on events (i.e., the Timestamp
**   field in the SaHpiEventT structure), or the timestamps placed in the domain
**   RPT info record.  Setting the clocks used to generate timestamps other than
**   Event Log timestamps is implementation-dependent, and should be documented
**   by the HPI implementation provider.
**
**   Some underlying implementations may not be able to handle the same relative
**   and absolute time ranges, as those defined in HPI.  Such limitations should
**   be documented.  When a time value is set in a region that is not supported
**   by the implementation, an error code of SA_ERR_HPI_INVALID_DATA is
**   returned.  However, all HPI implementations must support setting the event
**   log time to relative times in the range of 0 to the longest time since
**   "startup" that is ever expected to be encountered and absolute times
**   representing current time throughout the expected life of the system.  See
**   Section  for more details on time ranges.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogTimeSet (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiResourceIdT     ResourceId,
    SAHPI_IN SaHpiTimeT           Time
);

/*******************************************************************************
**
** Name: saHpiEventLogStateGet()
**
** Description:
**   This function enables an HPI User to get the Event Log state.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   EnableState - [out] Pointer to the current Event Log enable state.  True
**      indicates that the Event Log is enabled; False indicates that it is
**      disabled.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the EnableState pointer is passed
**      in as NULL.
**
** Remarks:
**   If the Event Log is disabled, no events generated within the HPI
**   implementation are added to the Event Log. Events may still be added to the
**   Event Log with the saHpiEventLogEntryAdd() function. When the Event Log is
**   enabled, events may be automatically added to the Event Log as they are
**   generated in a resource or a domain, however, it is implementation-specific
**   which events are automatically added to any Event Log.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogStateGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_OUT SaHpiBoolT          *EnableState
);

/*******************************************************************************
**
** Name: saHpiEventLogStateSet()
**
** Description:
**   This function enables an HPI User to set the Event Log enabled state.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**   EnableState - [in] Event Log state to be set. True indicates that the Event
**      Log is to be enabled; False indicates that it is to be disabled.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_CMD is returned if the addressed event log does not
**      support this function.
**
** Remarks:
**   If the Event Log is disabled no events generated within the HPI
**   implementation are added to the Event Log. Events may still be added to the
**   Event Log using the saHpiEventLogEntryAdd() function. When the Event Log is
**   enabled events may be automatically added to the Event Log as they are
**   generated in a resource or a domain. The actual set of events that are
**   automatically added to any Event Log is implementation-specific.
**
**   The HPI implementation provides an appropriate default value for this
**   parameter, which may vary by resource. This function is provided so that an
**   HPI User can override the default, if desired.
**
**   If a resource hosting an Event Log is re-initialized (e.g., because of a
**   hot swap action), the HPI implementation may reset the value of this
**   parameter.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogStateSet (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiResourceIdT     ResourceId,
    SAHPI_IN SaHpiBoolT           EnableState
);

/*******************************************************************************
**
** Name: saHpiEventLogOverflowReset()
**
** Description:
**   This function clears the OverflowFlag in the Event Log info record of the
**   specified Event Log, that is it sets it to False.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Identifier for the Resource containing the Event Log.
**      Set to SAHPI_UNSPECIFIED_RESOURCE_ID to address the Domain Event Log.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not have an Event
**      Log capability (SAHPI_CAPABILITY_EVENT_LOG) set.  Note this condition
**      only applies to Resource Event Logs.  Domain Event Logs are mandatory,
**      and should not return this code.
**   SA_ERR_HPI_INVALID_CMD is returned if the addressed event log does not
**      support this function.
**
** Remarks:
**   The only effect of this function is to clear the OverflowFlag field in the
**   Event Log info record for the specified Event Log, that is, to set it to
**   False.  If the Event Log is still full, the OverflowFlag will be set to
**   True again as soon as another entry needs to be added to the Event Log.
**
**   Some Event Log implementations may not allow resetting of the OverflowFlag
**   except as a by-product of clearing the entire Event Log with the
**   saHpiEventLogClear() function, or at all.  Such an implementation returns
**   the error code, SA_ERR_HPI_INVALID_CMD to this function.  The
**   OverflowResetable flag in the Event Log info record and the
**   SAHPI_EVTLOG_CAPABILITY_OVERFLOW_RESETABLE bit in the value returned by
**   saHpiEventLogCapabilitiesGet() indicate whether or not the implementation
**   supports resetting the OverflowFlag with this function.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogOverflowReset (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId
);

/*******************************************************************************
**
** Name: saHpiSubscribe()
**
** Description:
**   This function allows an HPI User to subscribe for events. This single call
**   provides subscription to all session events, regardless of event type or
**   event severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_DUPLICATE is returned when a subscription is already in place
**      for this session.
**
** Remarks:
**   Only one subscription is allowed per session, and additional subscribers
**   receive an appropriate error code. No event filtering is done by the HPI
**   implementation.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSubscribe (
    SAHPI_IN SaHpiSessionIdT      SessionId
);

/*******************************************************************************
**
** Name: saHpiUnsubscribe()
**
** Description:
**   This function removes the event subscription for the session.
**
** Parameters:
**   SessionId - [in] Session for which event subscription is to be closed.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the session is not currently
**      subscribed for events.
**
** Remarks:
**   After removal of a subscription, additional saHpiEventGet() calls are not
**   allowed on the session unless an HPI User re-subscribes for events on the
**   session first. Any events that are still in the event queue when this
**   function is called are cleared from it.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiUnsubscribe (
    SAHPI_IN SaHpiSessionIdT  SessionId
);

/*******************************************************************************
**
** Name: saHpiEventGet()
**
** Description:
**   This function allows an HPI User to get an event.  This call is only valid
**   within a session that has subscribed for events.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   Timeout - [in] The number of nanoseconds to wait for an event to arrive.
**      Reserved time out values:
**      SAHPI_TIMEOUT_IMMEDIATE
**         Time out immediately if there are no events available (non-blocking
**         call).
**      SAHPI_TIMEOUT_BLOCK
**         Call should not return until an event is retrieved.
**   Event - [out] Pointer to the next available event.
**   Rdr - [in/out] Pointer to structure to receive the resource data associated
**      with the event.  If NULL, no RDR is returned.
**   RptEntry - [in/out] Pointer to structure to receive the RPT entry
**      associated with the resource that generated the event.  If NULL, no RPT
**      entry is returned.
**   EventQueueStatus - [in/out] Pointer to location to store event queue
**      status.  If NULL, event queue status is not returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_REQUEST is returned if an HPI User is not currently
**      subscribed for events in this session.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * Event pointer is passed in as NULL.
**      * Timeout parameter is not set to SAHPI_TIMEOUT_BLOCK,
**         SAHPI_TIMEOUT_IMMEDIATE or a positive value.
**   SA_ERR_HPI_TIMEOUT is returned if no event is available to return within
**      the timeout period.  If SAHPI_TIMEOUT_IMMEDIATE is passed in the Timeout
**      parameter, this error return is used if there is no event queued when
**      the function is called.
**
** Remarks:
**   SaHpiEventGet()also returns an EventQueueStatus flag to an HPI User.  This
**   flag indicates whether or not a queue overflow has occurred.  The overflow
**   flag is set if any events were unable to be queued because of space
**   limitations in the interface implementation.  The overflow flag is reset
**   whenever saHpiEventGet() is called.
**
**   If there are one or more events on the event queue when this function is
**   called, it immediately returns the next event on the queue.  Otherwise, if
**   the Timeout parameter is SAHPI_TIMEOUT_IMMEDIATE, it returns
**   SA_ERR_HPI_TIMEOUT immediately.  Otherwise, it blocks for the time
**   specified by the timeout parameter; if an event is added to the queue
**   within that time it is returned immediately when added; if not,
**   saHpiEventGet() returns SA_ERR_HPI_TIMEOUT at the end of the timeout
**   period.  If the Timeout parameter is SAHPI_TIMEOUT_BLOCK, the
**   saHpiEventGet()blocks indefinitely, until an event becomes available, and
**   then returns that event.  This provides for notification of events as they
**   occur.
**
**   If an HPI User provides a pointer for an RPT entry, but the event does not
**   include a valid ResourceId for a resource in the domain, then the
**   RptEntry->ResourceCapabilities field is set to zero.  No valid RPT entry
**   has a ResourceCapabilities field equal to zero.
**
**   If an HPI User provides a pointer for an RDR, but there is no valid RDR
**   associated with the event being returned, then the Rdr->RdrType field is
**   set to SAHPI_NO_RECORD. RDRs should be returned for events associated with
**   management instruments.   Event types that are associated with management
**   instruments are identified in Section 8.18.
**
**   The timestamp reported in the returned event structure is the best
**   approximation an implementation has to when the event actually occurred.
**   The implementation may need to make an approximation (such as the time the
**   event was placed on the event queue) because it may not have access to the
**   actual time the event occurred.  The value SAHPI_TIME_UNSPECIFIED indicates
**   that the time of the event cannot be determined.
**
**   If the implementation cannot supply an absolute timestamp, then it may
**   supply a timestamp relative to some system-defined epoch, such as system
**   startup.  If the timestamp value is less than or equal to
**   SAHPI_TIME_MAX_RELATIVE, but not SAHPI_TIME_UNSPECIFIED, then it is
**   relative; if it is greater than SAHPI_TIME_MAX_RELATIVE, then it is
**   absolute.
**
**   If an HPI User passes a NULL pointer for the returned EventQueueStatus
**   pointer, the event status is not returned, but the overflow flag, if set,
**   is still reset.  Thus, if an HPI User needs to know about event queue
**   overflows, the EventQueueStatus parameter should never be NULL, and the
**   overflow flag should be checked after every call to saHpiEventGet().
**
**   If saHpiEventGet() is called with a timeout value other than
**   SAHPI_TIMEOUT_IMMEDIATE, and the session is subsequently closed from
**   another thread, this function returns with SA_ERR_HPI_INVALID_SESSION.  If
**   saHpiEventGet() is called with a timeout value other than
**   SAHPI_TIMEOUT_IMMEDIATE, and an HPI User subsequently calls
**   saHpiUnsubscribe() from another thread, this function returns with
**   SA_ERR_HPI_INVALID_REQUEST.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventGet (
    SAHPI_IN SaHpiSessionIdT            SessionId,
    SAHPI_IN SaHpiTimeoutT              Timeout,
    SAHPI_OUT SaHpiEventT               *Event,
    SAHPI_INOUT SaHpiRdrT               *Rdr,
    SAHPI_INOUT SaHpiRptEntryT          *RptEntry,
    SAHPI_INOUT SaHpiEvtQueueStatusT    *EventQueueStatus
);

/*******************************************************************************
**
** Name: saHpiEventAdd()
**
** Description:
**   This function enables an HPI User to add events to the HPI domain
**   identified by the SessionId.  The domain controller processes an event
**   added with this function as if the event originated from within the domain.
**   The domain controller attempts to publish events to all active event
**   subscribers and may attempt to log events in the Domain Event Log, if room
**   is available.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   EvtEntry - [in] Pointer to event to add to the domain.  Event must be of
**      type SAHPI_ET_USER, and the Source field must be
**      SAHPI_UNSPECIFIED_RESOURCE_ID.
**
** Return Value:
**   SA_OK is returned if the event is successfully added to the domain;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * EvtEntry parameter is NULL.
**      * Event structure passed via the EvtEntry parameter is not an event of
**         type SAHPI_ET_USER with the Source field being
**         SAHPI_UNSPECIFIED_RESOURCE_ID.
**      * Severity field within the EvtEntry parameter is not one of the valid
**         enumerated values for this type, or it is equal to
**         SAHPI_ALL_SEVERITIES.
**      * SaHpiTextBufferT structure passed as part of the User Event structure
**         is not valid. This would occur when:
**            * The DataType is not one of the enumerated values
**               for that type, or 
**            * The data field contains characters that are not legal according
**               to the value of DataType, or
**            * The Language is not one of the enumerated values for that type
**               when the DataType is SAHPI_TL_TYPE_UNICODE or
**               SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_INVALID_DATA is returned if the event data does not meet
**      implementation-specific restrictions on how much event data may be
**      provided in a SAHPI_ET_USER event.
**
** Remarks:
**   Specific implementations of HPI may have restrictions on how much data may
**   be included in a SAHPI_ET_USER event.  If more event data is provided than
**   can be processed, an error is returned.  The event data size restriction
**   for the SAHPI_ET_USER event type is provided in the UserEventMaxSize field
**   in the domain Event Log info structure.  An HPI User should call the
**   function saHpiEventLogInfoGet() to retrieve the Event Log info structure.
**
**   The domain controller attempts to publish the event to all sessions within
**   the domain with active event subscriptions; however, a session's event
**   queue may overflow due to the addition of the new event.
**
**   The domain controller may attempt to log the event in the Domain Event Log;
**   however, because it is implementation-specific what events are added to
**   event logs, the domain controller is not required to add the event to the
**   Domain Event Log.  Also, an attempt to add the event to the Domain Event
**   Log may fail if the log overflows.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventAdd (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiEventT          *EvtEntry
);

/*******************************************************************************
**
** Name: saHpiAlarmGetNext()
**
** Description:
**   This function allows retrieval of an alarm from the current set of alarms
**   held in the Domain Alarm Table (DAT).
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   Severity - [in] Severity level of alarms to retrieve.  Set to
**      SAHPI_ALL_SEVERITIES to retrieve alarms of any severity; otherwise, set
**      to requested severity level.
**   UnacknowledgedOnly - [in] Set to True to indicate only unacknowledged
**      alarms should be returned.  Set to False to indicate either an
**      acknowledged or unacknowledged alarm may be returned.
**   Alarm - [in/out] Pointer to the structure to hold the returned alarm entry.
**      Also, on input, Alarm->AlarmId and Alarm->Timestamp are used to identify
**      the previous alarm.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned when:
**      * Severity is not one of the valid enumerated values for this type.
**      * The Alarm parameter is passed in as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned:
**      * If there are no additional alarms in the DAT that meet the criteria
**         specified by the Severity and UnacknowledgedOnly parameters.
**      * If the passed Alarm->AlarmId field was set to SAHPI_FIRST_ENTRY and
**         there are no alarms in the DAT that meet the criteria specified by
**         the Severity and UnacknowledgedOnly parameters.
**   SA_ERR_HPI_INVALID_DATA is returned if the passed Alarm->AlarmId matches an
**      alarm in the DAT, but the passed Alarm->Timestamp does not match the
**      timestamp of that alarm.
**
** Remarks:
**   All alarms contained in the DAT are maintained in the order in which they
**   were added.  This function returns the next alarm meeting the
**   specifications given by an HPI User that was added to the DAT after the
**   alarm whose AlarmId and Timestamp is passed by an HPI User, even if the
**   alarm associated with the AlarmId and Timestamp has been deleted.  If
**   SAHPI_FIRST_ENTRY is passed as the AlarmId, the first alarm in the DAT
**   meeting the specifications given by an HPI User is returned.
**
**   Alarm selection can be restricted to only alarms of a specified severity,
**   and/or only unacknowledged alarms.
**
**   To retrieve all alarms contained within the DAT meeting specific
**   requirements, call saHpiAlarmGetNext() with the Alarm->AlarmId field set to
**   SAHPI_FIRST_ENTRY and the Severity and UnacknowledgedOnly parameters set to
**   select what alarms should be returned.  Then, repeatedly call
**   saHpiAlarmGetNext() passing the previously returned alarm as the Alarm
**   parameter, and the same values for Severity and UnacknowledgedOnly until
**   the function returns with the error code SA_ERR_HPI_NOT_PRESENT.
**
**   SAHPI_FIRST_ENTRY and SAHPI_LAST_ENTRY are reserved AlarmId values, and are
**   never assigned to an alarm in the DAT.
**
**   The elements AlarmId and Timestamp are used in the Alarm parameter to
**   identify the previous alarm; the next alarm added to the table after this
**   alarm that meets the Severity and UnacknowledgedOnly requirements is
**   returned.  Alarm->AlarmId may be set to SAHPI_FIRST_ENTRY to select the
**   first alarm in the DAT meeting the Severity and UnacknowledgedOnly
**   requirements.  If Alarm->AlarmId is SAHPI_FIRST_ENTRY, then
**   Alarm->Timestamp is ignored.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmGetNext( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiSeverityT             Severity,
	SAHPI_IN SaHpiBoolT                 UnacknowledgedOnly,
	SAHPI_INOUT SaHpiAlarmT             *Alarm
);

/*******************************************************************************
**
** Name: saHpiAlarmGet()
**
** Description:
**   This function allows retrieval of a specific alarm in the Domain Alarm
**   Table (DAT) by referencing its AlarmId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   AlarmId - [in] AlarmId of the alarm to be retrieved from the DAT.
**   Alarm - [out] Pointer to the structure to hold the returned alarm entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_NOT_PRESENT is returned if the requested AlarmId does not
**      correspond to an alarm contained in the DAT.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * The Alarm parameter is passed in as NULL.
**      * The AlarmId parameter passed is SAHPI_FIRST_ENTRY or SAHPI_LAST_ENTRY.
**
** Remarks:
**   SAHPI_FIRST_ENTRY and SAHPI_LAST_ENTRY are reserved AlarmId values, and are
**   never assigned to an alarm in the DAT.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmGet( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiAlarmIdT              AlarmId,
	SAHPI_OUT SaHpiAlarmT               *Alarm
);

/*******************************************************************************
**
** Name: saHpiAlarmAcknowledge()
**
** Description:
**   This function allows an HPI User to acknowledge a single alarm entry or a
**   group of alarm entries by severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   AlarmId - [in] Identifier of the alarm to be acknowledged.  Reserved
**      AlarmId values:
**      * SAHPI_ENTRY_UNSPECIFIED	Ignore this parameter.
**   Severity - [in] Severity level of alarms to acknowledge.  Ignored unless
**      AlarmId is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_NOT_PRESENT is returned if an alarm entry identified by the
**      AlarmId parameter does not exist in the DAT.
**   SA_ERR_HPI_INVALID_PARAMS is returned if AlarmId is SAHPI_ENTRY_UNSPECIFIED
**      and Severity is not one of the valid enumerated values for this type.
**
** Remarks:
**   An HPI User acknowledges an alarm to indicate that it is aware of the alarm
**   and to influence platform-specific alarm annunciation that may be provided
**   by the implementation.  Typically, an implementation ignores acknowledged
**   alarms when announcing an alarm on annunciation devices such as audible
**   sirens and dry contact closures.  However, alarm annunciation is
**   implementation-specific.
**
**   An acknowledged alarm has the Acknowledged field in the alarm entry set to
**   True.
**
**   Alarms are acknowledged one of two ways: a single alarm entry by AlarmId
**   regardless of severity or a group of alarm entries by Severity regardless
**   of AlarmId.
**
**   To acknowledge all alarms contained within the DAT, set the Severity
**   parameter to SAHPI_ALL_SEVERITIES, and set the AlarmId parameter to
**   SAHPI_ENTRY_UNSPECIFIED.
**
**   To acknowledge all alarms of a specific severity contained within the DAT,
**   set the Severity parameter to the appropriate value, and set the AlarmId
**   parameter to SAHPI_ENTRY_UNSPECIFIED.
**
**   To acknowledge a single alarm entry, set the AlarmId parameter to a value
**   other than SAHPI_ENTRY_UNSPECIFIED.  The AlarmId must be a valid identifier
**   for an alarm entry present in the DAT at the time of the function call.
**
**   If an alarm has been previously acknowledged, acknowledging it again has no
**   effect.  However, this is not an error.
**
**   If the AlarmId parameter has a value other than SAHPI_ENTRY_UNSPECIFIED,
**   the Severity parameter is ignored.
**
**   If the AlarmId parameter is passed as SAHPI_ENTRY_UNSPECIFIED, and no
**   alarms are present that meet the requested Severity, this function has no
**   effect.  However, this is not an error.
**
**   SAHPI_ENTRY_UNSPECIFIED is defined as the same value as SAHPI_FIRST_ENTRY,
**   so using either symbol has the same effect.  However,
**   SAHPI_ENTRY_UNSPECIFIED should be used with this function for clarity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmAcknowledge( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiAlarmIdT              AlarmId,
	SAHPI_IN SaHpiSeverityT             Severity
);

/*******************************************************************************
**
** Name: saHpiAlarmAdd()
**
** Description:
**   This function is used to add a User Alarm to the DAT.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   Alarm- [in/out] Pointer to the alarm entry structure that contains the new
**      User Alarm to add to the DAT.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * Alarm pointer is passed in as NULL.
**      * Alarm->Severity is not one of the following enumerated values:
**         SAHPI_MINOR, SAHPI_MAJOR, or SAHPI_CRITICAL.
**      * Alarm->AlarmCond.Type is not SAHPI_STATUS_COND_TYPE_USER.
**      * SaHpiTextBufferT structure passed as part of the Alarm structure is
**         not valid. This would occur when:
**         * The DataType is not one of the enumerated values for that type, or 
**         * The data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the DAT is not able to add an
**      additional User Alarm due to space limits or limits imposed on the
**      number of User Alarms permitted in the DAT.
**
** Remarks:
**   The AlarmId, and Timestamp fields within the Alarm parameter are not used
**   by this function.  Instead, on successful completion, these fields are set
**   to new values associated with the added alarm.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmAdd( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_INOUT SaHpiAlarmT             *Alarm
);

/*******************************************************************************
**
** Name: saHpiAlarmDelete()
**
** Description:
**   This function allows an HPI User to delete a single User Alarm or a group
**   of User Alarms from the DAT.  Alarms may be deleted individually by
**   specifying a specific AlarmId, or they may be deleted as a group by
**   specifying a Severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   AlarmId - [in] Alarm identifier of the alarm entry to delete.  Reserved
**      values:
**      * SAHPI_ENTRY_UNSPECIFIED	Ignore this parameter.
**   Severity - [in] Severity level of alarms to delete.  Ignored unless AlarmId
**      is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if AlarmId is SAHPI_ENTRY_UNSPECIFIED
**      and Severity is not one of the valid enumerated values for this type.
**   SA_ERR_HPI_NOT_PRESENT is returned if an alarm entry identified by the
**      AlarmId parameter does not exist in the DAT.
**   SA_ERR_HPI_READ_ONLY is returned if the AlarmId parameter indicates a
**      non-User Alarm.
**
** Remarks:
**   Only User Alarms added to the DAT can be deleted.  When deleting alarms by
**   severity, only User Alarms of the requested severity are deleted.
**
**   To delete a single, specific alarm, set the AlarmId parameter to a value
**   representing an actual User Alarm in the DAT.  The Severity parameter is
**   ignored when the AlarmId parameter is set to a value other than
**   SAHPI_ENTRY_UNSPECIFIED.
**
**   To delete a group of User Alarms, set the AlarmId parameter to
**   SAHPI_ENTRY_UNSPECIFIED, and set the Severity parameter to identify which
**   severity of alarms should be deleted.  To clear all User Alarms contained
**   within the DAT, set the Severity parameter to SAHPI_ALL_SEVERITIES.
**
**   If the AlarmId parameter is passed as SAHPI_ENTRY_UNSPECIFIED, and no User
**   Alarms are present that meet the specified Severity, this function has no
**   effect.  However, this is not an error.
**
**   SAHPI_ENTRY_UNSPECIFIED is defined as the same value as SAHPI_FIRST_ENTRY,
**   so using either symbol has the same effect.  However,
**   SAHPI_ENTRY_UNSPECIFIED should be used with this function for clarity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmDelete( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiAlarmIdT              AlarmId,
	SAHPI_IN SaHpiSeverityT             Severity
);

/*******************************************************************************
**
** Name: saHpiRdrGet()
**
** Description:
**   This function returns a resource data record from the addressed resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   EntryId - [in] Identifier of the RDR entry to retrieve. Reserved EntryId
**      values:
**      * SAHPI_FIRST_ENTRY    Get first entry.
**      * SAHPI_LAST_ENTRY     Reserved as delimiter for end of list. Not a
**         valid entry identifier.
**   NextEntryId - [out] Pointer to location to store EntryId of next entry in
**      RDR repository.
**   Rdr - [out] Pointer to the structure to receive the requested resource data
**      record.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource contains no RDR records
**      (and thus does not have the SAHPI_CAPABILITY_RDR flag set in its RPT
**      entry).
**   SA_ERR_HPI_NOT_PRESENT is returned if an EntryId (other than
**      SAHPI_FIRST_ENTRY) is passed that does not correspond to an actual
**      EntryId in the resource's RDR repository.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * SAHPI_LAST_ENTRY is passed in to EntryId.
**      * NextEntryId pointer is passed in as NULL.
**      * Rdr pointer is passed in as NULL.
**
** Remarks:
**   Submitting an EntryId of SAHPI_FIRST_ENTRY results in the first RDR being
**   read. A returned NextEntryId of SAHPI_LAST_ENTRY indicates the last RDR has
**   been returned. A successful retrieval includes the next valid EntryId. To
**   retrieve the entire list of RDRs, call this function first with an EntryId
**   of SAHPI_FIRST_ENTRY and then use the returned NextEntryId in the next
**   call. Proceed until the NextEntryId returned is SAHPI_LAST_ENTRY.
**
**   If the resource configuration changes, and a "Resource Updated" event is 
**   issued, the contents of the RDR repository can change.  If this happens 
**   while an HPI User is reading the repository with this function, 
**   inconsistent data may be read.  To protect against this, an HPI User may 
**   examine the RDR update counter before and after reading the RDR repository
**   to make sure no configuration change occurred while the repository was 
**   being read.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRdrGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_IN  SaHpiEntryIdT         EntryId,
    SAHPI_OUT SaHpiEntryIdT         *NextEntryId,
    SAHPI_OUT SaHpiRdrT             *Rdr
);

/*******************************************************************************
**
** Name: saHpiRdrGetByInstrumentId()
**
** Description:
**   This function returns the Resource Data Record (RDR) for a specific
**   management instrument hosted by the addressed resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   RdrType - [in] Type of RDR being requested.
**   InstrumentId - [in] Instrument number identifying the specific RDR to be
**      returned.  This is a Sensor number, Control number, Watchdog Timer
**      number, IDR number, Annunciator number, FUMI number, or DIMI number
**      depending on the value of the RdrType parameter.
**   Rdr - [out] Pointer to the structure to receive the requested RDR.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the:
**      * Resource contains no RDR records (and thus does not have the
**         SAHPI_CAPABILITY_RDR flag set in its RPT entry).
**      * Type of management instrument specified in the RdrType parameter is
**         not supported by the resource, as indicated by the Capability field
**         in its RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the specific management instrument
**      identified in the InstrumentId parameter is not present in the addressed
**      resource.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the:
**      * RdrType parameter is not a valid enumerated value for the type.
**      * RdrType is SAHPI_NO_RECORD.
**      * Rdr pointer is passed in as NULL.
**
** Remarks:
**   The RDR to be returned is identified by RdrType and InstrumentId for the
**   specific management instrument for the RDR being requested.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRdrGetByInstrumentId (
    SAHPI_IN  SaHpiSessionIdT        SessionId,
    SAHPI_IN  SaHpiResourceIdT       ResourceId,
    SAHPI_IN  SaHpiRdrTypeT          RdrType,
    SAHPI_IN  SaHpiInstrumentIdT     InstrumentId,
    SAHPI_OUT SaHpiRdrT              *Rdr
);

/*******************************************************************************
**
** Name: saHpiRdrUpdateCountGet()
**
** Description:
**   This function returns an update counter for the resource data records for
**   the addressed resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   UpdateCount – [out] Pointer to the update counter.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource contains no RDR records
**      (and thus does not have the SAHPI_CAPABILITY_RDR flag set in its RPT 
**      entry).
**   SA_ERR_HPI_INVALID_RESOURCE is returned if the specified resource does not 
**      exist.
**
** Remarks:
**   This function provides a mechanism for the user to detect updates of the 
**   RDR for a resource. This can happen while the user is reading the RDRs. 
**   To protect against these updates, an HPI User may use this function before 
**   and after reading the RDR repository to make sure no configuration change
**   occurred while the repository was being read. An HPI user may also use this
**   function after receiving a "Resource Updated" or "Resource Restored" event 
**   to detect whether the RDR repository has changed.
**
**   There is no significance attached to the value of the RDR update counter, 
**   except that it must change any time the RDR data for a resource is updated 
**   and should remain constant when no changes are made to the RDR data.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRdrUpdateCountGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_OUT SaHpiUint32T          *UpdateCount
);

/*******************************************************************************
**
** Name: saHpiSensorReadingGet()
**
** Description:
**   This function is used to retrieve a Sensor reading.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the Sensor reading is being
**      retrieved.
**   Reading - [in/out] Pointer to a structure to receive Sensor reading values.
**      If NULL, the Sensor reading value is not returned.
**   EventState - [in/out] Pointer to location to receive Sensor event states.
**      If NULL, the Sensor event states is not returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the Sensor is currently disabled.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**
** Remarks:
**   For Sensors that return a type of SAHPI_SENSOR_READING_TYPE_BUFFER, the
**   format of the returned data buffer is implementation-specific.
**
**   If the Sensor does not provide a reading, the Reading structure returned by
**   the saHpiSensorReadingGet() function indicates the reading is not supported
**   by setting the IsSupported flag to False.
**
**   If the Sensor does not support any event states, a value of 0x0000 is
**   returned for the EventState value.  This is indistinguishable from the
**   return for a Sensor that does support event states, but currently has no
**   event states asserted.  The Sensor RDR Events field can be examined to
**   determine if the Sensor supports any event states.
**
**   It is legal for both the Reading parameter and the EventState parameter to
**   be NULL.  In this case, no data is returned other than the return code.
**   This can be used to determine if a Sensor is present and enabled without
**   actually returning current Sensor data.  If the Sensor is present and
**   enabled, SA_OK is returned; otherwise, an error code is returned.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorReadingGet (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiSensorNumT       SensorNum,
    SAHPI_INOUT SaHpiSensorReadingT   *Reading,
    SAHPI_INOUT SaHpiEventStateT      *EventState
);

/*******************************************************************************
**
** Name: saHpiSensorThresholdsGet()
**
** Description:
**   This function retrieves the thresholds for the given Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which threshold values are being
**      retrieved.
**   SensorThresholds - [out] Pointer to returned Sensor thresholds.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SensorThresholds pointer is
**      passed in as NULL.
**   SA_ERR_HPI_INVALID_CMD is returned if:
**      * Getting a threshold on a Sensor that is not a threshold type.
**      * The Sensor does not have any readable threshold values.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**
** Remarks:
**   This function only applies to Sensors that support readable thresholds, as
**   indicated by the IsAccessible field in the SaHpiSensorThdDefnT structure of
**   the Sensor's RDR being set to True and the ReadThold field in the same
**   structure having a non-zero value.
**
**   For thresholds that do not apply to the identified Sensor, the IsSupported
**   flag of the threshold value field is set to False.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorThresholdsGet (
    SAHPI_IN  SaHpiSessionIdT            SessionId,
    SAHPI_IN  SaHpiResourceIdT           ResourceId,
    SAHPI_IN  SaHpiSensorNumT            SensorNum,
    SAHPI_OUT SaHpiSensorThresholdsT     *SensorThresholds
);

/*******************************************************************************
**
** Name: saHpiSensorThresholdsSet()
**
** Description:
**   This function sets the specified thresholds for the given Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which threshold values are being set.
**   SensorThresholds - [in] Pointer to the Sensor thresholds values being set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_DATA is returned if any of the threshold values are
**      provided in a format not supported by the Sensor.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**   SA_ERR_HPI_INVALID_CMD is returned when:
**      * Writing to a threshold that is not writable.
**      * Setting a threshold on a Sensor that is not a threshold type as
**         indicated by the IsAccessible field of the SaHpiSensorThdDefnT
**         structure.
**      * Setting a threshold outside of the Min-Max range as defined by the
**         Range field of the SensorDataFormat of the RDR.
**   SA_ERR_HPI_INVALID_DATA is returned when:
**      * Thresholds are set out-of-order (see Remarks).
**      * A negative number is provided for either the postive hysteresis value
**         (SensorThresholds -> PosThdHysteresis), or for the negative
**         hysteresis value (SensorThreshold -> NegThdHysteresis).
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SensorThresholds pointer is
**      passed in as NULL.
**
** Remarks:
**   This function only applies to Sensors that support writable thresholds, as
**   indicated by the IsAccessible field in the SaHpiSensorThdDefnT structure of
**   the Sensor's RDR being set to True and the WriteThold field in the same
**   structure having a non-zero value.
**
**   The type of value provided for each threshold setting must correspond to
**   the reading format supported by the Sensor, as defined by the reading type
**   in the DataFormat field of the Sensor's RDR (saHpiSensorRecT).
**
**   Sensor thresholds cannot be set outside of the range defined by the Range
**   field of the SensorDataFormat of the Sensor RDR.  If SAHPI_SRF_MAX
**   indicates that a maximum reading exists, no Sensor threshold may be set
**   greater than the Max value.  If SAHPI_SRF_MIN indicates that a minimum
**   reading exists, no Sensor threshold may be set less than the Min value.
**
**   Thresholds are required to be set progressively in-order, so that, for all
**   threshold values that are defined on a Sensor, Upper Critical >= Upper
**   Major >= Upper Minor >= Lower Minor >= Lower Major >= Lower Critical.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorThresholdsSet (
    SAHPI_IN  SaHpiSessionIdT          SessionId,
    SAHPI_IN  SaHpiResourceIdT         ResourceId,
    SAHPI_IN  SaHpiSensorNumT          SensorNum,
    SAHPI_IN  SaHpiSensorThresholdsT   *SensorThresholds
);

/*******************************************************************************
**
** Name: saHpiSensorTypeGet()
**
** Description:
**   This function retrieves the Sensor type and event category for the
**   specified Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the type is being retrieved.
**   Type - [out] Pointer to returned enumerated Sensor type for the specified
**      Sensor.
**   Category - [out] Pointer to location to receive the returned Sensor event
**      category.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * Type pointer is passed in as NULL.
**      * Category pointer is passed in as NULL.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorTypeGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_IN  SaHpiSensorNumT       SensorNum,
    SAHPI_OUT SaHpiSensorTypeT      *Type,
    SAHPI_OUT SaHpiEventCategoryT   *Category
);

/*******************************************************************************
**
** Name: saHpiSensorEnableGet()
**
** Description:
**   This function returns the current Sensor enable status for an addressed
**   Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the Sensor enable status is being
**      requested.
**   SensorEnabled - [out] Pointer to the location to store the Sensor enable
**      status.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the SensorEnabled pointer is set
**      to NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**
** Remarks:
**   The SaHpiBoolT value pointed to by the SensorEnabled parameter is set to
**   True if the Sensor is enabled, or False if the Sensor is disabled.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEnableGet (
    SAHPI_IN  SaHpiSessionIdT         SessionId,
    SAHPI_IN  SaHpiResourceIdT        ResourceId,
    SAHPI_IN  SaHpiSensorNumT         SensorNum,
    SAHPI_OUT SaHpiBoolT              *SensorEnabled
);

/*******************************************************************************
**
** Name: saHpiSensorEnableSet()
**
** Description:
**   This function sets the Sensor enable status for an addressed Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the Sensor enable status is being
**      set.
**   SensorEnabled - [in] Sensor enable status to be set for the Sensor.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**   SA_ERR_HPI_READ_ONLY is returned if the Sensor does not support changing
**      the enable status (i.e., the EnableCtrl field in the Sensor RDR is set
**      to False).
**
** Remarks:
**   If a Sensor is disabled, any calls to saHpiSensorReadingGet() for that
**   Sensor return an error, and no events are generated for the Sensor.
**
**   Calling saHpiSensorEnableSet() with a SensorEnabled parameter of True
**   enables the Sensor.  A SensorEnabled parameter of False disables the
**   Sensor.
**
**   If the Sensor enable status changes as the result of this function call, an
**   event is generated.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEnableSet (
    SAHPI_IN  SaHpiSessionIdT         SessionId,
    SAHPI_IN  SaHpiResourceIdT        ResourceId,
    SAHPI_IN  SaHpiSensorNumT         SensorNum,
    SAHPI_IN  SaHpiBoolT              SensorEnabled
);

/*******************************************************************************
**
** Name: saHpiSensorEventEnableGet()
**
** Description:
**   This function returns the current Sensor event enable status for an
**   addressed Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the Sensor event enable status is
**      being requested.
**   SensorEventsEnabled - [out] Pointer to the location to store the Sensor
**      event enable status.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the SensorEventsEnabled pointer
**      is set to NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**
** Remarks:
**   The SaHpiBoolT value pointed to by the SensorEventsEnabled parameter is set
**   to True if event generation for the Sensor is enabled, or False if event
**   generation for the Sensor is disabled.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventEnableGet (
    SAHPI_IN  SaHpiSessionIdT         SessionId,
    SAHPI_IN  SaHpiResourceIdT        ResourceId,
    SAHPI_IN  SaHpiSensorNumT         SensorNum,
    SAHPI_OUT SaHpiBoolT              *SensorEventsEnabled
);

/*******************************************************************************
**
** Name: saHpiSensorEventEnableSet()
**
** Description:
**   This function sets the Sensor event enable status for an addressed Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the Sensor enable status is being
**      set.
**   SensorEventsEnabled - [in] Sensor event enable status to be set for the
**      Sensor.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**   SA_ERR_HPI_READ_ONLY is returned if the Sensor does not support changing
**      the event enable status (i.e., the EventCtrl field in the Sensor RDR is
**      set to SAHPI_SEC_READ_ONLY).
**
** Remarks:
**   If event generation for a Sensor is disabled, no events are generated as a
**   result of the assertion or deassertion of any event state, regardless of
**   the setting of the assert or deassert event masks for the Sensor.  If event
**   generation for a Sensor is enabled, events are generated when event states
**   are asserted or deasserted, according to the settings of the assert and
**   deassert event masks for the Sensor.  Event states may still be read for a
**   Sensor even if event generation is disabled, by using the
**   saHpiSensorReadingGet() function.
**
**   Calling saHpiSensorEventEnableSet() with a SensorEventsEnabled parameter of
**   True enables event generation for the Sensor.  A SensorEventsEnabled
**   parameter of False disables event generation for the Sensor.
**
**   If the Sensor event enabled status changes as a result of this function
**   call, an event is generated.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventEnableSet (
    SAHPI_IN  SaHpiSessionIdT         SessionId,
    SAHPI_IN  SaHpiResourceIdT        ResourceId,
    SAHPI_IN  SaHpiSensorNumT         SensorNum,
    SAHPI_IN  SaHpiBoolT              SensorEventsEnabled
);

/*******************************************************************************
**
** Name: saHpiSensorEventMasksGet()
**
** Description:
**   This function returns the assert and deassert event masks for a Sensor.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the event enable configuration is
**      being requested.
**   AssertEventMask - [in/out] Pointer to location to store Sensor assert event
**      mask.  If NULL, assert event mask is not returned.
**   DeassertEventMask - [in/out] Pointer to location to store Sensor deassert
**      event mask.  If NULL, deassert event mask is not returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**
** Remarks:
**   Two bit-mask values are returned by the saHpiSensorEventMasksGet()
**   function; one for the Sensor assert event mask, and one for the Sensor
**   deassert event mask. A bit set to "1" in the AssertEventMask value
**   indicates that an event is generated by the Sensor when the corresponding
**   event state for that Sensor changes from deasserted to asserted.  A bit set
**   to "1" in the DeassertEventMask value indicates that an event is generated
**   by the Sensor when the corresponding event state for that Sensor changes
**   from asserted to deasserted.
**
**   Events are only generated by the Sensor if the appropriate AssertEventMask
**   or DeassertEventMask bit is set, Sensor events are enabled, and the Sensor
**   is enabled.
**
**   For Sensors hosted by resources that have the
**   SAHPI_CAPABILITY_EVT_DEASSERTS flag set in its RPT entry, the
**   AssertEventMask and the DeassertEventMask values are always the same.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventMasksGet (
    SAHPI_IN  SaHpiSessionIdT         SessionId,
    SAHPI_IN  SaHpiResourceIdT        ResourceId,
    SAHPI_IN  SaHpiSensorNumT         SensorNum,
    SAHPI_INOUT SaHpiEventStateT      *AssertEventMask,
    SAHPI_INOUT SaHpiEventStateT      *DeassertEventMask
);

/*******************************************************************************
**
** Name: saHpiSensorEventMasksSet()
**
** Description:
**   This function provides the ability to change the settings of the Sensor
**   assert and deassert event masks.  Two parameters contain bit-mask values
**   indicating which bits in the Sensor assert and deassert event masks should
**   be updated.  In addition, there is an Action parameter.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   SensorNum - [in] Sensor number for which the event enable configuration is
**      being set.
**   Action - [in] Enumerated value describing what change should be made to the
**      Sensor event masks:
**      * SAHPI_SENS_ADD_EVENTS_TO_MASKS - for each bit set to "1" in the
**         AssertEventMask and DeassertEventMask parameters, set the
**         corresponding bit in the Sensor's assert and deassert event masks,
**         respectively, to "1".
**      * SAHPI_SENS_REMOVE_EVENTS_FROM_MASKS - for each bit set to "1" in the
**         AssertEventMask and DeassertEventMask parameters, clear the
**         corresponding bit in the Sensor's assert and deassert event masks,
**         respectively, that is, set the corresponding bits in the event masks
**         to "0".
**   AssertEventMask - [in] Bit mask or special value indicating which bits in
**      the Sensor's assert event mask should be set or cleared. (But see
**      Remarks concerning resources with the SAHPI_CAPABILITY_EVT_DEASSERTS
**      flag set.)
**   DeassertEventMask - [in] Bit mask or special value indicating which bits in
**      the Sensor's deassert event mask should be set or cleared.  (But see
**      Remarks concerning resources with the SAHPI_CAPABILITY_EVT_DEASSERTS
**      flag set.)
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Sensors,
**      as indicated by SAHPI_CAPABILITY_SENSOR in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_DATA is returned if the Action parameter is
**      SAHPI_SENS_ADD_EVENTS_TO_MASKS, and:
**      * The AssertEventMask parameter is not SAHPI_ALL_EVENT_STATES, and it
**         includes a bit for an event state that is not supported by the
**         Sensor.
**      * The resource does not have the SAHPI_CAPABILITY_EVT_DEASSERTS
**         capability set, and the DeassertEventMask parameter is not
**         SAHPI_ALL_EVENT_STATES, and it includes a bit for an event state that
**         is not supported by the Sensor.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Action parameter is out of
**      range.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Sensor is not present.
**   SA_ERR_HPI_READ_ONLY is returned if the Sensor does not support updating
**      the assert and deassert event masks (i.e., the EventCtrl field in the
**      Sensor RDR is set to SAHPI_SEC_READ_ONLY_MASKS or SAHPI_SEC_READ_ONLY).
**
** Remarks:
**   The bits in the Sensor assert and deassert event masks that correspond to
**   "1" bits in the bit-mask parameters are set or cleared, as indicated by the
**   Action parameter.  The bits in the Sensor assert and deassert event masks
**   corresponding to "0" bits in the bit-mask parameters are unchanged.
**
**   Assuming that a Sensor is enabled and event generation for the Sensor is
**   enabled, then for each bit set in the Sensor's assert event mask, an event
**   is generated when the Sensor's corresponding event state changes from
**   deasserted to asserted.  Similarly, for each bit set in the Sensor's
**   deassert event mask, an event is generated when the Sensor's corresponding
**   event state changes from asserted to deasserted.
**
**   For Sensors hosted by a resource that has the
**   SAHPI_CAPABILITY_EVT_DEASSERTS flag set in its RPT entry, the assert and
**   deassert event masks cannot be independently configured.  When
**   saHpiSensorEventMasksSet() is called for Sensors in a resource with this
**   capability, the DeassertEventMask parameter is ignored, and the
**   AssertEventMask parameter is used to determine which bits to set or clear
**   in both the assert event mask and deassert event mask for the Sensor.
**
**   The AssertEventMask or DeassertEventMask parameter may be set to the
**   special value, SAHPI_ALL_EVENT_STATES, indicating that all event states
**   supported by the Sensor should be added to or removed from, the
**   corresponding Sensor event mask.
**
**   If the Sensor assert and/or deassert event masks change as a result of this
**   function call, an event is generated.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventMasksSet (
    SAHPI_IN  SaHpiSessionIdT                 SessionId,
    SAHPI_IN  SaHpiResourceIdT                ResourceId,
    SAHPI_IN  SaHpiSensorNumT                 SensorNum,
    SAHPI_IN  SaHpiSensorEventMaskActionT     Action,
    SAHPI_IN  SaHpiEventStateT                AssertEventMask,
    SAHPI_IN  SaHpiEventStateT                DeassertEventMask
);

/*******************************************************************************
**
** Name: saHpiControlTypeGet()
**
** Description:
**   This function retrieves the Control type of a Control object.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   CtrlNum - [in] Control number for which the type is being retrieved.
**   Type - [out] Pointer to SaHpiCtrlTypeT variable to receive the enumerated
**      Control type for the specified Control.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Controls, as indicated by SAHPI_CAPABILITY_CONTROL in the resource's RPT
**      entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Control is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Type pointer is passed in as
**      NULL.
**
** Remarks:
**   The Type parameter must point to a variable of type SaHpiCtrlTypeT. Upon
**   successful completion, the enumerated Control type is returned in the
**   variable pointed to by Type.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlTypeGet (
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_IN  SaHpiResourceIdT    ResourceId,
    SAHPI_IN  SaHpiCtrlNumT       CtrlNum,
    SAHPI_OUT SaHpiCtrlTypeT      *Type
);

/*******************************************************************************
**
** Name: saHpiControlGet()
**
** Description:
**   This function retrieves the current state and mode of a Control object.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   CtrlNum - [in] Control number for which the state and mode are being
**      retrieved.
**   CtrlMode - [out] Pointer to the mode of the Control.  If NULL, the
**      Control's mode is not returned.
**   CtrlState - [in/out] Pointer to a Control data structure into which the
**      current Control state is placed. For text Controls, the line number to
**      read is passed in via CtrlState->StateUnion.Text.Line. If NULL, the
**      Control's state is not returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_CMD is returned if the Control is a write-only Control,
**      as indicated by the WriteOnly flag in the Control's RDR (see remarks).
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Controls, as indicated by the SAHPI_CAPABILITY_CONTROL in the resource's
**      RPT entry.
**   SA_ERR_HPI_INVALID_DATA is returned if the addressed Control is a text
**      Control, and the line number passed in CtrlState->StateUnion.Text.Line
**      does not exist in the Control and is not SAHPI_TLN_ALL_LINES.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Control is not present.
**
** Remarks:
**   Note that the CtrlState parameter is both an input and an output parameter
**   for this function.  This is necessary to support line number inputs for
**   text Controls, as discussed below.
**
**   As an input parameter, the CtrlState->Type value is not significant for
**   this function and is ignored. It is thus not necessary to set
**   CtrlState->Type to any valid value, even when CtrlState is being used to
**   provide input parameters for specific types of Controls.
**   In some cases, the state of a Control may be set, but the corresponding
**   state cannot be read at a later time.  Such Controls are delineated with
**   the WriteOnly flag in the Control's RDR.
**
**   Note that text Controls are unique in that they have a state associated
**   with each line of the Control - the state being the text on that line. The
**   line number to be read is passed in to saHpiControlGet() via
**   CtrlState->StateUnion.Text.Line; the contents of that line of the Control
**   are returned in
**   CtrlState->StateUnion.Text.Text.  The first line of the text Control is
**   line number "1".
**
**   If the line number passed in is SAHPI_TLN_ALL_LINES, then saHpiControlGet()
**   returns the entire text of the Control, or as much of it as can fit in a
**   single SaHpiTextBufferT, in CtrlState->StateUnion.Text.Text. This value
**   consists of the text of all the lines concatenated, using the maximum
**   number of characters for each line (no trimming of trailing blanks).
**
**   Note that depending on the data type and language, the text may be encoded
**   in 2-byte Unicode, which requires two bytes of data per character.
**
**   Note that the number of lines and columns in a text Control can be obtained
**   from the Control's Resource Data Record.
**
**   Write-only Controls allow the Control's state to be set, but the Control
**   state cannot be subsequently read.  Such Controls are indicated in the RDR,
**   when the WriteOnly flag is set to True.  SA_ERR_HPI_INVALID_CMD is returned
**   when calling this function for a write-only Control.
**
**   It is legal for both the CtrlMode parameter and the CtrlState parameter to
**   be NULL.  In this case, no data is returned other than the return code.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlGet (
    SAHPI_IN    SaHpiSessionIdT      SessionId,
    SAHPI_IN    SaHpiResourceIdT     ResourceId,
    SAHPI_IN    SaHpiCtrlNumT        CtrlNum,
    SAHPI_OUT   SaHpiCtrlModeT       *CtrlMode,
    SAHPI_INOUT SaHpiCtrlStateT      *CtrlState
);

/*******************************************************************************
**
** Name: saHpiControlSet()
**
** Description:
**   This function is used for setting the state and/or mode of the specified
**   Control object.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   CtrlNum - [in] Control number for which the state and/or mode is being set.
**   CtrlMode - [in] The mode to set on the Control.  
**   CtrlState - [in] Pointer to a Control state data structure holding the
**      state to be set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Controls, as indicated by the SAHPI_CAPABILITY_CONTROL in the resource's
**      RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the Control is not present.
**   SA_ERR_HPI_INVALID_DATA is returned when the:
**      * CtrlState->Type field is not the correct type for the Control
**         identified by the CtrlNum parameter.
**      * CtrlState->StateUnion.Analog is out of range of the Control record's
**         analog Min and Max values.
**      * CtrlState->StateUnion.Text.Text.DataLength, combined with the
**         CtrlState->StateUnion.Text.Line, overflows the remaining text Control
**         space.
**      * CtrlState->StateUnion.Text.Text.DataType is not set to the DataType
**         specified in the RDR.
**      * DataType specified in the RDR is SAHPI_TL_TYPE_UNICODE or
**         SAHPI_TL_TYPE_TEXT and
**      * CtrlState->StateUnion.Text.Text.Language is not set to the Language
**         specified in the RDR.
**      * OEM Control data is invalid (see remarks below).
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * CtrlMode is not one of the valid enumerated values for this type.
**      * CtrlMode parameter is not SAHPI_CTRL_MODE_AUTO and the CtrlState
**         pointer is passed in as NULL.
**      * CtrlState->StateUnion.Digital is not one of the valid enumerated
**         values for this type.
**      * CtrlState->StateUnion.Stream.StreamLength is bigger than
**         SAHPI_CTRL_MAX_STREAM_LENGTH.
**      * SaHpiTextBufferT structure passed as CtrlState->StateUnion.Text.Text
**         contains text characters that are not allowed according to the value
**         of CtrlState->StateUnion.Text.Text.DataType.
**   SA_ERR_HPI_INVALID_REQUEST is returned when SAHPI_CTRL_STATE_PULSE_ON is
**      issued to a digital Control, which is ON (in either manual or auto
**      mode).  It is also returned when SAHPI_CTRL_STATE_PULSE_OFF is issued
**      to a digital Control, which is OFF (in either manual or auto mode).
**   SA_ERR_HPI_READ_ONLY is returned when attempting to change the mode of a
**      Control with a read-only mode.
**   SA_ERR_HPI_UNSUPPORTED_PARAMS is returned if an otherwise legal CtrlState
**      value passed is not supported by the addressed Control.  For example, a
**      Discrete Control may only be settable to certain values, or a Digital
**      Control may only be able to accept a "Pulse" operation.  The HPI
**      implementation should provide documentation describing the limitations
**      of specific Controls which may result in this return value.
**
** Remarks:
**   If the CtrlMode parameter is set to SAHPI_CTRL_MODE_AUTO, then the
**   CtrlState parameter is not evaluated, and may be set to any value by an HPI
**   User, including a NULL pointer. When CtrlMode is SAHPI_CTRL_MODE_MANUAL,
**   the CtrlState parameter must be of the correct type for the specified
**   Control.
**
**   Text Controls include a line number and a line of text in the CtrlState
**   parameter, allowing update of just a single line of a text Control. The
**   first line of the text Control is line number "1".  If less than a full
**   line of data is written, the Control clears all spaces beyond those written
**   on the line. Thus writing a zero-length string clears the addressed line.
**
**   It is also possible to include more characters in the text passed in the
**   CtrlState structure than fits on one line; in this case, the Control wraps
**   to the next line (still clearing the trailing characters on the last line
**   written). Thus, there are two ways to write multiple lines to a text
**   Control: (a) call saHpiControlSet() repeatedly for each line, or (b) call
**   saHpiControlSet() once and send more characters than fit on one line. An
**   HPI User should not assume any "cursor positioning" characters are
**   available to use, but rather should always write full lines and allow
**   "wrapping" to occur. When calling saHpiControlSet() for a text Control, an
**   HPI User may set the line number to SAHPI_TLN_ALL_LINES; in this case, the
**   entire Control is cleared, and the data is written starting on line 1.
**   (This is different from simply writing at line 1, which only alters the
**   lines written to.)
**
**   This feature may be used to clear the entire Control, which can be
**   accomplished by setting:
**   * CtrlState->StateUnion.Text.Line = SAHPI_TLN_ALL_LINES;
**   * CtrlState->StateUnion.Text.Text.DataLength = 0;
**   * Note that the number of lines and columns in a text Control can be
**      obtained from the Control's RDR.
**   * The ManufacturerId (MId) field of an OEM Control is ignored by the
**      implementation when calling saHpiControlSet().
**   * On an OEM Control, it is up to the implementation to determine what is
**      invalid data, and to return the specified error code.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlSet (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiResourceIdT     ResourceId,
    SAHPI_IN SaHpiCtrlNumT        CtrlNum,
    SAHPI_IN SaHpiCtrlModeT       CtrlMode,
    SAHPI_IN SaHpiCtrlStateT      *CtrlState
);

/*******************************************************************************
**
** Name: saHpiIdrInfoGet()
**
** Description:
**   This function returns the Inventory Data Repository information including
**   the number of areas contained within the IDR and the update counter.  The
**   Inventory Data Repository is associated with a specific entity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   IdrInfo - [out] Pointer to the information describing the requested
**      Inventory Data Repository.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the IDR is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the IdrInfo pointer is passed in
**      as NULL.
**
** Remarks:
**   The update counter provides a means for insuring that no additions or
**   changes are missed when retrieving the IDR data.  To use this feature, an
**   HPI User should call saHpiIdrInfoGet(), and retrieve the update counter
**   value before retrieving the first Area.  After retrieving all Areas and
**   Fields of the IDR, the HPI User should again call saHpiIdrInfoGet(). If the
**   update counter value has not been incremented, no modification or additions
**   were made to the IDR during data retrieval.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrInfoGet( 
	SAHPI_IN SaHpiSessionIdT         SessionId,
	SAHPI_IN SaHpiResourceIdT        ResourceId,
	SAHPI_IN SaHpiIdrIdT             IdrId,
	SAHPI_OUT SaHpiIdrInfoT          *IdrInfo
);

/*******************************************************************************
**
** Name: saHpiIdrAreaHeaderGet()
**
** Description:
**   This function returns the Inventory Data Area header information for a
**   specific area associated with a particular Inventory Data Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaType - [in] Type of Inventory Data Area.
**   AreaId - [in] Identifier of Area entry to retrieve from the IDR.  Reserved
**      AreaId values:
**      * SAHPI_FIRST_ENTRY    Get first entry.
**      * SAHPI_LAST_ENTRY     Reserved as a delimiter for end of list.
**                             Not a valid AreaId.
**   NextAreaId - [out] Pointer to location to store the AreaId of next area of
**      the requested type within the IDR.
**   Header - [out] Pointer to Inventory Data Area Header into which the header
**      information is placed.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * AreaType parameter is set to SAHPI_IDR_AREATYPE_UNSPECIFIED, and the
**         area specified by the AreaId parameter does not exist in the IDR.
**      * AreaType parameter is set to a specific area type, but an area
**         matching both the AreaId parameter and AreaType parameter does not
**         exist.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * AreaType is not one of the valid enumerated values for this type.
**      * The AreaId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**      * The NextAreaId pointer is passed in as NULL.
**      * The Header pointer is passed in as NULL.
**
** Remarks:
**   This function allows retrieval of an Inventory Data Area Header by one of
**   two ways: by AreaId regardless of type or by AreaType and AreaId.
**
**   To retrieve all areas contained within an IDR, set the AreaType parameter
**   to SAHPI_IDR_AREATYPE_UNSPECIFIED, and set the AreaId parameter to
**   SAHPI_FIRST_ENTRY for the first call.  For each subsequent call, set the
**   AreaId parameter to the value returned in the NextAreaId parameter.
**   Continue calling this function until the NextAreaId parameter contains the
**   value SAHPI_LAST_ENTRY.
**
**   To retrieve areas of specific type within an IDR, set the AreaType
**   parameter to a valid AreaType enumeration.  Use the AreaId parameter in the
**   same manner described above to retrieve all areas of the specified type.
**   Set the AreaId parameter to SAHPI_FIRST_ENTRY to retrieve the first area of
**   that type.  Use the value returned in NextAreaId to retrieve the remaining
**   areas of that type until SAHPI_LAST_ENTRY is returned.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrAreaHeaderGet( 
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_IN SaHpiIdrAreaTypeT        AreaType,
	SAHPI_IN SaHpiEntryIdT            AreaId,
	SAHPI_OUT SaHpiEntryIdT           *NextAreaId,
	SAHPI_OUT SaHpiIdrAreaHeaderT     *Header
);

/*******************************************************************************
**
** Name: saHpiIdrAreaAdd()
**
** Description:
**   This function is used to add an Area to the specified Inventory Data
**   Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaType - [in] Type of Area to add.
**   AreaId- [out] Pointer to location to store the AreaId of the newly created
**      area.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the IDR is not present.
**   SA_ERR_HPI_INVALID_DATA is returned when attempting to add an area with an
**      AreaType of SAHPI_IDR_AREATYPE_UNSPECIFIED or when adding an area that
**      does not meet the implementation-specific restrictions.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the IDR does not have enough free
**      space to allow the addition of the area.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the: 
**      * AreaId pointer is passed in as NULL.
**      * AreaType is not one of the valid enumerated values for this type.
**   SA_ERR_HPI_READ_ONLY is returned if the IDR is read-only and does not allow
**      the addition of the area.
**
** Remarks:
**   On successful completion, the AreaId parameter contains the AreaId of the
**   newly created area.
**
**   On successful completion, the ReadOnly element in the new Area's header is
**   always False.
**
**   SAHPI_IDR_AREATYPE_UNSPECIFIED is not a valid area type, and should not be
**   used with this function.  If SAHPI_IDR_AREATYPE_UNSPECIFIED is specified as
**   the area type, an error code of SA_ERR_HPI_INVALID_DATA is returned.  This
**   area type is only valid when used with the saHpiIdrAreaHeaderGet() function
**   to retrieve areas of an unspecified type.
**
**   Some implementations may restrict the types of areas that may be added.
**   These restrictions should be documented.  SA_ERR_HPI_INVALID_DATA is
**   returned when attempting to add an invalid area type.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrAreaAdd(
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_IN SaHpiIdrAreaTypeT        AreaType,
	SAHPI_OUT SaHpiEntryIdT           *AreaId
);

/*******************************************************************************
**
** Name: saHpiIdrAreaAddById()
**
** Description:
**   This function is used to add an Area with a specified AreaId to a given
**   Inventory Data Repository. This function differs from saHpiIdrAreaAdd() in
**   that it allows the HPI User to add an Area with a specific AreaId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaType - [in] Type of Area to add.
**   AreaId- [in] AreaId for the new Area to be created.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the IDR is not present.
**   SA_ERR_HPI_INVALID_DATA is returned when attempting to add an area with an
**      AreaType of SAHPI_IDR_AREATYPE_UNSPECIFIED or when adding an area that
**      does not meet the implementation-specific restrictions.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the IDR does not have enough free
**      space to allow the addition of the Area.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the: 
**      * The AreaId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**      * AreaType is not one of the valid enumerated values for this type.
**   SA_ERR_HPI_DUPLICATE is returned if Area identified with AreaId already
**      exists in the IDR.
**   SA_ERR_HPI_READ_ONLY is returned if the IDR is read-only and does not allow
**      the addition of the area.
**
** Remarks:
**   On successful completion, a new Area with the specified AreaId and AreaType
**   is added to the IDR.
**
**   On successful completion, the ReadOnly element in the new Area's header is
**   always False.
**
**   SAHPI_IDR_AREATYPE_UNSPECIFIED is not a valid area type, and should not be
**   used with this function.  If SAHPI_IDR_AREATYPE_UNSPECIFIED is specified as
**   the area type, an error code of SA_ERR_HPI_INVALID_DATA is returned.  This
**   area type is only valid when used with the saHpiIdrAreaHeaderGet() function
**   to retrieve areas of an unspecified type.
**
**   SAHPI_FIRST_ENTRY is a valid AreaId and can be used with this function,
**   provided the IDR does not have a pre-existing Area with that AreaId. Upon
**   successful addition, this new Area becomes the first Area in the IDR and is
**   returned when retrieving Areas using saHpiIdrAreaHeaderGet() with
**   AreaId=SAHPI_FIRST_ENTRY. Implementations should document how added Areas
**   are ordered in an IDR.
**
**   Some implementations may restrict the types of Areas that may be added.
**   These restrictions should be documented.  SA_ERR_HPI_INVALID_DATA is
**   returned when attempting to add an invalid area type.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrAreaAddById(
    SAHPI_IN SaHpiSessionIdT	SessionId,
    SAHPI_IN SaHpiResourceIdT	ResourceId,
    SAHPI_IN SaHpiIdrIdT		IdrId,
    SAHPI_IN SaHpiIdrAreaTypeT	AreaType,
    SAHPI_IN SaHpiEntryIdT		AreaId
);

/*******************************************************************************
**
** Name: saHpiIdrAreaDelete()
**
** Description:
**   This function is used to delete an Inventory Data Area, including the Area
**   header and all fields contained with the area, from a particular Inventory
**   Data Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaId - [in] Identifier of Area entry to delete from the IDR.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by the AreaId parameter does not exist within the IDR.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the AreaId is an invalid
**      reserved value such as SAHPI_LAST_ENTRY.
**   SA_ERR_HPI_READ_ONLY is returned if the:
**      * IDA is read-only and therefore cannot be deleted.
**      * IDA contains a read-only Field and therefore cannot be deleted.
**      * IDR is read-only as deletions are not permitted for an area from a
**         read-only IDR.
**
** Remarks:
**   Deleting an Inventory Data Area also deletes all fields contained within
**   the area.
**
**   In some implementations, certain Areas are intrinsically read-only.  The
**   ReadOnly flag in the area header indicates if the Area is read-only.
**
**   If the Inventory Data Area is not read-only, but contains a Field that is
**   read-only, the Area cannot be deleted.  An attempt to delete an Area that
**   contains a read-only Field returns an error.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrAreaDelete( 
	SAHPI_IN SaHpiSessionIdT        SessionId,
	SAHPI_IN SaHpiResourceIdT       ResourceId,
	SAHPI_IN SaHpiIdrIdT            IdrId,
	SAHPI_IN SaHpiEntryIdT          AreaId
);

/*******************************************************************************
**
** Name: saHpiIdrFieldGet()
**
** Description:
**   This function returns the Inventory Data Field information from a
**   particular Inventory Data Area and Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaId - [in] Area identifier for the IDA.
**   FieldType - [in] Type of Inventory Data Field.
**   FieldId - [in] Identifier of Field to retrieve from the IDA.  Reserved
**      FieldId values:
**      * SAHPI_FIRST_ENTRY   Get first entry.
**      * SAHPI_LAST_ENTRY    Reserved as a delimiter for end of list.
**                            Not a valid FieldId.
**   NextFieldId - [out] Pointer to location to store the FieldId of next field
**      of the requested type in the IDA.
**   Field - [out] Pointer to Inventory Data Field into which the field
**      information is placed.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by AreaId is not present.
**      * FieldType parameter is set to SAHPI_IDR_FIELDTYPE_UNSPECIFIED, and the
**         field specified by the FieldId parameter does not exist in the IDA.
**      * FieldType parameter is set to a specific field type, but a field
**         matching both the FieldId parameter and FieldType parameter does not
**         exist.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * FieldType is not one of the valid enumerated values for this type.
**      * The AreaId or FieldId is an invalid reserved value such as
**         SAHPI_LAST_ENTRY.
**      * The NextFieldId pointer is passed in as NULL.
**      * The Field pointer is passed in as NULL.
**
** Remarks:
**   This function allows retrieval of an Inventory Data Field by one of two
**   ways: by FieldId regardless of type or by FieldType and FieldId.
**
**   To retrieve all fields contained within an IDA, set the FieldType parameter
**   to SAHPI_IDR_FIELDTYPE_UNSPECIFIED, and set the FieldId parameter to
**   SAHPI_FIRST_ENTRY for the first call.  For each subsequent call, set the
**   FieldId parameter to the value returned in the NextFieldId parameter.
**   Continue calling this function until the NextFieldId parameter contains the
**   value SAHPI_LAST_ENTRY.
**
**   To retrieve fields of a specific type within an IDA, set the FieldType
**   parameter to a valid Field type enumeration.  Use the FieldId parameter in
**   the same manner described above to retrieve all fields of the specified
**   type.  Set the FieldId parameter to SAHPI_FIRST_ENTRY to retrieve the first
**   field of that type.  Use the value returned in NextFieldId to retrieve the
**   remaining fields of that type until SAHPI_LAST_ENTRY is returned.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrFieldGet( 
	SAHPI_IN SaHpiSessionIdT         SessionId,
	SAHPI_IN SaHpiResourceIdT        ResourceId,
	SAHPI_IN SaHpiIdrIdT             IdrId,
	SAHPI_IN SaHpiEntryIdT           AreaId,
	SAHPI_IN SaHpiIdrFieldTypeT      FieldType,
	SAHPI_IN SaHpiEntryIdT           FieldId,
	SAHPI_OUT SaHpiEntryIdT          *NextFieldId,
	SAHPI_OUT SaHpiIdrFieldT         *Field
);

/*******************************************************************************
**
** Name: saHpiIdrFieldAdd()
**
** Description:
**   This function is used to add a field to the specified Inventory Data Area
**   with a specific Inventory Data Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   Field- [in/out] Pointer to Inventory Data Field, which contains the new
**      field information to add to the IDA.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by Field->AreaId does not exist within the IDR.
**   SA_ERR_HPI_INVALID_DATA is returned if the Field data is incorrectly
**      formatted or does not meet the restrictions for the implementation.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the IDR does not have enough free
**      space to allow the addition of the field.
**   SA_ERR_HPI_READ_ONLY is returned if the Area identified by Field->AreaId is
**      read-only and does not allow addition of  Fields.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      The Field type is not one of the valid field type enumerated values.
**      Field type, Field->Type, is set to SAHPI_IDR_FIELDTYPE_UNSPECIFIED.
**      SaHpiTextBufferT structure passed as part of the Field parameter is not
**      valid.  This occurs when:
**         * The DataType is not one of the enumerated values for that type, or 
**         * The data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**         * Field pointer is passed in as NULL.
**
** Remarks:
**   The FieldId element within the Field parameter is not used by this
**   function.  Instead, on successful completion, the FieldId field is set to
**   the new value associated with the added Field.
**
**   The ReadOnly element in the Field parameter is not used by this function.
**   On successful completion, the ReadOnly element in the new Field is always
**   False.
**
**   Addition of a read-only Inventory Data Field is not allowed; therefore the
**   ReadOnly element in the SaHpiIdrFieldT parameter is ignored.
**
**   SAHPI_IDR_FIELDTYPE_UNSPECIFIED is not a valid field type, and should not
**   be used with this function.  If SAHPI_IDR_FIELDTYPE_UNSPECIFIED is
**   specified as the field type, an error code of SA_ERR_HPI_INVALID_DATA is
**   returned.  This field type is only valid when used with the
**   saHpiIdrFieldGet() function to retrieve fields of an unspecified type.
**
**   Some implementations have restrictions on what fields are valid in specific
**   areas and/or the data format of some fields.  These restrictions should be
**   documented.  SA_ERR_HPI_INVALID_DATA is returned when the field type or
**   field data does not meet the implementation-specific restrictions.
**
**   The AreaId element within the Field parameter identifies the specific IDA
**   into which the new field is added.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrFieldAdd( 
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_INOUT SaHpiIdrFieldT        *Field
);

/*******************************************************************************
**
** Name: saHpiIdrFieldAddById()
**
** Description:
**   This function is used to add a field with a specific FieldId to the
**   specified Inventory Data Area of a specific Inventory Data Repository. This
**   function differs from saHpiIdrFieldAdd() in that it allows the HPI User to
**   add a Field with a specific FieldId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   Field- [in] Pointer to Inventory Data Field, which contains the new field
**      information to add to the IDA.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by Field->AreaId does not exist within the IDR.
**   SA_ERR_HPI_INVALID_DATA is returned if the Field data is incorrectly
**      formatted or does not meet the restrictions for the implementation.
**   SA_ERR_HPI_DUPLICATE is returned if a Field identified with Field->FieldId
**      already exists in the IDA.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the IDR does not have enough free
**      space to allow the addition of the field.
**   SA_ERR_HPI_READ_ONLY is returned if the Area identified by Field->AreaId is
**      read-only and does not allow addition of Fields.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * Field->FieldId  or Field->AreaId  is an invalid reserved value such as
**         SAHPI_LAST_ENTRY.
**      * The Field type is not one of the valid field type enumerated values.
**      * Field type, Field->Type, is set to SAHPI_IDR_FIELDTYPE_UNSPECIFIED.
**      * SaHpiTextBufferT structure passed as part of the Field parameter is
**         not valid.  This occurs when:
**         * The DataType is not one of the enumerated values for that type, or 
**         * The data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**         * Field pointer is passed in as NULL.
**
** Remarks:
**   On successful completion a new Field with the FieldId specified within the
**   Field parameter is added to the IDA identified by Field->AreaId. Thus this
**   function can be used to add Fields with user specified FieldIds to non
**   read-only Areas.
**
**   The ReadOnly element in the Field parameter is not used by this function.
**   On successful completion, the ReadOnly element in the new Field is always
**   False.
**
**   Addition of a read-only Inventory Data Field is not allowed; therefore the
**   ReadOnly element in the SaHpiIdrFieldT parameter is ignored.
**
**   SAHPI_IDR_FIELDTYPE_UNSPECIFIED is not a valid field type, and should not
**   be used with this function.  If SAHPI_IDR_FIELDTYPE_UNSPECIFIED is
**   specified as the field type, an error code of SA_ERR_HPI_INVALID_DATA is
**   returned.  This field type is only valid when used with the
**   saHpiIdrFieldGet() function to retrieve fields of an unspecified type.
**
**   Some implementations have restrictions on what fields are valid in specific
**   areas and/or the data format of some fields.  These restrictions should be
**   documented.  SA_ERR_HPI_INVALID_DATA is returned when the field type or
**   field data does not meet the implementation-specific restrictions.
**
**   SAHPI_FIRST_ENTRY is a valid FieldId and can be used with this function,
**   provided the IDA does not have a pre-existing Field with that FieldId. Upon
**   successful addition, this new Field becomes the first field in the IDA and
**   is returned when retrieving a Field using saHpiIdrFieldGet() with
**   FieldId=SAHPI_FIRST_ENTRY. Implementations should document how added Fields
**   are ordered in IDAs.
**
**   The AreaId element within the Field parameter identifies the specific IDA
**   into which the new field is added.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrFieldAddById( 
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_INOUT SaHpiIdrFieldT        *Field
);

/*******************************************************************************
**
** Name: saHpiIdrFieldSet()
**
** Description:
**   This function is used to update an Inventory Data Field for a particular
**   Inventory Data Area and Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   Field - [in] Pointer to Inventory Data Field, which contains the updated
**      field information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by Field->AreaId does not exist within the IDR or if
**         the Field does not exist within the Inventory Data Area.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * Field pointer is passed in as NULL.
**      * Field type, Field->Type,  is not set to one of the valid field type
**         enumerated values.
**      * Field type, Field->Type, is set to SAHPI_IDR_FIELDTYPE_UNSPECIFIED.
**   SA_ERR_HPI_INVALID_DATA is returned if the field data is incorrectly
**      formatted or does not meet the restrictions for the implementation.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the IDR does not have enough free
**      space to allow the modification of the field due to an increase in the
**      field size.
**   SA_ERR_HPI_READ_ONLY is returned if the Field is read-only and does not
**      allow modifications.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SaHpiTextBufferT structure
**      passed as part of the Field parameter is not valid.  This occurs when:
**         * The DataType is not one of the enumerated values for that type, or 
**         * The data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**
** Remarks:
**   This function allows modification of both the FieldType and Field data of a
**   given Inventory Data Field.  This function does not allow modification of
**   the read-only attribute of the Inventory Data Field; therefore after a
**   successful update, the ReadOnly element in the updated Field is always
**   False. The input value for ReadOnly is ignored.
**
**   SAHPI_IDR_FIELDTYPE_UNSPECIFIED is not a valid field type, and should not
**   be used with this function.  If SAHPI_IDR_FIELDTYPE_UNSPECIFIED is
**   specified as the new field type, an error code of SA_ERR_HPI_INVALID_DATA
**   is returned.  This field type is only valid when used with the
**   saHpiIdrFieldGet() function to retrieve fields of an unspecified type.
**
**   Some implementations have restrictions on what fields are valid in specific
**   areas and/or the data format of some fields.  These restrictions should be
**   documented.  SA_ERR_HPI_INVALID_DATA is returned when the field type or
**   field data does not meet the implementation-specific restrictions.
**
**   In some implementations, certain Fields are intrinsically read-only.  The
**   ReadOnly flag, in the field structure, indicates if the Field is read-only.
**
**   SAHPI_FIRST_ENTRY is a valid FieldId for this function and can be used to
**   update the first Field in the Area. If a Field with FieldId equal to
**   SAHPI_FIRST_ENTRY exists within the Area then it is always the first Field
**   in the Area.
**
**   The Field to update is identified by the AreaId and FieldId elements within
**   the Field parameter.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrFieldSet( 
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_IN SaHpiIdrFieldT           *Field
);

/*******************************************************************************
**
** Name: saHpiIdrFieldDelete()
**
** Description:
**   This function is used to delete an Inventory Data Field from a particular
**   Inventory Data Area and Repository.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   IdrId - [in] Identifier for the Inventory Data Repository.
**   AreaId - [in] Area identifier for the IDA.
**   FieldId - [in] Identifier of Field to delete from the IDA.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support an
**      Inventory Data Repository, as indicated by
**      SAHPI_CAPABILITY_INVENTORY_DATA in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * IDR is not present.
**      * Area identified by the AreaId parameter does not exist within the IDR,
**         or if the Field identified by the FieldId parameter does not exist
**         within the IDA.
**   SA_ERR_HPI_READ_ONLY is returned if the Field or the IDA it exists in is
**      read-only and does not allow deletion.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the AreaId or FieldId is an
**      invalid reserved value such as SAHPI_LAST_ENTRY.
**
** Remarks:
**   In some implementations, certain fields are intrinsically read-only. The
**   ReadOnly flag, in the field structure, indicates if the field is read-only.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiIdrFieldDelete( 
	SAHPI_IN SaHpiSessionIdT          SessionId,
	SAHPI_IN SaHpiResourceIdT         ResourceId,
	SAHPI_IN SaHpiIdrIdT              IdrId,
	SAHPI_IN SaHpiEntryIdT            AreaId,
	SAHPI_IN SaHpiEntryIdT            FieldId
);

/*******************************************************************************
**
** Name: saHpiWatchdogTimerGet()
**
** Description:
**   This function retrieves the current Watchdog Timer settings and
**   configuration.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   WatchdogNum - [in] Watchdog number that specifies the Watchdog Timer on a
**      resource.
**   Watchdog - [out] Pointer to Watchdog data structure.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support a
**      Watchdog Timer, as indicated by SAHPI_CAPABILITY_WATCHDOG in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the WatchdogNum is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Watchdog pointer is passed
**      in as NULL.
**
** Remarks:
**   See the description of the SaHpiWatchdogT structure in Section 8.11 for
**   details on what information is returned by this function.
**
**   When the Watchdog action is SAHPI_WA_RESET, the type of reset (warm or
**   cold) is implementation-dependent.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerGet (
    SAHPI_IN  SaHpiSessionIdT        SessionId,
    SAHPI_IN  SaHpiResourceIdT       ResourceId,
    SAHPI_IN  SaHpiWatchdogNumT      WatchdogNum,
    SAHPI_OUT SaHpiWatchdogT         *Watchdog
);

/*******************************************************************************
**
** Name: saHpiWatchdogTimerSet()
**
** Description:
**   This function provides a method for initializing the Watchdog Timer
**   configuration.
**   Once the appropriate configuration has been set using
**   saHpiWatchdogTimerSet(), an HPI User must then call
**   saHpiWatchdogTimerReset() to initially start the Watchdog Timer, unless the
**   Watchdog Timer was already running prior to calling saHpiWatchdogTimerSet()
**   and the Running field in the passed Watchdog structure is True.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   WatchdogNum - [in] The Watchdog number specifying the specific Watchdog
**      Timer on a resource.
**   Watchdog - [in] Pointer to Watchdog data structure.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support a
**      Watchdog Timer, as indicated by SAHPI_CAPABILITY_WATCHDOG in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the WatchdogNum is not present.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the:
**      * Watchdog->TimerUse is not one of the valid enumerated values for this
**         type.
**      * Watchdog->TimerAction is not one of the valid enumerated values for
**         this type.
**      * Watchdog->PretimerInterrupt is not one of the valid enumerated values
**         for this type.
**      * Watchdog pointer is passed in as NULL.
**   SA_ERR_HPI_INVALID_DATA is returned when the:
**      * Watchdog->PreTimeoutInterval is outside the acceptable range for the
**         implementation.
**      * Watchdog->InitialCount is outside the acceptable range for the
**         implementation.
**      * Value of Watchdog->PreTimeoutInterval is greater than the value of
**         Watchdog->InitialCount.
**      * Watchdog->PretimerInterrupt is set to an unsupported value.
**         See remarks.
**      * Watchdog->TimerAction is set to an unsupported value.  See remarks.
**      * Watchdog->TimerUse is set to an unsupported value.  See remarks.
**
** Remarks:
**   Configuring the Watchdog Timer with the saHpiWatchdogTimerSet() function
**   does not start the timer running.  If the timer was previously stopped when
**   this function is called, then it remains stopped, and must be started by
**   calling saHpiWatchdogTimerReset().  If the timer was previously running,
**   then it continues to run if the Watchdog->Running field passed is True, or
**   is stopped if the Watchdog->Running field passed is False.  If it continues
**   to run, it restarts its count-down from the newly configured initial count
**   value.
**
**   If the initial counter value is set to 0, then any configured pre-timer
**   interrupt action or Watchdog Timer expiration action is taken immediately
**   when the Watchdog Timer is started.  This provides a mechanism for software
**   to force an immediate recovery action should that be dependent on a
**   Watchdog timeout occurring.
**
**   See the description of the SaHpiWatchdogT structure for more details on the
**   effects of this command related to specific data passed in that structure.
**
**   Some implementations impose a limit on the acceptable ranges for the
**   PreTimeoutInterval or InitialCount. These limitations must be documented.
**   SA_ERR_HPI_INVALID_DATA is returned if an attempt is made to set a value
**   outside of the supported range.
**
**   Some implementations cannot accept all of the enumerated values for
**   TimerUse, TimerAction, or PretimerInterrupt.  These restrictions should be
**   documented.  SA_ERR_HPI_INVALID_DATA is returned if an attempt is made to
**   select an unsupported option.
**
**   When the Watchdog action is set to SAHPI_WA_RESET, the type of reset (warm
**   or cold) is implementation-dependent.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerSet (
    SAHPI_IN SaHpiSessionIdT       SessionId,
    SAHPI_IN SaHpiResourceIdT      ResourceId,
    SAHPI_IN SaHpiWatchdogNumT     WatchdogNum,
    SAHPI_IN SaHpiWatchdogT        *Watchdog
);

/*******************************************************************************
**
** Name: saHpiWatchdogTimerReset()
**
** Description:
**   This function provides a method to start or restart the Watchdog Timer from
**   the initial countdown value.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   WatchdogNum - [in] The Watchdog number specifying the specific Watchdog
**      Timer on a resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support a
**      Watchdog Timer, as indicated by SAHPI_CAPABILITY_WATCHDOG in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the WatchdogNum is not present.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the current Watchdog state does
**      not allow a reset.
**
** Remarks:
**   If the Watchdog has been configured to issue a Pre-Timeout interrupt, and
**   that interrupt has already occurred, the saHpiWatchdogTimerReset() function
**   cannot be used to reset the Watchdog Timer. The only way to stop a Watchdog
**   from timing out once a Pre-Timeout interrupt has occurred is to use the
**   saHpiWatchdogTimerSet() function to reconfigure and/or stop the timer.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerReset (
    SAHPI_IN SaHpiSessionIdT       SessionId,
    SAHPI_IN SaHpiResourceIdT      ResourceId,
    SAHPI_IN SaHpiWatchdogNumT     WatchdogNum
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorGetNext()
**
** Description:
**   This function allows retrieval of an announcement from the current set of
**   announcements held in the Annunciator.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   Severity - [in] Severity level of announcements to retrieve.  Set to
**      SAHPI_ALL_SEVERITIES to retrieve announcement of any severity;
**      otherwise, set to requested severity level.
**   UnacknowledgedOnly - [in] Set to True to indicate only unacknowledged
**      announcements should be returned.  Set to False to indicate either an
**      acknowledged or unacknowledged announcement may be returned.
**   Announcement  - [in/out] Pointer to the structure to hold the returned
**      announcement. Also, on input, Announcement->EntryId and
**      Announcement->Timestamp are used to identify the "previous"
**      announcement.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when Severity is not one of the valid
**      enumerated values for this type.
**   SA_ERR_HPI_NOT_PRESENT is returned if:
**      * The AnnunciatorNum passed does not address a valid Annunciator
**         supported by the resource.
**      * There are no announcements (or no unacknowledged announcements if
**         UnacknowledgedOnly is True) that meet the specified Severity in the
**         current set after the announcement identified by
**         Announcement->EntryId and Announcement->Timestamp.
**      * There are no announcements (or no unacknowledged announcements if
**         UnacknowledgedOnly is True) that meet the specified Severity in the
**         current set if the passed Announcement->EntryId field was set to
**         SAHPI_FIRST_ENTRY.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Announcement parameter is
**      passed in as NULL.
**   SA_ERR_HPI_INVALID_DATA is returned if the passed Announcement->EntryId
**      matches an announcement in the current set, but the passed
**      Announcement->Timestamp does not match the timestamp of that
**      announcement.
**
** Remarks:
**   All announcements contained in the set are maintained in the order in which
**   they were added.  This function returns the next announcement meeting the
**   specifications given by an HPI User that was added to the set after the
**   announcement whose EntryId and timestamp is passed by an HPI User.   If
**   SAHPI_FIRST_ENTRY is passed as the EntryId, the first announcement in the
**   set meeting the specifications given by an HPI User is returned.  This
**   function should operate even if the announcement associated with the
**   EntryId and Timestamp passed by an HPI User has been deleted.
**
**   Selection can be restricted to only announcements of a specified severity,
**   and/or only unacknowledged announcements. To retrieve all announcements
**   contained meeting specific requirements, call saHpiAnnunciatorGetNext()
**   with the Status->EntryId field set to SAHPI_FIRST_ENTRY and the Severity
**   and UnacknowledgedOnly parameters set to select what announcements should
**   be returned.  Then, repeatedly call saHpiAnnunciatorGetNext() passing the
**   previously returned announcement as the Announcement parameter, and the
**   same values for Severity and UnacknowledgedOnly until the function returns
**   with the error code SA_ERR_HPI_NOT_PRESENT.
**
**   SAHPI_FIRST_ENTRY and SAHPI_LAST_ENTRY are reserved EntryId values, and are
**   never assigned to an announcement.
**
**   The elements EntryId and Timestamp are used in the Announcement parameter
**   to identify the "previous" announcement; the next announcement added to the
**   table after this announcement that meets the Severity and
**   UnacknowledgedOnly requirements is returned.  Announcement->EntryId may be
**   set to SAHPI_FIRST_ENTRY to select the first announcement in the current
**   set meeting the Severity and UnacknowledgedOnly requirements.  If
**   Announcement->EntryId is SAHPI_FIRST_ENTRY, then Announcement->Timestamp is
**   ignored.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorGetNext( 
    SAHPI_IN SaHpiSessionIdT            SessionId,
    SAHPI_IN SaHpiResourceIdT           ResourceId,
    SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
    SAHPI_IN SaHpiSeverityT             Severity,
    SAHPI_IN SaHpiBoolT                 UnacknowledgedOnly,
    SAHPI_INOUT SaHpiAnnouncementT      *Announcement
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorGet()
**
** Description:
**   This function allows retrieval of a specific announcement in the
**   Annunciator's current set by referencing its EntryId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   EntryId - [in] Identifier of the announcement to retrieve from the
**      Annunciator.
**   Announcement - [out] Pointer to the structure to hold the returned
**      announcement.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if:
**      * The AnnunciatorNum passed does not address a valid Annunciator
**         supported by the resource.
**      * The requested EntryId does not correspond to an announcement contained
**         in the Annunciator.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * The Announcement parameter is passed in as NULL.
**      * The EntryId parameter passed is SAHPI_FIRST_ENTRY or SAHPI_LAST_ENTRY.
**
** Remarks:
**   SAHPI_FIRST_ENTRY and SAHPI_LAST_ENTRY are reserved EntryId values, and are
**   never assigned to announcements.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorGet( 
    SAHPI_IN SaHpiSessionIdT            SessionId,
    SAHPI_IN SaHpiResourceIdT           ResourceId,
    SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
    SAHPI_IN SaHpiEntryIdT              EntryId,
    SAHPI_OUT SaHpiAnnouncementT        *Announcement
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorAcknowledge()
**
** Description:
**   This function allows an HPI User to acknowledge a single announcement or a
**   group of announcements by severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   EntryId - [in] Entry identifier of the announcement to acknowledge.
**      Reserved EntryId values:
**      * SAHPI_ENTRY_UNSPECIFIED	Ignore this parameter.
**   Severity - [in] Severity level of announcements to acknowledge.  Ignored
**      unless EntryId is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if:
**      * The AnnunciatorNum passed does not address a valid Annunciator
**         supported by the resource.
**      * An announcement identified by the EntryId parameter does not exist in
**         the current set.
**   SA_ERR_HPI_INVALID_PARAMS is returned if EntryId is SAHPI_ENTRY_UNSPECIFIED
**      and Severity is not one of the valid enumerated values for this type.
**
** Remarks:
**   Announcements are acknowledged one of two ways: a single announcement by
**   EntryId regardless of severity or a group of announcements by severity
**   regardless of EntryId.
**
**   An HPI User acknowledges an announcement to influence the platform-specific
**   annunciation provided by the Annunciator management instrument.
**
**   An acknowledged announcement has the Acknowledged field set to True.
**
**   To acknowledge all announcements contained within the current set, set the
**   Severity parameter to SAHPI_ALL_SEVERITIES, and set the EntryId parameter
**   to SAHPI_ENTRY_UNSPECIFIED.
**
**   To acknowledge all announcements of a specific severity, set the Severity
**   parameter to the appropriate value, and set the EntryId parameter to
**   SAHPI_ENTRY_UNSPECIFIED.
**
**   To acknowledge a single announcement, set the EntryId parameter to a value
**   other than SAHPI_ENTRY_UNSPECIFIED.  The EntryId must be a valid identifier
**   for an announcement present in the current set.
**
**   If an announcement has been previously acknowledged, acknowledging it again
**   has no effect.  However, this is not an error.
**
**   If the EntryId parameter has a value other than SAHPI_ENTRY_UNSPECIFIED,
**   the Severity parameter is ignored.
**
**   If the EntryId parameter is passed as SAHPI_ENTRY_UNSPECIFIED, and no
**   announcements are present that meet the requested Severity, this function
**   has no effect.  However, this is not an error.
**
**   SAHPI_ENTRY_UNSPECIFIED is defined as the same value as SAHPI_FIRST_ENTRY,
**   so using either symbol has the same effect.  However,
**   SAHPI_ENTRY_UNSPECIFIED should be used with this function for clarity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorAcknowledge( 
    SAHPI_IN SaHpiSessionIdT            SessionId,
    SAHPI_IN SaHpiResourceIdT           ResourceId,
    SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
    SAHPI_IN SaHpiEntryIdT              EntryId,
    SAHPI_IN SaHpiSeverityT             Severity
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorAdd()
**
** Description:
**   This function is used to add an announcement to the set of items held by an
**   Annunciator management instrument.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   Announcement - [in/out] Pointer to structure that contains the new
**      announcement to add to the set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the AnnunciatorNum passed does not
**      address a valid Annunciator supported by the resource.
**   SA_ERR_HPI_INVALID_PARAMS is returned when:
**      * The Announcement pointer is passed in as NULL.
**      * The Announcement->Severity field is not one of the enumerated values
**         for that type, or it is equal to SAHPI_ALL_SEVERITIES.
**      * The Announcement->StatusCond.Type field is not one of the enumerated
**         values for that type.
**      * The SaHpiNameT structure passed as part of the Announcement structure
**         has a Length field greater than SA_HPI_MAX_NAME_LENGTH.
**      * The SaHpiTextBufferT structure passed as part of the Announcement
**         structure is not valid.  This would occur when:
**         * The DataType is not one of the enumerated values for that type, or 
**         * The Data field contains characters that are not legal according to
**            the value of DataType, or
**         * The Language is not one of the enumerated values for that type when
**            the DataType is SAHPI_TL_TYPE_UNICODE or SAHPI_TL_TYPE_TEXT.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the Annunciator is not able to add
**      an additional announcement due to resource limits.
**   SA_ERR_HPI_READ_ONLY is returned if the Annunciator is in auto mode.
**
** Remarks:
**   The EntryId, Timestamp, and AddedByUser fields within the Announcement
**   parameter are not used by this function.  Instead, on successful
**   completion, these fields are set to new values associated with the added
**   announcement.  AddedByUser is always set to True.
**
**   The Entity, DomainId, ResourceId, SensorNum, Name, and Mid fields in the
**   SaHpiConditionT structure passed as part of the Announcement parameter
**   should not be validated by the HPI implementation.  Any values passed by an
**   HPI user in these fields should be accepted.
**
**   Unless one of the error conditions described above are detected, or a
**   generic error condition from Section  is returned, the announcement should
**   be added to the set of items held by the Annunciator.  Note, however, that
**   unless the Annunciator is in "User" mode, the announcement may be removed
**   by the implementation at any time.  Other actions taken by the HPI
**   implementation when announcements are added to the Annunciator are
**   implementation-specific.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorAdd( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiResourceIdT           ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
	SAHPI_INOUT SaHpiAnnouncementT      *Announcement
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorDelete()
**
** Description:
**   This function allows an HPI User to delete a single announcement or a group
**   of announcements from the current set of an Annunciator.  Announcements may
**   be deleted individually by specifying a specific EntryId, or they may be
**   deleted as a group by specifying a severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   EntryId - [in] Entry identifier of the announcement to delete.  Reserved
**      EntryId values:
**      * SAHPI_ENTRY_UNSPECIFIED	Ignore this parameter.
**   Severity - [in] Severity level of announcements to delete.  Ignored unless
**      EntryId is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if EntryId is SAHPI_ENTRY_UNSPECIFIED
**      and Severity is not one of the valid enumerated values for this type.
**   SA_ERR_HPI_NOT_PRESENT is returned if:
**      * The AnnunciatorNum passed does not address a valid Annunciator
**         supported by the resource
**      * An announcement identified by the EntryId parameter does not exist in
**         the current set of the Annunciator.
**   SA_ERR_HPI_READ_ONLY is returned if the Annunciator is in auto mode.
**
** Remarks:
**   To delete a single, specific announcement, set the EntryId parameter to a
**   value representing an actual announcement in the current set.  The Severity
**   parameter is ignored when the EntryId parameter is set to a value other
**   than SAHPI_ENTRY_UNSPECIFIED.
**
**   To delete a group of announcements, set the EntryId parameter to
**   SAHPI_ENTRY_UNSPECIFIED, and set the Severity parameter to identify which
**   severity of announcements should be deleted.  To clear all announcements
**   contained within the Annunciator, set the Severity parameter to
**   SAHPI_ALL_SEVERITIES.
**
**   If the EntryId parameter is passed as SAHPI_ENTRY_UNSPECIFIED, and no
**   announcements are present that meet the specified Severity, this function
**   has no effect.  However, this is not an error.
**
**   SAHPI_ENTRY_UNSPECIFIED is defined as the same value as SAHPI_FIRST_ENTRY,
**   so using either symbol has the same effect.  However,
**   SAHPI_ENTRY_UNSPECIFIED should be used with this function for clarity.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorDelete( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiResourceIdT           ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
	SAHPI_IN SaHpiEntryIdT              EntryId,
	SAHPI_IN SaHpiSeverityT             Severity
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorModeGet()
**
** Description:
**   This function allows an HPI User to retrieve the current operating mode of
**   an Annunciator. The mode indicates whether or not an HPI User is allowed to
**   add or delete items in the set, and whether or not the HPI implementation
**   automatically adds or deletes items in the set.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   Mode - [out] Pointer to location to store the current operating mode of the
**      Annunciator where the returned value is one of the following:
**      * SAHPI_ANNUNCIATOR_MODE_AUTO - the HPI implementation may add or delete
**         announcements in the set; an HPI User may not add or delete
**         announcements in the set.
**      * SAHPI_ANNUNCIATOR_MODE_USER - the HPI implementation may not add or
**         delete announcements in the set; an HPI User may add or delete
**         announcements in the set.
**      * SAHPI_ANNUNCIATOR_MODE_SHARED - both the HPI implementation and an HPI
**         User may add or delete announcements in the set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the AnnunciatorNum passed does not
**      address a valid Annunciator supported by the resource.
**   SA_ERR_HPI_INVALID_PARAMS is returned if Mode is passed in as NULL.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorModeGet( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiResourceIdT           ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
	SAHPI_OUT SaHpiAnnunciatorModeT     *Mode
);

/*******************************************************************************
**
** Name: saHpiAnnunciatorModeSet()
**
** Description:
**   This function allows an HPI User to change the current operating mode of an
**   Annunciator. The mode indicates whether or not an HPI User is allowed to
**   add or delete announcements in the set, and whether or not the HPI
**   implementation automatically adds or deletes announcements in the set.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   AnnunciatorNum - [in] Annunciator number for the addressed Annunciator.
**   Mode - [out] Mode to set for the Annunciator:
**      * SAHPI_ANNUNCIATOR_MODE_AUTO - the HPI implementation may add or delete
**         announcements in the set; an HPI User may not add or delete
**         announcements in the set.
**      * SAHPI_ANNUNCIATOR_MODE_USER - the HPI implementation may not add or
**         delete announcements in the set; an HPI User may add or delete
**         announcements in the set.
**      * SAHPI_ANNUNCIATOR_MODE_SHARED - both the HPI implementation and an
**         HPI User may add or delete announcements in the set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      Annunciators, as indicated by SAHPI_CAPABILITY_ANNUNCIATOR in the
**      resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the AnnunciatorNum passed does not
**      address a valid Annunciator supported by the resource.
**   SA_ERR_HPI_INVALID_PARAMS is returned if Mode is not a valid enumerated
**      value for this type.
**   SA_ERR_HPI_READ_ONLY is returned if mode changing is not permitted for this
**      Annunciator.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAnnunciatorModeSet( 
	SAHPI_IN SaHpiSessionIdT            SessionId,
	SAHPI_IN SaHpiResourceIdT           ResourceId,
	SAHPI_IN SaHpiAnnunciatorNumT       AnnunciatorNum,
	SAHPI_IN SaHpiAnnunciatorModeT      Mode
);

/*******************************************************************************
**
** Name: saHpiDimiInfoGet()
**
** Description:
**   This function gets information about the DIMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   DimiInfo- [out] Dimi Info
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if DimiInfo is passed as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a vaild DIMI supported by the resource.
**
** Remarks:
**   An HPI user uses this function to get the number of tests provided by DIMI
**   at any given time. An Update Counter is also returned that can be used to
**   detect that the set of tests has changed since a previous call to this
**   function.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiInfoGet (
    SAHPI_IN    SaHpiSessionIdT     SessionId,
    SAHPI_IN    SaHpiResourceIdT    ResourceId,
    SAHPI_IN    SaHpiDimiNumT	    DimiNum,
    SAHPI_OUT   SaHpiDimiInfoT      *DimiInfo
);

/*******************************************************************************
**
** Name: saHpiDimiTestInfoGet()
**
** Description:
**   This function gets information about a particular test.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**   DimiTest- [out] Pointer to location to store test information
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a valid DIMI supported by the resource.
**      * TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_PARAMS is returned if DimiTest is passed as NULL.
**
** Remarks:
**   The HPI user can use this function to create a list of all available tests
**   on a DIMI and get information for each test. Any time a DIMI is updated
**   (the user is notified by an event), the HPI user should obtain the new test
**   list by calling this function repeatedly for each TestNum. The output
**   parameter DimiTest points to the structure providing the test information.
**
**   DIMI test numbering is sequential starting at 0. If there are 'n' tests in
**   a DIMI, then first test is always 0 and the last test is (n-1).
**
**   The test information returned by this call can contain a list of test
**   parameters. Each parameter is defined by its name, type, minimum and
**   maximum values (if applicable for the type), and a default value. A short
**   human readable description can also be provided. This parameter list can be
**   used to populate a user interface, which is used to control the tests. The
**   HPI User should use the definition of the test parameters to build a
**   parameter list for invoking the test.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestInfoGet (
    SAHPI_IN    SaHpiSessionIdT      SessionId,
    SAHPI_IN    SaHpiResourceIdT     ResourceId,
    SAHPI_IN    SaHpiDimiNumT	     DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT    TestNum,
    SAHPI_OUT   SaHpiDimiTestT	     *DimiTest
);

/*******************************************************************************
**
** Name: saHpiDimiTestReadinessGet()
**
** Description:
**   This function provides the readiness of a DIMI to run a particular test.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**   DimiReady - [out] Pointer to location to store value that indicates if DIMI
**      is ready to run indicated test
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMIs,
**      as indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a valid DIMI supported by the resource.
**      * TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_PARAMS is returned if Dimiready is passed as NULL.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestReadinessGet (
    SAHPI_IN    SaHpiSessionIdT      SessionId,
    SAHPI_IN    SaHpiResourceIdT     ResourceId,
    SAHPI_IN    SaHpiDimiNumT	     DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT    TestNum,
    SAHPI_OUT   SaHpiDimiReadyT      *DimiReady
);

/*******************************************************************************
**
** Name: saHpiDimiTestStart()
**
** Description:
**   This function starts execution of a given test on the DIMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**   NumberOfParams - [in] Number of variable parameters that are contained in
**      the VariableParams array.
**   ParamsList  - [in] Pointer to array containing variable parameters for the
**      test. The parameters must be created based on the information returned
**      by the saHpiDimiTestInfoGet() function. The names and parameter types
**      must match exactly.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      DimiNum does not address a valid DIMI supported by the resource.
**      TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_STATE is returned if the Test Readiness status for the
**      requested test is not SAHPI_DIMI_READY
**   SA_ERR_HPI_INVALID_PARAMS is returned if ParamsList is NULL and
**      NumberOfParams is non-zero.
**   SA_ERR_HPI_INVALID_DATA is returned if a passed parameter
**      * Has a name that does not correspond to any of the parameters for the
**         test Is of the wrong type for the named parameter
**      * Has an invalid value for the named parameter
**
** Remarks:
**   The saHpiDimiTestStart function starts execution of given test. An HPI User
**   should prepare the system before running a test. When the system is
**   prepared to run a test, the DIMI will report a status of SAHPI_DIMI_READY
**   to the saHpiDimiTestReadinessGet() function.  If an attempt is made to
**   start a test that does not report that status, no action is taken and
**   SA_ERR_HPI_INVALID_REQUEST is returned.  HPI Events are generated when the
**   test is started and when it ends.
**
**   ParamsList  is a pointer to an array of SaHpiDimiTestVariableParamsT
**   structures, the number of elements given by NumberOfParams. The parameters
**   list is created based on the information returned by the
**   saHpiDimiTestInfoGet() function. The names and parameter types must match
**   exactly. For unspecified parameters, the test is started using the default
**   parameters defined in the test structure.  If NumberOfParams is zero,
**   ParamsList is ignored and may be NULL.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestStart (
    SAHPI_IN    SaHpiSessionIdT                SessionId,
    SAHPI_IN    SaHpiResourceIdT               ResourceId,
    SAHPI_IN    SaHpiDimiNumT	               DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT              TestNum,
    SAHPI_IN    SaHpiUint8T                    NumberOfParams,
    SAHPI_IN    SaHpiDimiTestVariableParamsT   *ParamsList
);

/*******************************************************************************
**
** Name: saHpiDimiTestCancel()
**
** Description:
**   This function cancels the test running on the DIMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a valid DIMI supported by the resource.
**      * TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_STATE is returned if the test is not running
**   SA_ERR_INVALID_REQUEST returned if DIMI cannot cancel the test.
**
** Remarks:
**   The saHpiDimiTestCancel function is used to stop a running test. An HPI
**   User must stop each test individually. An asynchronous event is generated
**   to indicate test completion or cancellation. The saHpiDimiTestStatusGet and
**   saHpiDimiTestResultsGet functions also reflect the status of the stopped
**   test.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestCancel (
    SAHPI_IN    SaHpiSessionIdT      SessionId,
    SAHPI_IN    SaHpiResourceIdT     ResourceId,
    SAHPI_IN    SaHpiDimiNumT	     DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT    TestNum
);

/*******************************************************************************
**
** Name: saHpiDimiTestStatusGet()
**
** Description:
**   This function returns the status of a particular test.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**   PercentCompleted- [out] Pointer to location to store percentage of test
**      completed, based on implementation capability. Value = 0 - 100, 0xFF
**      returned if capability not available.  If NULL, no value will be
**      returned.
**   RunStatus- [out] Pointer to location to store the status of the last run of
**      the test.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a valid DIMI supported by the resource.
**      * TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_PARAMS is returned if RunStatus is passed as NULL.
**
** Remarks:
**   The saHpiDimiTestStatusGet function is used to obtain the current status of
**   tests. If PercentCompleted is passed as NULL, the function returns only
**   RunStatus.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestStatusGet (
    SAHPI_IN    SaHpiSessionIdT                   SessionId,
    SAHPI_IN    SaHpiResourceIdT                  ResourceId,
    SAHPI_IN    SaHpiDimiNumT	                  DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT	              TestNum,
    SAHPI_OUT   SaHpiDimiTestPercentCompletedT    *PercentCompleted,
    SAHPI_OUT   SaHpiDimiTestRunStatusT	          *RunStatus
);

/*******************************************************************************
**
** Name: saHpiDimiTestResultsGet()
**
** Description:
**   This function retrieves the results from the last run of a Test on the
**   DIMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   DimiNum- [in] DIMI number
**   TestNum- [in] Test number
**   TestResults- [out] Pointer to SaHpiDimiTestResultsT for results from last
**      run.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support DIMI, as
**      indicated by SAHPI_CAPABILITY_DIMI in the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * DimiNum does not address a valid DIMI supported by the resource.
**      * TestNum is not a valid test number currently supported by the DIMI.
**   SA_ERR_HPI_INVALID_PARAMS is returned if TestResults is passed as NULL.
**
** Remarks:
**   The saHpiDimiTestResultsGet() function is used to obtain the result of last
**   run of a test. TestResultString in theTestResults structure can return a
**   string or an URI that has the entire results stored (e.g. file, etc). This
**   URI is interpreted by the HPI user.
**
**   If test is cancelled, TestResults still provides results from the cancelled
**   run.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiDimiTestResultsGet (
    SAHPI_IN    SaHpiSessionIdT          SessionId,
    SAHPI_IN    SaHpiResourceIdT         ResourceId,
    SAHPI_IN    SaHpiDimiNumT            DimiNum,
    SAHPI_IN    SaHpiDimiTestNumT        TestNum,
    SAHPI_OUT   SaHpiDimiTestResultsT    *TestResults
);

/*******************************************************************************
**
** Name: saHpiFumiSpecInfoGet()
**
** Description:
**   This function is used to identify the specification-defined framework, if 
**   any, underlying a FUMI implementation.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which the spec information is to be 
**      returned.
**   SpecInfo – [out] Pointer to the location to store spec information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SpecInfo parameter is passed 
**      as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the FumiNum does not address a valid
**      FUMI supported by the resource.
**
** Remarks:
**   A FUMI provides a high-level abstraction of the firmware upgrade process. 
**   FUMI implementations can use any mechanisms or upgrade protocol to 
**   accomplish the actual upgrade.  Some FUMIs use a specification-defined 
**   framework, and it may be helpful for an HPI User to be aware of any such 
**   framework that underlies the FUMI implementation.  For some such 
**   frameworks, for instance, there may be additional sensors or controls 
**   associated with the FUMI that provide supplementary functionality specific
**   to that framework.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiSpecInfoGet (
    SAHPI_IN    SaHpiSessionIdT		SessionId,
    SAHPI_IN    SaHpiResourceIdT  	ResourceId,
    SAHPI_IN    SaHpiFumiNumT		FumiNum,
    SAHPI_OUT   SaHpiFumiSpecInfoT	*SpecInfo
);

/*******************************************************************************
**
** Name: saHpiFumiServiceImpactGet()
**
** Description:
**   This function is used to obtain information about the potential service 
**   impact of an upgrade process on a FUMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which the service impact information is to 
**      be returned.
**   ServiceImpact – [out] Pointer to the location to store information about 
**      the service impact.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the ServiceImpact parameter is 
**      passed as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the FumiNum does not address a valid
**      FUMI supported by the resource.
**
** Remarks:
**   An upgrade process on a FUMI can affect one or more entities associated 
**   with the resource that implements a FUMI.  For instance, new FPGA content
**   may not take effect until the FPGA, and possibly other entities, are reset.
**
**   An HPI User may call this function to check possible consequences of an 
**   upgrade process on a FUMI.
**
**   The function returns a list of affected entities and the level of impact 
**   for each entity. If a listed entity has other entities below it in an entity 
**   tree, they are also assumed to be affected.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiServiceImpactGet(
    SAHPI_IN   SaHpiSessionIdT			SessionId,
    SAHPI_IN   SaHpiResourceIdT			ResourceId,
    SAHPI_IN   SaHpiFumiNumT			FumiNum,
    SAHPI_OUT  SaHpiFumiServiceImpactDataT	*ServiceImpact
);

/*******************************************************************************
**
** Name: saHpiFumiSourceSet()
**
** Description:
**   This function is used to set new source information to the target.  When
**   explicit banks are supported, different source information can be assigned
**   to each bank.  This allows the FUMI to support loading different sources to
**   different banks.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which the source information is being set.
**   BankNum - [in] Bank number; 0 for the logical bank.
**   SourceUri - [in] Text buffer containing URI of the source.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if:
**      * SourceUri is passed as NULL.
**      * the SaHpiTextBufferT structure passed as SourceUri does not have a
**         DataType of SAHPI_TL_TYPE_TEXT.
**      * the SourceUri text buffer does not contain a properly formatted URI as
**         defined by RFC 3986.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**
** Remarks:
**   A FUMI identifies the location of the source data using the URI provided 
**   by the HPI User. The URI also identifies the protocol for accessing the 
**   source data.  Protocols that the FUMI supports are reported by the FUMI’s
**   RDR. An HPI User shall make sure that the source URI corresponds to one of
**   the supported protocols and that the location of the source data is 
**   accessible by the FUMI.
**
**   An HPI User calls this function to provide the source information to the 
**   FUMI. The actual upgrade process can only be initiated after this
**   information has been set. 
**
**   When a FUMI supports explicit banks, each bank has separate source 
**   information.  The Bank ID of 0 is used to set the source information for
**   the logical bank.  When a FUMI does not support explicit banks, only Bank 0
**   can be used with this function.
**
**   This function does not necessarily validate the source image.  If it does 
**   not, it may execute successfully, even if the image is not correct.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiSourceSet (
    SAHPI_IN    SaHpiSessionIdT	    SessionId,
    SAHPI_IN    SaHpiResourceIdT  	ResourceId,
    SAHPI_IN    SaHpiFumiNumT		FumiNum,
    SAHPI_IN    SaHpiBankNumT       BankNum,
    SAHPI_IN    SaHpiTextBufferT	*SourceUri
);

/*******************************************************************************
**
** Name: saHpiFumiSourceInfoValidateStart()
**
** Description:
**   This function initiates the validation of the integrity of the source image
**   associated with the designated bank.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which the source information is to be
**      validated.
**   BankNum - [in] Bank number on FUMI; 0 for the logical bank.
**
** Return Value:
**   SA_OK is returned when the source validation is successfully started;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the source was not set prior to
**      calling this function.
**
** Remarks:
**   This function checks the validity of the source image. It should also
**   report if the image represented by the URI matches the entity which the
**   FUMI is representing (i.e. if the image can be understood by the FUMI logic
**   and programmed into the underlying hardware). It could also do any type of
**   implementation dependent validation of the source. Some of the examples are
**   hash key or message digest validation.
**
**   The FUMI validates the URI on invocation of this API. Optionally, the FUMI
**   can start the validation process internally after the saHpiFumiSourceSet()
**   function is called and can cache the result. It can then return the result
**   of validation immediately, once the saHpiFumiValidateSource() function is
**   called.
**
**   When a FUMI supports explicit banks, each bank has separate source 
**   information, which should be separately validated.  The Bank ID of 0 is 
**   used to validate the source information for the logical bank.  When a FUMI
**   does not support explicit banks, only Bank 0 can be used with this 
**   function.
**
**   The status of a source validate operation is reported via events and can be
**   queried with the saHpiFumiUpgradeStatusGet() function for the appropriate 
**   bank.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiSourceInfoValidateStart (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum
);

/*******************************************************************************
**
** Name: saHpiFumiSourceInfoGet()
**
** Description:
**   This function returns the information about the source image assigned to
**   the designated bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using 
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number for which source information is to be returned.
**   BankNum – [in] Bank number on FUMI; 0 for the logical bank.
**   SourceInfo – [out] Pointer to the location to store source information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the SourceInfo parameter is NULL.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the source was not set prior to
**      calling this function.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**
** Remarks:
**   This function returns information about the firmware image addressed by the
**   URI set previously.  To be meaningful, this function can only be called 
**   after the source is set and validated.  This information can be used to 
**   identify the firmware to be downloaded by the FUMI.  This information 
**   contains the image name, version, and build date-time.
**
**   When a FUMI supports explicit banks, each bank has separate source 
**   information, which should be separately read.  The Bank ID of 0 is used
**   to get the source information for the logical bank.  When a FUMI does not
**   support explicit banks, only Bank 0 can be used with this function.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiSourceInfoGet (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum,
    SAHPI_OUT   SaHpiFumiSourceInfoT  *SourceInfo
);

/*******************************************************************************
**
** Name: saHpiFumiSourceComponentInfoGet()
**
** Description:
**   This function returns the information about the specified subsidiary 
**   firmware component in the source image assigned to the designated bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using 
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number for which source information is to be returned.
**   BankNum – [in] Bank number on FUMI; 0 for the logical bank.
**   SourceInfo – [out] Pointer to the location to store source information.
**   ComponentEntryId – [in] Identifier of the component to retrieve. Reserved 
**      ComponentEntryId values:
**      * SAHPI_FIRST_ENTRY - Get first component.
**      * SAHPI_LAST_ENTRY - Reserved as delimiter for end of list. Not a valid
**         component identifier.
**   NextComponentEntryId – [out] Pointer to location to store the 
**      ComponentEntryId of the next component.
**   ComponentInfo – [out] Pointer to the location to store the component 
**      information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the:
**      * resource does not support Firmware Upgrade management instruments, 
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * FUMI does not support subsidiary firmware component structures, as
**         indicated by SAHPI_FUMI_CAP_COMPONENTS in the FUMI RDR.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * ComponentInfo parameter is NULL.
**      * NextComponentEntryId pointer is passed in as NULL.
**      * ComponentEntryId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the source was not set prior to 
**      calling this function.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**      * Component identified by ComponentEntryId is not present. 
**      * ComponentEntryId is SAHPI_FIRST_ENTRY and no components are avaliable
**         in source image
**
** Remarks:
**   This function returns information about the subsidiary firmware 
**   component(s) in the source image represented by the URI set previously.  
**   To be meaningful, this function can only be called after the source is set
**   and validated.  This information contains component firmware version, 
**   description, and build date-time and can be used to further characterize
**   the firmware to be installed by the FUMI.
**
**   When a FUMI supports explicit banks, each bank may have separate component 
**   structure in source image, which should be separately read. The Bank ID of 
**   0 can be used to get the component structure information for the logical
**   bank. When a FUMI does not support explicit banks, only Bank 0 can be used
**   with this function.
**
**   If the ComponentEntryId parameter is set to SAHPI_FIRST_ENTRY, the first 
**   component in firmware source image is returned. When an entry is 
**   successfully retrieved, NextComponentEntryId is set to the identifier of
**   the next valid entry; however, when the last entry has been retrieved, 
**   NextComponentEntryId is set to SAHPI_LAST_ENTRY. To retrieve an entire list
**   of entries, an HPI User needs to call this function first with a 
**   ComponentEntryId of SAHPI_FIRST_ENTRY and then use the returned 
**   NextComponentEntryId in the next call, until the NextComponentEntryId 
**   returned is SAHPI_LAST_ENTRY.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiSourceComponentInfoGet (
    SAHPI_IN    SaHpiSessionIdT       	SessionId,
    SAHPI_IN    SaHpiResourceIdT      	ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    	FumiNum,
    SAHPI_IN    SaHpiBankNumT		    BankNum,
    SAHPI_IN    SaHpiEntryIdT		    ComponentEntryId,
    SAHPI_OUT   SaHpiEntryIdT		    *NextComponentEntryId,
    SAHPI_OUT   SaHpiFumiComponentInfoT *ComponentInfo
);

/*******************************************************************************
**
** Name: saHpiFumiTargetInfoGet()
**
** Description:
**   This function returns information about the specified bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number which contains the bank.
**   BankNum – [in] Bank number for on FUMI; 0 for the logical bank.
**   BankInfo – [out] Pointer to location to store information about the bank.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if BankInfo is NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**
** Remarks:
**   This function returns information about the current image on the target
**   including bank number, validity state, executable firmware name, version
**   number, build date-time, and position in the boot order. The validity 
**   state and position in the boot order information is applicable only to 
**   explicit banks.
**
**   When a FUMI supports explicit banks, each bank has separate information, 
**   which should be separately read.  The Bank ID of 0 is used to get 
**   information about the main firmware instance in the logical bank.  
**   This is the instance that is executing or, if no instance is currently 
**   executing, this is the instance that will execute when execution starts.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiTargetInfoGet (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum,
    SAHPI_OUT   SaHpiFumiBankInfoT    *BankInfo
);

/*******************************************************************************
**
** Name: saHpiFumiTargetComponentInfoGet()
**
** Description:
**   This function returns the information about the specified subsidiary 
**   firmware component in the designated bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using 
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number for which source information is to be returned.
**   BankNum – [in] Bank number on FUMI; 0 for the logical bank.
**   ComponentEntryId – [in] Identifier of the component to retrieve. Reserved 
**      ComponentEntryId values:
**      * SAHPI_FIRST_ENTRY - Get first component.
**      * SAHPI_LAST_ENTRY - Reserved as delimiter for end of list. Not a valid
**         component identifier.
**   NextComponentEntryId – [out] Pointer to location to store the 
**      ComponentEntryId of the next component.
**   ComponentInfo – [out] Pointer to the location to store the component 
**      information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the:
**      * resource does not support Firmware Upgrade management instruments, 
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * FUMI does not support subsidiary firmware component structures, as
**         indicated by SAHPI_FUMI_CAP_COMPONENTS in the FUMI RDR.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * ComponentInfo parameter is NULL.
**      * NextComponentEntryId pointer is passed in as NULL.
**      * ComponentEntryId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**      * Component identified by ComponentEntryId is not present. 
**      * ComponentEntryId is SAHPI_FIRST_ENTRY and no components are avaliable
**         in the designated bank
**
** Remarks:
**   This function returns information about the subsidiary firmware 
**   component(s) within the current firmware of the designated bank. This 
**   information contains component firmware version, description, and build 
**   date-time.
**
**   When a FUMI supports explicit banks, each bank's component structure is 
**   independent and, therefore, queried separately. The Bank ID of 0 can be 
**   used to get the component structure information for the main firmware 
**   instance in the logical bank.  If the ComponentEntryId parameter is set to
**   SAHPI_FIRST_ENTRY, the first component in the bank firmware is returned. 
**   When an entry is successfully retrieved, NextComponentEntryId is set to the
**   identifier of the next valid entry; however, when the last entry has been 
**   retrieved, NextComponentEntryId is set to SAHPI_LAST_ENTRY. To retrieve an
**   entire list of entries, call this function first with a ComponentEntryId of
**   SAHPI_FIRST_ENTRY and then use the returned NextComponentEntryId in the 
**   next call. Proceed until the NextComponentEntryId returned is 
**   SAHPI_LAST_ENTRY.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiTargetComponentInfoGet (
    SAHPI_IN    SaHpiSessionIdT       	SessionId,
    SAHPI_IN    SaHpiResourceIdT      	ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    	FumiNum,
    SAHPI_IN    SaHpiBankNumT		    BankNum,
    SAHPI_IN    SaHpiEntryIdT		    ComponentEntryId,
    SAHPI_OUT   SaHpiEntryIdT		    *NextComponentEntryId,
    SAHPI_OUT   SaHpiFumiComponentInfoT *ComponentInfo
);

/*******************************************************************************
**
** Name: saHpiFumiLogicalTargetInfoGet()
**
** Description:
**   This function returns additional target information about the logical bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using 
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number for which source information is to be returned.
**   ComponentEntryId – [in] Identifier of the component to retrieve. Reserved 
**      ComponentEntryId values:
**      * SAHPI_FIRST_ENTRY - Get first component.
**      * SAHPI_LAST_ENTRY - Reserved as delimiter for end of list. Not a valid
**         component identifier.
**   NextComponentEntryId – [out] Pointer to location to store the 
**      ComponentEntryId of the next component.
**   ComponentInfo – [out] Pointer to the location to store the component 
**      information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if BankInfo is NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned if the FumiNum does not address a valid 
**      FUMI supported by the resource.
**
** Remarks:
**   As previously described, there can be up to three logical locations for the
**   firmware instances in the logical bank. Those locations are all accessed 
**   using Bank ID 0, and their contents changes automatically depending on the
**   executed function. Therefore, a special function is necessary to return
**   additional information about the firmware instances in the logical bank
**   target, specifically regarding the rollback and pending firmware instances.
**   This information also includes the number of persistent locations for 
**   firmware instances and a flag indicating whether the currently executing
**   main firmware instance has a persistent copy or not.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiLogicalTargetInfoGet (
    SAHPI_IN    SaHpiSessionIdT       		SessionId,
    SAHPI_IN    SaHpiResourceIdT      		ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    		FumiNum,
    SAHPI_OUT   SaHpiFumiLogicalBankInfoT	*BankInfo
);

/*******************************************************************************
**
** Name: saHpiFumiLogicalTargetComponentInfoGet()
**
** Description:
**   This function returns additional information about the specified subsidiary 
**   firmware component in the logical bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using 
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number for which source information is to be returned.
**   BankInfo – [out] Pointer to location to store additional information about 
**      the bank.
**
** Return Value:
**   SA_ERR_HPI_CAPABILITY is returned if the:
**      * resource does not support Firmware Upgrade management instruments, 
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * FUMI does not support subsidiary firmware component structures, as
**         indicated by SAHPI_FUMI_CAP_COMPONENTS in the FUMI RDR.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the:
**      * ComponentInfo parameter is NULL.
**      * NextComponentEntryId pointer is passed in as NULL.
**      * ComponentEntryId is an invalid reserved value such as SAHPI_LAST_ENTRY.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * Component identified by ComponentEntryId is not present. 
**      * ComponentEntryId is SAHPI_FIRST_ENTRY and no components are avaliable
**
** Remarks:
**   This function returns additional information about the subsidiary firmware 
**   component in the logical bank, specifically regarding the subsidiary 
**   components of the rollback and pending firmware instances.
**
**   If the ComponentEntryId parameter is set to SAHPI_FIRST_ENTRY, the first
**   component in a firmware instance is returned. When an entry is successfully
**   retrieved, NextComponentEntryId is set to the identifier of the next valid 
**   entry; however, when the last entry has been retrieved, 
**   NextComponentEntryId is set to SAHPI_LAST_ENTRY. To retrieve an entire list
**   of entries, call this function first with a ComponentEntryId of 
**   SAHPI_FIRST_ENTRY and then use the returned NextComponentEntryId in the 
**   next call. Proceed until the NextComponentEntryId returned is 
**   SAHPI_LAST_ENTRY.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiLogicalTargetComponentInfoGet (
    SAHPI_IN    SaHpiSessionIdT       		SessionId,
    SAHPI_IN    SaHpiResourceIdT      		ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    		FumiNum,
    SAHPI_IN    SaHpiEntryIdT			    ComponentEntryId,
    SAHPI_OUT   SaHpiEntryIdT			    *NextComponentEntryId,
    SAHPI_OUT   SaHpiFumiLogicalComponentInfoT	*ComponentInfo
);

/*******************************************************************************
**
** Name: saHpiFumiBackupStart()
**
** Description:
**   This function initiates a backup of the main instance in the logical bank 
**   on the target, resulting in a rollback instance in the logical bank.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number that should perform the backup operation.
**
** Return Value:
**   SA_OK is returned if the backup operation is successfully started;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support backup capability, as indicated by
**         SAHPI_FUMI_CAP_BACKUP in the FUMI's RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if the FumiNum does not address a valid
**      FUMI supported by the resource.
**
** Remarks:
**   This function is used for taking a backup of the main firmware instance in
**   the logical bank on the target in preparation for upgrades.  Depending on 
**   the implementation, this may mean taking a backup on the hard drive or into
**   another bank, or in any other location. If the logical bank already has a 
**   copy of the main instance as the rollback instance, the FUMI may 
**   immediately report the backup is complete without making an additional 
**   copy. 
**
**   The status of a backup operation is reported via events and can be queried
**   with the saHpiFumiUpgradeStatusGet() function.
**
**   The saHpiFumiBackupStart() function cannot be directly targeted to an 
**   explicit bank, but may have undefined side effects on an explicit bank if
**   the FUMI implements explicit banks.  To copy an image to a specific bank,
**   the saHpiFumiBankCopy() function should be used.
**
**   If an implementation supports HPI Users initiating backups of the logical
**   bank, it is reflected in the FUMI capability flag SAHPI_FUMI_CAP_BACKUP in
**   the RDR.  If an implementation does not support this capability, backups
**   may be created automatically during the upgrade operation by the 
**   implementation; such automatic backups are available if 
**   SAHPI_FUMI_CAP_ROLLBACK is set and SAHPI_FUMI_CAP_BACKUP is not set. 
**   If an implementation does support the backup capability using this 
**   function, it does not create automatic backups that would override the 
**   ones created by this function.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiBackupStart(
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum
);


/*******************************************************************************
**
** Name: saHpiFumiBankBootOrderSet()
**
** Description:
**   This function sets the position of an explicit bank in the boot order.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number which contains the bank.
**   BankNum – [in] Explicit bank number; a logical bank reference (BankNum = 0) 
**      is not allowed.
**   Position - [in] Position of the bank in boot order.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support bank reorder capability, as indicated by
**         SAHPI_FUMI_CAP_BANKREORDER in the FUMI's RDR.
**   SA_ERR_HPI_INVALID_DATA is returned if the Position is more than number of
**      banks on the FUMI.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not a valid explicit bank number supported by the FUMI.
**
** Remarks:
**   For boot ordering, explicit banks are assigned positions between 1 and n, 
**   inclusive, where n is the number of banks in a FUMI.
**
**   Boot order can be represented as a list. Setting a bank to a certain
**   position causes all the subsequent banks to move to next higher positions.
**
**   For example, suppose the ordered list for a FUMI has four expliict banks in
**   following order:
**   bank3, bank 2, bank 4, bank 1. 
**
**   Moving the bank 1 to second position causes the list to get rearranged as: 
**   bank 3, bank 1, bank 2, bank4.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiBankBootOrderSet (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum,
    SAHPI_IN    SaHpiUint32T          Position
);

/*******************************************************************************
**
** Name: saHpiFumiBankCopyStart()
**
** Description:
**   This function initiates the copy of the contents of one explicit bank to 
**   another explicit bank in the same FUMI. 
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number which contains the source and destination 
**      explicit banks.
**   SourceBankNum – [in] Explicit Source Bank number to copy from; a logical
**      bank reference (SourceBankNum = 0) is not allowed.
**   TargetBankNum – [in] Explicit Target Bank number to copy to; a logical bank
**      reference (TargetBankNum = 0) is not allowed.
**
** Return Value:
**   SA_OK is returned if the backup operation is successfully started;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support bank copy capability, as indicated by
**         SAHPI_FUMI_CAP_BANKCOPY in the FUMI's RDR.
**   SA_ERR_HPI_INVALID_REQUEST is returned if SourceBankNum and  TargetBankNum
**      are same.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**   SA_ERR_HPI_INVALID_DATA is returned if SourceBankNum or TargetBankNum is
**      not a valid explicit bank number supported by the FUMI.
**
** Remarks:
**   The status of a bank copy operation is reported via events, and can be
**   queried with the saHpiFumiUpgradeStatusGet() function, using the source
**   bank as the value of the BankNum.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiBankCopyStart(
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      SourceBankNum,
    SAHPI_IN    SaHpiBankNumT	      TargetBankNum
);

/*******************************************************************************
**
** Name: saHpiFumiInstallStart()
**
** Description:
**   This function starts an installation process, loading firmware to the 
**   logical bank or to a specified explicit bank. 
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number which that should perform the install operation.
**   BankNum – [in] Target bank number; 0 for the logical bank.
**
** Return Value:
**   SA_OK is returned if the install operation is successfully started; 
**      otherwise, an error code is returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the source is not valid.
**      That is, if saHpiFumiSourceInfoGet() returns any value other than
**      SAHPI_FUMI_SRC_VALID or SAHPI_FUMI_SRC_VALIDITY_UNKNOWN for
**      SourceInfo–>SourceStatus.
**
** Remarks:
**   This function can be called after setting up and optionally validating the
**   source for the given target. It initiates the install process. The target
**   is programmed with the new source image on completion of the upgrade
**   process. The whole process is atomic to the HPI user.
**
**   If the install process on the logical bank fails, the FUMI can 
**   automatically initiate a rollback process if the FUMI’s RDR indicates the
**   capability SAHPI_FUMI_CAP_AUTOROLLBACK.  However, if that capability is 
**   present, the capability SAHPI_FUMI_CAP_AUTOROLLBACK_CAN_BE_DISABLED 
**   indicates that the automatic rollback can be disabled and enabled via the
**   corresponding FUMI function.
**
**   When a FUMI supports explicit banks, each bank can be individually 
**   installed.  The Bank ID of 0 can be used to install the logical bank. When
**   a FUMI does not support explicit banks, an HPI User shall use Bank 0 with
**   this function.
**
**   The status of the install operation is reported via events, and can be 
**   queried with the saHpiFumiUpgradeStatusGet() function, using the source
**   bank as the value of the BankNum. Some events (and the corresponding status
**   values) indicate that an HPI User should take follow up action, such as
**   initiating an explicit rollback operation.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiInstallStart (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum
);

/*******************************************************************************
**
** Name: saHpiFumiUpgradeStatusGet()
**
** Description:
**   This function is used to query the upgrade status of the FUMI.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which status is to be returned.
**   BankNum - [in] Target bank number; 0 for the logical bank.
**   UpgradeStatus - [out] Pointer to location to store upgrade status for the
**      bank.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**   SA_ERR_HPI_INVALID_PARAMS is returned if UpgradeStatus is passed as NULL.
**
** Remarks:
**   This function can be used by an HPI user to determine the status of an
**   asynchronous operation that may be active on a particular bank.
**
**   The status of an asynchronous operation is reported via events, and can be
**   queried with the saHpiFumiUpgradeStatusGet() function.  When a FUMI
**   supports multiple banks, asynchronous operations are associated with
**   individual banks, so this function returns the status of an operation for a
**   specified bank.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiUpgradeStatusGet (
    SAHPI_IN    SaHpiSessionIdT         SessionId,
    SAHPI_IN    SaHpiResourceIdT        ResourceId,
    SAHPI_IN    SaHpiFumiNumT	        FumiNum,
    SAHPI_IN    SaHpiBankNumT	        BankNum,
    SAHPI_OUT   SaHpiFumiUpgradeStatusT *UpgradeStatus
);

/*******************************************************************************
**
** Name: saHpiFumiTargetVerifyStart()
**
** Description:
**   This function starts the verification process of the upgraded image.
**   Verification compares the programmed data with the source assigned for the
**   bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number which should perform the target verify
**      operation.
**   BankNum – [in] Bank number; 0 for the logical bank.
**
** Return Value:
**   SA_OK is returned when source verify operation is successfully started;
**      otherwise, an error code is returned. 
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support target verify capability, as indicated by
**         SAHPI_FUMI_CAP_TARGET_VERIFY in the FUMI’s RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the source for the target to be
**      verified is not valid. That is, if saHpiFumiSourceInfoGet() returns any
**      value other than SAHPI_FUMI_SRC_VALID or SAHPI_FUMI_SRC_VALIDITY_UNKNOWN
**      for SourceInfo–>SourceStatus.
**
** Remarks:
**   This function verifies that the installed image at the target bank matches 
**   with the source image that was assigned to it.
**
**   The Bank ID of 0 can be used to validate the source information for the 
**   logical bank.  In this case, this function verifies that the pending 
**   firmware instance at the target matches the source image that was set for
**   the bank.
**
**   The status of a target verify operation is reported via events and can be
**   queried with the saHpiFumiUpgradeStatusGet() function, using the target 
**   bank as the value of the BankNum parameter.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiTargetVerifyStart (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum
);

/*******************************************************************************
**
** Name: saHpiFumiTargetVerifyMainStart()
**
** Description:
**   This function starts the verification process of the main firmware instance
**   in the logical bank.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number that should perform the target verify operation.
**
** Return Value:
**   SA_OK is returned if the target verify operation is successfully started;
**      otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support target verify capability, as indicated by
**         SAHPI_FUMI_CAP_TARGET_VERIFY_MAIN in the FUMI's RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if the FumiNum does not address a valid
**      FUMI supported by the resource.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the:
**      * source for the target to be verified is not valid.  That is, if 
**         saHpiFumiSourceInfoGet() returns any value other than 
**         SAHPI_FUMI_SRC_VALID or SAHPI_FUMI_SRC_VALIDITY_UNKNOWN for
**         SourceInfo–>SourceStatus.
**      * main firmware instance is not avaliable. This can happen in the case 
**         of a failed activation or a failed rollback.
**
** Remarks:
**   Verification compares the main firmware instance with the source assigned 
**   for the logical bank. Note that an explicit source set operation may be 
**   needed to align the source with the main firmware instance on the target.
**
**   The logical bank may hold up to three instances of the firmware.  Two of 
**   these, the main and pending instances, can be verified against the source.
**   This function verifies the main instance.  To verify the pending instance,
**   use the saHpiFumiVerifyTargetStart() function with a Bank ID of 0.
**
**   The status of a target verify against main firmware instance operation is
**   reported via the same events as target verification against pending 
**   firmware instance and can be queried with the saHpiFumiUpgradeStatusGet()
**   function, using 0 as the value of the BankNum parameter.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiTargetVerifyMainStart (
    SAHPI_IN    SaHpiSessionIdT		SessionId,
    SAHPI_IN    SaHpiResourceIdT	ResourceId,
    SAHPI_IN    SaHpiFumiNumT		FumiNum
);


/*******************************************************************************
**
** Name: saHpiFumiUpgradeCancel()
**
** Description:
**   This function stops an asynchronous operation in process on a designated
**      bank.
**
** Parameters:
**   SessionId – [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId – [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number which contains this bank.
**   BankNum – [in] Target bank number; 0 for the logical bank.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned. 
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      FumiNum does not address a valid FUMI supported by the resource.
**      BankNum is not 0 and not a valid bank number supported by the FUMI.
**   SA_ERR_HPI_INVALID_REQUEST is returned if an asynchronous operation is not
**      in progress.
**
** Remarks:
**   This function cancels any asynchronous operation running on a specific bank
**   of a FUMI.  Asynchronous operations are started by
**   saHpiFumiSourceInfoVerifyStart(), saHpiFumiInstallStart(),
**   saHpiFumiTargetVerifyStart(), saHpiFumiRollbackStart(),
**   saHpiFumiActivateStart(), saHpiFumiBankCopyStart(), and 
**   saHpiFumiBackupStart().
**
**   The only mandatory result of this function is an event issued by the FUMI
**   identifying the operation that was cancelled. Other persistent effects of 
**   such a cancellation are implementation-specific. Some implementations may
**   automatically roll back to a previous version. Target bank firmware 
**   instances may be corrupted and not available after the cancellation.
**
**   Asynchronous operations are activated for specific banks.  The Bank ID of 0
**   can be used to cancel an asynchronous operation on the logical bank.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiUpgradeCancel (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum,
    SAHPI_IN    SaHpiBankNumT	      BankNum
);

/*******************************************************************************
**
** Name: saHpiFumiAutoRollbackDisableGet()
**
** Description:
**   This function is used to query whether automatic rollback is disabled or
**   not on the FUMI logical bank.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which automatic rollback policy is to be 
**      returned.
**   Disable – [out] Pointer to location to store information about automatic 
**      rollback disable status. Set to True if automatic rollback is disabled, 
**      or False automatic rollback is enabled.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * FUMI does not support automatic rollback, as indicated by 
**         SAHPI_FUMI_CAP_AUTOROLLBACK capability in FUMI RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**   SA_ERR_HPI_INVALID_PARAMS is returned if Disable is passed as NULL.
**
** Remarks:
**   This function can be used by an HPI User to determine whether automatic 
**   rollback is disabled or not on the FUMI logical bank.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiAutoRollbackDisableGet (
    SAHPI_IN    SaHpiSessionIdT   SessionId,
    SAHPI_IN    SaHpiResourceIdT  ResourceId,
    SAHPI_IN    SaHpiFumiNumT	FumiNum,
    SAHPI_OUT   SaHpiBoolT		*Disable
);

/*******************************************************************************
**
** Name: saHpiFumiAutoRollbackDisableSet()
**
** Description:
**   This function is used to disable or enable automatic rollback on the FUMI
**   logical bank.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number for which automatic rollback policy is to be 
**      set.
**   Disable – [in] Boolean variable that indicates whether an HPI User requests
**      disabling or enabling automatic rollback. Set to True to disable 
**      automatic rollback or to False to enable automatic rollback.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * FUMI does not support automatic rollback, as indicated by 
**         SAHPI_FUMI_CAP_AUTOROLLBACK capability in FUMI RDR.
**      * FUMI does not support disabling the automatic rollback policy, as 
**         indicated by SAHPI_FUMI_CAP_AUTOROLLBACK_CAN_BE_DISABLED in FUMI RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**
** Remarks:
**   This function can be used by an HPI User to enable or disable automatic 
**   rollback on the FUMI logical bank.  By default, automatic rollback is 
**   enabled.  For some implementations, automatic rollback cannot be disabled.
**   To change the automatic rollback policy, an HPI User must call this 
**   function before calling a function (typically an install or activate) that
**   could result in the rollback.  The state of the rollback policy is intended
**   to persist until explicitly changed.  Entities that use multiple firmware 
**   instances for reliability benefits beyond protecting the upgrade process 
**   itself may apply this policy in those broader contexts as well.  
**   For instance, an implementation may have the capability to switch to a 
**   rollback firmware instance if the main instance becomes inoperable for some
**   reason not associated with a firmware upgrade; such an implementation may 
**   inhibit that switch if automatic rollback has been disabled via a FUMI 
**   operation.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiAutoRollbackDisableSet (
    SAHPI_IN    SaHpiSessionIdT     SessionId,
    SAHPI_IN    SaHpiResourceIdT    ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    FumiNum,
    SAHPI_IN    SaHpiBoolT		    Disable
);

/*******************************************************************************
**
** Name: saHpiFumiRollbackStart()
**
** Description:
**   This function initiates a rollback operation to replace the main firmware
**   instance of the logical bank with the rollback instance.  The rollback 
**   instance may have been explicitly created with saHpiFumiBackupStart(), 
**   or automatically by the implementation.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum – [in] FUMI number that should perform the rollback operation.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if:
**      * the resource does not support Firmware Upgrade management instruments,
**         as indicated by SAHPI_CAPABILITY_FUMI in the resource's RPT entry.
**      * the FUMI does not support rollback capability, as indicated by
**         SAHPI_FUMI_CAP_ROLLBACK in the FUMI's RDR.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**   SA_ERR_HPI_INVALID_REQUEST is returned if rollback firmware instance is 
**      not available.
**
** Remarks:
**   The intent of this function is to roll back to the backed up version. Some
**   implementations may not support it; in those cases, if upgrade failure
**   happens then the HPI user will need to explicitly restore the target by
**   initiating a new upgrade process or copying an image from a redundant
**   location.
**
**   The status of a rollback operation is reported via events, and can be
**   queried with the saHpiFumiUpgradeStatusGet() function.  This function
**   starts the rollback operation for the logical bank.  Status of the rollback
**   operation will be associated with that bank.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiRollbackStart (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum
);

/*******************************************************************************
**
** Name: saHpiFumiActivate()
**
** Note:
**   This function should not be used in new HPI User programs. The function
**   saHpiFumiActivateStart() should be used instead.
**
** Description:
**   This function starts execution of the firmware in the first valid explicit
**   bank in the boot order list on the FUMI, if the FUMI supports explicit 
**   banks, or activates an appropriate firmware instance on the logical bank,
**   if FUMI does not support explicit banks.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**   SA_ERR_HPI_INVALID_REQUEST is returned if:
**      * NumBanks is 0, but there is no valid firmware instance in the logical
**         bank to execute on the entity; that is, if there is no valid pending
**         or main instance.
**      * NumBanks is positive, but no explicit bank has both a valid BankState 
**         and a positive Position in the boot order list.
**
** Remarks:
**   This function provides a synchronous method for firmware activation.
**
**   When a FUMI supports explicit banks, this function ensures that the 
**   firmware loaded in the first valid explicit bank in the boot order list is
**   executing.  If the required firmware instance is already executing before
**   this function was called, the function has no effect.  If the entity is not
**   executing any firmware, or is executing some inappropriate instance, 
**   calling this function will cause the entity to be restarted with the 
**   correct firmware.
**
**   When a FUMI does not support explicit banks, this function ensures that the
**   pending firmware instance becomes the main firmware instance and starts 
**   executing.  If there is no pending instance, this function ensures that the
**   main instance is executing, if necessary by restarting the entity with that
**   instance.  If the main instance is already executing, this function has no 
**   effect.
**
**   This function is preserved for compatibility with previous HPI 
**   specification revisions.  The more flexible asynchronous function 
**   saHpiFumiActivateStart() is preferred for new applications.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFumiActivate (
    SAHPI_IN    SaHpiSessionIdT       SessionId,
    SAHPI_IN    SaHpiResourceIdT      ResourceId,
    SAHPI_IN    SaHpiFumiNumT	      FumiNum
);

/*******************************************************************************
**
** Name: saHpiFumiActivateStart()
**
** Description:
**   This function initiates firmware activation in either the logical bank or
**   the explicit banks on the FUMI. 
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number
**   Logical – [in] Boolean variable. When True, indicates a logical bank 
**      activation. When False, indicates an explicit bank activation according
**      to the boot order list.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if FumiNum does not address a valid FUMI
**      supported by the resource.
**   SA_ERR_HPI_INVALID_REQUEST is returned if:
**      * Logical is False, but NumBanks is 0 indicating that the FUMI does not 
**         support explicit banks.
**      * Logical is False, NumBanks is positive, but no explicit bank has both
**         a valid BankState and a positive Position in the boot order list.
**      * Logical is True, but there is no valid firmware instance in the 
**         logical bank to execute on the entity; that is, if there is no valid
**         pending or main instance.
**
** Remarks:
**   This function provides an asynchronous method for firmware activation. Some
**   implementations may require a long time to start execution of a new image
**   on the FUMI.  For example, an implementation may need to copy firmware from
**   a non-executable SEEPROM to an executable flash location.  Alternatively, 
**   the old firmware may need a long time to shut down or the new firmware may
**   need a long time to start up.
**
**   If Logical is True, this function ensures that the pending firmware 
**   instance become the main firmware instance and starts executing.  If there
**   is no pending instance, this function ensures that the main instance is
**   executing, if necessary by restarting the entity with that instance.  If
**   the main instance is already executing, this function has no effect.
**
**   If Logical is True and the activate process fails, the FUMI can 
**   automatically restore the rollback firmware instance if the FUMI’s RDR
**   indicates the capability SAHPI_FUMI_CAP_AUTOROLLBACK.  However, if that
**   capability is present, the capability 
**   SAHPI_FUMI_CAP_AUTOROLLBACK_CAN_BE_DISABLED indicates that the automatic
**   rollback can be disabled and enabled via the corresponding FUMI functions.
**
**   If Logical is False, this function ensures that the firmware loaded in the
**   first valid explicit bank in the boot order list is executing. If the 
**   required firmware instance is already executing before this function was
**   called, the function has no effect. If the entity is not executing any 
**   firmware, or is executing some inappropriate instance, calling this 
**   function will cause the entity to be restarted with the correct 
**   firmware.
**
**   The status of an asynchronous activation operation is reported via events
**   and can be queried with the saHpiFumiUpgradeStatusGet() function. Some
**   events (and the corresponding status values) indicate that an HPI User 
**   should take follow up action, such as initiating an explicit rollback
**   operation.
**
**   There are some considerations that make this function more flexible than
**   the saHpiFumiActivate() variant. One such consideration is that the 
**   asynchronous variant is better suited to FUMI implementations that 
**   require a long time to start execution of a new image.  For example, an
**   implementation may need to copy firmware from a non-executable SEEPROM to
**   an executable flash location.  Alternatively, the old firmware may need a
**   long time to shut down, or the new firmware may need a long time to start 
**   up.  Doing firmware activation synchronously can lead to timeout problems
**   in the HPI implementation.
**
**   Another such consideration is that the saHpiFumiActivateStart() function
**   has an additional parameter that explicitly states whether the logical bank
**   or explicit bank activation is intended.  For FUMIs that support explicit
**   banks, this parameter allows an HPI User to unambiguously determine the 
**   activation type.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiActivateStart (
    SAHPI_IN    SaHpiSessionIdT	    SessionId,
    SAHPI_IN    SaHpiResourceIdT	ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    FumiNum,
    SAHPI_IN    SaHpiBoolT		    Logical
);

/*******************************************************************************
**
** Name: saHpiFumiActivateStart()
**
** Description:
**   This function performs cleanup after an upgrade process on the specified 
**   bank, returning it to a predefined state.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   FumiNum - [in] FUMI number
**   BankNum – [in] Target bank number; 0 for the logical bank.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support Firmware
**      Upgrade management instruments, as indicated by SAHPI_CAPABILITY_FUMI in
**      the resource's RPT entry.
**   SA_ERR_HPI_NOT_PRESENT is returned if the:
**      * FumiNum does not address a valid FUMI supported by the resource.
**      * BankNum is not 0 and not a valid bank number supported by the FUMI.
**
** Remarks:
**   This function takes the following actions:
**      * Cancels all asynchronous operations in progress or scheduled for the 
**         designated bank.  An asynchronous bank to bank copying operation is 
**         canceled only if the designated bank is the source bank for the 
**         operation.
**      * Unsets the source for designated bank.
**      * Returns the designated bank state to SAHPI_FUMI_OPERATION_NOTSTARTED.
**      * Does any other cleanup (such as deleting a downloaded source file or 
**         freeing allocated memory) that is needed by the implementation.
**
*******************************************************************************/

SaErrorT SAHPI_API saHpiFumiCleanup (
    SAHPI_IN    SaHpiSessionIdT     SessionId,
    SAHPI_IN    SaHpiResourceIdT    ResourceId,
    SAHPI_IN    SaHpiFumiNumT	    FumiNum,
    SAHPI_IN    SaHpiBankNumT	    BankNum
);

/*******************************************************************************
**
** Name: saHpiHotSwapPolicyCancel()
**
** Description:
**   A resource supporting hot swap typically supports default policies for
**   insertion and extraction. On insertion, the default policy may be for the
**   resource to turn the associated FRU's local power on and to de-assert
**   reset. On extraction, the default policy may be for the resource to
**   immediately power off the FRU and turn on a hot swap indicator. This
**   function allows an HPI User, after receiving a hot swap event with
**   HotSwapState equal to SAHPI_HS_STATE_INSERTION_PENDING or
**   SAHPI_HS_STATE_EXTRACTION_PENDING, to prevent the auto insertion or 
**   auto extraction policy from being automatically executed by the HPI 
**   implementation.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the resource is:
**      * Not in the INSERTION PENDING or EXTRACTION PENDING state.
**      * Processing an auto-insertion or auto-extraction policy.
**
** Remarks:
**   Each time the resource transitions to either the INSERTION PENDING or
**   EXTRACTION PENDING state, the default policies execute after the configured
**   timeout period, unless cancelled using saHpiHotSwapPolicyCancel() after the
**   transition to INSERTION PENDING or EXTRACTION PENDING state and before the
**   timeout expires. The policy cannot be canceled once it has begun to
**   execute.
**
**   This function is only supported by resources that support Managed Hot Swap.  
**   If a resource with the FRU capability set but without the Managed Hot Swap 
**   capability set transitions to INSERTION PENDING or EXTRACTION PENDING, 
**   there is no way for an HPI User to control its insertion or extraction 
**   policy.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapPolicyCancel (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiResourceIdT     ResourceId
);

/*******************************************************************************
**
** Name: saHpiResourceActiveSet()
**
** Description:
**   This function allows an HPI User to request a resource to transition to the
**   ACTIVE state from the INSERTION PENDING or EXTRACTION PENDING state, 
**   executing the auto insertion policy to condition the hardware 
**   appropriately.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the resource is:
**      * Not in the INSERTION PENDING or EXTRACTION PENDING state.
**      * Processing an auto-insertion or auto-extraction policy.
**
** Remarks:
**   During insertion, a resource supporting managed hot swap generates an 
**   event to indicate that it is in the INSERTION PENDING state.  While the
**   resource is in this state, and before it begins execution of the auto 
**   insertion policy, an HPI User may call this function to request that the 
**   auto insertion policy be started immediately.
**
**   If an HPI User calls saHpiHotSwapPolicyCancel() before the resource begins
**   executing the auto insertion policy, the resource remains in INSERTION 
**   PENDING state while actions can be taken to integrate the FRU associated 
**   with this resource into the system.  Once completed with the integration of
**   the FRU, an HPI User must call this function to signal that the resource 
**   should now execute the auto insertion policy and transition into the ACTIVE
**   state.
**
**   An HPI User may also use this function to request a resource to return to
**   the ACTIVE state from the EXTRACTION PENDING state to reject an extraction
**   request.
**
**   This function is only supported by resources that support Managed Hot Swap.  
**   It is only valid if the resource is in INSERTION PENDING or EXTRACTION 
**   PENDING state, and an auto insertion or auto extraction policy has not been
**   initiated.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceActiveSet (
    SAHPI_IN SaHpiSessionIdT     SessionId,
    SAHPI_IN SaHpiResourceIdT    ResourceId
);

/*******************************************************************************
**
** Name: saHpiResourceInactiveSet()
**
** Description:
**   This function allows an HPI User to request a resource to transition to the
**   INACTIVE state from the INSERTION PENDING or EXTRACTION PENDING state, 
**   executing the auto extraction policy to condition the hardware 
**   appropriately.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the resource is:
**      * Not in the INSERTION PENDING or EXTRACTION PENDING state.
**      * Already processing an auto-insertion or auto-extraction policy.
**
** Remarks:
**   During extraction, a resource supporting managed hot swap generates an 
**   event to indicate that it is in the EXTRACTION PENDING state.  While the
**   resource is in this state, and before it begins execution of the auto 
**   extraction policy, an HPI User may call this function to request that the
**   auto extraction policy be started immediately. If an HPI User calls 
**   saHpiHotSwapPolicyCancel() before the resource begins executing the auto
**   extraction policy, the resource remains in EXTRACTION PENDING state while
**   actions can be taken to prepare the system for the FRU associated with this
**   resource to be out of service.  Once the system is prepared for the 
**   shutdown of the FRU, an HPI User must call this function to signal that the
**   resource should now execute the auto extraction policy and transition into 
**   the INACTIVE state.
**
**   An HPI User may also use this function to request a resource to return to
**   the INACTIVE state from the INSERTION PENDING state to abort a hot-swap
**   insertion action.
**
**   This function is only supported by resources that support Managed Hot Swap. 
**   It is only valid if the resource is in EXTRACTION PENDING or INSERTION 
**   PENDING state, and an auto extraction or auto insertion policy has not been
**   initiated.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceInactiveSet (
    SAHPI_IN SaHpiSessionIdT     SessionId,
    SAHPI_IN SaHpiResourceIdT    ResourceId
);

/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutGet()
**
** Description:
**   This function allows an HPI User to request the auto-insert timeout value
**   within a specific domain. This value indicates how long a resource that 
**   supports managed hot swap waits after transitioning to INSERTION PENDING 
**   state before the default auto-insertion policy is invoked.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   Timeout - [out] Pointer to location to store the number of nanoseconds to
**      wait before autonomous handling of the hot swap event. Reserved time out
**      values:
**      * SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is immediate.
**      * SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not occur.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Timeout pointer is passed in
**      as NULL.
**
** Remarks:
**   Each domain maintains a single auto-insert timeout value and it is applied
**   to all contained resources upon insertion, which support managed hot swap.
**   Further information on the auto-insert timeout can be found in the function
**   saHpiAutoInsertTimeoutSet().
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoInsertTimeoutGet(
    SAHPI_IN  SaHpiSessionIdT     SessionId,
    SAHPI_OUT SaHpiTimeoutT       *Timeout
);

/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutSet()
**
** Description:
**   This function allows an HPI User to configure a timeout for how long to 
**   wait before the default auto insertion policy is invoked on a resource that
**   supports managed hot swap within a specific domain.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   Timeout - [in] The number of nanoseconds to wait before autonomous handling
**      of the hot swap event.  Reserved time out values:
**      * SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**         handling.
**      * SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_READ_ONLY is returned if the auto-insert timeout for the domain
**      is fixed as indicated by the SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY flag
**      in the DomainInfo structure.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Timeout parameter is not set
**      to SAHPI_TIMEOUT_BLOCK, SAHPI_TIMEOUT_IMMEDIATE or a positive value.
**
** Remarks:
**   This function accepts a parameter instructing each resource that supports 
**   managed hot swap to impose a delay before performing its hot swap policy 
**   for auto insertion.  The parameter may be set to SAHPI_TIMEOUT_IMMEDIATE to
**   direct resources to proceed immediately to auto insertion, or to 
**   SAHPI_TIMEOUT_BLOCK to prevent auto insertion policy from running until an
**   HPI User calls saHpiResourceActiveSet().  If the parameter is set to 
**   another value, it defines the number of nanoseconds between the time a hot
**   swap event with HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING is 
**   generated and the time that the auto insertion policy is started for that
**   resource.  If, during this time period, an saHpiHotSwapPolicyCancel() 
**   function call is processed, the timer is stopped, and the auto insertion 
**   policy is not invoked.  Each domain maintains a single auto insertion 
**   timeout value, and this value is applied to all contained resources that
**   support managed hot swap.
**
**   Once the auto-insertion policy begins, an HPI User is not allowed to cancel
**   the insertion policy; hence, the timeout should be set appropriately to
**   allow for this condition.  Note that the timeout period begins when the hot
**   swap event with HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING is
**   initially generated; not when it is received by an HPI User with a
**   saHpiEventGet() function call, or even when it is placed in a session event
**   queue.
**
**   A resource may exist in multiple domains, which themselves may have
**   different auto-insertion timeout values.  Upon insertion, the resource
**   begins its auto-insertion policy based on the smallest auto-insertion
**   timeout value.  As an example, if a resource is inserted into two domains,
**   one with an auto-insertion timeout of 5 seconds, and one with an
**   auto-insertion timeout of 10 seconds, the resource begins its
**   auto-insertion policy at 5 seconds. A resource may also be designed to 
**   always immediately begin execution of its auto insertion policy upon 
**   insertion.  The SAHPI_HS_CAPABILITY_AUTOINSERT_IMMEDIATE flag will be set
**   in the RPT entry of such a resource.
**
**   A domain may have a fixed, preset timeout value used for all resources.  
**   In such cases, the SAHPI_DOMAIN_CAP_AUTOINSERT_READ_ONLY flag is set to 
**   indicate that an HPI User cannot change the auto insertion timeout value. 
**   SA_ERR_HPI_READ_ONLY is returned if an HPI User attempts to change a 
**   read-only timeout.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoInsertTimeoutSet( 
    SAHPI_IN SaHpiSessionIdT        SessionId,
    SAHPI_IN SaHpiTimeoutT          Timeout
);

/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutGet()
**
** Description:
**   This function allows an HPI User to request the timeout for how long a
**   resource that supports managed hot swap waits after transitioning to 
**   EXTRACTION PENDING state before the default auto-extraction policy is 
**   invoked.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   Timeout - [out] Pointer to location to store the number of nanoseconds to
**      wait before autonomous handling of the hot swap event. Reserved time out
**      values:
**      * SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is immediate.
**      * SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not occur.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the Timeout pointer is passed in
**      as NULL.
**
** Remarks:
**   This function is only supported by resources that support Managed Hot Swap.  
**   Further information on auto extraction timeouts is detailed in 
**   saHpiAutoExtractTimeoutSet().
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoExtractTimeoutGet(
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_IN  SaHpiResourceIdT     ResourceId,
    SAHPI_OUT SaHpiTimeoutT        *Timeout
);

/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutSet()
**
** Description:
**   This function allows an HPI User to configure a timeout for how long a 
**   resource that supports managed hot swap will wait before the auto 
**   extraction policy is invoked. 
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   Timeout - [in] The number of nanoseconds to wait before autonomous handling
**      of the hot swap event. Reserved timeout values:
**      * SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**         handling.
**      * SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Timeout parameter is not set
**      to SAHPI_TIMEOUT_BLOCK, SAHPI_TIMEOUT_IMMEDIATE or a positive value.
**   SA_ERR_HPI_READ_ONLY is returned if the auto-extract timeout for the
**      resource is fixed, as indicated by the
**      SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY in the resource's RPT entry.
**
** Remarks:
**   This function accepts a parameter instructing the resource to impose a
**   delay before performing its hot swap policy for auto extraction.  The 
**   parameter may be set to SAHPI_TIMEOUT_IMMEDIATE to direct the resource
**   to proceed immediately to auto extraction, or to SAHPI_TIMEOUT_BLOCK to
**   prevent the auto extraction policy from running until an HPI User calls
**   saHpiResourceInactiveSet().  If the parameter is set to another value, it
**   defines the number of nanoseconds between the time a hot swap event with 
**   HotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING is generated and the time 
**   that the auto extraction policy is invoked for the resource.  If, during
**   this time period, an saHpiHotSwapPolicyCancel() function call is processed,
**   the timer is stopped, and the auto extraction policy is not invoked. 
**
**   Once the auto-extraction policy begins, an HPI User is not allowed to
**   cancel the extraction policy; hence, the timeout should be set
**   appropriately to allow for this condition. Note that the timeout period
**   begins when the hot swap event with HotSwapState =
**   SAHPI_HS_STATE_EXTRACTION_PENDING is initially generated; not when it is
**   received by a HPI User with a saHpiEventGet() function call, or even when
**   it is placed in a session event queue.
**
**   The auto extraction timeout period is set at the resource level and is 
**   only supported by resources supporting the Managed Hot Swap capability.  
**   After discovering that a newly inserted resource supports Managed Hot Swap,
**   an HPI User may use this function to change the timeout of the auto
**   extraction policy for that resource.
**
**   An implementation may enforce a fixed, preset timeout value.  In such 
**   cases, the RPT entry for the resource will have the 
**   SAHPI_HS_CAPABILITY_AUTOEXTRACT_READ_ONLY flag set to indicate that an
**   HPI User cannot change the auto extraction timeout value.  
**   SA_ERR_HPI_READ_ONLY is returned if an HPI User attempts to change a 
**   read-only timeout.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoExtractTimeoutSet(
    SAHPI_IN  SaHpiSessionIdT      SessionId,
    SAHPI_IN  SaHpiResourceIdT     ResourceId,
    SAHPI_IN  SaHpiTimeoutT        Timeout
);

/*******************************************************************************
**
** Name: saHpiHotSwapStateGet()
**
** Description:
**   This function allows an HPI User to retrieve the current hot swap state of
**   a resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   State - [out] Pointer to location to store returned state information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the State pointer is passed in as
**      NULL.
**
** Remarks:
**   The returned state is one of the following four states:
**      * SAHPI_HS_STATE_INSERTION_PENDING
**      * SAHPI_HS_STATE_ACTIVE
**      * SAHPI_HS_STATE_EXTRACTION_PENDING
**      * SAHPI_HS_STATE_INACTIVE
**
**   The state SAHPI_HS_STATE_NOT_PRESENT is never returned, because a resource
**   that is not present cannot be addressed by this function in the first
**   place.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapStateGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_OUT SaHpiHsStateT         *State
);

/*******************************************************************************
**
** Name: saHpiHotSwapActionRequest()
**
** Description:
**   This function allows an HPI User to invoke an insertion or extraction
**   process via software on a resource that supports managed hot swap.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   Action - [in] Requested action:  
**      * SAHPI_HS_ACTION_INSERTION
**      * SAHPI_HS_ACTION_EXTRACTION
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support managed
**      hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the resource is not in an
**      appropriate hot swap state, or if the requested action is inappropriate
**      for the current hot swap state.  See the Remarks section below.
**   SA_ERR_HPI_INVALID_PARAMS is returned when Action is not one of the valid
**      enumerated values for this type.
**
** Remarks:
**   A resource supporting hot swap typically requires a physical action on the
**   associated FRU to invoke an insertion or extraction process. An insertion
**   process is invoked by physically inserting the FRU into a chassis.
**   Physically opening an ejector latch or pressing a button invokes the
**   extraction process.  This function provides an alternative means to invoke
**   an insertion or extraction process via software.
**   This function may only be called:
**      * To request an insertion action when the resource is in INACTIVE state.
**      * To request an extraction action when the resource is in the ACTIVE
**         state.
**   The function may not be called when the resource is in any other state.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapActionRequest (
    SAHPI_IN SaHpiSessionIdT     SessionId,
    SAHPI_IN SaHpiResourceIdT    ResourceId,
    SAHPI_IN SaHpiHsActionT      Action
);

/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateGet()
**
** Description:
**   This function allows an HPI User to retrieve the state of the hot swap
**   indicator. A FRU associated with a resource that supports managed hot swap 
**   may include a hot swap indicator such as a blue LED. This indicator 
**   signifies that the FRU is ready for removal.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   State - [out] Pointer to location to store state of hot swap indicator.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support:
**      * Managed hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in
**         the resource's RPT entry.
**      * A hot swap indicator on the FRU as indicated by the
**         SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the State pointer is passed in as
**      NULL.
**
** Remarks:
**   The returned state is either SAHPI_HS_INDICATOR_OFF or
**   SAHPI_HS_INDICATOR_ON. This function returns the state of the indicator,
**   regardless of what hot swap state the resource is in.
**
**   This function is only supported by resources that support managed hot swap.
**   Furthermore, not all resources supporting managed hot swap necessarily 
**   support this function.  Whether a resource supports the hot swap indicator
**   is specified in the Hot Swap Capabilities field of the RPT entry.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapIndicatorStateGet (
    SAHPI_IN  SaHpiSessionIdT            SessionId,
    SAHPI_IN  SaHpiResourceIdT           ResourceId,
    SAHPI_OUT SaHpiHsIndicatorStateT     *State
);

/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateSet()
**
** Description:
**   This function allows an HPI User to set the state of the hot swap
**   indicator. A FRU associated with a resource that supports managed hot swap 
**   may include a hot swap indicator such as a blue LED. This indicator 
**   signifies that the FRU is ready for removal.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   State - [in] State of hot swap indicator to be set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support:
**      * Managed hot swap, as indicated by SAHPI_CAPABILITY_MANAGED_HOTSWAP in
**         the resource's RPT entry.
**      * A hot swap indicator on the FRU as indicated by the
**         SAHPI_HS_CAPABILITY_INDICATOR_SUPPORTED in the resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when State is not one of the valid
**      enumerated values for this type.
**
** Remarks:
**   Valid states include SAHPI_HS_INDICATOR_OFF or SAHPI_HS_INDICATOR_ON. This
**   function sets the indicator regardless of what hot swap state the resource
**   is in, though it is recommended that this function be used only in
**   conjunction with moving the resource to the appropriate hot swap state.
**
**   This function is only supported by resources that support managed hot swap.
**   Furthermore, not all resources supporting managed hot swap necessarily 
**   support this function. Whether or not a resource supports the hot swap 
**   indicator is specified in the Hot Swap Capabilities field of the RPT entry.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapIndicatorStateSet (
    SAHPI_IN SaHpiSessionIdT           SessionId,
    SAHPI_IN SaHpiResourceIdT          ResourceId,
    SAHPI_IN SaHpiHsIndicatorStateT    State
);

/*******************************************************************************
**
** Name: saHpiParmControl()
**
** Description:
**   This function allows an HPI User to save and restore parameters associated
**   with a specific resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   Action - [in] Action to perform on resource parameters.
**      * SAHPI_DEFAULT_PARM	Restores the factory default settings for a
**         specific resource. Factory defaults include Sensor thresholds and
**         configurations, and resource- specific configuration parameters.
**      * SAHPI_SAVE_PARM	Stores the resource configuration parameters in
**         non-volatile stor"age. Resource configuration parameters stored in
**         non-volatile stor"age survive power cycles and resource resets.
**      * SAHPI_RESTORE_PARM	Restores resource configuration parameters from
**         non-volatile stor"age. Resource configuration parameters include
**         Sensor thresholds and Sensor configurations, as well as
**         resource-specific parameters.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support
**      parameter control, as indicated by SAHPI_CAPABILITY_CONFIGURATION in the
**      resource's RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when Action is not set to a proper
**      value.
**
** Remarks:
**   Resource-specific parameters should be documented in an implementation
**   guide for the HPI implementation.
**
**   When this API is called with SAHPI_RESTORE_PARM as the action prior to
**   having made a call with this API where the action parameter was set to
**   SAHPI_SAVE_PARM, the default parameters are restored.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiParmControl (
    SAHPI_IN SaHpiSessionIdT      SessionId,
    SAHPI_IN SaHpiResourceIdT     ResourceId,
    SAHPI_IN SaHpiParmActionT     Action
);

/*******************************************************************************
**
** Name: saHpiResourceLoadIdGet()
**
** Description:
**   This function gets the Load Id of an entity, allowing an HPI User to
**   discover what software the entity will load the next time it initiates a
**   software load.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.  The entity
**      defined in the RPT entry for the Resource is the entity addressed by
**      this API.
**   LoadId - [out] The current Load Id.  Load Id is determined by the contents
**      of LoadId->LoadNumber and LoadId->LoadName.  If LoadId->LoadNumber is:
**      * SAHPI_LOAD_ID_DEFAULT: The entity will load implementation-defined
**         default software,
**      * SAHPI_LOAD_ID_BYNAME:  The entity will load software identified by the
**         contents of LoadId->LoadName,
**      * Any other value:  The entity will load software identified by the value
**        of LoadId->LoadNumber.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support load id
**      management, as indicated by SAHPI_CAPABILITY_LOAD_ID in the resource's
**      RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the LoadId  pointer is passed in
**      as NULL.
**
** Remarks:
**   Mapping of LoadId->LoadNumber and LoadId->LoadName to specific software
**   loads is implementation specific.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceLoadIdGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_OUT SaHpiLoadIdT         *LoadId
);

/*******************************************************************************
**
** Name: saHpiResourceLoadIdSet()
**
** Description:
**   This function sets the Load Id of an entity, allowing an HPI User to
**   control which software the entity will load the next time it initiates a
**   software load.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.  The entity
**      defined in the RPT entry for the Resource is the entity addressed by
**      this API.
**   LoadId - [in] The Load Id to be set.  Load Id is determined by the contents
**      of LoadId->LoadNumber and LoadId->LoadName.  If LoadId->LoadNumber is:
**      * SAHPI_LOAD_ID_DEFAULT: The entity will load implementation-defined
**         default software,
**      * SAHPI_LOAD_ID_BYNAME:  The entity will load software identified by the
**         contents of LoadId->LoadName,
**      * Any other value:  The entity will load software identified by the
**         value of LoadId->LoadNumber.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support load
**      management, as indicated by SAHPI_CAPABILITY_LOAD in the resource's RPT
**      entry.
**   SA_ERR_HPI_INVALID_DATA is returned if:
**      * LoadId->LoadNumber is SAHPI_LOAD_DEFAULT and the resource does not
**         have a default load defined.
**      * LoadId->LoadNumber is SAHPI_LOAD_BYNAME and the resource does not
**         support identifying loads by name.
**      * LoadId->LoadNumber is SAHPI_LOAD_BYNAME and the contents of
**      * LoadId->LoadName do not correspond to a currently valid software load.
**      * LoadId->LoadNumber is some value other than SAHPI_LOAD_DEFAULT or
**         SAHPI_LOAD_BYNAME and the value of LoadId->LoadNumber does not
**         correspond to a currently valid software load.
**
** Remarks:
**   Interpretation of LoadId->LoadNumber and LoadId->LoadName is implementation
**   specific. Any particular value or contents of the LoadId->LoadName may be
**   invalid for an implementation.  If an invalid value is passed, the function
**   returns an error.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceLoadIdSet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_IN  SaHpiLoadIdT         *LoadId
);

/*******************************************************************************
**
** Name: saHpiResourceResetStateGet()
**
** Description:
**   This function gets the reset state of an entity, allowing an HPI User to
**   determine if the entity is being held with its reset asserted. This
**   function addresses the entity that is identified in the RPT entry for the
**   resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   ResetAction - [out] The current reset state of the entity. Valid reset
**      states are:
**      * SAHPI_RESET_ASSERT: 	The entity's reset is asserted, e.g., for hot swap
**         insertion/extraction purposes.
**      * SAHPI_RESET_DEASSERT:	The entity's reset is not asserted.
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the reset state
**      has successfully been determined; otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support reset
**      control as indicated by SAHPI_CAPABILITY_RESET in the resource's RPT
**      entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the ResetAction pointer is passed
**      in as NULL.
**
** Remarks:
**   SAHPI_COLD_RESET and SAHPI_WARM_RESET are pulsed resets, and are not valid
**   values to be returned in ResetAction. If the entity is not being held in
**   reset (using SAHPI_RESET_ASSERT), the appropriate value is
**   SAHPI_RESET_DEASSERT.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceResetStateGet (
    SAHPI_IN SaHpiSessionIdT        SessionId,
    SAHPI_IN SaHpiResourceIdT       ResourceId,
    SAHPI_OUT SaHpiResetActionT     *ResetAction
);

/*******************************************************************************
**
** Name: saHpiResourceResetStateSet()
**
** Description:
**   This function directs the resource to perform the specified reset type on
**   the entity that it manages. This function addresses the entity that is
**   identified in the RPT entry for the resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   ResetAction - [in] Type of reset to perform on the entity. Valid reset
**      actions are:
**      * SAHPI_COLD_RESET:	Perform a "Cold Reset" on the entity (pulse),
**         leaving reset de-asserted,
**      * SAHPI_WARM_RESET:	Perform a "Warm Reset" on the entity (pulse),
**         leaving reset de-asserted,
**      * SAHPI_RESET_ASSERT:	Put the entity into reset state and hold reset
**         asserted, e.g., for hot swap insertion/extraction purposes,
**      * SAHPI_RESET_DEASSERT:	Bring the entity out of the reset state.
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the requested
**      reset action has succeeded; otherwise, an error code is returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support resource
**      reset control, as indicated by SAHPI_CAPABILITY_RESET in the resource's
**      RPT entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the ResetAction is not one of
**      the valid enumerated values for this type.
**   SA_ERR_HPI_INVALID_CMD is returned if the requested ResetAction is
**      SAHPI_RESET_ASSERT or SAHPI_WARM_RESET and the resource does not support
**      this action.
**   SA_ERR_HPI_INVALID_REQUEST is returned if the ResetAction is
**      SAHPI_COLD_RESET or SAHPI_WARM_RESET and reset is currently asserted.
**
** Remarks:
**   Some resources may not support holding the entity in reset or performing a
**   warm reset.  If this is the case, the resource should return
**   SA_ERR_HPI_INVALID_CMD if the unsupported action is requested.  All
**   resources that have the SAHPI_CAPABILITY_RESET flag set in their RPTs must
**   support SAHPI_COLD_RESET.
*
**   SAHPI_RESET_ASSERT is used to hold an entity in reset, and
**   SAHPI_RESET_DEASSERT is used to bring the entity out of an asserted reset
**   state.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceResetStateSet (
    SAHPI_IN SaHpiSessionIdT       SessionId,
    SAHPI_IN SaHpiResourceIdT      ResourceId,
    SAHPI_IN SaHpiResetActionT     ResetAction
);

/*******************************************************************************
**
** Name: saHpiResourcePowerStateGet()
**
** Description:
**   This function gets the power state of an entity, allowing an HPI User to
**   determine if the entity is currently powered on or off. This function
**   addresses the entity which is identified in the RPT entry for the resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   State - [out] The current power state of the resource.  Valid power states
**      are:
**      * SAHPI_POWER_OFF: The entity's primary power is OFF,
**      * SAHPI_POWER_ON: The entity's primary power is ON.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support power
**      management, as indicated by SAHPI_CAPABILITY_POWER in the resource's RPT
**      entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned if the State pointer is passed in as
**      NULL.
**
** Remarks:
**   SAHPI_POWER_CYCLE is a pulsed power operation and is not a valid return for
**   the power state.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourcePowerStateGet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_OUT SaHpiPowerStateT      *State
);

/*******************************************************************************
**
** Name: saHpiResourcePowerStateSet()
**
** Description:
**   This function directs the resource to perform a power control action on
**   the entity that is dientified in the RPT entry for the resource.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously obtained using
**      saHpiSessionOpen().
**   ResourceId - [in] Resource identified for this operation.
**   State - [in] The requested power control action. Valid values are:
**      * SAHPI_POWER_OFF: The entity's primary power is turned OFF,
**      * SAHPI_POWER_ON: The entity's primary power is turned ON,
**      * SAHPI_POWER_CYCLE: The entity's primary power is turned OFF, then
**         turned ON.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is
**      returned.
**   SA_ERR_HPI_CAPABILITY is returned if the resource does not support power
**      management, as indicated by SAHPI_CAPABILITY_POWER in the resource's RPT
**      entry.
**   SA_ERR_HPI_INVALID_PARAMS is returned when State is not one of the valid
**      enumarated values for this type.
**
** Remarks:
**   This function controls the hardware power on a FRU entity regardless of
**   what hot-swap state the resource is in. For example, it is legal (and may
**   be desirable) to cycle power on the FRU even while it is in ACTIVE state
**   to attempt to clear a fault condition. Similarly, a resource could be
**   instructed to power on a FRU even while it is in INACTIVE state, for
**   example, to run off-line diagnostics.
**   
**   This function may also be supported for non-FRU entities if power control
**   is available for those entities.
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourcePowerStateSet (
    SAHPI_IN  SaHpiSessionIdT       SessionId,
    SAHPI_IN  SaHpiResourceIdT      ResourceId,
    SAHPI_IN  SaHpiPowerStateT      State
);

#ifdef __cplusplus
}
#endif

#endif /* __SAHPI_H */
