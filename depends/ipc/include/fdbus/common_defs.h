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

#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

#include <stdint.h>

#define FDB_VERSION_MAJOR 5
#define FDB_VERSION_MINOR 4
#define FDB_VERSION_BUILD 0

#define FDB_INVALID_ID (~0)
typedef uint16_t FdbEndpointId_t;
typedef int32_t FdbSessionId_t;
typedef int32_t FdbSocketId_t;
typedef int32_t FdbMsgCode_t;
typedef uint32_t FdbMsgSn_t;
typedef FdbMsgCode_t FdbEventCode_t;
typedef uint32_t FdbObjectId_t;
typedef uint8_t FdbEventGroup_t;
typedef uint16_t FdbContextId_t;
typedef int32_t FdbInstanceId_t;
typedef uint8_t FdbDomainId_t;

#define FDB_NAME_SERVER_NAME            "org.fdbus.name-server"
#define FDB_HOST_SERVER_NAME            "org.fdbus.host-server"
#define FDB_LOG_SERVER_NAME             "org.fdbus.log-server"
#define FDB_NOTIFICATION_CENTER_NAME    "org.fdbus.notification-center"
#define FDB_XTEST_NAME                  "org.fdbus.xtest-server"
#define FDB_DATAPOOL_SERVER_NAME        "org.fdbus.datapool"

#define FDB_URL_TCP_IND "tcp"
#define FDB_URL_IPC_IND "ipc"
#define FDB_URL_SVC_IND "svc"
#define FDB_URL_UDP_IND "udp"

#define FDB_URL_TCPS_IND "tcps"
#define FDB_URL_UDPS_IND "udps"

#define FDB_URL_TCP FDB_URL_TCP_IND "://"
#define FDB_URL_IPC FDB_URL_IPC_IND "://"
#define FDB_URL_SVC FDB_URL_SVC_IND "://"
#define FDB_URL_UDP FDB_URL_UDP_IND "://"
#define FDB_URL_TCPS FDB_URL_TCPS_IND "://"
#define FDB_URL_UDPS FDB_URL_UDPS_IND "://"

#define FDB_IP_ALL_INTERFACE "0"

#define FDB_OBJECT_MAIN 0
#define FDB_OBJECT_SN_SHIFT 16
#define FDB_OBJECT_CLASS_MASK 0xFFFF
#define FDB_OBJECT_MAKE_ID(_sn, _class) (((_sn) << FDB_OBJECT_SN_SHIFT) | \
                                        ((_class) & FDB_OBJECT_CLASS_MASK))
#define FDB_OBJECT_GET_CLASS(_id) ((_id) & FDB_OBJECT_CLASS_MASK)
#define FDB_CUSTOM_OBJECT_BEGIN (FDB_OBJECT_MAIN + 1)

#define FDB_LOCAL_HOST "127.0.0.1"

#define FDB_INET_PORT_INVALID   -2
#define FDB_INET_PORT_NOBIND    -1
#define FDB_INET_PORT_AUTO      0
#define FDB_VALID_PORT(_port)   ((_port) > FDB_INET_PORT_AUTO)

#define FDB_SECURITY_LEVEL_NONE     -1
#define FDB_SECURITY_LEVEL_UNKNOWN  -2

#define FDB_UDP_PORT            60000

typedef uint32_t tFdbIpV4;

#ifdef __cplusplus
namespace ipc {
namespace fdbus {
template <typename T>
bool fdbValidFdbId(T id)
{
    return id != (T)FDB_INVALID_ID;
}
template<typename To, typename From>
inline To fdb_dynamic_cast_if_available(From from) {
#if defined(CONFIG_FDB_NO_RTTI) || !defined(__cpp_rtti)
  return static_cast<To>(from);
#else
  return dynamic_cast<To>(from);
#endif

#define fdb_remove_value_from_container(_container, _value) do { \
    auto it = std::find((_container).begin(), (_container).end(), _value); \
    if (it != (_container).end()) \
    { \
        (_container).erase(it); \
    } \
} while (0)

}
}
}
#endif

#ifndef Fdb_Num_Elems
#define Fdb_Num_Elems(_arr_) ((int32_t) (sizeof(_arr_) / sizeof((_arr_)[0])))
#endif

#if !defined(FDB_CFG_CONFIG_PATH)
#define FDB_CFG_CONFIG_PATH "/etc/fdbus"
#endif

#define FDB_CFG_CONFIG_FILE_SUFFIX ".fdb"

#if !defined(FDB_CFG_NR_SECURE_LEVEL)
#define FDB_CFG_NR_SECURE_LEVEL 4
#endif

#if !defined(FDB_CFG_TOKEN_LENGTH)
#define FDB_CFG_TOKEN_LENGTH 32
#endif

enum EFdbLogLevel
{
    FDB_LL_VERBOSE = 0,
    FDB_LL_DEBUG = 1,
    FDB_LL_INFO = 2,
    FDB_LL_WARNING = 3,
    FDB_LL_ERROR = 4,
    FDB_LL_FATAL = 5,
    FDB_LL_SILENT = 6,
    FDB_LL_MAX = 7
};

