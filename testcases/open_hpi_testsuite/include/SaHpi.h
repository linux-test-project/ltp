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
**   SAI-HPI-A.01.01
**
** DATE: 
**   Thu Oct  3 14:48:41  2002
**     Updated Legal language February 12, 2003
**
** LEGAL:
**   BSD license
**    
**   Copyright(c) 2002, Service Availability(TM) Forum. All rights
**   reserved. 
**
**   Redistribution and use in source and binary forms, with or without
**   modification, are permitted provided that the following conditions
**   are met:
**
**   Redistributions of source code must retain the above copyright notice,
**   this list of conditions and the following disclaimer.  Redistributions
**   in binary form must reproduce the above copyright notice, this list of
**   conditions and the following disclaimer in the documentation and/or
**   other materials provided with the distribution.  Neither the name of
**   the Service Availability(TM) Forum nor the names of its contributors 
**   may be used to endorse or promote products derived from this software 
**   without specific prior written permission.
**   
**   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
**   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
**   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
**   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
**   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
**   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
**   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
**   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
**   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**   
**
*******************************************************************************/

#ifndef __SAHPI_H
#define __SAHPI_H

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                 Basic Data Types and Values                **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* General Types - need to be specified correctly for the host architecture */
typedef unsigned char  SaHpiUint8T;
typedef unsigned short SaHpiUint16T;
typedef unsigned int   SaHpiUint32T;
typedef char           SaHpiInt8T;
typedef short          SaHpiInt16T;
typedef int            SaHpiInt32T;
typedef long long      SaHpiInt64T;
typedef float          SaHpiFloat32T;
typedef double         SaHpiFloat64T;

typedef SaHpiUint8T     SaHpiBoolT;
#define SAHPI_TRUE      1
#define SAHPI_FALSE     0 

/* Platform, O/S, or Vendor dependent */
#define SAHPI_API
#define SAHPI_IN
#define SAHPI_OUT
#define SAHPI_INOUT

/* 
** Identifier for the manufacturer 
**
** This is the IANA-assigned private enterprise number for the
** manufacturer of the resource or FRU, or of the manufacturer
** defining an OEM control or event type. A list of current
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

/* Version Types  */
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
#define SAHPI_INTERFACE_VERSION (SaHpiVersionT)0x010101  /* A.01.01 */

/*
** Return Codes
**
** SaErrorT is defined in the HPI specification.  In the future a
** common SAF types definition may be created to contain this type. At
** that time, this typedef should be removed.
*/
typedef SaHpiInt32T SaErrorT; /* Return code */

/*
** SA_OK: This code indicates that a command completed successfully.
*/
#define SA_OK                          (SaErrorT)0x0000

/* This value is the base for all HPI-specific error codes. */
#define SA_HPI_ERR_BASE                -1000

/*
** SA_ERR_HPI_ERROR:  An unspecified error occurred.  This code should
** be returned only as a last resort; eg. if the cause of an error
** cannot be determined.
*/
#define SA_ERR_HPI_ERROR               (SaErrorT)(SA_HPI_ERR_BASE - 1)

/*
** SA_ERR_HPI_UNSUPPORTED_API:  The HPI implementation does not support
** this API.  This code is appropriate, for example, if invoking the
** Hot Swap functions on an HPI implementation which has no hot swap
** support.  Note that such an implementation would not report any hot
** swap capabilities via its RDRs.
*/
#define SA_ERR_HPI_UNSUPPORTED_API     (SaErrorT)(SA_HPI_ERR_BASE - 2)

/*
** SA_ERR_HPI_BUSY:  The command cannot be performed because the
** targeted device is busy.
*/
#define SA_ERR_HPI_BUSY                (SaErrorT)(SA_HPI_ERR_BASE - 3)

/*
** SA_ERR_HPI_INVALID:  The request is fundamentally invalid.
*/
#define SA_ERR_HPI_INVALID             (SaErrorT)(SA_HPI_ERR_BASE - 4)

/*
** SA_ERR_HPI_INVALID_CMD:  The specific object to which a command was
** directed does not support that command (which was otherwise valid).
*/
#define SA_ERR_HPI_INVALID_CMD         (SaErrorT)(SA_HPI_ERR_BASE - 5)

/*
** SA_ERR_HPI_TIMEOUT: The requested operation, which had a timeout
** value specified, timed out.  For example, when reading input with a
** timeout value, if no input arrives within the timeout interval,
** this code should be returned.  This should only be returned in
** cases where a timeout is anticipated as a valid consequence of the
** operation; if the addressed entity is not responding due to a
** fault, use SA_ERR_HPI_NO_RESPONSE (qv).
*/
#define SA_ERR_HPI_TIMEOUT             (SaErrorT)(SA_HPI_ERR_BASE - 6)

/*
** SA_ERR_HPI_OUT_OF_SPACE:  The requested command failed due to
** resource limits.
*/
#define SA_ERR_HPI_OUT_OF_SPACE        (SaErrorT)(SA_HPI_ERR_BASE - 7)

/*
** SA_ERR_HPI_DATA_TRUNCATED:  The returned data was truncated.  For
** example, when reading data into a fixed-size buffer, if the data is
** larger than the buffer, this code should be returned.
*/
#define SA_ERR_HPI_DATA_TRUNCATED      (SaErrorT)(SA_HPI_ERR_BASE - 8)

/*
** SA_ERR_HPI_DATA_LEN_INVALID:  The specified data length is invalid.
*/
#define SA_ERR_HPI_DATA_LEN_INVALID    (SaErrorT)(SA_HPI_ERR_BASE - 9)

/*
** SA_ERR_HPI_DATA_EX_LIMITS:  The supplied data exceeds limits.
*/
#define SA_ERR_HPI_DATA_EX_LIMITS      (SaErrorT)(SA_HPI_ERR_BASE - 10)

/*
** SA_ERR_HPI_INVALID_PARAMS:  One or more parameters to the command
** were invalid.
*/
#define SA_ERR_HPI_INVALID_PARAMS      (SaErrorT)(SA_HPI_ERR_BASE - 11)

/*
** SA_ERR_HPI_INVALID_DATA:  Cannot return requested data; eg. the
** specific object to which a command was directed does not support
** the data required by the command.
*/
#define SA_ERR_HPI_INVALID_DATA        (SaErrorT)(SA_HPI_ERR_BASE - 12)

/*
** SA_ERR_HPI_NOT_PRESENT:  The requested object was not present.  For
** example, this code would be returned when attempting to access an
** entry in a RPT or RDR which is not present.  As another example, this
** code would also be returned when accessing an invalid management 
** instrument on a valid resource. 
*/
#define SA_ERR_HPI_NOT_PRESENT         (SaErrorT)(SA_HPI_ERR_BASE - 13)

/*
** SA_ERR_HPI_INVALID_DATA_FIELD:  Invalid data field.
*/
#define SA_ERR_HPI_INVALID_DATA_FIELD  (SaErrorT)(SA_HPI_ERR_BASE - 14)

/*
** SA_ERR_HPI_INVALID_SENSOR_CMD:  Invalid sensor command.
*/
#define SA_ERR_HPI_INVALID_SENSOR_CMD  (SaErrorT)(SA_HPI_ERR_BASE - 15)

/*
** SA_ERR_HPI_NO_RESPONSE: There was no response from the domain or
** object targeted by the command, due to some fault.  This code
** indicates an un-anticipated failure to respond; compare with
** SA_ERR_HPI_TIMEOUT.
*/
#define SA_ERR_HPI_NO_RESPONSE         (SaErrorT)(SA_HPI_ERR_BASE - 16)

/*
** SA_ERR_HPI_DUPLICATE:  Duplicate request -- such as attempting to
** initialize something which has already been initialized (and which
** cannot be initialized twice).
*/
#define SA_ERR_HPI_DUPLICATE           (SaErrorT)(SA_HPI_ERR_BASE - 17)

/*
** SA_ERR_HPI_UPDATING:  The command could not be completed because
** the targeted object is in update mode.
*/
#define SA_ERR_HPI_UPDATING            (SaErrorT)(SA_HPI_ERR_BASE - 18)

/*
** SA_ERR_HPI_INITIALIZING:  The command could not be completed because
** the targeted object is initializing.
*/
#define SA_ERR_HPI_INITIALIZING        (SaErrorT)(SA_HPI_ERR_BASE - 19)

/*
** SA_ERR_HPI_UNKNOWN:  This code should be returned if, for some
** reason, the HPI implementation cannot determine the proper response
** to a command.  For example, there may be a proper value to return
** for a given call, but the implementation may be unable to determine
** which one it is.
*/
#define SA_ERR_HPI_UNKNOWN             (SaErrorT)(SA_HPI_ERR_BASE - 20)

/*
** SA_ERR_HPI_INVALID_SESSION:  An invalid session ID was specified in
** the command.
*/
#define SA_ERR_HPI_INVALID_SESSION     (SaErrorT)(SA_HPI_ERR_BASE - 21)

/*
** SA_ERR_HPI_INVALID_DOMAIN:  Invalid domain ID specified -- ie. a
** domain ID which does not correspond to any real domain was
** specified in the command.
*/
#define SA_ERR_HPI_INVALID_DOMAIN      (SaErrorT)(SA_HPI_ERR_BASE - 22)

/*
** SA_ERR_HPI_INVALID_RESOURCE:  Invalid resource ID specified -- ie. a
** resource ID which does not correspond to a resource in the addressed
** domain was specified in the command.
*/
#define SA_ERR_HPI_INVALID_RESOURCE    (SaErrorT)(SA_HPI_ERR_BASE - 23)

/*
** SA_ERR_HPI_INVALID_REQUEST:  The request is invalid in the current
** context.  An example would be attempting to unsubscribe for events,
** when the caller has not subscribed to events.
*/
#define SA_ERR_HPI_INVALID_REQUEST     (SaErrorT)(SA_HPI_ERR_BASE - 24)

/*
** SA_ERR_HPI_ENTITY_NOT_PRESENT:  The addressed management instrument is not active
** because the entity with which it is associated is not present.  This 
** condition could occur, for instance, when an alarm module is managing a 
** fan tray FRU.  The alarm module would contain management instruments (sensors,
** etc) for the fan tray.  The fan tray may be removed, even though the
** management instruments are still represented in the alarm module.  In this
** case, SA_ERR_HPI_ENTITY_NOT_PRESENT would be returned if a management instrument
** associated with a removed entity is accessed.
*/
#define SA_ERR_HPI_ENTITY_NOT_PRESENT         (SaErrorT)(SA_HPI_ERR_BASE - 25)

/*
** SA_ERR_HPI_UNINITIALIZED: This code is returned when a request is
** made, and the HPI has not, yet, been initialized via saHpiInitialize().
*/
#define SA_ERR_HPI_UNINITIALIZED   (SaErrorT)(SA_HPI_ERR_BASE - 26)


/*
** Domain, Session and Resource Type Definitions
*/

/* Domain ID. */
typedef SaHpiUint32T SaHpiDomainIdT;
#define SAHPI_DEFAULT_DOMAIN_ID   (SaHpiDomainIdT)0

/* The SAHPI_UNSPECIFIED_DOMAIN_ID value may be used by an implementation
** when populating the ResourceId value for an RPT entry that is a
** domain only.
*/
#define SAHPI_UNSPECIFIED_DOMAIN_ID (SaHpiDomainIdT) 0xFFFFFFFF

/* Session ID. */
typedef SaHpiUint32T SaHpiSessionIdT;

/* Resource identifier. */
typedef SaHpiUint32T SaHpiResourceIdT;

/* The SAHPI_UNSPECIFIED_RESOURCE_ID value may be used by an implementation
** when populating the DomainId value for an RPT entry that is a 
** resource only.  Note that this reserved value (0xFFFFFFFF) is also used
** to designate the domain controller, for domain-based event log access.
*/
#define SAHPI_UNSPECIFIED_RESOURCE_ID (SaHpiResourceIdT) 0xFFFFFFFF

/* The SAHPI_DOMAIN_CONTROLLER_ID value is a reserved resource ID
** value which is used to select the domain controller's event log
** (as opposed to a real resource's event log) when accessing logs.
** This value must not be used as the ID of any real resource. 
*/
#define SAHPI_DOMAIN_CONTROLLER_ID    (SaHpiResourceIdT) 0xFFFFFFFE

/* Table Related Type Definitions  */
typedef SaHpiUint32T SaHpiEntryIdT;
#define SAHPI_FIRST_ENTRY (SaHpiEntryIdT)0x00000000
#define SAHPI_LAST_ENTRY  (SaHpiEntryIdT)0xFFFFFFFF

/*
** Time Related Type Definitions
**
** An HPI time value represents the local time as the number of nanoseconds 
** from 00:00:00, January 1, 1970, in a 64-bit signed integer. This format
** is sufficient to represent times with nano-second resolution from the
** year 1678 to 2262. Every API which deals with time values must define
** the timezone used.
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
** 
** Some subsystems may need the flexibility to report either absolute or
** relative (eg. to system boot) times. This will typically be in the
** case of a board which may or may not, depending on the system setup,
** have an idea of absolute time. For example, some boards may have
** "time of day" clocks which start at zero, and never get set to the
** time of day.
**
** In these cases, times which represent "current" time (in events, for
** example) can be reported based on the clock value, whether it has been
** set to the actual date/time, or whether it represents the elapsed time
** since boot. If it is the time since boot, the value will be (for 27
** years) less than 0x0C00000000000000, which is Mon May 26 16:58:48 1997.
** If the value is greater than this, then it can be assumed to be an
** absolute time.
**
** Every API which can report either absolute or relative times must
** state this rule clearly in its interface specification. 
*/
typedef SaHpiInt64T SaHpiTimeT;    /* Time in nanoseconds */

/* Unspecified or unknown time */
#define SAHPI_TIME_UNSPECIFIED     ((SaHpiTimeT) 0x8000000000000000LL)

/* Maximum time that can be specified as relative */
#define SAHPI_TIME_MAX_RELATIVE    ((SaHpiTimeT) 0x0C00000000000000LL)
typedef SaHpiInt64T SaHpiTimeoutT; /* Timeout in nanoseconds */

/* Non-blocking call */
#define SAHPI_TIMEOUT_IMMEDIATE    (SaHpiTimeoutT) 0x0000000000000000

/* Blocking call, wait indefinitely for call to complete */
#define SAHPI_TIMEOUT_BLOCK        (SaHpiTimeoutT) -1

/*
** Language
**
** This enumeration lists all of the languages that can be associated with text.
**
** SAHPI_LANG_UNDEF indicates that the language is unspecified or
** unknown.
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
    SAHPI_LANG_CHINESE, SAHPI_LANG_ZULU
} SaHpiLanguageT;

/*
** Text Buffers
**
** These structures are used for defining the type of data in the text buffer 
** and the length of the buffer. Text buffers are used in the inventory data,
** RDR, RPT, etc. for variable length strings of data.
*/

#define SAHPI_MAX_TEXT_BUFFER_LENGTH  255

typedef enum {
    SAHPI_TL_TYPE_BINARY = 0,     /* String of bytes, any values legal */
    SAHPI_TL_TYPE_BCDPLUS,        /* String of 0-9, space, dash, period ONLY */
    SAHPI_TL_TYPE_ASCII6,         /* Reduced ASCII character set: 0x20-0x5F 
                                     ONLY */
    SAHPI_TL_TYPE_LANGUAGE        /* ASCII or UNICODE depending on language */
} SaHpiTextTypeT;

typedef struct {
    SaHpiTextTypeT DataType;
    SaHpiLanguageT Language;      /* Language the text is in. */
    SaHpiUint8T    DataLength;    /* Bytes used in Data buffer  */ 
    SaHpiUint8T    Data[SAHPI_MAX_TEXT_BUFFER_LENGTH];  /* Data buffer */
} SaHpiTextBufferT;

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
** Entities are used to associate specific hardware components with sensors, 
** controls, watchdogs, or resources. Entities are defined with an entity 
** type enumeration, and an entity instance number (to distinguish between 
** multiple instances of a particular type of entity; e.g., multiple power 
** supplies in a system).
**
** Entities are uniquely identified in a system with an ordered series of
** Entity Type / Entity Instance pairs called an "Entity Path". Each subsequent
** Entity Type/Entity Instance in the path is the next higher "containing"
** entity. The "root" of the Entity Path (the outermost level of containment)
** is designated with an Entity Type of SAHPI_ENT_ROOT if the entire Entity Path 
** is fewer than SAHPI_MAX_ENTITY_PATH entries in length.
**
** Enumerated Entity Types include those types enumerated by the IPMI Consortium
** for IPMI-managed entities, as well as additional types defined by the 
** HPI specification. Room is left in the enumeration for the inclusion of
** Entity Types taken from other lists, if needed in the future.
*/
/* Base values for entity types from various sources. */
#define SAHPI_ENT_IPMI_GROUP 0
#define SAHPI_ENT_SAFHPI_GROUP 0x10000
#define SAHPI_ENT_ROOT_VALUE 0xFFFF
typedef enum
{
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
                                     boards that hold SECC modules) */
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
                                    provided to allow a sensor data
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
    SAHPI_ENT_CHASSIS_SPECIFIC    = SAHPI_ENT_IPMI_GROUP + 0x90,
    SAHPI_ENT_BOARD_SET_SPECIFIC  = SAHPI_ENT_IPMI_GROUP + 0xB0,
    SAHPI_ENT_OEM_SYSINT_SPECIFIC = SAHPI_ENT_IPMI_GROUP + 0xD0,
    SAHPI_ENT_ROOT = SAHPI_ENT_ROOT_VALUE,
    SAHPI_ENT_RACK = SAHPI_ENT_SAFHPI_GROUP,
    SAHPI_ENT_SUBRACK,
    SAHPI_ENT_COMPACTPCI_CHASSIS,
    SAHPI_ENT_ADVANCEDTCA_CHASSIS,
    SAHPI_ENT_SYSTEM_SLOT,
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
    SAHPI_ENT_ALARM_MANAGER_BLADE,       /* Blade-based alarm manager */
    SAHPI_ENT_SUBBOARD_CARRIER_BLADE     /* Includes PMC Carrier Blade --
                                            Use only if "carrier" is only
                                            function of blade. Else use
                                            primary function (SBC_BLADE,
                                            DSP_BLADE, etc.). */
 } SaHpiEntityTypeT;

