// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * Copyright (c) 2020, 2022 Linaro Limited
 */

 #include <config.h>
 #include <crypto/crypto.h>
 //#include <kernel/mutex.h>
 //#include <kernel/tee_misc.h>
 #include <kernel/tee_ta_manager.h>
 //#include <kernel/ts_manager.h>
 //#include <kernel/user_access.h>
 //#include <memtag.h>
 //#include <mm/vm.h>
 #include <string.h>
 #include <tee_api_defines_extensions.h>
 #include <tee_api_defines.h>
 #include <tee/tee_fs.h>
 #include <tee/tee_obj.h>
 #include <tee/tee_pobj.h>
 #include <tee/tee_svc_cryp.h>
 #include <tee/tee_svc.h>
 #include <tee/tee_svc_storage.h>
 #include <trace.h>


 
 /* Header of GP formated secure storage files */
 struct tee_svc_storage_head {
     uint32_t attr_size;
     uint32_t objectSize;
     uint32_t maxObjectSize;
     uint32_t objectUsage;
     uint32_t objectType;
     uint32_t have_attrs;
 };
 
 struct tee_storage_enum {
     TAILQ_ENTRY(tee_storage_enum) link;
     struct tee_fs_dir *dir;
     const struct tee_file_operations *fops;
 };
 


// Criar a cabeça da lista global
TAILQ_HEAD(tee_pobj_list, tee_pobj);
// Inicializar a lista global
struct tee_pobj_list global_pobj_list = TAILQ_HEAD_INITIALIZER(global_pobj_list);


