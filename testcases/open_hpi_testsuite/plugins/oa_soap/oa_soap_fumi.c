/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *      Raja Kumar Thatte <raja-kumar.thatte@hp.com>
 **/

#include "oa_soap_fumi.h"

/**
 * oa_soap_set_fumi_source
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *      @sourceuri:   Text buffer containing URI of the source
 *
 * Purpose:
 *      Set new source image URI information to the given bank
 *      of the given FUMI.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
SaErrorT oa_soap_set_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiTextBufferT *sourceuri)
{
        err("oa_soap_set_fumi_source not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_validate_fumi_source
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *
 * Purpose:
 *      Validate the integrity of the source image associated with
 *      the given bank.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_validate_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum)
{
        err("oa_soap_validate_fumi_source not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_fumi_source
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *      @sourceuri:   Source Image URI
 *
 * Purpose:
 *      Get the source image URI information assigned to the given bank.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_get_fumi_source(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiSourceInfoT *sourceinfo)
{
        err("oa_soap_get_fumi_source not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_fumi_target
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *      @bankinfo:    Current Image details of the give bank
 *
 * Purpose:
 *      Get current image information on the target.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_get_fumi_target(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiBankInfoT *bankinfo)
{
        err("oa_soap_get_fumi_target not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_fumi_backup
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *
 * Purpose:
 *     Take the backup of the currently active bank image.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_start_fumi_backup(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num)
{
        err("oa_soap_start_fumi_backup not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_set_fumi_bank_order
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *      @position:    bank position in boot order
 *
 * Purpose:
 *      Set the bank position in boot order .
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_set_fumi_bank_order(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiUint32T position)
{
        err("oa_soap_set_fumi_bank_order not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_fumi_bank_copy
 *      @oh_handler:        Handler data pointer
 *      @resource_id:       Resource ID
 *      @num:               FUMI number
 *      @sourcebanknum:     Bank number
 *      @targetbanknum:     Text buffer containing URI of the source
 *
 * Purpose:
 *      Copy the image from source bank to target bank.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_start_fumi_bank_copy(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT sourcebanknum,
                                SaHpiBankNumT targetbanknum)
{
        err("oa_soap_start_fumi_bank_copy not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_fumi_install
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *
 * Purpose:
 *      To install the image in the given bank.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_start_fumi_install(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum)
{
        err("oa_soap_start_fumi_install not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_get_fumi_status
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *      @status:      Image Upgrade status
 *
 * Purpose:
 *      To know the image upgrade progress status..
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_get_fumi_status(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum,
                                SaHpiFumiUpgradeStatusT *status)
{
        err("oa_soap_get_fumi_status not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_fumi_verify
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *
 * Purpose:
 *      To validate the upgraded image.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_start_fumi_verify(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum)
{
        err("oa_soap_start_fumi_verify not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_cancel_fumi_upgrade
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *      @banknum:     Bank number
 *
 * Purpose:
 *      To stop the image upgrade process.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_cancel_fumi_upgrade(void *oh_handler,

                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num,
                                SaHpiBankNumT banknum)
{
        err("oa_soap_cancel_fumi_upgrade not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_start_fumi_rollback
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *
 * Purpose:
 *      To rollback the image
 *      (Stop the image upgrade process and restore the backup image).
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_start_fumi_rollback(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num)
{
        err("oa_soap_start_fumi_rollback not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}

/**
 * oa_soap_activate_fumi
 *      @oh_handler:  Handler data pointer
 *      @resource_id: Resource ID
 *      @num:         FUMI number
 *
 * Purpose:
 *      To start the newly upgraded image.
 *
 * Detailed Description: NA
 *
 * Return values:
 *      SA_ERR_HPI_UNSUPPORTED_API - current oa_soap implementation does not
 *                                   support this API
 **/
 SaErrorT oa_soap_activate_fumi(void *oh_handler,
                                SaHpiResourceIdT resource_id,
                                SaHpiFumiNumT num)
{
        err("oa_soap_activate_fumi not implemented");
        return SA_ERR_HPI_UNSUPPORTED_API;
}



void * oh_set_fumi_source (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiTextBufferT *)
                __attribute__ ((weak, alias("oa_soap_set_fumi_source")));

void * oh_validate_fumi_source (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT)
                __attribute__ ((weak, alias("oa_soap_validate_fumi_source")));

void * oh_get_fumi_source (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiFumiSourceInfoT *)
                __attribute__ ((weak, alias("oa_soap_get_fumi_source")));

void * oh_get_fumi_target (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiFumiBankInfoT *)
                __attribute__ ((weak, alias("oa_soap_get_fumi_target")));

void * oh_start_fumi_backup (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT)
                __attribute__ ((weak, alias("oa_soap_start_fumi_backup")));

void * oh_set_fumi_bank_order (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiUint32T)
                __attribute__ ((weak, alias("oa_soap_set_fumi_bank_order")));

void * oh_start_fumi_bank_copy (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiBankNumT)
                __attribute__ ((weak, alias("oa_soap_start_fumi_bank_copy")));

void * oh_start_fumi_install (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT)
                __attribute__ ((weak, alias("oa_soap_start_fumi_install")));

void * oh_get_fumi_status (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT,
                                SaHpiFumiUpgradeStatusT *)
                __attribute__ ((weak, alias("oa_soap_get_fumi_status")));

void * oh_start_fumi_verify (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT)
                __attribute__ ((weak, alias("oa_soap_start_fumi_verify")));

void * oh_cancel_fumi_upgrade (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT,
                                SaHpiBankNumT)
                __attribute__ ((weak, alias("oa_soap_cancel_fumi_upgrade")));

void * oh_start_fumi_rollback (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT)
                __attribute__ ((weak, alias("oa_soap_start_fumi_rollback")));

void * oh_activate_fumi (void *,
                                SaHpiResourceIdT,
                                SaHpiFumiNumT)
                __attribute__ ((weak, alias("oa_soap_activate_fumi")));