typedef SaHpiUint32T SaHpiEntityInstanceT;

typedef struct {
    SaHpiEntityTypeT     EntityType;
    SaHpiEntityInstanceT EntityInstance;
} SaHpiEntityT;

#define SAHPI_MAX_ENTITY_PATH 16

typedef struct {
    SaHpiEntityT  Entry[SAHPI_MAX_ENTITY_PATH];
} SaHpiEntityPathT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                        Events, part 1                      **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Category
**
** Sensor events contain an event category and event state. Depending on the 
** event category, the event states take on different meanings for events 
** generated by specific sensors.
**
** The SAHPI_EC_GENERIC category can be used for discrete sensors which have 
** state meanings other than those identified with other event categories.
*/
typedef SaHpiUint8T SaHpiEventCategoryT;

#define SAHPI_EC_UNSPECIFIED  (SaHpiEventCategoryT)0x00 /* Unspecified */
#define SAHPI_EC_THRESHOLD    (SaHpiEventCategoryT)0x01 /* Threshold 
                                                          events */
#define SAHPI_EC_USAGE        (SaHpiEventCategoryT)0x02 /* Usage state
                                                          events */
#define SAHPI_EC_STATE        (SaHpiEventCategoryT)0x03 /* Generic state
                                                          events */
#define SAHPI_EC_PRED_FAIL    (SaHpiEventCategoryT)0x04 /* Predictive fail 
                                                          events */
#define SAHPI_EC_LIMIT        (SaHpiEventCategoryT)0x05 /* Limit events */
#define SAHPI_EC_PERFORMANCE  (SaHpiEventCategoryT)0x06 /* Performance 
                                                          events    */
#define SAHPI_EC_SEVERITY     (SaHpiEventCategoryT)0x07 /* Severity  
                                                          indicating
                                                          events */
#define SAHPI_EC_PRESENCE     (SaHpiEventCategoryT)0x08 /* Device presence
                                                          events */
#define SAHPI_EC_ENABLE       (SaHpiEventCategoryT)0x09 /* Device enabled
                                                          events */
#define SAHPI_EC_AVAILABILITY (SaHpiEventCategoryT)0x0A /* Availability
                                                          state events */

#define SAHPI_EC_REDUNDANCY   (SaHpiEventCategoryT)0x0B /* Redundancy 
                                                          state events */
#define SAHPI_EC_USER                   (SaHpiEventCategoryT)0x7E /* User defined
                                                          events */
#define SAHPI_EC_GENERIC           (SaHpiEventCategoryT)0x7F /* OEM defined
                                                          events */

/*
** Event States
**
** The following event states are specified relative to the categories listed 
** above. The event types are only valid for their given category. Each set of 
** events is labeled as to which category it belongs to.
** Each event will have only one event state associated with it. When retrieving 
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
** SaHpiEventCategoryT == SAHPI_EC_GENERIC || SAHPI_EC_USER
** These event states are defined by the OEM or the user of the
** implementation. 
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
typedef SaHpiUint8T SaHpiSensorNumT;

/* Type of Sensor */
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
    SAHPI_OPERATIONAL = 0xA0,
    SAHPI_OEM_SENSOR=0xC0
}  SaHpiSensorTypeT;

/*
** Interpreted Sensor Reading Type
**
** These definitions list the available data types that can be
** used for interpreted sensor readings. Interpreted sensor readings are provided
** because typically sensors measure their associated entities in a way that is
** not human readable/understandable. For example a fan sensor may measure the 
** number of ticks that it takes a fan blade to move passed a sensor. The human 
** readable reading type would be revolutions per minute (RPM). 
*/

#define SAHPI_SENSOR_BUFFER_LENGTH 32

typedef enum {
      SAHPI_SENSOR_INTERPRETED_TYPE_UINT8,
      SAHPI_SENSOR_INTERPRETED_TYPE_UINT16,
      SAHPI_SENSOR_INTERPRETED_TYPE_UINT32,
      SAHPI_SENSOR_INTERPRETED_TYPE_INT8,
      SAHPI_SENSOR_INTERPRETED_TYPE_INT16,
      SAHPI_SENSOR_INTERPRETED_TYPE_INT32,
      SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32,
           SAHPI_SENSOR_INTERPRETED_TYPE_BUFFER    /* 32 byte array */
} SaHpiSensorInterpretedTypeT;

typedef union {
    SaHpiUint8T          SensorUint8;
    SaHpiUint16T         SensorUint16;
    SaHpiUint32T         SensorUint32;
    SaHpiInt8T           SensorInt8;
    SaHpiInt16T          SensorInt16;
    SaHpiInt32T          SensorInt32;
    SaHpiFloat32T        SensorFloat32; 
    SaHpiUint8T          SensorBuffer[SAHPI_SENSOR_BUFFER_LENGTH];
} SaHpiSensorInterpretedUnionT;

typedef struct {
    SaHpiSensorInterpretedTypeT  Type;
    SaHpiSensorInterpretedUnionT Value;
} SaHpiSensorInterpretedT;

/*
** Sensor Status
**
** The sensor status structure is used to determine if sensor scanning is 
** enabled and if events are enabled. If events are enabled, the structure will 
** have valid data for the outstanding sensor event states.
*/
typedef SaHpiUint8T SaHpiSensorStatusT;
#define SAHPI_SENSTAT_EVENTS_ENABLED (SaHpiSensorStatusT)0x80
#define SAHPI_SENSTAT_SCAN_ENABLED   (SaHpiSensorStatusT)0x40
#define SAHPI_SENSTAT_BUSY           (SaHpiSensorStatusT)0x20

typedef struct {
    SaHpiSensorStatusT SensorStatus;
    SaHpiEventStateT   EventStatus;
} SaHpiSensorEvtStatusT;

/* Sensor Event Enables */
typedef struct {
    SaHpiSensorStatusT SensorStatus;
    SaHpiEventStateT   AssertEvents; 
    SaHpiEventStateT   DeassertEvents;
} SaHpiSensorEvtEnablesT;

/*
** Sensor Reading
**
** The sensor reading type is the data structure returned from a call to get 
** sensor reading. The structure is also used when setting and getting sensor 
** threshold values and reporting sensor ranges.
** Each sensor may support one or more of raw, interpreted, or event status 
** representations of the sensor data. For analog sensors the raw value is the 
** raw value from the sensor (such as ticks per fan blade) and the interpreted 
** value is the raw value converted in to a usable format (such as RPM). The 
** interpreted value can be calculated by the HPI implementation using the 
** sensor factors or by another OEM means.
*/
typedef SaHpiUint8T SaHpiSensorReadingFormatsT;
#define SAHPI_SRF_RAW         (SaHpiSensorReadingFormatsT)0x01
#define SAHPI_SRF_INTERPRETED (SaHpiSensorReadingFormatsT)0x02 
#define SAHPI_SRF_EVENT_STATE (SaHpiSensorReadingFormatsT)0x04

typedef struct {
      SaHpiSensorReadingFormatsT  ValuesPresent;
      SaHpiUint32T                Raw;
      SaHpiSensorInterpretedT     Interpreted;
      SaHpiSensorEvtStatusT       EventStatus;
} SaHpiSensorReadingT;

/*
** Threshold Values
** This structure encompasses all of the thresholds that can be set.
*/
typedef struct {
    SaHpiSensorReadingT LowCritical;      /* Lower Critical Threshold */
    SaHpiSensorReadingT LowMajor;         /* Lower Major Threshold */
    SaHpiSensorReadingT LowMinor;         /* Lower Minor Threshold */
    SaHpiSensorReadingT UpCritical;       /* Upper critical Threshold */
    SaHpiSensorReadingT UpMajor;          /* Upper major Threshold */
    SaHpiSensorReadingT UpMinor;          /* Upper minor Threshold */
    SaHpiSensorReadingT PosThdHysteresis; /* Positive Threshold Hysteresis */
    SaHpiSensorReadingT NegThdHysteresis; /* Negative Threshold Hysteresis */
}SaHpiSensorThresholdsT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                  Sensor Resource Data Records              **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** Sensor Factors
**
** The sensor factors structure defines the conversion factors for linear and  
** linearized sensors. 
** The SaHpiSensorLinearizationT enumeration coupled with the various other 
** sensor factors define a formula that can be applied to raw sensor data to 
** convert it to appropriate engineering units. If linearization is 
** SAHPI_SL_NONLINEAR, SAHPI_SL_UNSPECIFIED, or SAHPI_SL_OEM  then there is no 
** predefined conversion from raw to interpreted and the sensor factors may or 
** may not be meaningful depending on the implementation.
** For other linearization values, raw readings may be converted to interpreted 
** values using the formula:
**Interpreted = L [( M*raw + B*10^ExpB )*10^ExpR ]  
**where "L[x]" is the indicated linearization function  
** (for SAHPI_SL_LINEAR, L[x]=x).
** The Tolerance Factor is given as +/- 1/2 raw counts, so tolerance in 
** interpreted values can be calculated as:
** L[ M * ToleranceFactor/2 * 10^ExpR ]
** The Accuracy Factor is given as 1/100 of a percent, scaled up by ExpA. Thus 
** the accuracy is:  
**( ( AccuracyFactor/100 ) / 10^ExpA )%
*/
typedef enum {
    SAHPI_SL_LINEAR = 0,       /* Already linear */
    SAHPI_SL_LN,
    SAHPI_SL_LOG10,
    SAHPI_SL_LOG2,
    SAHPI_SL_E,
    SAHPI_SL_EXP10,
    SAHPI_SL_EXP2,
    SAHPI_SL_1OVERX,
    SAHPI_SL_SQRX,
    SAHPI_SL_CUBEX,
    SAHPI_SL_SQRTX,
    SAHPI_SL_CUBERTX,
    SAHPI_SL_NONLINEAR = 0x70, /* Cannot be linearized with a predefind formula*/
    SAHPI_SL_OEM,
    SAHPI_SL_UNSPECIFIED = 0xFF
} SaHpiSensorLinearizationT;

typedef struct {
    SaHpiInt16T                M_Factor;        /* M Factor */
    SaHpiInt16T                B_Factor;        /* B Factor */
    SaHpiUint16T               AccuracyFactor;  /* Accuracy */
    SaHpiUint8T                ToleranceFactor; /* Tolerance */
    SaHpiUint8T                ExpA;            /* Accuracy Exp */
    SaHpiInt8T                 ExpR;            /* Result Exp */
    SaHpiInt8T                 ExpB;            /* B Exp */
    SaHpiSensorLinearizationT  Linearization;  
} SaHpiSensorFactorsT;

/*
**  Sensor Range
** Sensor range values can include minimum, maximum, normal minimum, normal 
** maximum, and nominal values.
*/
typedef SaHpiUint8T SaHpiSensorRangeFlagsT;
#define SAHPI_SRF_MIN        (SaHpiSensorRangeFlagsT)0x10 
#define SAHPI_SRF_MAX        (SaHpiSensorRangeFlagsT)0x08 
#define SAHPI_SRF_NORMAL_MIN (SaHpiSensorRangeFlagsT)0x04
#define SAHPI_SRF_NORMAL_MAX (SaHpiSensorRangeFlagsT)0x02 
#define SAHPI_SRF_NOMINAL    (SaHpiSensorRangeFlagsT)0x01

typedef struct {
    SaHpiSensorRangeFlagsT Flags;
    SaHpiSensorReadingT     Max;
    SaHpiSensorReadingT     Min;
    SaHpiSensorReadingT     Nominal;
    SaHpiSensorReadingT     NormalMax;
    SaHpiSensorReadingT     NormalMin;
} SaHpiSensorRangeT;

/*
** Sensor Units
** This is a list of all the sensor units supported by HPI.
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
    SAHPI_SU_UNCORRECTABLE_ERRORS
} SaHpiSensorUnitsT;

/*
** Modifier Unit Use
** This type defines how the modifier unit is used. For example: base unit == 
** meter, modifier unit == seconds, and modifier unit use == 
** SAHPI_SMUU_BASIC_OVER_MODIFIER. The resulting unit would be meters per second.
*/
typedef enum {
    SAHPI_SMUU_NONE = 0,
    SAHPI_SMUU_BASIC_OVER_MODIFIER,  /* Basic Unit / Modifier Unit */
    SAHPI_SMUU_BASIC_TIMES_MODIFIER  /* Basic Unit * Modifier Unit */
} SaHpiSensorModUnitUseT;

/*
** Sign Format
** This type defines what the sign format of the sensor's raw value is (1's 
** complement, unsigned, etc.).
*/
typedef enum {
    SAHPI_SDF_UNSIGNED = 0,
    SAHPI_SDF_1S_COMPLEMENT,
    SAHPI_SDF_2S_COMPLEMENT
} SaHpiSensorSignFormatT;

/*
** Data Format
** This structure encapsulates all of the various types that make up the 
** definition of sensor data.
*/
typedef struct {
    SaHpiSensorReadingFormatsT ReadingFormats; /* Indicates if sensor supports
                                                       readings in raw, interpreted,
                                                  and/or event status formats */
    SaHpiBoolT                 IsNumeric;      /* If FALSE, rest of this
                                                  structure is not
                                                  meaningful */
    SaHpiSensorSignFormatT     SignFormat;     /* Signed format */
    SaHpiSensorUnitsT          BaseUnits;      /* Base units (meters, etc.)    */
    SaHpiSensorUnitsT          ModifierUnits;  /* Modifier unit (second, etc.) */
    SaHpiSensorModUnitUseT     ModifierUse;    /* Modifier use(m/sec, etc.)    */ 
    SaHpiBoolT                 FactorsStatic;  /* True if the sensor factors
                                                  are static. If false 
                                                  factors vary over sensor
                                                  range, and are not 
                                                  accessible through HPI */
    SaHpiSensorFactorsT        Factors;
    SaHpiBoolT                 Percentage;     /* Is value a percentage */
    SaHpiSensorRangeT          Range;          /* Valid range of sensor */
} SaHpiSensorDataFormatT;

/*
** Threshold Support
**
** These types define what threshold values are readable, writable, and fixed. 
** It also defines how the threshold values are read and written.
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

typedef SaHpiUint8T SaHpiSensorThdCapT;
#define SAHPI_STC_RAW         (SaHpiSensorThdMaskT)0x01 /* read/write as 
                                                           raw counts */
#define SAHPI_STC_INTERPRETED (SaHpiSensorThdMaskT)0x02 /* read/write as
                                                           interpreted */

typedef struct {
    SaHpiBoolT            IsThreshold;  /* True if the sensor 
                                           supports thresholds. If false,
                                           rest of structure is not
                                           meaningful. */
    SaHpiSensorThdCapT    TholdCapabilities;
    SaHpiSensorThdMaskT   ReadThold;    /* Readable thresholds */
    SaHpiSensorThdMaskT   WriteThold;   /* Writable thresholds */
    SaHpiSensorThdMaskT   FixedThold;   /* Fixed thresholds */
} SaHpiSensorThdDefnT;

/*
** Event Control
**
** This type defines how sensor event messages can be controlled (can be turned 
** off and on for each type of event, etc.).
*/
typedef enum {
    SAHPI_SEC_PER_EVENT = 0,  /* Event message control per event */
    SAHPI_SEC_ENTIRE_SENSOR,  /* Control for entire sensor only */
    SAHPI_SEC_GLOBAL_DISABLE, /* Global disable of events only */
    SAHPI_SEC_NO_EVENTS       /* Events not supported */
} SaHpiSensorEventCtrlT;