TEE_Result utee_storage_obj_create(unsigned long storage_id, void *object_id,
    size_t object_id_len, unsigned long flags,
    unsigned long attr, void *data, size_t len,
    uint32_t *obj)
{
	TEE_Result res = TEE_SUCCESS;
	struct tee_pobj *po = NULL;

    struct tee_ta_session *sess;

    struct tee_file_operations *fops; //used to save datasss

    tee_ta_get_current_session(&sess); 

    //////////////////// following functions will substitute this commnent function
    //res = tee_pobj_get((void *)&sess->ctx->uuid, object_id, object_id_len, flags, TEE_POBJ_USAGE_CREATE, NULL, &po);
    ////////////////////

	/* new file */
	po = TEE_Malloc(sizeof(struct tee_pobj), TEE_MALLOC_FILL_ZERO);

	// o = calloc(1, sizeof(struct tee_pobj));
	if (!po) {
		res = TEE_ERROR_OUT_OF_MEMORY;
	}
	po->refcnt = 1;
	memcpy(&po->uuid, &sess->ctx->uuid, sizeof(TEE_UUID));
	po->flags = flags;
	po->fops = NULL;
	po->temporary = TEE_POBJ_USAGE_CREATE;

	po->obj_id = TEE_Malloc(object_id_len, TEE_MALLOC_FILL_ZERO);
	if (po->obj_id == NULL) {
		TEE_Free(po);
		res = TEE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(po->obj_id, object_id, object_id_len); //here
	po->obj_id_len = object_id_len;

    /* save data */
    po->data = TEE_Malloc(len, TEE_MALLOC_FILL_ZERO);
    if (!po->data) {
        TEE_Free(po->obj_id);
        TEE_Free(po);
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    memcpy(po->data, data, len);
    po->data_len = len;

    TAILQ_INSERT_HEAD(&global_pobj_list, po, link);

	res = TEE_SUCCESS;
    ////////////////////

   return res;
}

static TEE_Result tee_pobj_check_access(uint32_t oflags, uint32_t nflags)
{
	/* meta is exclusive */
	if ((oflags & TEE_DATA_FLAG_ACCESS_WRITE_META) ||
	    (nflags & TEE_DATA_FLAG_ACCESS_WRITE_META))
		return TEE_ERROR_ACCESS_CONFLICT;

	/*
	 * Excerpt of TEE Internal Core API Specification v1.1:
	 * If more than one handle is opened on the same  object, and if any
	 * of these object handles was opened with the flag
	 * TEE_DATA_FLAG_ACCESS_READ, then all the object handles MUST have been
	 * opened with the flag TEE_DATA_FLAG_SHARE_READ
	 */
	if (((oflags & TEE_DATA_FLAG_ACCESS_READ) ||
	     (nflags & TEE_DATA_FLAG_ACCESS_READ)) &&
	    !((nflags & TEE_DATA_FLAG_SHARE_READ) &&
	      (oflags & TEE_DATA_FLAG_SHARE_READ)))
		return TEE_ERROR_ACCESS_CONFLICT;

	/*
	 * Excerpt of TEE Internal Core API Specification v1.1:
	 * An object can be opened with only share flags, which locks the access
	 * to an object against a given mode.
	 * An object can be opened with no flag set, which completely locks all
	 * subsequent attempts to access the object
	 */
	if ((nflags & TEE_DATA_FLAG_SHARE_READ) !=
	    (oflags & TEE_DATA_FLAG_SHARE_READ))
		return TEE_ERROR_ACCESS_CONFLICT;

	/* Same on WRITE access */
	if (((oflags & TEE_DATA_FLAG_ACCESS_WRITE) ||
	     (nflags & TEE_DATA_FLAG_ACCESS_WRITE)) &&
	    !((nflags & TEE_DATA_FLAG_SHARE_WRITE) &&
	      (oflags & TEE_DATA_FLAG_SHARE_WRITE)))
		return TEE_ERROR_ACCESS_CONFLICT;
	if ((nflags & TEE_DATA_FLAG_SHARE_WRITE) !=
	    (oflags & TEE_DATA_FLAG_SHARE_WRITE))
		return TEE_ERROR_ACCESS_CONFLICT;

	return TEE_SUCCESS;
}


 TEE_Result utee_storage_obj_open(unsigned long storage_id, void *object_id,
                     size_t object_id_len, unsigned long flags,
                     uint32_t *obj)
{

	const unsigned long valid_flags = TEE_DATA_FLAG_ACCESS_READ |
					  TEE_DATA_FLAG_ACCESS_WRITE |
					  TEE_DATA_FLAG_ACCESS_WRITE_META |
					  TEE_DATA_FLAG_SHARE_READ |
					  TEE_DATA_FLAG_SHARE_WRITE;

	struct tee_ta_session *sess; tee_ta_get_current_session(&sess); 

    struct user_ta_ctx *utc = sess->ctx;
	
    TEE_Result res = TEE_SUCCESS;
	struct tee_pobj *po = NULL;
    struct tee_pobj *aux_po = NULL;
	struct tee_obj *o = NULL;

    enum tee_pobj_usage usage = TEE_POBJ_USAGE_OPEN;

	if (flags & ~valid_flags)
		return TEE_ERROR_BAD_PARAMETERS;


	if (object_id_len > TEE_OBJECT_ID_MAX_LEN) {
		res = TEE_ERROR_BAD_PARAMETERS;
		goto exit;
	}

    //////////////////// following functions will substitute this commnent function
    //res = tee_pobj_get((void *)&sess->ctx->uuid, object_id, object_id_len, flags, TEE_POBJ_USAGE_OPEN, NULL, &po);
    ////////////////////

	//CHECK IF EMPTY LIST
	if(global_pobj_list.tqh_first==NULL){
		res = TEE_SUCCESS;
		*obj = NULL;
        goto exit;
	}

    //	mutex_lock(&pobjs_mutex);
	/* Check if file is open */
	TAILQ_FOREACH(aux_po, &global_pobj_list, link) {
        if(object_id_len == aux_po->obj_id_len){
            if(memcmp(object_id, aux_po->obj_id, object_id_len) == 0){
                if(memcmp(&(sess->ctx->uuid), &(aux_po->uuid), sizeof(TEE_UUID)) == 0){
                    po = aux_po;
                }else{
                    res = TEE_SUCCESS;
					*obj = NULL;
                    goto exit;
                }
            }else{
				res = TEE_SUCCESS;
				*obj = NULL;
				goto exit;
			}
        }else{
			res = TEE_SUCCESS;
			*obj = NULL;
			goto exit;
		}
	}

	// if (po) {
	// 	if (usage == TEE_POBJ_USAGE_ENUM) {
	// 		(po)->refcnt++;
	// 		goto err;
	// 	}
	// 	if ((po)->creating || (usage == TEE_POBJ_USAGE_CREATE &&
	// 				 !(flags & TEE_DATA_FLAG_OVERWRITE))) {
	// 		res = TEE_ERROR_ACCESS_CONFLICT;
	// 		goto err;
	// 	}
	// 	res = tee_pobj_check_access((po)->flags, flags);
	// 	if (res == TEE_SUCCESS)
	// 		(po)->refcnt++;
	// 	goto err;
	// }

    ////////////////////

	o = tee_obj_alloc();
	if (o == NULL) {
		tee_pobj_release(po);
		res = TEE_ERROR_OUT_OF_MEMORY;
		goto err;
	}

	o->info.handleFlags = TEE_HANDLE_FLAG_PERSISTENT |
			      TEE_HANDLE_FLAG_INITIALIZED | flags;
	o->pobj = po;

	// res = tee_svc_storage_read_head(o);
	// if (res != TEE_SUCCESS) {
	// 	if (res == TEE_ERROR_CORRUPT_OBJECT) {
	// 		EMSG("Object corrupt");
	// 		goto err;
	// 	}
	// 	goto oclose;
	// }

    o->pobj->data = po->data;
    o->pobj->data_len = po->data_len;

    *obj = o;

	//tee_obj_add(utc, o); //need this to implement close 
	// res = copy_kaddr_to_uref(obj, o);
	// if (res != TEE_SUCCESS)
	// 	goto oclose;

	goto exit;

oclose:
	tee_obj_close(utc, o);
	o = NULL;

err:
	if (res == TEE_ERROR_NO_DATA || res == TEE_ERROR_BAD_FORMAT)
		res = TEE_ERROR_CORRUPT_OBJECT;
	if (res == TEE_ERROR_CORRUPT_OBJECT && o)
		remove_corrupt_obj(utc, o);

exit:
	return res;

}


TEE_Result utee_storage_obj_del(unsigned long obj)
{
	struct tee_ta_session *sess; tee_ta_get_current_session(&sess); 
    struct user_ta_ctx *utc = sess->ctx;
	TEE_Result res = TEE_SUCCESS;

	struct tee_obj *o = NULL;

	//res = tee_obj_get(utc, uref_to_vaddr(obj), &o);

	o = obj;

	if (!(o->info.handleFlags & TEE_DATA_FLAG_ACCESS_WRITE_META))
		return TEE_ERROR_ACCESS_CONFLICT;

	if (o->pobj == NULL || o->pobj->obj_id == NULL)
		return TEE_ERROR_BAD_STATE;


	//res = o->pobj->fops->remove(o->pobj);
	TAILQ_REMOVE(&global_pobj_list, o->pobj, link);

	//tee_obj_close(utc, o);

	//if ((o->info.handleFlags & TEE_HANDLE_FLAG_PERSISTENT)) {
		//o->pobj->fops->close(&o->fh);
		tee_pobj_release(o->pobj);
	//}

	tee_obj_free(o);

	return res;
}


TEE_Result utee_storage_obj_read(unsigned long obj, void *data, size_t len,
	uint64_t *count)
{
	//struct tee_ta_session *sess; tee_ta_get_current_session(&sess); 
    //struct user_ta_ctx *utc = sess->ctx;
	TEE_Result res = TEE_SUCCESS;
	struct tee_obj *o = NULL;
	uint64_t u_count = 0;
	size_t pos_tmp = 0;
	//size_t bytes = 0;

	// res = tee_obj_get(utc, obj, &o);
	// if (res != TEE_SUCCESS)
	// goto exit;

	//joao this need to be implemented
	// if (!(o->info.handleFlags & TEE_HANDLE_FLAG_PERSISTENT)) {
	// res = TEE_ERROR_BAD_STATE;
	// goto exit;
	// }

	// if (!(o->info.handleFlags & TEE_DATA_FLAG_ACCESS_READ)) {
	// res = TEE_ERROR_ACCESS_CONFLICT;
	// goto exit;
	// }

	//joao to be implemented
	/* Guard o->info.dataPosition += bytes below from overflowing */
	// if (ADD_OVERFLOW(o->info.dataPosition, len, &pos_tmp)) {
	// res = TEE_ERROR_OVERFLOW;
	// goto exit;
	// }

	//joao maybe memory translation of location of data to physica address
	//data = memtag_strip_tag(data);
	// /* check rights of the provided buffer */
	// res = vm_check_access_rights(utc->uctx, TEE_MEMORY_ACCESS_WRITE,
	// 	(uaddr_t)data, len);
	// if (res != TEE_SUCCESS)
	// goto exit;

	//bytes = len;
	//joao this should be implemented in near future
	// if (ADD_OVERFLOW(o->ds_pos, o->info.dataPosition, &pos_tmp)) {
	// res = TEE_ERROR_OVERFLOW;
	// goto exit;
	// }
	

	//joao here wants to collect from structure object->pobj the amount of bytes on data
	//res = o->pobj->fops->read(o->fh, pos_tmp, data, &bytes);
	o = ( struct tee_obj *)obj;

	memcpy(data, o->pobj->data, len);

	// if (res != TEE_SUCCESS) {
	// if (res == TEE_ERROR_CORRUPT_OBJECT) {
	// EMSG("Object corrupt");
	// remove_corrupt_obj(utc, o);
	// }
	// goto exit;
	// }

	o->info.dataPosition += len;

	//joao why do we need this?
	//u_count = bytes;
	//res = copy_to_user_private(count, &u_count, sizeof(*count));
	exit:
	return res;
}