// hint for QOS of transportation and which QOS is used depends on real
// connection. For example, if two endpoints are located in the same
// node, UDS is used without regard to QOS specified.
enum EFdbQOS
{
    // local connection like UDS; reliable; secure
    // but cannot go across system boundary
    FDB_QOS_LOCAL = 0,

    // reliable connection like TCP, UDS; insecure
    FDB_QOS_RELIABLE = 1,

    // unreliable connection like UDP; insecure
    FDB_QOS_BEST_EFFORTS = 2,

    // reliable connection like TCP, UDS;
    // security (authentication, crypto...) is enabled
    FDB_QOS_SECURE_RELIABLE = 3,

    // unreliable connection like UDP;
    // security (authentication, crypto...) is enabled
    // (Not supported yet)
    FDB_QOS_SECURE_BEST_EFFORTS = 4,

    // reliable connection like TCP, UDS;
    // try security (authentication, crypto...) first;
    // if not available, fallback to insecure
    FDB_QOS_TRY_SECURE_RELIABLE = 5,

    // unreliable connection like UDP;
    // try security (authentication, crypto...) first;
    // if not available, fallback to insecure
    FDB_QOS_TRY_SECURE_BEST_EFFORTS = 6,

    FDB_QOS_INVALID,

    // default QOS
    FDB_QOS_DEFAULT = FDB_QOS_TRY_SECURE_RELIABLE,
};

#define FDB_EVENT_GROUP_SHIFT 24
#define FDB_EVENT_GROUP_BITS 0xFF
#define FDB_EVENT_ALL_GROUPS FDB_EVENT_GROUP_BITS
#define FDB_DEFAULT_GROUP 0
#define FDB_EVENT_GROUP_MASK (FDB_EVENT_GROUP_BITS << FDB_EVENT_GROUP_SHIFT)
#define FDB_EVENT_ID_MASK (~FDB_EVENT_GROUP_MASK)
#define fdbMakeGroup(_event) ((FdbMsgCode_t)((_event) | FDB_EVENT_ID_MASK))

#define fdbMakeEventCode(_group, _event) ((FdbMsgCode_t)(((((uint32_t)(_group) & FDB_EVENT_GROUP_BITS) << FDB_EVENT_GROUP_SHIFT) | \
                                         ((uint32_t)(_event) & FDB_EVENT_ID_MASK))))
#define fdbMakeGroupEvent(_group) fdbMakeEventCode(_group, FDB_EVENT_ID_MASK)
#define fdbMergeEventCode(_group, _event) (((uint32_t)(_group) & FDB_EVENT_GROUP_MASK) | (uint32_t)(_event))

#define fdbIsGroup(_event) (((_event) & FDB_EVENT_ID_MASK) == FDB_EVENT_ID_MASK)
#define fdbEventGroup(_event) ((FdbEventGroup_t)((((uint32_t)(_event)) >> FDB_EVENT_GROUP_SHIFT) & FDB_EVENT_GROUP_BITS))
#define fdbEventCode(_event) ((FdbMsgCode_t)((_event) & FDB_EVENT_ID_MASK))
#define fdbEventInGroup(_event, _group) (((uint32_t)(_event) & FDB_EVENT_GROUP_MASK) == \
                                         (((uint32_t)(_group) & FDB_EVENT_GROUP_BITS) << FDB_EVENT_GROUP_SHIFT))
#define fdbSameGroup(_event1, _event2) (((uint32_t)(_event1) & FDB_EVENT_GROUP_MASK) == ((uint32_t)(_event2) & FDB_EVENT_GROUP_MASK))

#define FDB_DEFAULT_INSTANCE    0
#define FDB_AUTO_INSTANCE_BEGIN 0x00ff0000
#define FDB_AUTO_INSTANCE_END   (FDB_EVENT_ID_MASK - 1)
#define FDB_ANY_INSTANCE        "x"
#define FDB_ANY_INSTANCE_CHAR   'x'

#define FDB_WATCHDOG_INTERVAL   2000
#define FDB_WATCHDOG_RETRIES    5

#define FDB_ADDRESS_CONNECT_RETRY_NR    3
#define FDB_ADDRESS_CONNECT_RETRY_INTERVAL 50

#define FDB_ADDRESS_BIND_RETRY_NR    3
#define FDB_ADDRESS_BIND_RETRY_INTERVAL 50

#define FDB_HOUSEKEEPING_INTRVAL    4000
#define FDB_MAX_MSG_LIFETIME        5

/* For C that doesn't define 'boolean' type */
#define fdb_false 0
#define fdb_true (!fdb_false)

#define FDB_QUERY_MAX_TIMEOUT       12000

typedef uint8_t fdb_bool_t;

#define FDB_EXPORTABLE_NODE_INTERNAL    0 // visible inside node (not exportable)
#define FDB_EXPORTABLE_DOMAIN           1 // visible inside the same domain
#define FDB_EXPORTABLE_SITE             9 // visible insied a site
#define FDB_EXPORTABLE_CUSTOM_BEGIN     10 // from here exportable level can be customized
#define FDB_EXPORTABLE_ANY              INT32_MAX

#define FDB_DEFAULT_DOMAIN              0

enum EFdbSecureType
{
    FDB_SEC_INSECURE,
    FDB_SEC_SECURE,
    FDB_SEC_TRY_SECURE,
    FDB_SEC_NO_CHECK
};

#endif