/*
** Record
**
** This is the sensor resource data record which describes all of the static 
** data associated with a sensor.
*/
typedef struct {
    SaHpiSensorNumT         Num;           /* Sensor Number/Index */
    SaHpiSensorTypeT        Type;          /* General Sensor Type */
    SaHpiEventCategoryT     Category;      /* Event category */
    SaHpiSensorEventCtrlT   EventCtrl;     /* How events can be controlled */
    SaHpiEventStateT        Events;        /* Bit mask of event states 
                                              supported */
    SaHpiBoolT              Ignore;        /* Ignore sensor (entity not 
                                              present, disabled, etc.) */
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

/* These are the default sensor numbers for aggregate status. */
#define SAHPI_DEFAGSENS_OPER (SaHpiSensorNumT)0xFE
#define SAHPI_DEFAGSENS_PWR  (SaHpiSensorNumT)0xFD
#define SAHPI_DEFAGSENS_TEMP (SaHpiSensorNumT)0xFC

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Controls                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Control Number  */
typedef SaHpiUint8T SaHpiCtrlNumT;

/*
** Type of Control
**
** This enumerated type defines the different types of generic controls.
*/
typedef enum {
    SAHPI_CTRL_TYPE_DIGITAL = 0x00,
    SAHPI_CTRL_TYPE_DISCRETE,
    SAHPI_CTRL_TYPE_ANALOG,
    SAHPI_CTRL_TYPE_STREAM,
    SAHPI_CTRL_TYPE_TEXT,
    SAHPI_CTRL_TYPE_OEM = 0xC0
} SaHpiCtrlTypeT;

/*
** Control State Type Definitions
**
** Defines the types of control states.
*/
typedef enum {
    SAHPI_CTRL_STATE_OFF = 0,
    SAHPI_CTRL_STATE_ON,
    SAHPI_CTRL_STATE_PULSE_OFF,
    SAHPI_CTRL_STATE_PULSE_ON,
    SAHPI_CTRL_STATE_AUTO = 0xFF
} SaHpiCtrlStateDigitalT;

typedef SaHpiUint32T SaHpiCtrlStateDiscreteT;

typedef SaHpiInt32T  SaHpiCtrlStateAnalogT;

#define SAHPI_CTRL_MAX_STREAM_LENGTH 4
typedef struct { 
    SaHpiBoolT   Repeat;       /* Repeat flag */
    SaHpiUint32T StreamLength; /* Length of the data, in bytes, 
                              ** stored in the stream. */
    SaHpiUint8T  Stream[SAHPI_CTRL_MAX_STREAM_LENGTH];
} SaHpiCtrlStateStreamT;

typedef SaHpiUint8T SaHpiTxtLineNumT;

/* Reserved number for sending output to all lines */
#define SAHPI_TLN_ALL_LINES (SaHpiTxtLineNumT)0xFF 

typedef struct {
    SaHpiTxtLineNumT    Line; /* Operate on line # */ 
    SaHpiTextBufferT    Text; /* Text to display */
} SaHpiCtrlStateTextT;

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
    SaHpiCtrlTypeT          Type;       /* Type of control */
    SaHpiCtrlStateUnionT    StateUnion; /* Data for control type */
} SaHpiCtrlStateT;



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
**  This enumeration defines the what the control's output will be.
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
    SAHPI_CTRL_OEM
} SaHpiCtrlOutputTypeT;

/*
** Specific Record Types
** These types represent the specific types of control resource data records.
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
    SaHpiUint8T             MaxChars; /* Maximum chars per line */
    SaHpiUint8T             MaxLines; /* Maximum # of lines */
    SaHpiLanguageT          Language; /* Language Code */
    SaHpiTextTypeT          DataType; /* Permitted Data */
    SaHpiCtrlStateTextT     Default;
} SaHpiCtrlRecTextT;

#define SAHPI_CTRL_OEM_CONFIG_LENGTH 10
typedef struct {
    SaHpiManufacturerIdT   MId;
    SaHpiUint8T             ConfigData[SAHPI_CTRL_OEM_CONFIG_LENGTH];
    SaHpiCtrlStateOemT Default;
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
** Record Definition
** Definition of the control resource data record.
*/
typedef struct {
    SaHpiCtrlNumT        Num;       /* Control Number/Index */
    SaHpiBoolT           Ignore;    /* Ignore control (entity 
                                       not  present, disabled, etc.) */
    SaHpiCtrlOutputTypeT OutputType;
    SaHpiCtrlTypeT       Type;      /* Type of control */
    SaHpiCtrlRecUnionT   TypeUnion; /* Specific control record */
    SaHpiUint32T         Oem;       /* Reserved for OEM use */
} SaHpiCtrlRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                    Entity Inventory Data                   **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/*
** These structures are used to read and write inventory data to entity inventory  
** repositories within a resource. 
*/
/*
** Entity Inventory Repository ID
** Identifier for an entity inventory repository.
*/
typedef SaHpiUint8T SaHpiEirIdT;
#define SAHPI_DEFAULT_INVENTORY_ID (SaHpiEirIdT)0x00

/* Data Validity */
typedef enum {
    SAHPI_INVENT_DATA_VALID,
    SAHPI_INVENT_DATA_INVALID,
    SAHPI_INVENT_DATA_OVERFLOW
} SaHpiInventDataValidityT;

/* Inventory Record definitions */
typedef enum {
    SAHPI_INVENT_RECTYPE_INTERNAL_USE = 0xB0,
    SAHPI_INVENT_RECTYPE_CHASSIS_INFO,
    SAHPI_INVENT_RECTYPE_BOARD_INFO,
    SAHPI_INVENT_RECTYPE_PRODUCT_INFO,
    SAHPI_INVENT_RECTYPE_OEM = 0xC0
} SaHpiInventDataRecordTypeT;

typedef enum {
    SAHPI_INVENT_CTYP_OTHER = 1,
    SAHPI_INVENT_CTYP_UNKNOWN,
    SAHPI_INVENT_CTYP_DESKTOP,
    SAHPI_INVENT_CTYP_LOW_PROFILE_DESKTOP,
    SAHPI_INVENT_CTYP_PIZZA_BOX,
    SAHPI_INVENT_CTYP_MINI_TOWER,
    SAHPI_INVENT_CTYP_TOWER,
    SAHPI_INVENT_CTYP_PORTABLE,
    SAHPI_INVENT_CTYP_LAPTOP,
    SAHPI_INVENT_CTYP_NOTEBOOK,
    SAHPI_INVENT_CTYP_HANDHELD,
    SAHPI_INVENT_CTYP_DOCKING_STATION,
    SAHPI_INVENT_CTYP_ALLINONE,
    SAHPI_INVENT_CTYP_SUBNOTEBOOK,
    SAHPI_INVENT_CTYP_SPACE_SAVING,
    SAHPI_INVENT_CTYP_LUNCH_BOX,
    SAHPI_INVENT_CTYP_MAIN_SERVER,
    SAHPI_INVENT_CTYP_EXPANSION,
    SAHPI_INVENT_CTYP_SUBCHASSIS,
    SAHPI_INVENT_CTYP_BUS_EXPANSION_CHASSIS,
    SAHPI_INVENT_CTYP_PERIPHERAL_CHASSIS,
    SAHPI_INVENT_CTYP_RAID_CHASSIS,
    SAHPI_INVENT_CTYP_RACKMOUNT
} SaHpiInventChassisTypeT;

typedef struct {
    SaHpiUint8T Data[1];  /* Variable length opaque data */
} SaHpiInventInternalUseDataT;

typedef struct {
    SaHpiTimeT            MfgDateTime;    /* May be set to 
                                             SAHPI_TIME_UNSPECIFIED
                                             if manufacturing
                                             date/time not available */
    SaHpiTextBufferT     *Manufacturer;
    SaHpiTextBufferT     *ProductName;
    SaHpiTextBufferT     *ProductVersion;
    SaHpiTextBufferT     *ModelNumber;
    SaHpiTextBufferT     *SerialNumber;
    SaHpiTextBufferT     *PartNumber;
    SaHpiTextBufferT     *FileId; 
    SaHpiTextBufferT     *AssetTag;
    SaHpiTextBufferT     *CustomField[1]; /* Variable number of fields,
                                             last is NULL */
} SaHpiInventGeneralDataT;

typedef struct {
    SaHpiInventChassisTypeT Type;        /* Type of chassis */
    SaHpiInventGeneralDataT GeneralData;
} SaHpiInventChassisDataT;

typedef struct {
    SaHpiManufacturerIdT MId;/* OEM Manuf. ID */
    SaHpiUint8T Data[1];      /* Variable length data, defined by OEM, 
                                Length derived from DataLength in 
                                SaHpiInventDataRecordT structure: 
                                DataLength - 4(because DataLength 
                                includes the MId)  */
} SaHpiInventOemDataT;

typedef union { 
    SaHpiInventInternalUseDataT InternalUse;
    SaHpiInventChassisDataT     ChassisInfo;
    SaHpiInventGeneralDataT     BoardInfo; 
    SaHpiInventGeneralDataT     ProductInfo;
    SaHpiInventOemDataT         OemData;
} SaHpiInventDataUnionT;

typedef struct {
    SaHpiInventDataRecordTypeT  RecordType;
    SaHpiUint32T                DataLength;  /* Length of Data field for
                                                this record */
    SaHpiInventDataUnionT       RecordData;  /* Variable length data */
} SaHpiInventDataRecordT;

typedef struct {
    SaHpiInventDataValidityT Validity; /* Indication as to whether data
                                        Returned by
                                        saHpiEntityInventoryDataRead() is
                                        complete and valid. Unless this
                                        flag indicates valid data, 
                                        saHpiEntityInventoryDataWrite() will
                                        not take any actions except to
                                        return an error.*/
    SaHpiInventDataRecordT *DataRecords[1];  /* Array of pointers to inventory
                                                Data Records.  Variable
                                                number of entries. Last
                                                entry is NULL. */
} SaHpiInventoryDataT;


/*******************************************************************************
********************************************************************************
**********                                                            **********
**********               Inventory Resource Data Records              **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** All inventory data contained in an entity inventory repository
** must be represented in the RDR repository
** with an SaHpiInventoryRecT.
*/
typedef struct {
    SaHpiEirIdT   EirId;
    SaHpiUint32T              Oem;
} SaHpiInventoryRecT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                          Watchdogs                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*
** This section defines all of the data types associated with watchdog timers.
*/

/* Watchdog Number - Identifier for a watchdog timer. */
typedef SaHpiUint8T SaHpiWatchdogNumT;
/* Using SaHpiWatchdogNumT instead of SaHpiWatchdogT as in spec. */
/* Spec believed to be in error since SaHpiWatchdogT is a struct. */
#define SAHPI_DEFAULT_WATCHDOG_NUM (SaHpiWatchdogNumT)0x00

/*
** Watchdog Timer Action
**
** These enumerations represent the possible actions to be taken upon watchdog 
** timer timeout and the events that are generated for watchdog actions. 
*/
typedef enum { 
    SAHPI_WA_NO_ACTION = 0,
    SAHPI_WA_RESET,
    SAHPI_WA_POWER_DOWN,
    SAHPI_WA_POWER_CYCLE
} SaHpiWatchdogActionT;

typedef enum {
    SAHPI_WAE_NO_ACTION = 0,
    SAHPI_WAE_RESET,
    SAHPI_WAE_POWER_DOWN,
    SAHPI_WAE_POWER_CYCLE,
    SAHPI_WAE_TIMER_INT=0x08   /* Used if Timer Preinterrupt only */
} SaHpiWatchdogActionEventT;

/*
** Watchdog Pre-timer Interrupt
**
** These enumerations represent the possible types of interrupts that may be 
** triggered by a watchdog pre-timer event. The actual meaning of these 
** operations may differ depending on the hardware architecture.
*/
typedef enum { 
    SAHPI_WPI_NONE = 0,
    SAHPI_WPI_SMI,
    SAHPI_WPI_NMI,
    SAHPI_WPI_MESSAGE_INTERRUPT,
    SAHPI_WPI_OEM = 0x0F
} SaHpiWatchdogPretimerInterruptT;

/*
** Watchdog Timer Use 
**
** These enumerations represent the possible watchdog users that may have caused 
** the watchdog to expire. For instance, if watchdog is being used during power 
** on self test (POST), and it expires, the SAHPI_WTU_BIOS_POST expiration type 
** will be set. Most specific uses for Watchdog timer by users of HPI should 
** indicate SAHPI_WTU_SMS_OS  if the use is to provide an OS-healthy heartbeat, 
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
    SAHPI_WTU_UNSPECIFIED = 0x0F
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
** saHpiWatchdogTimerSet() functions. The use of the structure varies slightly by 
** each function.
**
** For saHpiWatchdogTimerGet() :
**
**   Log -                indicates whether or not the Watchdog is configured to 
**                        issue an event when it next times out. TRUE=event will 
**                        be issued on timeout.
**   Running -            indicates whether or not the Watchdog is currently 
**                        running or stopped. TRUE=Watchdog is running.
**   TimerUse -           indicates the current use of the timer; one of five 
**                        preset uses which was included on the last
**                        saHpiWatchdogTimerSet() function call, or through some 
**                        other implementation-dependent means to start the 
**                        Watchdog timer.
**   TimerAction -        indicates what action will be taken when the Watchdog 
**                        times out.
**   PretimerInterrupt -  indicates which action will be taken 
**                        "PreTimeoutInterval" seconds prior to Watchdog timer 
**                        expiration. 
**   PreTimeoutInterval - indicates how many  milliseconds prior to timer time 
**                        out the PretimerInterrupt action will be taken. If 
**                        "PreTimeoutInterval" = 0, the PretimerInterrupt action 
**                        will occur concurrently with "TimerAction." HPI 
**                        implementations may not be able to support millisecond 
**                        resolution and may have a maximum value restriction. 
**                        These restrictions should be documented by the 
**                        provider of the HPI interface.
**   TimerUseExpFlags -   set of five bit flags which indicate that a Watchdog 
**                        timer timeout has occurred while the corresponding 
**                        TimerUse value was set. Once set, these flags stay 
**                        set until specifically cleared with a 
**                        saHpiWatchdogTimerSet() call, or by some other 
**                        implementation-dependent means.
**   InitialCount -       The time, in milliseconds, before the timer will time 
**                        out after a saHpiWatchdogTimerReset() function call is
**                        made, or some other implementation-dependent strobe is
**                        sent to the Watchdog. HPI implementations may not be
**                        able to support millisecond resolution and may have a 
**                        maximum value restriction. These restrictions should 
**                        be documented by the provider of the HPI interface.
**   PresentCount -       The remaining time in milliseconds before the timer 
**                        will time out unless a saHpiWatchdogTimerReset()
**                        function call is made, or some other implementation-
**                        dependent strobe is sent to the Watchdog. 
**                        HPI implementations may not be able to support 
**                        millisecond resolution on watchdog timers, but will 
**                        return the number of clock ticks remaining times the 
**                        number of milliseconds between each tick.
**
** For saHpiWatchdogTimerSet():
**
**   Log -                indicates whether or not the Watchdog should  issue 
**                        an event when it next times out. TRUE=event will be 
**                        issued on timeout.
**   Running -            indicates whether or not the Watchdog should be 
**                        stopped before updating. 
**                        TRUE =  Watchdog is not stopped. If it is already 
**                                stopped, it will remain stopped, but if it is 
**                                running, it will continue to run, with the 
**                                countdown timer reset to the new InitialCount. 
**                                Note that there is a race condition possible 
**                                with this setting, so it should be used with 
**                                care. 
**                        FALSE = Watchdog is stopped. After 
**                                saHpiWatchdogTimerSet() is called, a subsequent 
**                                call to saHpiWatchdogTimerReset() is required to 
**                                start the timer.
**   TimerUse -           indicates the current use of the timer. Will control 
**                        which TimerUseExpFlag is set if the timer expires.
**   TimerAction -        indicates what action will be taken when the Watchdog 
**                        times out.
**   PretimerInterrupt -  indicates which action will be taken 
**                        "PreTimeoutInterval" seconds prior to  Watchdog timer 
**                        expiration. 
**   PreTimeoutInterval - indicates how many milliseconds prior to timer time 
**                        out the PretimerInterrupt action will be taken. If 
**                        "PreTimeoutInterval" = 0, the PretimerInterrupt action 
**                        will occur concurrently with "TimerAction." HPI 
**                        implementations may not be able to support millisecond 
**                        resolution and may have a maximum value restriction. 
**                        These restrictions should be documented by the 
**                        provider of the HPI interface.
**   TimerUseExpFlags -   Set of five bit flags corresponding to the five 
**                        TimerUse values. For each bit set, the corresponding 
**                        Timer Use Expiration Flag will be CLEARED. Generally, 
**                        a program should only clear the Timer Use Expiration 
**                        Flag corresponding to its own TimerUse, so that other 
**                        software, which may have used the timer for another 
**                        purpose in the past can still read its TimerUseExpFlag 
**                        to determine whether or not the timer expired during 
**                        that use.
**   InitialCount -       The time, in milliseconds, before the timer will time 
**                        out after a saHpiWatchdogTimerReset() function call is
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
** When the "Watchdog" capability is set in a resource, a watchdog with an 
** identifier of SAHPI_DEFAULT_WATCHDOG_NUM is required. All watchdogs must be 
** represented in the RDR repository with an SaHpiWatchdogRecT, including the
** watchdog with an identifier of SAHPI_DEFAULT_WATCHDOG_NUM.
*/
typedef struct {
    SaHpiWatchdogNumT  WatchdogNum;
    SaHpiUint32T       Oem;
} SaHpiWatchdogRecT;

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
** (sensor, inventory data, watchdog, etc.).
*/
typedef enum {
    SAHPI_NO_RECORD,
    SAHPI_CTRL_RDR,
    SAHPI_SENSOR_RDR,
    SAHPI_INVENTORY_RDR,
    SAHPI_WATCHDOG_RDR
} SaHpiRdrTypeT;

typedef union {
    SaHpiCtrlRecT        CtrlRec;
    SaHpiSensorRecT      SensorRec;
    SaHpiInventoryRecT   InventoryRec;
    SaHpiWatchdogRecT    WatchdogRec;
} SaHpiRdrTypeUnionT;

typedef struct {
    SaHpiEntryIdT        RecordId;
    SaHpiRdrTypeT        RdrType;
    SaHpiEntityPathT     Entity;        /* Entity to which this RDR relates. */
    SaHpiRdrTypeUnionT   RdrTypeUnion;
    SaHpiTextBufferT     IdString;
} SaHpiRdrT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                           Hot Swap                         **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Power State  */
typedef enum {
    SAHPI_HS_POWER_OFF = 0,
    SAHPI_HS_POWER_ON,
    SAHPI_HS_POWER_CYCLE
} SaHpiHsPowerStateT;

/* Hot Swap Indicator State */
typedef enum {
    SAHPI_HS_INDICATOR_OFF = 0,
    SAHPI_HS_INDICATOR_ON
} SaHpiHsIndicatorStateT;

/* Hot Swap Action  */
typedef enum {
    SAHPI_HS_ACTION_INSERTION = 0,
    SAHPI_HS_ACTION_EXTRACTION
} SaHpiHsActionT;

/* Hot Swap State */
typedef enum {
    SAHPI_HS_STATE_INACTIVE = 0,
    SAHPI_HS_STATE_INSERTION_PENDING,
    SAHPI_HS_STATE_ACTIVE_HEALTHY,
    SAHPI_HS_STATE_ACTIVE_UNHEALTHY,
    SAHPI_HS_STATE_EXTRACTION_PENDING,
    SAHPI_HS_STATE_NOT_PRESENT
} SaHpiHsStateT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                        Events, Part 2                      **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/* Event Data Structures */

/*
** Sensor Optional Data
**
** Sensor events may contain optional data items passed and stored with the 
** event. If these optional data items are present, they will be included with 
** the event data returned in response to a saHpiEventGet() or 
** saHpiEventLogEntryGet() function call. Also, the optional data items may be 
** included with the event data passed to the saHpiEventLogEntryAdd() function.
**
** Specific implementations of HPI may have restrictions on how much data may
** be passed to saHpiEventLogEntryAdd(). These restrictions should be documented
** by the provider of the HPI interface.
*/
typedef enum {
    SAHPI_CRITICAL = 0,
    SAHPI_MAJOR,
    SAHPI_MINOR,
    SAHPI_INFORMATIONAL,
    SAHPI_OK,
    SAHPI_DEBUG = 0xF0
} SaHpiSeverityT;

typedef SaHpiUint8T SaHpiSensorOptionalDataT;

#define SAHPI_SOD_TRIGGER_READING   (SaHpiSensorOptionalDataT)0x01
#define SAHPI_SOD_TRIGGER_THRESHOLD (SaHpiSensorOptionalDataT)0x02
#define SAHPI_SOD_OEM               (SaHpiSensorOptionalDataT)0x04
#define SAHPI_SOD_PREVIOUS_STATE    (SaHpiSensorOptionalDataT)0x08
#define SAHPI_SOD_SENSOR_SPECIFIC   (SaHpiSensorOptionalDataT)0x10

typedef struct {
    SaHpiSensorNumT           SensorNum;
    SaHpiSensorTypeT          SensorType;
    SaHpiEventCategoryT       EventCategory;
    SaHpiBoolT                Assertion;      /* TRUE = Event State asserted
                                                 FALSE = deasserted */
    SaHpiEventStateT          EventState;     /* State being asserted 
                                                 deasserted */
    SaHpiSensorOptionalDataT  OptionalDataPresent;
    SaHpiSensorReadingT       TriggerReading; /* Reading that triggered
                                                 the event */ 
    SaHpiSensorReadingT       TriggerThreshold;
    SaHpiEventStateT          PreviousState;
    SaHpiUint32T              Oem;
    SaHpiUint32T              SensorSpecific;
} SaHpiSensorEventT;

typedef struct {
    SaHpiHsStateT HotSwapState;
    SaHpiHsStateT PreviousHotSwapState;
} SaHpiHotSwapEventT;

typedef struct {
    SaHpiWatchdogNumT               WatchdogNum;
    SaHpiWatchdogActionEventT       WatchdogAction;
    SaHpiWatchdogPretimerInterruptT WatchdogPreTimerAction;
    SaHpiWatchdogTimerUseT          WatchdogUse;
} SaHpiWatchdogEventT;

#define SAHPI_OEM_EVENT_DATA_SIZE 32
typedef struct {
    SaHpiManufacturerIdT MId;
    SaHpiUint8T OemEventData[SAHPI_OEM_EVENT_DATA_SIZE];
} SaHpiOemEventT;

/*
** User events may be used for storing custom events created by the application / middleware;
** eg. when injecting events into the event log using saHpiEventLogEntryAdd().
*/
#define SAHPI_USER_EVENT_DATA_SIZE 32
typedef struct {
    SaHpiUint8T UserEventData[SAHPI_USER_EVENT_DATA_SIZE];
} SaHpiUserEventT;

typedef enum {
    SAHPI_ET_SENSOR,
    SAHPI_ET_HOTSWAP,
    SAHPI_ET_WATCHDOG,
    SAHPI_ET_OEM,
    SAHPI_ET_USER
} SaHpiEventTypeT;

typedef union {
    SaHpiSensorEventT   SensorEvent;
    SaHpiHotSwapEventT  HotSwapEvent;
    SaHpiWatchdogEventT WatchdogEvent;
    SaHpiOemEventT      OemEvent;
    SaHpiUserEventT     UserEvent;
} SaHpiEventUnionT;

typedef struct { 
    SaHpiResourceIdT  Source;
    SaHpiEventTypeT   EventType;
    SaHpiTimeT        Timestamp;
    SaHpiSeverityT    Severity;
    SaHpiEventUnionT  EventDataUnion;
} SaHpiEventT;


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
    SAHPI_RESTORE_PARM
} SaHpiParmActionT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                      Reset                                 **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

