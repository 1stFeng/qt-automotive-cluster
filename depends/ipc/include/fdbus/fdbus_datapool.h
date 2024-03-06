/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "fdbus_c_common.h"

#ifndef __FDBUS_DATAPOOL_H__
#define __FDBUS_DATAPOOL_H__

#ifdef __cplusplus
extern "C"
{
#endif
struct fdb_datapool_tag;
typedef void (*fdb_data_request_fn_t)(struct fdb_datapool_tag *self, fdb_message_t *msg);
typedef void (*fdb_data_publish_notify_fn_t)(struct fdb_datapool_tag *self, fdb_message_t *msg);

typedef struct fdb_datapool_tag
{
    void *native_handle;
    void *user_data;
}fdb_datapool_t;

LIB_EXPORT
fdb_datapool_t *fdb_dp_create(FdbDomainId_t domain_id, const char *pool_name, void *user_pool);
LIB_EXPORT
void fdb_dp_destroy(fdb_datapool_t *handle);
LIB_EXPORT
fdb_bool_t fdb_dp_start(fdb_datapool_t *handle,
                        fdb_bool_t is_topic_owner,
                        fdb_bool_t is_topic_borrower);
LIB_EXPORT
FdbDomainId_t fdb_dp_get_domain(fdb_datapool_t *handle);

LIB_EXPORT
fdb_bool_t fdb_dp_create_data_ic(fdb_datapool_t *handle,
                                 FdbMsgCode_t topic_id,
                                 const char *topic,
                                 fdb_data_request_fn_t on_data_request_fn,
                                 const uint8_t *init_data,
                                 int32_t size);
LIB_EXPORT
fdb_bool_t fdb_dp_destroy_data_ic(fdb_datapool_t *handle,
                                  FdbMsgCode_t topic_id,
                                  const char *topic);
LIB_EXPORT
fdb_bool_t fdb_dp_publish_data_ic(fdb_datapool_t *handle,
                                  FdbMsgCode_t topic_id,
                                  const char *topic,
                                  const uint8_t *data,
                                  int32_t size,
                                  fdb_bool_t force_update,
                                  enum EFdbQOS qos);
LIB_EXPORT
fdb_bool_t fdb_dp_subscribe_data_ic(fdb_datapool_t *handle,
                                    FdbMsgCode_t topic_id,
                                    const char *topic,
                                    fdb_data_publish_notify_fn_t on_data_publish_fn);

LIB_EXPORT
fdb_bool_t fdb_dp_create_data_i(fdb_datapool_t *handle,
                                FdbMsgCode_t topic_id,
                                fdb_data_request_fn_t on_data_request_fn,
                                const uint8_t *init_data,
                                int32_t size);
LIB_EXPORT
fdb_bool_t fdb_dp_destroy_data_i(fdb_datapool_t *handle, FdbMsgCode_t topic_id);
LIB_EXPORT
fdb_bool_t fdb_dp_publish_data_i(fdb_datapool_t *handle,
                                 FdbMsgCode_t topic_id,
                                 const uint8_t *data,
                                 int32_t size,
                                 fdb_bool_t force_update,
                                 enum EFdbQOS qos);
LIB_EXPORT
fdb_bool_t fdb_dp_subscribe_data_i(fdb_datapool_t *handle,
                                   FdbMsgCode_t topic_id,
                                   fdb_data_publish_notify_fn_t on_data_publish_fn);

LIB_EXPORT
fdb_bool_t fdb_dp_create_data_c(fdb_datapool_t *handle,
                                const char *topic,
                                fdb_data_request_fn_t on_data_request_fn,
                                const uint8_t *init_data,
                                int32_t size);
LIB_EXPORT
fdb_bool_t fdb_dp_destroy_data_c(fdb_datapool_t *handle, const char *topic);
LIB_EXPORT
fdb_bool_t fdb_dp_publish_data_c(fdb_datapool_t *handle,
                                 const char *topic,
                                 const uint8_t *data,
                                 int32_t size,
                                 fdb_bool_t force_update,
                                 enum EFdbQOS qos);
LIB_EXPORT
fdb_bool_t fdb_dp_subscribe_data_c(fdb_datapool_t *handle,
                                   const char *topic,
                                   fdb_data_publish_notify_fn_t on_data_publish_fn);

LIB_EXPORT
fdb_bool_t fdb_dp_create_data(fdb_datapool_t *handle,
                              fdb_data_request_fn_t on_data_request_fn);
LIB_EXPORT
fdb_bool_t fdb_dp_subscribe_data(fdb_datapool_t *handle,
                                 fdb_data_publish_notify_fn_t on_data_publish_fn);
#ifdef __cplusplus
}
#endif
#endif
