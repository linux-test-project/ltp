/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004, 2006
 * Copyright (c) 2003 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      David Judkovics <djudkovi@us.ibm.com>
 *      Renier Morales <renier@openhpi.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <config.h>
#include <oh_utils.h>
#include <oh_error.h>

#ifdef OH_DBG_MSGS
#define dbg_uid_lock(format, ...) \
        do { \
                if (getenv("OPENHPI_DBG_UID_LOCK") && !strcmp("YES",getenv("OPENHPI_DBG_UID_LOCK"))){ \
                        fprintf(stderr, "        UID_LOCK: %s:%d:%s: ", __FILE__, __LINE__, __func__); \
                        fprintf(stderr, format "\n", ## __VA_ARGS__); \
                } \
        } while(0)
#else
#define dbg_uid_lock(format, ...)
#endif

#define uid_lock(uidmutex) \
        do { \
                dbg_uid_lock("Locking UID mutex..."); \
                g_static_mutex_lock(uidmutex); \
                dbg_uid_lock("OK. UID mutex locked."); \
        } while (0)

#define uid_unlock(uidmutex) \
        do { \
                dbg_uid_lock("Unlocking UID mutex..."); \
                g_static_mutex_unlock(uidmutex); \
                dbg_uid_lock("OK. UID mutex unlocked."); \
        } while (0)


/* uid to entity path cross reference (xref) data structure */
typedef struct {
        SaHpiResourceIdT resource_id;
        SaHpiEntityPathT entity_path;
} EP_XREF;

static GStaticMutex oh_uid_lock = G_STATIC_MUTEX_INIT;
static GHashTable *oh_ep_table;
static GHashTable *oh_resource_id_table;
static guint       resource_id;
static int initialized = FALSE;


/* use to build memory resident map table from file */
static int uid_map_from_file(void);
static int build_uid_map_data(int file);

/* used by oh_uid_remove() */
static void write_ep_xref(gpointer key, gpointer value, gpointer file);

/* for hash table usage */
guint oh_entity_path_hash(gconstpointer key);
gboolean oh_entity_path_equal(gconstpointer a, gconstpointer b);

/*
 * oh_entity_path_hash: used by g_hash_table_new()
 * in oh_uid_initialize(). See glib library for
 * further details.
 */
guint oh_entity_path_hash(gconstpointer key)
{
        const char *p = key;
        guint h = *p;

        int i;

        int entity_path_len;

        entity_path_len = sizeof(SaHpiEntityPathT);

        p += 1;

        for( i=0; i<entity_path_len - 1; i++ ){
/*              h = (h << 5) - h + *p; */
                h = (h * 131) + *p;
                p++;
        }

        /* don't change the 1009, its magic */
        return( h % 1009 );

}

/*
 * oh_entity_path_equal: used by g_hash_table_new()
 * in oh_uid_initialize(). See glib library for
 * further details.
 */
gboolean oh_entity_path_equal(gconstpointer a, gconstpointer b)
{
        if (oh_cmp_ep(a,b)) {
                return 1;
        }
        else {
                return 0;
        }
}

/**
 * oh_uid_initialize
 *
 * UID utils initialization routine
 * This functions must be called before any other uid_utils
 * are made.
 *
 * Returns: success 0, failure -1.
 **/
SaErrorT oh_uid_initialize(void)
{
        SaErrorT rval = SA_OK;

        uid_lock(&oh_uid_lock);
        if (!initialized) {
                /* initialize hash tables */
                oh_ep_table = g_hash_table_new(oh_entity_path_hash, oh_entity_path_equal);
                oh_resource_id_table = g_hash_table_new(g_int_hash, g_int_equal);
                initialized = TRUE;
                resource_id = 1;

                /* initialize uid map */
                rval = uid_map_from_file();
        }
        uid_unlock(&oh_uid_lock);

        return rval;
}

SaHpiBoolT oh_uid_is_initialized()
{
        if (initialized)
                return SAHPI_TRUE;
        else
                return SAHPI_FALSE;
}

/**
 * oh_uid_from_entity_path
 * @ep: value to be removed from used
 *
 * This function returns an unique value to be used as
 * an uid/resourceID base upon a unique entity path specified
 * by @ep.  If the entity path already exists, the already assigned
 * resource id is returned.  Before returning, this call updates the
 * uid map file saved on disk.
 *
 * Returns: positive unsigned int, failure is 0.
 **/
SaHpiUint32T oh_uid_from_entity_path(SaHpiEntityPathT *ep)
{
        gpointer key;
        gpointer value;
        SaHpiResourceIdT ruid;

        EP_XREF *ep_xref;

        char *uid_map_file;
        int file;

        SaHpiEntityPathT entitypath;

        if (!ep) return 0;
        if (!oh_uid_is_initialized()) return 0;

        oh_init_ep(&entitypath);
        oh_concat_ep(&entitypath,ep);
        key = &entitypath;

        uid_lock(&oh_uid_lock);
        /* check for presence of EP and */
        /* previously assigned uid      */
        ep_xref = (EP_XREF *)g_hash_table_lookup (oh_ep_table, key);
        if (ep_xref) {
                /*dbg("Entity Path already assigned uid. Use oh_uid_lookup().");*/
                uid_unlock(&oh_uid_lock);
                return ep_xref->resource_id;
        }

        /* allocate storage for EP cross reference data structure*/
        ep_xref = (EP_XREF *)g_malloc0(sizeof(EP_XREF));
        if(!ep_xref) {
                err("malloc failed");
                uid_unlock(&oh_uid_lock);
                return 0;
        }

        memset(ep_xref, 0, sizeof(EP_XREF));
        memcpy(&ep_xref->entity_path, &entitypath, sizeof(SaHpiEntityPathT));

        ep_xref->resource_id = resource_id;
        resource_id++;
        ruid = ep_xref->resource_id;

        value = (gpointer)ep_xref;

        /* entity path based key */
        key = (gpointer)&ep_xref->entity_path;
        g_hash_table_insert(oh_ep_table, key, value);

        /* resource id based key */
        key = (gpointer)&ep_xref->resource_id;
        g_hash_table_insert(oh_resource_id_table, key, value);

        /* save newly created ep xref (iud/resource_id) to map file */
        uid_map_file = (char *)getenv("OPENHPI_UID_MAP");
        if (uid_map_file == NULL) {
                uid_map_file = OH_DEFAULT_UID_MAP;
        }

        file = open(uid_map_file, O_WRONLY);
        if (file >= 0) {
                lseek(file, 0, SEEK_END);
                if (write(file,ep_xref, sizeof(EP_XREF)) != sizeof(EP_XREF)) {
			err("write ep_xref failed");
			close(file);
			return 0;
		}
                lseek(file, 0, SEEK_SET);
                if (write(file, &resource_id, sizeof(resource_id)) != sizeof(resource_id)) {
			err("write resource_id failed");
			close(file);
			return 0;
		}
        }
        close(file);

        uid_unlock(&oh_uid_lock);

        return ruid;
}

/**
 * oh_uid_remove
 * @uid: value to be removed
 *
 * This functions removes the uid/entity path
 * pair from use and removes the use of the uid forever.
 * A new uid may be requested for this entity path
 * in the future. oh_uid_from_entity_path() writes
 * the entire uid/entity path pairings to file before
 * returning. oh_uid_remove() deletes the pairing from file.
 *
 * Returns: success 0, failure -1.
 **/
SaErrorT oh_uid_remove(SaHpiUint32T uid)
{
        EP_XREF *ep_xref;
        gpointer key;

        if (!oh_uid_is_initialized()) return SA_ERR_HPI_ERROR;

        /* check entry exist in oh_resource_id_table */
        key = (gpointer)&uid;
        uid_lock(&oh_uid_lock);
        ep_xref = (EP_XREF *)g_hash_table_lookup (oh_resource_id_table, key);
        if(!ep_xref) {
                err("error freeing oh_resource_id_table");
                uid_unlock(&oh_uid_lock);
                return SA_ERR_HPI_ERROR;
        }

        /* check netry exist in oh_resource_id_table */
        key = (gpointer)&ep_xref->entity_path;
        ep_xref = (EP_XREF *)g_hash_table_lookup (oh_ep_table, key);
        if(!ep_xref) {
                err("error freeing oh_resource_id_table");
                uid_unlock(&oh_uid_lock);
                return SA_ERR_HPI_ERROR;
        }

        g_hash_table_remove(oh_resource_id_table, &ep_xref->resource_id);
        g_hash_table_remove(oh_ep_table, &ep_xref->entity_path);

        free(ep_xref);

        uid_unlock(&oh_uid_lock);

        return oh_uid_map_to_file();
}

/**
 * oh_uid_lookup
 * @ep: pointer to entity path used to identify resourceID/uid
 *
 * Fetches resourceID/uid based on entity path in @ep.
 *
 * Returns: success returns resourceID/uid, failure is 0.
 **/
SaHpiUint32T oh_uid_lookup(SaHpiEntityPathT *ep)
{
        EP_XREF *ep_xref;
        SaHpiEntityPathT entitypath;
        SaHpiResourceIdT ruid;
        gpointer key;

        if (!ep) return 0;
        if (!oh_uid_is_initialized()) return 0;

        oh_init_ep(&entitypath);
        oh_concat_ep(&entitypath, ep);
        key = &entitypath;

        /* check hash table for entry in oh_ep_table */
        uid_lock(&oh_uid_lock);
        ep_xref = (EP_XREF *)g_hash_table_lookup (oh_ep_table, key);
        if(!ep_xref) {
                err("error looking up EP to get uid");
                uid_unlock(&oh_uid_lock);
                return 0;
        }

        ruid = ep_xref->resource_id;
        uid_unlock(&oh_uid_lock);

        return ruid;
}

/**
 * oh_entity_path_lookup
 * @id: resource_id/uid identifying entity path
 * @ep: pointer to memory to fill in with entity path
 *
 * Fetches entity path based upon resource id, @id.
 *
 * Returns: success 0, failed -1.
 **/
SaErrorT oh_entity_path_lookup(SaHpiUint32T id, SaHpiEntityPathT *ep)
{
        EP_XREF *ep_xref;
        gpointer key = &id;

        if (!id || !ep) return SA_ERR_HPI_ERROR;
        if (!oh_uid_is_initialized()) return SA_ERR_HPI_ERROR;

        /* check hash table for entry in oh_ep_table */
        uid_lock(&oh_uid_lock);
        ep_xref = (EP_XREF *)g_hash_table_lookup (oh_resource_id_table, key);
        if(!ep_xref) {
                err("error looking up EP to get uid");
                uid_unlock(&oh_uid_lock);
                return SA_ERR_HPI_ERROR ;
        }

        memcpy(ep, &ep_xref->entity_path, sizeof(SaHpiEntityPathT));

        uid_unlock(&oh_uid_lock);

        return SA_OK;
}

/**
 * oh_uid_map_to_file: saves current uid and entity path mappings
 * to file, first element in file is 4 bytes for resource id,
 * then repeat EP_XREF structures holding uid and entity path pairings
 *
 * Return value: success 0, failed -1.
 **/
SaErrorT oh_uid_map_to_file(void)
{
        char *uid_map_file;
        int file;

        uid_map_file = (char *)getenv("OPENHPI_UID_MAP");

        if (uid_map_file == NULL) {
                uid_map_file = OH_DEFAULT_UID_MAP;
        }

        uid_lock(&oh_uid_lock);

        file = open(uid_map_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
        if(file < 0) {
                err("Configuration file '%s' could not be opened", uid_map_file);
                uid_unlock(&oh_uid_lock);
                return SA_ERR_HPI_ERROR;
        }

        /* write resource id */
        if (write(file, (void *)&resource_id, sizeof(resource_id)) != sizeof(resource_id)) {
		err("write resource_id failed");
		close(file);
		return SA_ERR_HPI_ERROR;
	}

        /* write all EP_XREF data records */
        g_hash_table_foreach(oh_resource_id_table, write_ep_xref, &file);

        if(close(file) != 0) {
                err("Couldn't close file '%s'.", uid_map_file);
                uid_unlock(&oh_uid_lock);
                return SA_ERR_HPI_ERROR;
        }

        uid_unlock(&oh_uid_lock);

        return SA_OK;
}


/*
 * write_ep_xref: called by g_hash_table_foreach(), for each
 * hash table entry see glib manual for further details
 *
 * Return value: None (void).
 */
static void write_ep_xref(gpointer key, gpointer value, gpointer file)
{
        if (write(*(int *)file, value, sizeof(EP_XREF)) != sizeof(EP_XREF)) {
		err("write EP_XREF failed");
	}
}


/*
 * uid_map_from_file: called from oh_uid_initialize() during intialization
 * This function, if a uid map file exists, reads the current value for
 * uid and intializes the memory resident uid map file from file.
 *
 * Return value: success 0, error -1.
 */
static gint uid_map_from_file()
{
        char *uid_map_file;
        int file;
        int rval;

         /* initialize uid map file */
         uid_map_file = (char *)getenv("OPENHPI_UID_MAP");
         if (uid_map_file == NULL) {
                 uid_map_file = OH_DEFAULT_UID_MAP;
         }
         file = open(uid_map_file, O_RDONLY);
         if(file < 0) {
                 /* create map file with resource id initial value */
                 err("Configuration file '%s' does not exist, initializing", uid_map_file);
                 file = open(uid_map_file,
                             O_RDWR | O_CREAT | O_TRUNC,
                             S_IRUSR | S_IWUSR | S_IRGRP);
                 if(file < 0) {
                         err("Could not initialize uid map file, %s", uid_map_file );
                                         return -1;
                 }
                 /* write initial uid value */
                 if( write(file,(void *)&resource_id, sizeof(resource_id)) < 0 ) {
                         err("failed to write uid, on uid map file initialization");
                         close(file);
                         return -1;
                 }
                 if(close(file) != 0) {
                         err("Couldn't close file '%s'.during uid map file initialization", uid_map_file);
                         return -1;
                 }
                 /* return from successful initialization, from newly created uid map file */
                 return 0;
         }

         /* read uid/resouce_id highest count from uid map file */
         if (read(file,&resource_id, sizeof(resource_id)) != sizeof(resource_id)) {
                 err("error setting uid from existing uid map file");
                 return -1;
         }

         rval = build_uid_map_data(file);
         close(file);

         if (rval < 0)
                return -1;

         /* return from successful initialization from existing uid map file */
         return 0;
}

/*
 * build_uid_map_data: used by uid_map_from_file(),  recursively
 * reads map file and builds two hash tables and EP_XREF data
 * structures
 *
 * @file: key into a GHashTable
 *
 * Return value: success 0, error -1.
 */
static gint build_uid_map_data(int file)
{
        int rval;
        EP_XREF *ep_xref;
        EP_XREF ep_xref1;
        gpointer value;
        gpointer key;

        rval = read(file, &ep_xref1, sizeof(EP_XREF));

        while ( (rval != EOF) && (rval == sizeof(EP_XREF)) ) {

                /* copy read record from ep_xref1 to malloc'd ep_xref */
                ep_xref = (EP_XREF *)g_malloc0(sizeof(EP_XREF));
                if (!ep_xref)
                        return -1;
                memcpy(ep_xref, &ep_xref1, sizeof(EP_XREF));

                value = (gpointer)ep_xref;

                /* entity path based key */
                key = (gpointer)&ep_xref->entity_path;
                g_hash_table_insert(oh_ep_table, key, value);

                /* resource id based key */
                key = (gpointer)&ep_xref->resource_id;
                g_hash_table_insert(oh_resource_id_table, key, value);

                rval = read(file, &ep_xref1, sizeof(EP_XREF));
        }

        /* TODO thought EOF would return -1, its not so check other way */
        /* if (rval != EOF), rval of 0 seems to be EOF */
        if ( (rval > 0)  && (rval < sizeof(EP_XREF)) ) {
                err("error building ep xref from map file");
                return -1;
        }
        return 0;
}