typedef enum { 
    SAHPI_COLD_RESET = 0, 
    SAHPI_WARM_RESET, 
    SAHPI_RESET_ASSERT,
    SAHPI_RESET_DEASSERT
} SaHpiResetActionT;

/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                    Resource Presence Table                 **********
**********                                                            **********
********************************************************************************
*******************************************************************************/

/*  This section defines the types associated with the RPT. */

typedef struct {
    SaHpiUint32T UpdateCount;     /* This count is incremented any time the table
                                     is changed. It rolls over to zero when the
                                     maximum value is reached  */
    SaHpiTimeT   UpdateTimestamp; /* This time is set any time the table is
                                     changed. If the implementation cannot
                                     supply an absolute timestamp, then it may
                                     supply a timestamp relative to some system-
                                     defined epoch, such as system boot. The
                                     value SAHPI_TIME_UNSPECIFIED indicates that
                                     the time of the update cannot be determined.
                                     Otherwise, If the value is less than or
                                     equal to SAHPI_TIME_MAX_RELATIVE, then it
                                     is relative; if it is greater than
                                     SAHPI_TIME_MAX_RELATIVE, then it is absolute. */
} SaHpiRptInfoT;

/* 
** Resource Info Type Definitions
** 
** 
** SaHpiResourceInfoT contains static configuration data concerning the 
** management controller associated with the resource, or the resource itself. 
** Note this information is used to describe the resource; that is, the piece of
** infrastructure which manages an entity (or multiple entities) - NOT the entities
** for which the resource provides management. The purpose of the 
** SaHpiResourceInfoT structure is to provide information that the HPI user may 
** need in order to interact correctly with the resource (e.g., recognize a 
** specific management controller which may have defined OEM fields in sensors, 
** OEM controls, etc.).
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
} SaHpiResourceInfoT;

/*
** Resource Capabilities
**
** This definition defines the capabilities of a given resource. One resource 
** may support any number of capabilities using the bit mask. Every resource
** must set at least one of these capabilities; ie. zero is not a valid value
** for a resource's capabilities.
**
** SAHPI_CAPABILITY_DOMAIN           
** SAHPI_CAPABILITY_RESOURCE         
** SAHPI_CAPABILITY_EVT_DEASSERTS
**   Indicates that all sensors on the resource have the property that their
**   Assertion and Deassertion event enable flags are the same. That is,
**   for all event states whose assertion triggers an event, it is
**   guaranteed that the deassertion of that event will also
**   trigger an event. Thus, the user may track the state of sensors on the
**   resource by monitoring events rather than polling for state changes.
** SAHPI_CAPABILITY_AGGREGATE_STATUS 
** SAHPI_CAPABILITY_CONFIGURATION    
** SAHPI_CAPABILITY_MANAGED_HOTSWAP  
**   Indicates that the resource supports managed hotswap. Since hotswap only
**   makes sense for field-replaceable units, the SAHPI_CAPABILITY_FRU
**   capability bit must also be set for this resource.
** SAHPI_CAPABILITY_WATCHDOG         
** SAHPI_CAPABILITY_CONTROL          
** SAHPI_CAPABILITY_FRU
**   Indicates that the resource is a field-replaceable unit; i.e., it is
**   capable of being removed and replaced in a live system. This does not
**   necessarily imply that the resource supports managed hotswap.
** SAHPI_CAPABILITY_INVENTORY_DATA   
** SAHPI_CAPABILITY_SEL              
** SAHPI_CAPABILITY_RDR 
**   Indicates that a resource data record (RDR) repository is supplied
**   by the resource. Since the existence of an RDR is mandatory, this
**   capability must be asserted.         
** SAHPI_CAPABILITY_SENSOR           
*/

typedef SaHpiUint32T SaHpiCapabilitiesT;
#define SAHPI_CAPABILITY_DOMAIN           (SaHpiCapabilitiesT)0x80000000
#define SAHPI_CAPABILITY_RESOURCE         (SaHpiCapabilitiesT)0X40000000
#define SAHPI_CAPABILITY_EVT_DEASSERTS    (SaHpiCapabilitiesT)0x00008000
#define SAHPI_CAPABILITY_AGGREGATE_STATUS (SaHpiCapabilitiesT)0x00002000
#define SAHPI_CAPABILITY_CONFIGURATION    (SaHpiCapabilitiesT)0x00001000
#define SAHPI_CAPABILITY_MANAGED_HOTSWAP  (SaHpiCapabilitiesT)0x00000800
#define SAHPI_CAPABILITY_WATCHDOG         (SaHpiCapabilitiesT)0x00000400
#define SAHPI_CAPABILITY_CONTROL          (SaHpiCapabilitiesT)0x00000200
#define SAHPI_CAPABILITY_FRU              (SaHpiCapabilitiesT)0x00000100
#define SAHPI_CAPABILITY_INVENTORY_DATA   (SaHpiCapabilitiesT)0x00000008
#define SAHPI_CAPABILITY_SEL              (SaHpiCapabilitiesT)0x00000004
#define SAHPI_CAPABILITY_RDR              (SaHpiCapabilitiesT)0x00000002
#define SAHPI_CAPABILITY_SENSOR           (SaHpiCapabilitiesT)0x00000001

/*
** RPT Entry
**
** This structure is used to store the RPT entry information.
**
** The ResourceCapabilities field definies the capabilities of the resource.
** This field must be non-zero for all valid resources.
**
** The ResourceTag field is an informational value that supplies the caller with naming
** information for the resource. This should be set to the "user-visible" name for a
** resource, which can be used to identify the resource in messages to a human operator.
** For example, it could be set to match a physical printed label attached to the primary
** entity which the resource manages. See section 5.2.6, saHpiResourceTagSet(), on page 33.
*/
typedef struct {
    SaHpiEntryIdT        EntryId;
    SaHpiResourceIdT     ResourceId;
    SaHpiResourceInfoT   ResourceInfo;
    SaHpiEntityPathT     ResourceEntity;  /* If resource manages a FRU, entity path of the FRU */
                                          /* If resource manages a single entity, entity path of
                                              that entity. */
                                          /* If resource manages multiple entities, the
                                              entity path of the "primary" entity managed by the
                                              resource    */
                                          /* Must be set to the same value in every domain which
                                              contains this resource */
    SaHpiCapabilitiesT   ResourceCapabilities;  /* Must be non-0. */
    SaHpiSeverityT       ResourceSeverity; /* Indicates the criticality that
                                              should be raised when the resource
                                              is not responding   */
    SaHpiDomainIdT       DomainId;  /* The Domain ID is used when the resource 
                                       is also a domain. */
    SaHpiTextBufferT     ResourceTag;
} SaHpiRptEntryT; 




/*******************************************************************************
********************************************************************************
**********                                                            **********
**********                       System Event Log                     **********
**********                                                            **********
********************************************************************************
*******************************************************************************/
/* This section defines the types associated with the SEL. */
/* 
** Event Log Information
**
** The Entries entry denotes the number of active entries contained in the log.
** The Size entry denotes the total number of entries the log is able to hold. 
** The UpdateTimestamp entry denotes the timestamp of the last addition, 
**   deletion or log clear operation. 
** The CurrentTime entry denotes the log's idea of the current time; i.e the
**   timestamp that would be placed on an entry if it was added now. 
** The Enabled entry indicates whether the log is enabled. If the event log
**   is "disabled" no events generated within the HPI implementation will be
**   added to the event log. Events may still be added to the event log with
**   the saHpiEventLogEntryAdd() function. When the event log is "enabled"
**   events may be automatically added to the event log as they are generated
**   in a resource or a domain, however, it is implementation-specific which
**   events are automatically added to any event log.
** The OverflowFlag entry indicates the log has overflowed. Events have been 
**   dropped or overwritten due to a table overflow. 
** The OverflowAction entry indicates the behavior of the SEL when an overflow 
**   occurs. 
** The DeleteEntrySupported indicates whether the delete command is supported for 
**   event log entries. Note that clearing an entire log is valid even if this
**   flag is not set.
*/
typedef enum {
    SAHPI_SEL_OVERFLOW_DROP,        /* New entries are dropped when log is full*/
    SAHPI_SEL_OVERFLOW_WRAP,        /* Log wraps when log is full */
    SAHPI_SEL_OVERFLOW_WRITELAST    /* Last entry overwritten when log is full */
} SaHpiSelOverflowActionT;

typedef struct {
    SaHpiUint32T              Entries;
    SaHpiUint32T              Size;
    SaHpiTimeT                UpdateTimestamp;  
    SaHpiTimeT                CurrentTime;
    SaHpiBoolT                Enabled;
    SaHpiBoolT                OverflowFlag;
    SaHpiSelOverflowActionT   OverflowAction;
    SaHpiBoolT                DeleteEntrySupported;
} SaHpiSelInfoT;
/*
** Event Log Entry
** These types define the event log entry.
*/
typedef SaHpiUint32T SaHpiSelEntryIdT;
/* Reserved values for event log entry IDs */
#define SAHPI_OLDEST_ENTRY    (SaHpiSelEntryIdT)0x00000000
#define SAHPI_NEWEST_ENTRY    (SaHpiSelEntryIdT)0xFFFFFFFF
#define SAHPI_NO_MORE_ENTRIES (SaHpiSelEntryIdT)0xFFFFFFFE



typedef struct {
    SaHpiSelEntryIdT EntryId;   /* Entry ID for record */
    SaHpiTimeT       Timestamp; /* Time at which the event was placed
                                   in the event log. The value
                                   SAHPI_TIME_UNSPECIFIED indicates that
                                   the time of the event cannot be
                                   determined; otherwise, if less than
                                   or equal to SAHPI_TIME_MAX_RELATIVE,
                                   then it  relative; if it is greater than
                                   SAHPI_TIME_MAX_RELATIVE, then it is absolute. */
    SaHpiEventT      Event;     /* Logged Event */
} SaHpiSelEntryT;





/*******************************************************************************
**
** Name: saHpiInitialize
**
** Description:
**   This function allows the management service an opportunity to perform
**   platform-specific initialization. saHpiInitialize() must be called
**   before any other functions are called. 
**
** Parameters:
**   HpiImplVersion - [out] Pointer to the version of the HPI
**      implementation. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_DUPLICATE is returned when the HPI has already
**   been initialized. Once one saHpiInitialize() call has been made,
**   another one cannot be made until after a saHpiFinalize() call is made.
**   
**
** Remarks:
**   This function returns the version of the HPI implementation. Note:	If
**   the HPI interface version is needed it can be retrieved from the
**   SAHPI_INTERFACE_VERSION definition.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiInitialize(
     SAHPI_OUT SaHpiVersionT *HpiImplVersion
);


/*******************************************************************************
**
** Name: saHpiFinalize 
**
** Description:
**   This function allows the management service an opportunity to perform
**   platform-specific cleanup. All sessions should be closed (see
**   saHpiSessionClose()), before this function is executed. All open
**   sessions will be forcibly closed upon execution of this command. 
**
** Parameters:
**   None. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None.   5	Domains 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiFinalize ( void
);


/*******************************************************************************
**
** Name: saHpiSessionOpen 
**
** Description:
**   This function opens a HPI session for a given domain and set of
**   security characteristics (future). This function call assumes that a
**   pre-arranged agreement between caller and the HPI implementation
**   identifies the resources within the specified domain. As a future
**   roadmap item, functions for discovery of domains and allocation of
**   resources within domains may be developed. 
**
** Parameters:
**   DomainId - [in] Domain ID to be controlled by middleware/application.
**      A domain ID of SAHPI_DEFAULT_DOMAIN_ID indicates the default domain. 
**   SessionId - [out] Pointer to a location to store a handle to the newly
**      opened session. This handle is used for subsequent access to domain
**      resources and events. 
**   SecurityParams - [in] Pointer to security and permissions data
**      structure. This parameter is reserved for future use, and must be set
**      to NULL. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_DOMAIN is returned if no domain
**   matching the specified domain ID exists. 
**
** Remarks:
**   None.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSessionOpen (
     SAHPI_IN  SaHpiDomainIdT   DomainId,
     SAHPI_OUT SaHpiSessionIdT  *SessionId,
     SAHPI_IN  void             *SecurityParams
);


/*******************************************************************************
**
** Name: saHpiSessionClose 
**
** Description:
**   This function closes a HPI session. After closing a session, the
**   session ID will no longer be valid. 
**
** Parameters:
**   SessionId - [in] Session handle previously obtained using
**      saHpiSessionOpen(). 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
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
** Name: saHpiResourcesDiscover 
**
** Description:
**   This function requests the underlying management service to discover
**   information about the resources it controls for the domain associated
**   with the open session. This function may be called during operation to
**   regenerate the RPT table. For those FRUs that must be discovered by
**   polling, latency between FRU insertion and actual addition of the
**   resource associated with that FRU to the RPT exists. To overcome this
**   latency, a discovery of all present resources may be forced by calling
**   saHpiResourcesDiscover (). 
**
** Parameters:
**   SessionId - [in] Handle to session context.   
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourcesDiscover (
     SAHPI_IN SaHpiSessionIdT SessionId
);


/*******************************************************************************
**
** Name: saHpiRptInfoGet 
**
** Description:
**   This function is used for requesting information about the resource
**   presence table (RPT) such as an update counter and timestamp. This is
**   particularly useful when using saHpiRptEntryGet() (see page 31). 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   RptInfo - [out] Pointer to the information describing the resource
**      presence table. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRptInfoGet (
     SAHPI_IN  SaHpiSessionIdT SessionId,
     SAHPI_OUT SaHpiRptInfoT   *RptInfo
);


/*******************************************************************************
**
** Name: saHpiRptEntryGet 
**
** Description:
**   This function retrieves resource information for the specified entry
**   of the resource presence table. This function allows the caller to
**   read the RPT entry-by-entry. If the EntryID parameter is set to
**   SAHPI_FIRST_ENTRY, the first entry in the RPT will be returned. When
**   an entry is successfully retrieved,  *NextEntryID will be set to the
**   ID of the next valid entry; however, when the last entry has been
**   retrieved, *NextEntryID will be set to SAHPI_LAST_ENTRY. To retrieve
**   an entire list of entries, call this function first with an EntryID of
**   SAHPI_FIRST_ENTRY and then use the returned NextEntryID in the next
**   call. Proceed until the NextEntryID returned is SAHPI_LAST_ENTRY. At
**   initialization, the user may not wish to turn on eventing, since the
**   context of the events, as provided by the RPT, is not known. In this
**   instance, if a FRU is inserted into the system while the RPT is being
**   read entry by entry, the resource associated with that FRU may be
**   missed. (Keep in mind that there is no specified ordering for the RPT
**   entries.)  The update counter provides a means for insuring that no
**   resources are missed when stepping through the RPT. In order to use
**   this feature, the user should invoke saHpiRptInfoGet(), and get the
**   update counter value before retrieving the first RPT entry. After
**   reading the last entry, the user should again invoke the
**   saHpiRptInfoGet() to get the update counter value. If the update
**   counter has not been incremented, no new records have been added.   
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   EntryId - [in] Handle of the entry to retrieve from the RPT. Reserved
**      entry ID values:  SAHPI_FIRST_ENTRY  Get first entry  SAHPI_LAST_ENTRY
**        Reserved as delimiter for end of list. Not a valid entry identifier.
**      
**   NextEntryId - [out] Pointer to location to store the record ID of next
**      entry in RPT. 
**   RptEntry - [out] Pointer to the structure to hold the returned RPT
**      entry. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
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
** Name: saHpiRptEntryGetByResourceId 
**
** Description:
**   This function retrieves resource information from the resource
**   presence table for the specified resource using its resource ID.
**   Typically at start-up, the RPT is read entry-by-entry, using
**   saHpiRptEntryGet(). From this, the caller can establish the set of
**   resource IDs to use for future calls to the HPI functions. However,
**   there may be other ways of learning resource IDs without first reading
**   the RPT. For example, resources may be added to the domain while the
**   system is running in response to a hot-swap action. When a resource is
**   added, the application will receive a hot-swap event containing the
**   resource ID of the new resource. The application may then want to
**   search the RPT for more detailed information on the newly added
**   resource. In this case, the resource ID can be used to locate the
**   applicable RPT entry information. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource whose RPT entry should
**      be returned. 
**   RptEntry  - [out] Pointer to structure to hold the returned RPT entry.
**        
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiRptEntryGetByResourceId (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_OUT SaHpiRptEntryT   *RptEntry
);


/*******************************************************************************
**
** Name: saHpiResourceSeveritySet 
**
** Description:
**   This function allows the caller to set the severity level applied to
**   an event issued if a resource unexpectedly becomes unavailable to the
**   HPI. A resource may become unavailable for several reasons including:
**   ? The FRU associated with the resource is no longer present in the
**   system (a surprise extraction has occurred) ? A catastrophic failure
**   has occurred Typically, the HPI implementation will provide an
**   appropriate default value for this parameter, which may vary by
**   resource; management software can override this default value by use
**   of this function ? If a resource is removed from, then re-added to the
**   RPT (e.g., because of a hot-swap action), the HPI implementation may
**   reset the value of this parameter. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource for which the severity
**      level will be set. 
**   Severity - [in] Severity level of event issued when the resource
**      unexpectedly becomes unavailable to the HPI. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceSeveritySet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_IN  SaHpiSeverityT   Severity
);


/*******************************************************************************
**
** Name: saHpiResourceTagSet 
**
** Description:
**   This function allows the caller to set the resource tag for a
**   particular resource. The resource tag is an informational value that
**   supplies the caller with naming information for the resource. This
**   should be set to the "user-visible" name for a resource, which can be
**   used to identify the resource in messages to a human operator. For
**   example, it could be set to match a physical, printed label attached
**   to the entity associated with the resource. Typically, the HPI
**   implementation will provide an appropriate default value for this
**   parameter; this function is provided so that management software can
**   override the default, if desired. The value of the resource tag may be
**   retrieved from the resource's RPT entry. Note: If a resource is
**   removed from, then re-added to the RPT (e.g., because of a hot-swap
**   action), the HPI implementation may reset the value of this parameter.
**   
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource for which the resource
**      tag should be set. 
**   ResourceTag - [in] Pointer to string representing the resource tag. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceTagSet (
     SAHPI_IN  SaHpiSessionIdT    SessionId,
     SAHPI_IN  SaHpiResourceIdT   ResourceId,
     SAHPI_IN  SaHpiTextBufferT   *ResourceTag
);


/*******************************************************************************
**
** Name: saHpiResourceIdGet 
**
** Description:
**   This function returns the resource ID of the resource associated with
**   the entity upon which the caller is running. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [out] Pointer to location to hold the returned resource
**      ID. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_NOT_PRESENT is returned if the entity the
**   caller is running on is not manageable in the addressed domain.
**   SA_ERR_HPI_UNKNOWN is returned if the domain controller cannot
**   determine an appropriate response. That is, there may be an
**   appropriate resource ID in the domain to return, but it cannot be
**   determined. 
**
** Remarks:
**   This function must be issued within a session to a domain that
**   includes a resource associated with the entity upon which the caller
**   is running, or the SA_ERR_HPI_NOT_PRESENT return will be issued. Since
**   entities are contained within other entities, there may be multiple
**   possible resources that could be returned to this call. For example,
**   if there is a resource ID associated with a particular compute blade
**   upon which the caller is running, and another associated with the
**   chassis which contains the compute blade, either could logically be
**   returned as an indication of a resource associated with the entity
**   upon which the caller was running. The function should return the
**   resource ID of the "smallest" resource that is associated with the
**   caller. So, in the example above, the function should return the
**   resource ID of the compute blade. Once the function has returned the
**   resourceID, the caller may issue further HPI calls using that
**   resourceID to learn the type of resource that been identified. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceIdGet (
     SAHPI_IN  SaHpiSessionIdT   SessionId,
     SAHPI_OUT SaHpiResourceIdT  *ResourceId
);


/*******************************************************************************
**
** Name: saHpiEntitySchemaGet 
**
** Description:
**   This function returns the identifier of the Entity Schema for the HPI
**   implementation. This schema defines valid Entity Paths that may be
**   returned by the HPI implementation. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   SchemaId - [out] Pointer to the ID of the schema in use; zero
**      indicates that a custom schema is in use. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   This function may be issued on any session opened to any domain in the
**   system, and will return the same identifier. The identifier returned
**   should either be zero, indicating that the HPI implementation uses a
**   custom schema; or one of the schema identifiers defined in Appendix A,
**   "Pre-Defined Entity Schemas," page 107. In the case of a custom
**   schema, the HPI implementation may use arbitrary entity paths to
**   describe resources in the system; in the case of a pre-defined schema,
**   all entity paths should conform to the schema. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEntitySchemaGet (
     SAHPI_IN  SaHpiSessionIdT     SessionId,
     SAHPI_OUT SaHpiUint32T        *SchemaId
);


/*******************************************************************************
**
** Name: saHpiEventLogInfoGet 
**
** Description:
**   This function retrieves the number of entries in the system event log,
**   total size of the event log, timestamp for the most recent entry, the
**   log's idea of the current time (i.e., timestamp that would be placed
**   on an entry at this moment), enabled/disabled status of the log (see
**   saHpiEventLogStateSet()), the overflow flag, the overflow action, and
**   whether the log supports deletion of individual entries. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log. 
**   Info - [out] Pointer to the returned SEL information. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogInfoGet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_OUT SaHpiSelInfoT    *Info
);


/*******************************************************************************
**
** Name: saHpiEventLogEntryGet 
**
** Description:
**   This function retrieves an event log entry from a system event log.
**   The special entry IDs SAHPI_OLDEST_ENTRY and SAHPI_NEWEST_ENTRY are
**   used to select the oldest and newest entries, respectively, in the log
**   being read. A returned NextEntryID of SAHPI_NO_MORE_ENTRIES indicates
**   that the newest entry has been returned; there are no more entries
**   going forward (time-wise) in the log. A returned PrevEntryID of
**   SAHPI_NO_MORE_ENTRIES indicates that the oldest entry has been
**   returned. To retrieve an entire list of entries going forward (oldest
**   entry to newest entry) in the log, call this function first with an
**   EntryID of SAHPI_OLDEST_ENTRY and then use the returned NextEntryID as
**   the EntryID in the next call. Proceed until the NextEntryID returned
**   is SAHPI_NO_MORE_ENTRIES. To retrieve an entire list of entries going
**   backward (newest entry to oldest entry) in the log, call this function
**   first with an EntryID of SAHPI_NEWEST_ENTRY and then use the returned
**   PrevEntryID as the EntryID in the next call. Proceed until the
**   PrevEntryID returned is SAHPI_NO_MORE_ENTRIES. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be read. Set to SAHPI_DOMAIN_CONTROLLER_ID to address the
**      domain system event log. 
**   EntryId - [in] Handle of the entry to retrieve from the SEL. Reserved
**      event log entry ID values:  SAHPI_OLDEST_ENTRY Oldest entry in the
**      log.  SAHPI_NEWEST_ENTRY Newest entry in the log. 
**      SAHPI_NO_MORE_ENTRIES Not valid for this parameter. Used only when
**      retrieving the next and previous entry IDs. 
**   PrevEntryId - [out] Handle of previous (older adjacent) entry in event
**      log. Reserved event log entry ID values:  SAHPI_OLDEST_ENTRY Not valid
**      for this parameter. Used only for the EntryID parameter. 
**      SAHPI_NEWEST_ENTRY Not valid for this parameter. Used only for the
**      EntryID parameter.  SAHPI_NO_MORE_ENTRIES No more entries in the log
**      before the one referenced by the EntryId parameter. 
**   NextEntryId - [out] Handle of next (newer adjacent) entry in event
**      log. Reserved event log entry ID values:  SAHPI_OLDEST_ENTRY Not valid
**      for this parameter. Used only for the EntryID parameter. 
**      SAHPI_NEWEST_ENTRY Not valid for this parameter. Used only for the
**      EntryID parameter.  SAHPI_NO_MORE_ENTRIES No more entries in the log
**      after the one referenced by the EntryId parameter. 
**   EventLogEntry - [out] Pointer to retrieved event log entry. 
**   Rdr - [in/out] Pointer to structure to receive resource data record
**      associated with the event, if available. If NULL, no RDR data will be
**      returned. 
**   RptEntry - [in/out] Pointer to structure to receive RPT Entry
**      associated with the event, if available. If NULL, no RPT entry data
**      will be returned. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.   
**
** Remarks:
**   Event logs may include RPT entries and resource data records
**   associated with the resource and sensor issuing an event along with
**   the basic event data in the log. Because the system may be
**   reconfigured after the event was entered in the log, this stored
**   information may be important to interpret the event. If the event log
**   includes logged RPT Entries and/or RDRs, and if the caller provides a
**   pointer to a structure to receive this information, it will be
**   returned along with the event log entry. If the caller provides a
**   pointer for an RPT entry, but the event log does not include a logged
**   RPT entry for the event being returned, RptEntry->ResourceCapabilities
**   will be set to zero. No valid RPTEntry will have a zero value here. If
**   the caller provides a pointer for an RDR, but the event log does not
**   include a logged RDR for the event being returned, Rdr->RdrType will
**   be set to SAHPI_NO_RECORD. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogEntryGet (
     SAHPI_IN    SaHpiSessionIdT     SessionId,
     SAHPI_IN    SaHpiResourceIdT    ResourceId,
     SAHPI_IN    SaHpiSelEntryIdT    EntryId,
     SAHPI_OUT   SaHpiSelEntryIdT    *PrevEntryId,
     SAHPI_OUT   SaHpiSelEntryIdT    *NextEntryId,
     SAHPI_OUT   SaHpiSelEntryT      *EventLogEntry,
     SAHPI_INOUT SaHpiRdrT           *Rdr,
     SAHPI_INOUT SaHpiRptEntryT      *RptEntry
);


/*******************************************************************************
**
** Name: saHpiEventLogEntryAdd 
**
** Description:
**   This function enables system management software to add entries to the
**   system event log. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log. 
**   EvtEntry - [in] Pointer to event log entry data to write to the system
**      event log. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   This function forces a write of the event to the addressed event log.
**   Nothing else is done with the event. Specific implementations of HPI
**   may have restrictions on how much data may be passed to the
**   saHpiEventLogEntryAdd() function. These restrictions should be
**   documented by the provider of the HPI interface. If more event log
**   data is provided than can be written, an error will be returned. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogEntryAdd (
     SAHPI_IN SaHpiSessionIdT      SessionId,
     SAHPI_IN SaHpiResourceIdT     ResourceId,
     SAHPI_IN SaHpiSelEntryT       *EvtEntry
);


/*******************************************************************************
**
** Name: saHpiEventLogEntryDelete 
**
** Description:
**   This function deletes an event log entry. This operation is only valid
**   if so indicated by saHpiEventLogInfoGet(), via the
**   DeleteEntrySupported field in the SaHpiSelInfoT structure. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in]  ResourceID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log. 
**   EntryId - [in] Entry ID on the event log entry to delete. Reserved
**      event log entry ID values:  SAHPI_OLDEST_ENTRY - Oldest entry in the
**      log.  SAHPI_NEWEST_ENTRY - Newest entry in the log. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_CMD is returned if this log does not
**   support this operation. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogEntryDelete (
     SAHPI_IN SaHpiSessionIdT      SessionId,
     SAHPI_IN SaHpiResourceIdT     ResourceId,
     SAHPI_IN SaHpiSelEntryIdT     EntryId
);


/*******************************************************************************
**
** Name: saHpiEventLogClear 
**
** Description:
**   This function erases the contents of the specified system event log. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in]  ResourceID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Note that all event logs must support the "clear" operation,
**   regardless of the setting of the DeleteEntrySupported field in the
**   SaHpiSelInfoT structure returned by saHpiEventLogInfoGet(). 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogClear (
     SAHPI_IN  SaHpiSessionIdT   SessionId,
     SAHPI_IN  SaHpiResourceIdT  ResourceId
);


/*******************************************************************************
**
** Name: saHpiEventLogTimeGet 
**
** Description:
**   This function retrieves the current time from the event log's own time
**   clock. The value of this clock is used to timestamp log entries
**   written into the log. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] ResourceID of the resource that contains the System
**      Event Log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log. 
**   Time - [out] Pointer to the returned SEL current time. If the
**      implementation cannot supply an absolute time value, then it may
**      supply a time relative to some system-defined epoch, such as system
**      boot. If the time value is less than or equal to
**      SAHPI_TIME_MAX_RELATIVE, but not SAHPI_TIME_UNSPECIFIED, then it is
**      relative; if it is greater than SAHPI_TIME_MAX_RELATIVE, then it is
**      absolute. The value SAHPI_TIME_UNSPECIFIED indicates that the time is
**      not set, or cannot be determined. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogTimeGet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_OUT SaHpiTimeT       *Time
);


/*******************************************************************************
**
** Name: saHpiEventLogTimeSet 
**
** Description:
**   This function sets the event log's time clock, which is used to
**   timestamp events written into the log. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log. 
**   Time - [in] time to set the SEL clock to. If the implementation cannot
**      supply an absolute time, then it may supply a time relative to some
**      system-defined epoch, such as system boot. If the timestamp value is
**      less than or equal to SAHPI_TIME_MAX_RELATIVE, but not
**      SAHPI_TIME_UNSPECIFIED, then it is relative; if it is greater than
**      SAHPI_TIME_MAX_RELATIVE, then it is absolute. The value
**      SAHPI_TIME_UNSPECIFIED indicates that the time of the event cannot be
**      determined. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogTimeSet (
     SAHPI_IN SaHpiSessionIdT   SessionId,
     SAHPI_IN SaHpiResourceIdT  ResourceId,
     SAHPI_IN SaHpiTimeT        Time
);


/*******************************************************************************
**
** Name: saHpiEventLogStateGet 
**
** Description:
**   This function enables system management software to get the event log
**   state. If the event log is "disabled" no events generated within the
**   HPI implementation will be added to the event log. Events may still be
**   added to the event log with the saHpiEventLogEntryAdd() function. When
**   the event log is "enabled" events may be automatically added to the
**   event log as they are generated in a resource or a domain, however, it
**   is implementation-specific which events are automatically added to any
**   event log. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] ResourceID of the resource that contains the System
**      Event Log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log. 
**   Enable - [out] Pointer to the current SEL state. True indicates that
**      the SEL is enabled; false indicates that it is disabled. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogStateGet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_OUT SaHpiBoolT       *Enable
);


/*******************************************************************************
**
** Name: saHpiEventLogStateSet 
**
** Description:
**   This function enables system management software to set the event log
**   enabled state. If the event log is "disabled" no events generated
**   within the HPI implementation will be added to the event log. Events
**   may still be added to the event log using the saHpiEventLogEntryAdd()
**   function. When the event log is "enabled" events may be automatically
**   added to the event log as they are generated in a resource or a
**   domain. The actual set of events that are automatically added to any
**   event log is implementation-specific. Typically, the HPI
**   implementation will provide an appropriate default value for this
**   parameter, which may vary by resource. This function is provided so
**   that management software can override the default, if desired. Note:
**   If a resource hosting an event log is re-initialized (e.g., because of
**   a hot-swap action), the HPI implementation may reset the value of this
**   parameter. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log. 
**   Enable - [in] SEL state to be set. True indicates that the SEL is to
**      be enabled; false indicates that it is to be disabled. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventLogStateSet (
     SAHPI_IN SaHpiSessionIdT   SessionId,
     SAHPI_IN SaHpiResourceIdT  ResourceId,
     SAHPI_IN SaHpiBoolT        Enable
);


/*******************************************************************************
**
** Name: saHpiSubscribe 
**
** Description:
**   This function allows the caller to subscribe for session events. This
**   single call provides subscription to all session events, regardless of
**   event type or event severity. Only one subscription is allowed per
**   session, and additional subscribers will receive an appropriate error
**   code. No event filtering will be done by the underlying management
**   service. 
**
** Parameters:
**   SessionId - [in] Session for which event subscription will be opened. 
**   ProvideActiveAlarms - [in] Indicates whether or not alarms which are
**      active at the time of subscription should be queued for future
**      retrieval via the saHpiEventGet() function.   
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_DUPLICATE is returned when a subscription is
**   already in place for this session. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSubscribe (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiBoolT       ProvideActiveAlarms
);


/*******************************************************************************
**
** Name: saHpiUnsubscribe 
**
** Description:
**   This function removes the event subscription for the session. After
**   removal of a subscription, additional saHpiEventGet() calls will not
**   be allowed unless the caller re-subscribes for events first. Any
**   events that are still in the event queue when this function is called
**   will be cleared from it. 
**
** Parameters:
**   SessionId - [in] Session for which event subscription will be closed. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_REQUEST is returned if the caller is
**   not currently subscribed for events in this session. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiUnsubscribe (
     SAHPI_IN SaHpiSessionIdT SessionId
);


/*******************************************************************************
**
** Name: saHpiEventGet 
**
** Description:
**   This function allows the caller to get an event. This call is only
**   valid within a session, which has subscribed for events. If the
**   ProvideActiveAlarms parameter was set in the subscription, the first
**   events retrieved will reflect the state of currently active alarms for
**   the resources belonging to the domain. After all active alarms are
**   retrieved this function will begin returning newly generated events as
**   the domain controller receives them. If there are one or more events
**   on the event queue when this function is called, it will immediately
**   return the next event on the queue. Otherwise, if the Timeout
**   parameter is SAHPI_TIMEOUT_IMMEDIATE, it will return
**   SA_ERR_HPI_TIMEOUT immediately. Otherwise, it will block for a time
**   specified by the timeout parameter; if an event is added to the queue
**   within that time, it will be returned immediately; if not,
**   saHpiEventGet() will return SA_ERR_HPI_TIMEOUT. If the Timeout
**   parameter is SAHPI_TIMEOUT_BLOCK, then saHpiEventGet() will block
**   indefinitely, until an event becomes available, and then return that
**   event. This provides for notification of events as they occur. 
**
** Parameters:
**   SessionId - [in] Session for which events are retrieved. 
**   Timeout - [in] The number of nanoseconds to wait for an event to
**      arrive. Reserved time out values: SAHPI_TIMEOUT_IMMEDIATE Time out
**      immediately if there are no events available (non-blocking call).
**      SAHPI_TIMEOUT_BLOCK Call should not return until an event is
**      retrieved. 
**   Event - [out] Pointer to the next available event. 
**   Rdr - [in/out] Pointer to structure to receive the resource data
**      associated with the event. If NULL, no RDR will be returned. 
**   RptEntry - [in/out] Pointer to structure to receive the RPT entry
**      associated with the resource that generated the event. If NULL, no RPT
**      entry will be returned. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_TIMEOUT is returned if no event is available
**   to return within the timeout period. If SAHPI_TIMEOUT_IMMEDIATE is
**   passed in the Timeout parameter, this error return will be used if
**   there is no event queued when the function is called. 
**
** Remarks:
**   If the caller provides a pointer for an RPT entry, but the event does
**   not include a valid resource ID for a resource in the domain (possible
**   on OEM or USER type event), then the ResourceCapabilities field in
**   *RptEntry will be set to zero. No valid RPT entry will have a zero
**   value here. If the caller provides a pointer for an RDR, but there is
**   no valid RDR associated with the event being returned (e.g., returned
**   event is not a sensor event), Rdr->RdrType will be set to
**   SAHPI_NO_RECORD. The timestamp reported in the returned event
**   structure is the best approximation an implementation has to when the
**   event actually occurred. The implementation may need to make an
**   approximation (such as the time the event was placed on the event
**   queue) because it may not have access to the actual time the event
**   occurred. The value SAHPI_TIME_UNSPECIFIED indicates that the time of
**   the event cannot be determined. If the implementation cannot supply an
**   absolute timestamp, then it may supply a timestamp relative to some
**   system-defined epoch, such as system boot. If the timestamp value is
**   less than or equal to SAHPI_TIME_MAX_RELATIVE, but not
**   SAHPI_TIME_UNSPECIFIED, then it is relative; if it is greater than
**   SAHPI_TIME_MAX_RELATIVE, then it is absolute.   6	Resource Functions 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEventGet (
     SAHPI_IN    SaHpiSessionIdT      SessionId,
     SAHPI_IN    SaHpiTimeoutT        Timeout,
     SAHPI_OUT   SaHpiEventT          *Event,
     SAHPI_INOUT SaHpiRdrT            *Rdr,
     SAHPI_INOUT SaHpiRptEntryT       *RptEntry
);


/*******************************************************************************
**
** Name: saHpiRdrGet 
**
** Description:
**   This function returns a resource data record from the addressed
**   resource. Submitting an EntryId of SAHPI_FIRST_ENTRY results in the
**   first RDR being read. A returned NextEntryID of SAHPI_LAST_ENTRY
**   indicates the last RDR has been returned. A successful retrieval will
**   include the next valid EntryId. To retrieve the entire list of RDRs,
**   call this function first with an EntryId of SAHPI_FIRST_ENTRY and then
**   use the returned NextEntryId in the next call. Proceed until the
**   NextEntryId returned is SAHPI_LAST_ENTRY. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   EntryId - [in] Handle of the RDR to retrieve. Reserved entry ID
**      values: SAHPI_FIRST_ENTRY Get first entry SAHPI_LAST_ENTRY Reserved as
**      delimiter for end of list. Not a valid entry identifier. 
**   NextEntryId - [out] Pointer to location to store Entry ID of next
**      entry in RDR repository. 
**   Rdr - [out] Pointer to the structure to receive the requested resource
**      data record. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   A resource's RDR repository is static over the lifetime of the
**   resource; therefore no precautions are required against changes to the
**   content of the RDR repository while it is being accessed. 
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
** Name: saHpiSensorReadingGet 
**
** Description:
**   This function is used to retrieve a sensor reading. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which the sensor reading is being
**      retrieved. 
**   Reading - [out] Pointer to a structure to receive sensor reading
**      values. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorReadingGet (
     SAHPI_IN  SaHpiSessionIdT      SessionId,
     SAHPI_IN  SaHpiResourceIdT     ResourceId,
     SAHPI_IN  SaHpiSensorNumT      SensorNum,
     SAHPI_OUT SaHpiSensorReadingT  *Reading
);


/*******************************************************************************
**
** Name: saHpiSensorReadingConvert 
**
** Description:
**   This function converts between raw and interpreted sensor reading
**   values. The type of conversion done depends on the passed-in
**   ReadingInput parameter. If it contains only a raw value, then this is
**   converted to an interpreted value in ConvertedReading; if it contains
**   only an interpreted value, then this is converted to a raw value in
**   ConvertedReading. If it contains neither type of value, or both, then
**   an error is returned. The ReadingInput parameter is not altered in any
**   case. If the sensor does not use raw values - i.e., it directly
**   returns interpreted values - then this routine returns an error. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which reading is associated. 
**   ReadingInput - [in] Pointer to the structure that contains raw or
**      interpreted reading to be converted. 
**   ConvertedReading - [out] Pointer to structure to hold converted
**      reading. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_PARAMS is returned if the ReadingInput
**   parameter is invalid; e.g. if it contains neither a raw nor an
**   interpreted value; or if it contains both; or if it contains an
**   invalid value. SA_ERR_HPI_INVALID_DATA is returned if the sensor does
**   not support raw readings. SA_ERR_HPI_NOT_PRESENT is returned if the
**   sensor is not present. 
**
** Remarks:
**   The EventStatus field in ReadingInput is not used by this function. To
**   make conversions, sensor-specific data may be required. Thus, the
**   function references a particular sensor in the system through the
**   SessionID/ResourceID/SensorNum parameters. If this sensor is not
**   present, and sensor- specific information is required, the conversion
**   will fail and SA_ERR_HPI_NOT_PRESENT will be returned. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorReadingConvert (
     SAHPI_IN  SaHpiSessionIdT      SessionId,
     SAHPI_IN  SaHpiResourceIdT     ResourceId,
     SAHPI_IN  SaHpiSensorNumT      SensorNum,
     SAHPI_IN  SaHpiSensorReadingT  *ReadingInput,
     SAHPI_OUT SaHpiSensorReadingT  *ConvertedReading
);


/*******************************************************************************
**
** Name: saHpiSensorThresholdsGet 
**
** Description:
**   This function retrieves the thresholds for the given sensor. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which threshold values are being
**      retrieved. 
**   SensorThresholds - [out] Pointer to returned sensor thresholds. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorThresholdsGet (
     SAHPI_IN  SaHpiSessionIdT        SessionId,
     SAHPI_IN  SaHpiResourceIdT       ResourceId,
     SAHPI_IN  SaHpiSensorNumT        SensorNum,
     SAHPI_OUT SaHpiSensorThresholdsT *SensorThresholds
);


/*******************************************************************************
**
** Name: saHpiSensorThresholdsSet 
**
** Description:
**   This function sets the specified thresholds for the given sensor. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of addressed resource. 
**   SensorNum - [in] Sensor number for which threshold values are being
**      set. 
**   SensorThresholds - [in] Pointer to the sensor thresholds values being
**      set. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   For each threshold or hysteresis value to be set, the corresponding
**   sensor reading structure must indicate whether a raw or interpreted
**   value is present. If neither are present, then that threshold or
**   hysteresis value will not be set. Each sensor may require settings to
**   be done with raw, or interpreted values, or may permit either; this is
**   defined by the field ThresholdDefn.TholdCapabilities in the sensor's
**   RDR (saHpiSensorRecT). If the interpreted value and raw value are both
**   provided, and both are legal for the sensor, the interpreted value
**   will be ignored and the raw value will be used. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorThresholdsSet (
     SAHPI_IN  SaHpiSessionIdT        SessionId,
     SAHPI_IN  SaHpiResourceIdT       ResourceId,
     SAHPI_IN  SaHpiSensorNumT        SensorNum,
     SAHPI_IN  SaHpiSensorThresholdsT *SensorThresholds
);


/*******************************************************************************
**
** Name: saHpiSensorTypeGet 
**
** Description:
**   This function retrieves the sensor type and event category for the
**   specified sensor. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which the type is being retrieved 
**   Type - [out] Pointer to returned enumerated sensor type for the
**      specified sensor. 
**   Category - [out] Pointer to location to receive the returned sensor
**      event category. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorTypeGet (
     SAHPI_IN  SaHpiSessionIdT     SessionId,
     SAHPI_IN  SaHpiResourceIdT    ResourceId,
     SAHPI_IN  SaHpiSensorNumT     SensorNum,
     SAHPI_OUT SaHpiSensorTypeT    *Type,
     SAHPI_OUT SaHpiEventCategoryT *Category
);


/*******************************************************************************
**
** Name: saHpiSensorEventEnablesGet 
**
** Description:
**   This function provides the ability to get the disable or enable event
**   message generation status for individual sensor events. The sensor
**   event states are relative to the event category specified by the
**   sensor. See the SaHpiEventCategoryT definition in section 7.3,
**   "Events, Part 1," on page 83 for more information. Within the
**   structure returned, there are two elements that contain bit flags; one
**   for assertion events and one for de-assertion events. A bit set to '1'
**   in the "AssertEvents" element in the structure indicates that an event
**   will be generated when the corresponding event state changes from
**   de-asserted to asserted on that sensor. A bit set to '1' in the
**   "DeassertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from asserted
**   to de-asserted on that sensor. The saHpiSensorEventEnablesGet()
**   function also returns the general sensor status - whether the sensor
**   is completely disabled, or event generation is completely disabled. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which the event enable
**      configuration is being requested 
**   Enables - [out] Pointer to the structure for returning sensor status
**      and event enable information. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Disabling events means that events are disabled for all sessions, not
**   just the session referenced by the SessionId parameter. For sensors
**   hosted by resources that have the "SAHPI_CAPABILITY_EVT_DEASSERTS"
**   flag set in its RPT entry, the "AssertEvents" element and the
**   "DeassertsEvents" element will always have same value. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventEnablesGet (
     SAHPI_IN  SaHpiSessionIdT         SessionId,
     SAHPI_IN  SaHpiResourceIdT        ResourceId,
     SAHPI_IN  SaHpiSensorNumT         SensorNum,
     SAHPI_OUT SaHpiSensorEvtEnablesT  *Enables
);


/*******************************************************************************
**
** Name: saHpiSensorEventEnablesSet 
**
** Description:
**   This function provides the ability to set the disable or enable event
**   message generation status for individual sensor events. The sensor
**   event states are relative to the event category specified by the
**   sensor. See the SaHpiEventCategoryT definition for more information.
**   Within the structure passed, there are two elements, which contain bit
**   flags; one for assertion events and one for de-assertion events.
**   However, the use of these two elements depends on whether the resource
**   addressed has the "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set in its RPT
**   entry. This capability, if set, advertises that all sensors hosted by
**   the resource will always send a "de-assert" event when any state is
**   de-asserted whose assertion generates an "assert" event. Thus, for
**   sensors hosted by resources that advertise this behavior, it is not
**   meaningful to control assert events and de-assert events separately.
**   For sensors on resources that do not have the
**   "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set, a bit set to '1' in the
**   "AssertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from
**   de-asserted to asserted on that sensor., and a bit set to '1' in the
**   "DeassertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from asserted
**   to de-asserted on that sensor. For sensors on resources, which do have
**   the "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set, the "DeassertEvents"
**   element is not used. For sensors on these resources, a bit set to '1'
**   in the "AssertEvents" element in the structure indicates that an event
**   will be generated when the corresponding event state changes in either
**   direction (de-asserted to asserted or asserted to de-asserted). The
**   saHpiSensorEventEnablesSet() function also allows setting of general
**   sensor status - whether the sensor is completely disabled, or event
**   generation is completely disabled. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   SensorNum - [in] Sensor number for which the event enables are being
**      set. 
**   Enables - [in] Pointer to the structure containing the enabled status
**      for each event. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Disabling events means that events are disabled for all sessions, not
**   just the session referenced by the SessionId parameter.
**   saHpiSensorEventEnablesGet () will return the values which were last
**   set by saHpiSensorEventEnablesSet() for the "AssertEvents" and
**   "DeassertEvents" elements in the passed data structures. However, for
**   sensors hosted by any resource that has the
**   SAHPI_CAPABILITY_EVT_DEASSERTS flag set in its RPT entry, the passed
**   "AssertEvents" element on the saHpiSensorEventEnablesSet () function
**   is used for both assertion and de-assertion event enable flags. In
**   this case, this value will be returned in both the "AssertEvents" and
**   "DeassertEvents" elements on a subsequent saHpiSensorEventEnablesGet
**   () call.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiSensorEventEnablesSet (
     SAHPI_IN SaHpiSessionIdT        SessionId,
     SAHPI_IN SaHpiResourceIdT       ResourceId,
     SAHPI_IN SaHpiSensorNumT        SensorNum,
     SAHPI_IN SaHpiSensorEvtEnablesT *Enables
);


/*******************************************************************************
**
** Name: saHpiControlTypeGet 
**
** Description:
**   This function retrieves the control type of a control object. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   CtrlNum - [in] Control number 
**   Type - [out] Pointer to SaHpiCtrlTypeT variable to receive the
**      enumerated control type for the specified control. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   The Type parameter must point to a variable of type SaHpiCtrlTypeT.
**   Upon successful completion, the enumerated control type is returned in
**   the variable pointed to by Type. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlTypeGet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_IN  SaHpiCtrlNumT    CtrlNum,
     SAHPI_OUT SaHpiCtrlTypeT   *Type
);


/*******************************************************************************
**
** Name: saHpiControlStateGet 
**
** Description:
**   This function retrieves the current state (generally the last state
**   set) of a control object. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of addressed resource. 
**   CtrlNum - [in] Number of the control for which the state is being
**      retrieved. 
**   CtrlState - [in/out] Pointer to a control data structure into which
**      the current control state will be placed. For text controls, the line
**      number to read is passed in via CtrlState->StateUnion.Text.Line. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Note that Text controls are unique in that they have a state
**   associated with each line of the control - the state being the text on
**   that line. The line number to be read is passed in to
**   sahpiControlStateGet()via CtrlState- >StateUnion.Text.Line; the
**   contents of that line of the control will be returned in CtrlState-
**   >StateUnion.Text.Text. If the line number passed in is
**   SAHPI_TLN_ALL_LINES, then sahpiControlStateGet() will return the
**   entire text of the control, or as much of it as will fit in a single
**   SaHpiTextBufferT, in CtrlState- >StateUnion.Text.Text. This value will
**   consist of the text of all the lines concatenated, using the maximum
**   number of characters for each line (no trimming of trailing blanks).
**   Note that depending on the data type and language, the text may be
**   encoded in 2-byte Unicode, which requires two bytes of data per
**   character. Note that the number of lines and columns in a text control
**   can be obtained from the control's Resource Data Record. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlStateGet (
     SAHPI_IN    SaHpiSessionIdT  SessionId,
     SAHPI_IN    SaHpiResourceIdT ResourceId,
     SAHPI_IN    SaHpiCtrlNumT    CtrlNum,
     SAHPI_INOUT SaHpiCtrlStateT  *CtrlState
);


/*******************************************************************************
**
** Name: saHpiControlStateSet 
**
** Description:
**   This function is used for setting the state of the specified control
**   object. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   CtrlNum - [in] Number of the control for which the state is being set.
**      
**   CtrlState - [in] Pointer to a control state data structure holding the
**      state to be set 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   The CtrlState parameter must be of the correct type for the specified
**   control. Text controls include a line number and a line of text in the
**   CtrlState parameter, allowing update of just a single line of a text
**   control. If less than a full line of data is written, the control will
**   clear all spaces beyond those written on the line. Thus writing a
**   zero-length string will clear the addressed line. It is also possible
**   to include more characters in the text passed in the CtrlState
**   structure than will fit on one line; in this case, the control will
**   "wrap" to the next line (still clearing the trailing characters on the
**   last line written). Thus, there are two ways to write multiple lines
**   to a text control: (a) call saHpiControlStateSet() repeatedly for each
**   line, or (b) call saHpiControlStateSet() once and send more characters
**   than will fit on one line. The caller should not assume any "cursor
**   positioning" characters are available to use, but rather should always
**   write full lines and allow "wrapping" to occur. When calling
**   saHpiControlStateSet() for a text control, the caller may set the line
**   number to SAHPI_TLN_ALL_LINES; in this case, the entire control will
**   be cleared, and the data will be written starting on line 0. (This is
**   different from simply writing at line 0, which only alters the lines
**   written to.) This feature may be used to clear the entire control,
**   which can be accomplished by setting: CtrlState->StateUnion.Text.Line
**   = SAHPI_TLN_ALL_LINES; CtrlState->StateUnion.Text.Text.DataLength = 0;
**   Note that the number of lines and columns in a text control can be
**   obtained from the control's Resource Data Record. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiControlStateSet (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId,
     SAHPI_IN SaHpiCtrlNumT    CtrlNum,
     SAHPI_IN SaHpiCtrlStateT  *CtrlState
);


/*******************************************************************************
**
** Name: saHpiEntityInventoryDataRead 
**
** Description:
**   This function returns inventory data for a particular entity
**   associated with a resource. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   EirId - [in] Identifier for the entity inventory repository. 
**   BufferSize - [in] Size of the InventData buffer passed in. 
**   InventData - [out] Pointer to the buffer for the returned data. 
**   ActualSize - [out] Pointer to size of the actual amount of data
**      returned. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_INVENT_DATA_TRUNCATED is returned if the buffer
**   passed in the InventData structure is not large enough (as indicated
**   by the "BufferSize" parameter) to hold the entire InventData
**   structure.   
**
** Remarks:
**   Before calling saHpiEntityInventoryDataRead() the caller should
**   allocate a sufficiently large buffer to hold the data, and pass the
**   size of the buffer in the "BufferSize" parameter. The
**   saHpiEntityInventoryDataRead() function will return, at the location
**   pointed to by the ActualSize parameter, the actual space used in the
**   buffer to hold the returned data. If the data will not fit in the
**   buffer, as much as will fit will be returned, *ActualSize will be set
**   to indicated a suggested buffer size for the entire inventory data,
**   the "Validity" field in the InventData buffer will be set to
**   "SAHPI_INVENT_DATA_OVERFLOW," and an error return will be made. Since
**   it is impossible to know how large the inventory data may be without
**   actually reading and processing it from the entity inventory
**   repository, it may be advisable to err on the large side in allocating
**   the buffer. Note that the data includes many pointers to
**   SaHpiTextBufferT structures. The implementation of
**   saHpiEntityInventoryDataRead() may not reserve space for the maximum
**   size of each of these structures when formatting the data in the
**   returned buffer. Thus, if a user wishes to lengthen the data in one of
**   these structures, a new SaHpiTextBufferT structure should be
**   allocated, and the appropriate pointer reset to point to this new
**   structure in memory. See the description of the SaHpiInventoryDataT
**   structure in section 7.9, "Entity Inventory Data," on page 94, for
**   details on the format of the returned data. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEntityInventoryDataRead (
     SAHPI_IN    SaHpiSessionIdT         SessionId,
     SAHPI_IN    SaHpiResourceIdT        ResourceId,
     SAHPI_IN    SaHpiEirIdT             EirId,
     SAHPI_IN    SaHpiUint32T            BufferSize,
     SAHPI_OUT   SaHpiInventoryDataT     *InventData,
     SAHPI_OUT   SaHpiUint32T            *ActualSize
);


/*******************************************************************************
**
** Name: saHpiEntityInventoryDataWrite 
**
** Description:
**   This function writes the specified data to the inventory information
**   area. Note: If the resource hosting the inventory data is
**   re-initialized, or if the entity itself is removed and reinserted, the
**   inventory data may be reset to its default settings, losing data
**   written to the repository with this function. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   EirId - [in] Identifier for the entity inventory repository. 
**   InventData - [in] Pointer to data to write to the repository. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   The SaHpiInventoryDataT structure consists of a Validity field and
**   then a set of pointers to record structures. It is not required that
**   all pointers point to data within a single contiguous buffer. The
**   "Validity" field in the SaHpiInventoryDataT structure must be set to
**   "SAHPI_INVENT_DATA_VALID," or else the saHpiEntityInventoryDataWrite()
**   function will take no action and return an error. This is to help
**   prevent invalid data returned by a saHpiEntityInventoryDataRead()
**   function from being inadvertently written to the resource. For this
**   protection to work, the caller should not change the value of the
**   "Validity" field in the SaHpiInventoryDataT structure unless building
**   an entire Inventory Data set from scratch. Some implementations may
**   impose limitations on the languages of the strings passed in within
**   the InventData parameter.  Implementation-specific documentation
**   should identify these restrictions. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiEntityInventoryDataWrite (
     SAHPI_IN SaHpiSessionIdT          SessionId,
     SAHPI_IN SaHpiResourceIdT         ResourceId,
     SAHPI_IN SaHpiEirIdT              EirId,
     SAHPI_IN SaHpiInventoryDataT      *InventData
);


/*******************************************************************************
**
** Name: saHpiWatchdogTimerGet 
**
** Description:
**   This function retrieves the current watchdog timer settings and
**   configuration. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource, which contains the
**      watchdog timer being addressed. 
**   WatchdogNum - [in] The watchdog number that specifies the watchdog
**      timer on a resource. 
**   Watchdog - [out] Pointer to watchdog data structure. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   See the description of the SaHpiWatchdogT structure in 7.11,
**   "Watchdogs" on page 96 for details on what information is returned by
**   this function. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerGet (
     SAHPI_IN  SaHpiSessionIdT    SessionId,
     SAHPI_IN  SaHpiResourceIdT   ResourceId,
     SAHPI_IN  SaHpiWatchdogNumT  WatchdogNum,
     SAHPI_OUT SaHpiWatchdogT     *Watchdog
);


/*******************************************************************************
**
** Name: saHpiWatchdogTimerSet 
**
** Description:
**   This function provides a method for initializing the watchdog timer
**   configuration. Once the appropriate configuration has be set using
**   saHpiWatchdogTimerSet(), the user must then call
**   saHpiWatchdogTimerReset() to initially start the watchdog timer. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the resource that contains the
**      watchdog timer being addressed. 
**   WatchdogNum - [in] The watchdog number specifying the specific
**      watchdog timer on a resource. 
**   Watchdog - [in] Pointer to watchdog data structure. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   If the initial counter value in the SaHpiWatchdogT structure is set to
**   0, the Watchdog will immediately time out and take the pre-timeout and
**   timeout actions, as well as log an event. This provides a mechanism
**   for software to force an immediate recovery action should that be
**   dependent on a Watchdog timeout occurring. See the description of the
**   SaHpiWatchdogT structure for more details on the effects of this
**   command related to specific data passed in that structure.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerSet (
     SAHPI_IN SaHpiSessionIdT    SessionId,
     SAHPI_IN SaHpiResourceIdT   ResourceId,
     SAHPI_IN SaHpiWatchdogNumT  WatchdogNum,
     SAHPI_IN SaHpiWatchdogT     *Watchdog
);


/*******************************************************************************
**
** Name: saHpiWatchdogTimerReset 
**
** Description:
**   This function provides a method to start or restart the watchdog timer
**   from the initial countdown value. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID for the resource that contains the
**      watchdog timer being addressed. 
**   WatchdogNum - [in] The watchdog number specifying the specific
**      watchdog timer on a resource. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   If the Watchdog has been configured to issue a Pre-Timeout interrupt,
**   and that interrupt has already occurred, the saHpiWatchdogTimerReset()
**   function will not reset  the watchdog counter. The only way to stop a
**   Watchdog from timing out once a Pre-Timeout interrupt has occurred is
**   to use the saHpiWatchdogTimerSet() function to reset and/or stop the
**   timer. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiWatchdogTimerReset (
     SAHPI_IN SaHpiSessionIdT   SessionId,
     SAHPI_IN SaHpiResourceIdT  ResourceId,
     SAHPI_IN SaHpiWatchdogNumT WatchdogNum
);


/*******************************************************************************
**
** Name: saHpiHotSwapControlRequest 
**
** Description:
**   A resource supporting hot swap typically supports default policies for
**   insertion and extraction. On insertion, the default policy may be for
**   the resource to turn the associated FRU's local power on and to
**   de-assert reset. On extraction, the default policy may be for the
**   resource to immediately power off the FRU and turn on a hot swap
**   indicator. This function allows a caller, after receiving a hot swap
**   event with HotSwapState equal to SAHPI_HS_STATE_INSERTION_PENDING or
**   SAHPI_HS_STATE_EXTRACTION_PENDING, to request control of the hot swap
**   policy and prevent the default policy from being invoked. Because a
**   resource that supports the simplified hot swap model will never
**   transition into Insertion Pending or Extraction Pending states, this
**   function is not applicable to those resources. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapControlRequest (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId
);


/*******************************************************************************
**
** Name: saHpiResourceActiveSet 
**
** Description:
**   During insertion, a resource supporting hot swap will generate an
**   event to indicate that it is in the INSERTION PENDING state. If the
**   management middleware or other user software calls
**   saHpiHotSwapControlRequest() before the resource begins an auto-insert
**   operation, then the resource will remain in INSERTION PENDING state
**   while the user acts on the resource to integrate it into the system.
**   During this state, the user can instruct the resource to power on the
**   associated FRU, to de-assert reset, or to turn off its hot swap
**   indicator using the saHpiResourcePowerStateSet(),
**   saHpiResourceResetStateSet(), or saHpiHotSwapIndicatorStateSet()
**   functions, respectively. Once the user has completed with the
**   integration of the FRU, this function must be called to signal that
**   the resource should now transition into ACTIVE/HEALTHY or
**   ACTIVE/UNHEALTHY state (depending on whether or not there are active
**   faults). The user may also use this function to request a resource to
**   return to the ACTIVE/HEALTHY or ACTIVE/UNHEALTHY state from the
**   EXTRACTION PENDING state in order to reject an extraction request.
**   Because a resource that supports the simplified hot swap model will
**   never transition into Insertion Pending or Extraction Pending states,
**   this function is not applicable to those resources. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Only valid if resource is in INSERTION PENDING or EXTRACTION PENDING
**   state and an auto-insert or auto-extract policy action has not been
**   initiated.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceActiveSet (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId
);


/*******************************************************************************
**
** Name: saHpiResourceInactiveSet 
**
** Description:
**   During extraction, a resource supporting hot swap will generate an
**   event to indicate that it is in the EXTRACTION PENDING state. If the
**   management middleware or other user software calls
**   saHpiHotSwapControlRequest() before the resource begins an
**   auto-extract operation, then the resource will remain in EXTRACTION
**   PENDING state while the user acts on the resource to isolate the
**   associated FRU from the system. During this state, the user can
**   instruct the resource to power off the FRU, to assert reset, or to
**   turn on its hot swap indicator using the saHpiResourcePowerStateSet(),
**   saHpiResourceResetStateSet(), or saHpiHotSwapIndicatorStateSet()
**   functions, respectively. Once the user has completed the shutdown of
**   the FRU, this function must be called to signal that the resource
**   should now transition into INACTIVE state. The user may also use this
**   function to request a resource to return to the INACTIVE state from
**   the INSERTION PENDING state to abort a hot-swap insertion action.
**   Because a resource that supports the simplified hot swap model will
**   never transition into Insertion Pending or Extraction Pending states,
**   this function is not applicable to those resources. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Only valid if resource is in EXTRACTION PENDING or INSERTION PENDING
**   state and an auto-extract or auto-insert policy action has not been
**   initiated. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceInactiveSet (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId
);


/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutGet
**
** Description:
**   This function allows the caller to request the auto-insert timeout
**   value. This value indicates how long the HPI implementation will wait
**   before the default auto-insertion policy is invoked. Further
**   information on the auto-insert timeout can be found in the function
**   saHpiAutoInsertTimeoutSet(). 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   Timeout - [out] Pointer to location to store the number of nanoseconds
**      to wait before autonomous handling of the hotswap event. Reserved time
**      out values:  SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is
**      immediate.  SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not
**      occur.  
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoInsertTimeoutGet(
     SAHPI_IN  SaHpiSessionIdT SessionId,
     SAHPI_OUT SaHpiTimeoutT    *Timeout
);


/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutSet
**
** Description:
**   This function allows the caller to configure a timeout for how long to
**   wait before the default auto-insertion policy is invoked. This
**   function accepts a parameter instructing the implementation to impose
**   a delay before a resource will perform its default hot swap policy for
**   auto-insertion. The parameter may be set to SAHPI_TIMEOUT_IMMEDIATE to
**   direct resources to proceed immediately to auto-insertion, or to
**   SAHPI_TIMEOUT_BLOCK to prevent auto-insertion from ever occurring. If
**   the parameter is set to another value, then it defines the number of
**   nanoseconds between the time a hot swap event with HotSwapState =
**   SAHPI_HS_STATE_INSERTION_PENDING is generated, and the time that the
**   auto-insertion policy will be invoked for that resource. If, during
**   this time period, a saHpiHotSwapControlRequest() function is
**   processed, the timer will be stopped, and the auto-insertion policy
**   will not be invoked. Once the auto-insertion process begins, the user
**   software will not be allowed to take control of the insertion process;
**   hence, the timeout should be set appropriately to allow for this
**   condition. Note that the timeout period begins when the hot swap event
**   with HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING is initially
**   generated; not when it is received by a caller with a saHpiEventGet()
**   function call, or even when it is placed in a session event queue. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   Timeout - [in] The number of nanoseconds to wait before autonomous
**      handling of the hotswap event. Reserved time out values: 
**      SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**      handling.  SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoInsertTimeoutSet(
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiTimeoutT    Timeout
);


/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutGet
**
** Description:
**   This function allows the caller to request the timeout for how long
**   the implementation will wait before the default auto-extraction policy
**   is invoked. Further information on auto-extract time outs is detailed
**   in saHpiAutoExtractTimeoutSet(). 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   Timeout - [out] Pointer to location to store the number of nanoseconds
**      to wait before autonomous handling of the hotswap event. Reserved time
**      out values:  SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is
**      immediate.  SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not
**      occur. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoExtractTimeoutGet(
     SAHPI_IN  SaHpiSessionIdT   SessionId,
     SAHPI_IN  SaHpiResourceIdT  ResourceId,
     SAHPI_OUT SaHpiTimeoutT     *Timeout
);


/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutSet
**
** Description:
**   This function allows the caller to configure a timeout for how long to
**   wait before the default auto-extraction policy is invoked. This
**   function accepts a parameter instructing the implementation to impose
**   a delay before a resource will perform its default hot swap policy for
**   auto-extraction. The parameter may be set to SAHPI_TIMEOUT_IMMEDIATE
**   to direct the resource to proceed immediately to auto-extraction, or
**   to SAHPI_TIMEOUT_BLOCK to prevent auto-extraction from ever occurring
**   on a resource. If the parameter is set to another value, then it
**   defines the number of nanoseconds between the time a hot swap event
**   with HotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING is generated,
**   and the time that the auto- extraction policy will be invoked for the
**   resource. If, during this time period, a saHpiHotSwapControlRequest()
**   function is processed, the timer will be stopped, and the
**   auto-extraction policy will not be invoked. Once the auto-extraction
**   process begins, the user software will not be allowed to take control
**   of the extraction process; hence, the timeout should be set
**   appropriately to allow for this condition. Note that the timeout
**   period begins when the hot swap event with HotSwapState =
**   SAHPI_HS_STATE_EXTRACTION_PENDING is initially generated; not when it
**   is received by a caller with a saHpiEventGet() function call, or even
**   when it is placed in a session event queue. The auto-extraction policy
**   is set at the resource level and is only supported by resources
**   supporting the "Managed Hot Swap" capability. After discovering that a
**   newly inserted resource supports "Managed Hot Swap," middleware or
**   other user software may use this function to change the default
**   auto-extraction policy for that resource. If a resource supports the
**   simplified hot-swap model, setting this timer has no effect since the
**   resource will transition directly to "Not Present" state on an
**   extraction.   
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   Timeout - [in] The number of nanoseconds to wait before autonomous
**      handling of the hotswap event. Reserved time out values: 
**      SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**      handling.  SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAutoExtractTimeoutSet(
     SAHPI_IN  SaHpiSessionIdT   SessionId,
     SAHPI_IN  SaHpiResourceIdT  ResourceId,
     SAHPI_IN  SaHpiTimeoutT     Timeout
);


/*******************************************************************************
**
** Name: saHpiHotSwapStateGet 
**
** Description:
**   This function allows the caller to retrieve the current hot swap state
**   of a resource. The returned state will be one of the following five
**   states: ? SAHPI_HS_STATE_INSERTION_PENDING ?
**   SAHPI_HS_STATE_ACTIVE_HEALTHY ? SAHPI_HS_STATE_ACTIVE_UNHEALTHY ?
**   SAHPI_HS_STATE_EXTRACTION_PENDING ? SAHPI_HS_STATE_INACTIVE The state
**   SAHPI_HS_STATE_NOT_PRESENT will never be returned, because a resource
**   that is not present cannot be addressed by this function in the first
**   place. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   State - [out] Pointer to location to store returned state information.
**        
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   None. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapStateGet (
     SAHPI_IN  SaHpiSessionIdT  SessionId,
     SAHPI_IN  SaHpiResourceIdT ResourceId,
     SAHPI_OUT SaHpiHsStateT    *State
);


/*******************************************************************************
**
** Name: saHpiHotSwapActionRequest 
**
** Description:
**   A resource supporting hot swap typically requires a physical action on
**   the associated FRU to invoke an insertion or extraction process. An
**   insertion process is invoked by physically inserting the FRU into a
**   chassis. Physically opening an ejector latch or pressing a button
**   invokes the extraction process. This function allows the caller to
**   invoke an insertion or extraction process via software. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   Action - [in] Requested action: SAHPI_HS_ACTION_INSERTION or
**      SAHPI_HS_ACTION_EXTRACTION 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   There may be limitations on when saHpiHotSwapActionRequest() may be
**   called, and what value may be used for the "Action" parameter
**   depending on what state the resource is currently in. At the least,
**   this function may be called: ?	To request an Insertion action when the
**   resource is in INACTIVE state ?	To request an Extraction action when
**   the resource is in the ACTIVE/HEALTHY or ACTIVE/ UNHEALTHY state. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapActionRequest (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId,
     SAHPI_IN SaHpiHsActionT   Action
);


/*******************************************************************************
**
** Name: saHpiResourcePowerStateGet 
**
** Description:
**   A typical resource supporting hot swap will have the ability to
**   control local power on the FRU associated with the resource. During
**   insertion, the FRU can be instructed to power on. During extraction
**   the FRU can be requested to power off. This function allows the caller
**   to retrieve the current power state of the FRU associated with the
**   specified resource. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   State - [out] The current power state of the resource. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   This function returns the actual low-level power state of the FRU,
**   regardless of what hot-swap state the resource is in. Not all
**   resources supporting managed hot swap will necessarily support this
**   function. In particular, resources that use the simplified hot swap
**   model may not have the ability to control FRU power. An appropriate
**   error code will be returned if the resource does not support power
**   control on the FRU. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourcePowerStateGet (
     SAHPI_IN  SaHpiSessionIdT     SessionId,
     SAHPI_IN  SaHpiResourceIdT    ResourceId,
     SAHPI_OUT SaHpiHsPowerStateT  *State
);


/*******************************************************************************
**
** Name: saHpiResourcePowerStateSet 
**
** Description:
**   A typical resource supporting hot swap will have to ability to control
**   local power on the FRU associated with the resource. During insertion,
**   the FRU can be instructed to power on. During extraction the FRU can
**   be requested to power off. This function allows the caller to set the
**   current power state of the FRU associated with the specified resource.
**   
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   State - [in] the new power state that the specified resource will be
**      set to. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   This function controls the hardware power on the FRU of what hot-swap
**   state the resource is in. For example, it is legal (and may be
**   desirable) to cycle power on the FRU even while it is in ACTIVE state
**   in order to attempt to clear a fault condition. Similarly, a resource
**   could be instructed to power on a FRU even while it is in INACTIVE
**   state, for example, in order to run off-line diagnostics. Not all
**   resources supporting managed hot swap will necessarily support this
**   function. In particular, resources that use the simplified hot swap
**   model may not have the ability to control FRU power. An appropriate
**   error code will be returned if the resource does not support power
**   control on the FRU.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourcePowerStateSet (
     SAHPI_IN SaHpiSessionIdT     SessionId,
     SAHPI_IN SaHpiResourceIdT    ResourceId,
     SAHPI_IN SaHpiHsPowerStateT  State
);


/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateGet 
**
** Description:
**   A FRU associated with a hot-swappable resource may include a hot swap
**   indicator such as a blue LED. This indicator signifies that the FRU is
**   ready for removal.. This function allows the caller to retrieve the
**   state of this indicator. The returned state is either
**   SAHPI_HS_INDICATOR_OFF or SAHPI_HS_INDICATOR_ON. This function will
**   return the state of the indicator, regardless of what hot swap state
**   the resource is in. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   State - [out] Pointer to location to store state of hot swap
**      indicator. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Not all resources supporting managed hot swap will necessarily support
**   this function. In particular, resources that use the simplified hot
**   swap model may not have the ability to control a FRU hot swap
**   indicator (it is likely that none exists). An appropriate error code
**   will be returned if the resource does not support control of a hot
**   swap indicator on the FRU. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapIndicatorStateGet (
     SAHPI_IN  SaHpiSessionIdT         SessionId,
     SAHPI_IN  SaHpiResourceIdT        ResourceId,
     SAHPI_OUT SaHpiHsIndicatorStateT  *State
);


/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateSet 
**
** Description:
**   A FRU associated with a hot-swappable resource may include a hot swap
**   indicator such as a blue LED. This indicator signifies that the FRU is
**   ready for removal. This function allows the caller to set the state of
**   this indicator. Valid states include SAHPI_HS_INDICATOR_OFF or
**   SAHPI_HS_INDICATOR_ON. This function will set the indicator regardless
**   of what hot swap state the resource is in, though it is recommended
**   that this function be used only in conjunction with moving the
**   resource to the appropriate hot swap state. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource 
**   State - [in] State of hot swap indicator to be set. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.   
**
** Remarks:
**   Not all resources supporting managed hot swap will necessarily support
**   this function. In particular, resources that use the simplified hot
**   swap model may not have the ability to control a FRU hot swap
**   indicator (it is likely that none exists). An appropriate error code
**   will be returned if the resource does not support control of a hot
**   swap indicator on the FRU. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiHotSwapIndicatorStateSet (
     SAHPI_IN SaHpiSessionIdT         SessionId,
     SAHPI_IN SaHpiResourceIdT        ResourceId,
     SAHPI_IN SaHpiHsIndicatorStateT  State
);


/*******************************************************************************
**
** Name: saHpiParmControl 
**
** Description:
**   This function allows the user to save and restore parameters
**   associated with a specific resource. Valid actions for this function
**   include: SAHPI_DEFAULT_PARM Restores the factory default settings for
**   a specific resource. Factory defaults include sensor thresholds and
**   configurations, and resource- specific configuration parameters.
**   SAHPI_SAVE_PARM Stores the resource configuration parameters in
**   non-volatile storage. Resource configuration parameters stored in
**   non-volatile storage will survive power cycles and resource resets.
**   SAHPI_RESTORE_PARM Restores resource configuration parameters from
**   non-volatile storage. Resource configuration parameters include sensor
**   thresholds and sensor configurations, as well as resource-specific
**   parameters. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   Action - [in] Action to perform on resource parameters. 
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. 
**
** Remarks:
**   Resource-specific parameters should be documented in an implementation
**   guide for the HPI implementation.   
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiParmControl (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId,
     SAHPI_IN SaHpiParmActionT Action
);


/*******************************************************************************
**
** Name: saHpiResourceResetStateGet 
**
** Description:
**   This function gets the reset state of an entity, allowing the user to
**   determine if the entity is being held with its reset asserted. If a
**   resource manages multiple entities, this function will address the
**   entity which is identified in the RPT entry for the resource. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   ResetAction - [out] The current reset state of the entity. Valid reset
**      states are:  SAHPI_RESET_ASSERT: The entity's reset is asserted, e.g.,
**      for hot swap insertion/extraction purposes  SAHPI_RESET_DEASSERT: The
**      entity's reset is not asserted 
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the reset
**   state has successfully been determined; otherwise, an error code is
**   returned. SA_ERR_HPI_INVALID_CMD is returned if the resource has no
**   reset control. 
**
** Remarks:
**   SAHPI_RESET_COLD and SAHPI_RESET_WARM are pulsed resets, and are not
**   valid return values for ResetAction. If the entity is not being held
**   in reset (using SAHPI_RESET_ASSERT), the appropriate return value is
**   SAHPI_RESET_DEASSERT. 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceResetStateGet (
     SAHPI_IN SaHpiSessionIdT    SessionId,
     SAHPI_IN SaHpiResourceIdT   ResourceId,
     SAHPI_OUT SaHpiResetActionT *ResetAction
);


/*******************************************************************************
**
** Name: saHpiResourceResetStateSet 
**
** Description:
**   This function directs the resource to perform the specified reset type
**   on the entity that it manages. If a resource manages multiple
**   entities, this function addresses the entity that is identified in the
**   RPT entry for the resource. Entities may be reset for a variety of
**   reasons. A misbehaving entity may be reset to bring it to a known
**   state. In these cases, either a warm reset or a cold reset may be
**   performed. A warm reset preserves entity state, whereas a cold reset
**   does not. Both of these reset types are pulsed asserted and then
**   de-asserted by the HPI implementation. This allows the HPI
**   implementation to hold the reset asserted for the appropriate length
**   of time, as needed by each entity. saHpiResourceResetStateSet() can
**   also be used for insertion and extraction scenarios. A typical
**   resource supporting hot swap will have to ability to control local
**   reset within the FRU. During insertion, a resource can be instructed
**   to assert reset, while the FRU powers on. During extraction a resource
**   can be requested to assert reset before the FRU is powered off. This
**   function allows the caller to set the reset state of the specified
**   FRU. SAHPI_RESET_ASSERT is used to hold the resource in reset; the FRU
**   is brought out of the reset state by using either SAHPI_COLD_RESET or
**   SAHPI_WARM_RESET. 
**
** Parameters:
**   SessionId - [in] Handle to session context. 
**   ResourceId - [in] Resource ID of the addressed resource. 
**   ResetAction - [in] Type of reset to perform on the entity. Valid reset
**      actions are:  SAHPI_COLD_RESET: Perform a 'Cold Reset' on the entity
**      (pulse), leaving reset de-asserted  SAHPI_WARM_RESET: Perform a 'Warm
**      Reset' on the entity (pulse), leaving reset de-asserted 
**      SAHPI_RESET_ASSERT: Put the entity into reset state and hold reset
**      asserted, e.g., for hot swap insertion/extraction purposes 
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the requested
**   reset action has succeeded; otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_CMD is returned if the resource has no reset
**   control, or if the requested reset action is not supported by the
**   resource. 
**
** Remarks:
**   Some resources may not support reset, or may only support a subset of
**   the defined reset action types. Also, on some resources, cold and warm
**   resets may be equivalent.    7	Data Type Definitions 
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceResetStateSet (
     SAHPI_IN SaHpiSessionIdT  SessionId,
     SAHPI_IN SaHpiResourceIdT ResourceId,
     SAHPI_IN SaHpiResetActionT ResetAction
);



#endif

