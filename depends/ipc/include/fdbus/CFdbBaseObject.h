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

#ifndef _CFDBBASEOBJECT_H_
#define _CFDBBASEOBJECT_H_

#include <map>
#include <set>
#include <functional>
#include "CEventSubscribeHandle.h"
#include "CFdbMsgDispatcher.h"
#include "CMethodJob.h"
#include "CFdbMsgSubscribe.h"

namespace ipc {
namespace fdbus {
enum EFdbEndpointRole
{
    FDB_OBJECT_ROLE_SERVER,
    FDB_OBJECT_ROLE_NS_SERVER,
    FDB_OBJECT_ROLE_CLIENT,
    FDB_OBJECT_ROLE_UNKNOWN
};

class CBaseWorker;
class CFdbSession;
class CBaseEndpoint;
class IFdbMsgBuilder;
class CFdbMessage;
struct CFdbSessionInfo;
class CFdbWatchdog;
class CFdbMsgProcessList;

class CFdbMsgSubscribeList : public CFdbMsgTable 
{
public:
    void addNotifyItem(FdbMsgCode_t msg_code, const char *topic = 0)
    {
        auto item = add_subscribe_tbl();
        item->set_msg_code(msg_code);
        if (topic)
        {
            item->set_topic(topic);
        }
    }
    void addNotifyGroup(FdbEventGroup_t event_group, const char *topic = 0)
    {
        addNotifyItem(fdbMakeGroupEvent(event_group), topic);
    }
};

class CFdbMsgTriggerList : public CFdbMsgTable 
{
public:
    void addTriggerItem(FdbMsgCode_t msg_code, const char *topic = 0)
    {
        auto item = add_subscribe_tbl();
        item->set_msg_code(msg_code);
        if (topic)
        {
            item->set_topic(topic);
        }
    }
    void addTriggerGroup(FdbEventGroup_t event_group, const char *topic = 0)
    {
        addTriggerItem(fdbMakeGroupEvent(event_group), topic);
    }
};

struct CFdbOnlineInfo
{
    // session ID
    FdbSessionId_t mSid;
    // for online:
    // +---------------------------------------------------------------------------------------------------------+
    // |                                             mFirstOrLast                                                |
    // +----------------------------------------------------+----------------------------------------------------+
    // |                   true                             |                        false                       |
    // +----------------------------------------------------+----------------------------------------------------+
    // |                     mQOS                           |                       mQOS                         |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    // |   FDB_QOS_RELIABLE   | FDB_QOS_TRY_SECURE_RELIABLE |   FDB_QOS_RELIABLE   | FDB_QOS_TRY_SECURE_RELIABLE |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    // | First insecure TCP   | First secure TCP session is | 2nd, 3rd... insecure | 2nd, 3rd... secure TCP      |                              |
    // | session is connected | connected                   | TCP session is       | session is connected.       |
    // |                      |                             | connected            |                             |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    //
    // for offline:
    // +---------------------------------------------------------------------------------------------------------+
    // |                                             mFirstOrLast                                                |
    // +----------------------------------------------------+----------------------------------------------------+
    // |                   true                             |                        false                       |
    // +----------------------------------------------------+----------------------------------------------------+
    // |                     mQOS                           |                       mQOS                         |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    // |   FDB_QOS_RELIABLE   | FDB_QOS_TRY_SECURE_RELIABLE |   FDB_QOS_RELIABLE   | FDB_QOS_TRY_SECURE_RELIABLE |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    // | Last insecure TCP    | Last secure TCP session is  | 3rd, 2nd... insecure | 3rd, 2nd... secure TCP      |                              |
    // | session is dropped   | dropped                     | TCP session is       | session is dropped.         |
    // |                      |                             | dropped              |                             |
    // + ---------------------+-----------------------------+----------------------+-----------------------------+
    bool mFirstOrLast;
    EFdbQOS mQOS;
};

class CFdbBaseObject
{
private:
    typedef void (CFdbBaseObject::*tRemoteCallback)(CBaseJob::Ptr &msg_ref);
public:
#define FDB_OBJ_ENABLED_MIGRATE         (1 << 0)
#define FDB_OBJ_ENABLE_LOG              (1 << 1)
#define FDB_OBJ_ENABLE_EVENT_CACHE      (1 << 2)
#define FDB_OBJ_ENABLE_TIMESTAMP        (1 << 3)
#define FDB_OBJ_ENABLE_EVENT_ROUTE      (1 << 4)
#define FDB_OBJ_ENABLE_WATCHDOG         (1 << 5)
#define FDB_OBJ_AUTO_REMOVE             (1 << 6)
#define FDB_OBJ_ONLINE_SEPARATE_CHANNEL (1 << 7)

    typedef uint32_t tRegEntryId;
    typedef std::function<void(CFdbBaseObject *obj, const CFdbOnlineInfo &info, bool is_online)> tConnCallbackFn;
    struct CConnCbRegistryItem
    {
        tConnCallbackFn mCallback;
        CBaseWorker *mWorker;
    };
    typedef std::map<tRegEntryId, CConnCbRegistryItem> tConnCallbackTbl;

    typedef std::function<void(CBaseJob::Ptr &, CFdbBaseObject *)> tInvokeCallbackFn;

    CFdbBaseObject(const char *name = 0, CBaseWorker *worker = 0, CFdbBaseContext *dummy = 0,
                   EFdbEndpointRole role = FDB_OBJECT_ROLE_UNKNOWN);
    virtual ~CFdbBaseObject();

    /*
     * invoke[1]
     * Invoke method call at receiver asynchronously. It can be called from client
     * to server or from server to client.
     * @iparam receiver: the receiver the request is sent to
     * @iparam code: the message code
     * @iparam data: the message data sent to receiver
     * @iparam timeout: timeout in unit of ms; 0 if no timer is specified
     *
     * CBaseServer::onInvoke() or CBaseClient::onInvoke() will be called to
     * process the request and reply will be sent after the request is
     * processed, which trigger invokation of onResponse() of the message;
     * If timeout is specified but receiver doesn't reply within specified time,
     * status is returned with error code being FDB_ST_TIMEOUT.
     */
    bool invoke(FdbSessionId_t receiver
                , FdbMsgCode_t code
                , IFdbMsgBuilder &data
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * invoke[1.1]
     * Similiar to invoke[1] but instead of passing message code, message
     * object is given. Usually the object is subclass of CBaseMessage so
     * that extra data can be attached.
     * In onInvoke(CBaseJob::Ptr &msg_ref), castToMessage() can be used to
     * convert msg_ref to subclass of CBaseMessage so that the extra data
     * can be retrieved.
     */
    bool invoke(FdbSessionId_t receiver
                , CFdbMessage *msg
                , IFdbMsgBuilder &data
                , int32_t timeout = 0);

    /*
     * invoke[2]
     * Similiar to invoke[1] without parameter 'receiver'. The method is called at
     * CBaseClient to invoke method upon connected server.
     */
    bool invoke(FdbMsgCode_t code
                , IFdbMsgBuilder &data
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * invoke[2.1]
     * Similiar to invoke[1.1] without parameter 'receiver'. The method is called at
     * CBaseClient to invoke method upon connected server.
     */
    bool invoke(CFdbMessage *msg
                , IFdbMsgBuilder &data
                , int32_t timeout = 0);

    /*
     * invoke[3]
     * Similiar to invoke[1] but is synchronous, which means the method blocks
     * until receiver reply. The added parameter 'msg_ref' is used to pass
     * (subclass of) CBaseMessage object in when called, and pass the same object
     * out, in which return data can be retrieved.
     *
     * @ioparam msg_ref: input - object of CBaseMessage or its subclass
     *                   output - the same object containing return data
     */
    bool invoke(FdbSessionId_t receiver
                , CBaseJob::Ptr &msg_ref
                , IFdbMsgBuilder &data
                , int32_t timeout = 0);

    /*
     * invoke[4]
     * Similiar to invoke[3] but 'receiver' is not specified. The method is
     * called by CBaseClient to invoke method upon connected server.
     */
    bool invoke(CBaseJob::Ptr &msg_ref
                , IFdbMsgBuilder &data
                , int32_t timeout = 0);

    /*
     * invoke[5]
     * Similiar to invoke[1] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    bool invoke(FdbSessionId_t receiver
                , FdbMsgCode_t code
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT
                , const char *log_info = 0);

    bool invoke(FdbSessionId_t receiver
                , CFdbMessage *msg
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[6]
     * Similiar to invoke[2] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    bool invoke(FdbMsgCode_t code
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT
                , const char *log_data = 0);

    bool invoke(CFdbMessage *msg
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[7]
     * Similiar to invoke[3] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    bool invoke(FdbSessionId_t receiver
                , CBaseJob::Ptr &msg_ref
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    /*
     * invoke[8]
     * Similiar to invoke[4] but raw data is sent rather than proto-buffer.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    bool invoke(CBaseJob::Ptr &msg_ref
                , const void *buffer = 0
                , int32_t size = 0
                , int32_t timeout = 0);

    // invoke[9]
    // async invoke with callback: it is similiar to CFdbBaseObject::invoke[2.1] except
    // that it accepts a callback which will be triggered when server replies
    bool invoke(FdbMsgCode_t code
                , IFdbMsgBuilder &data
                , tInvokeCallbackFn callback
                , CBaseWorker *worker = 0
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT);

    // invoke[10]
    // async invoke with callback: it is similiar to CFdbBaseObject::invoke[6] except
    // that it accepts a callback which will be triggered when server replies
    bool invoke(FdbMsgCode_t code
                , tInvokeCallbackFn callback
                , const void *buffer = 0
                , int32_t size = 0
                , CBaseWorker *worker = 0
                , int32_t timeout = 0
                , EFdbQOS qos = FDB_QOS_DEFAULT
                , const char *log_info = 0);

    /*
     * send[1]
     * send protocol buffer to receiver. No reply is expected; just one-shot
     * @iparam receiver: the receiver to get the message
     * @iparam code: message code
     * @iparam data: message in protocol buffer format
     * @iparam qos:  QOS of transport
     */
    bool send(FdbSessionId_t receiver
              , FdbMsgCode_t code
              , IFdbMsgBuilder &data
              , EFdbQOS qos = FDB_QOS_DEFAULT);
              
    /*
     * send[2]
     * Similiar to send[1] without parameter 'receiver'. The method is called
     * from CBaseClient to send message to the connected server.
     */
    bool send(FdbMsgCode_t code, IFdbMsgBuilder &data, EFdbQOS qos = FDB_QOS_DEFAULT);

    bool set(FdbMsgCode_t code, IFdbMsgBuilder &data, EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * send[3]
     * Similiar to send[1] but raw data is sent.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     * @iparam qos:  QOS of transport
     * @iparam log_data: text collected by log service to show the message
     */
    bool send(FdbSessionId_t receiver
              , FdbMsgCode_t code
              , const void *buffer = 0
              , int32_t size = 0
              , EFdbQOS qos = FDB_QOS_DEFAULT
              , const char *log_data = 0);
    /*
     * send[4]
     * Similiar to send[2] but raw data is sent.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     */
    bool send(FdbMsgCode_t code
              , const void *buffer = 0
              , int32_t size = 0
              , EFdbQOS qos = FDB_QOS_DEFAULT
              , const char *log_data = 0);

    bool set(FdbMsgCode_t code
              , const void *buffer
              , int32_t size
              , EFdbQOS qos = FDB_QOS_DEFAULT
              , const char *log_data = 0);

    /*
     * publish[1]
     * Similiar to send()[1] but topic is added.
     * @topic: topic to be send
     */
    bool publish(FdbMsgCode_t code
                 , IFdbMsgBuilder &data
                 , const char *topic = 0
                 , bool force_update = true
                 , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * publish[2]
     * Similiar to send()[4] but topic is added.
     * @topic: topic to be send
     */
    bool publish(FdbMsgCode_t code
                , const void *buffer = 0
                , int32_t size = 0
                , const char *topic = 0
                , bool force_update = true 
                , EFdbQOS qos = FDB_QOS_DEFAULT
                , const char *log_data = 0);

    /*
     * get[1]
     * Get current value of event code/topic pair asynchronously
     * The value is returned in the same way as invoke(), i.e., from onReply()
     */
    bool get(FdbMsgCode_t code
             , const char *topic = 0
             , int32_t timeout = 0
             , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * get[2]
     * The same as get[1] except that the given 'msg' can convery extra user
     *     that can be retrieved from onReply()
     * The value is returned in the same way as invoke(), i.e., from onReply()
     */
    bool get(CFdbMessage *msg
             , const char *topic = 0
             , int32_t timeout = 0);

    /*
     * get[3]
     * Get current value of event code/topic pair synchronously 
     * Once return, the value is containd in msg_ref
     */
    bool get(CBaseJob::Ptr &msg_ref, const char *topic = 0, int32_t timeout = 0);

    /*
     * broadcast[1]
     * broadcast message to all connected clients.
     * The clients should have already registered message 'code' with
     * optional topic.
     * @iparam code: code of the message to be broadcasted
     * @iparam data: message of protocol buffer
     * @iparam qos:  QOS of transport
     * @iparam topic: the topic associated with the broadcasting
     */
    bool broadcast(FdbMsgCode_t code
                   , IFdbMsgBuilder &data
                   , const char *topic = 0
                   , EFdbQOS qos = FDB_QOS_DEFAULT);
    /*
     * broadcast[3]
     * Similiar to broadcast[1] except that raw data is broadcasted.
     * @iparam buffer: buffer holding raw data
     * @iparam size: size of buffer
     * @iparam qos:  QOS of transport
     * @iparam topic: the topic associated with the broadcasting
     * @iparam log_data: text collected by log service to show the message
     */
    bool broadcast(FdbMsgCode_t code
                   , const void *buffer = 0
                   , int32_t size = 0
                   , const char *topic = 0
                   , EFdbQOS qos = FDB_QOS_DEFAULT
                   , const char *log_data = 0);
    /*
     * Build subscribe list before calling subscribe().
     * The event added is updated by brocast() from server or update()
     * from client.
     *
     * @oparam msg_list: the list holding message sending subscribe
     *      request to server
     * @iparam msg_code: The message code to subscribe
     * @iparam topic: the topic associated with the message.
     */
    static void addNotifyItem(CFdbMsgSubscribeList &msg_list
                              , FdbMsgCode_t msg_code
                              , const char *topic = 0);

    /*
     * Build subscribe list before calling subscribe().
     * Instead of specific event, the whole event group is subscribed.
     * When broadcast() at server is called to broadcast an event, the
     * the clients subscribed the event group that the event belongs
     * to is notified (onBroadcast() is called).
     *
     * @oparam msg_list: the list holding message sending subscribe
     *      request to server
     * @iparam event_group: The event group to subscribe
     * @iparam topic: the topic associated with the message.
     */
    static void addNotifyGroup(CFdbMsgSubscribeList &msg_list
                              , FdbEventGroup_t event_group = FDB_DEFAULT_GROUP
                              , const char *topic = 0);

    /*
     * Build update list to trigger update manually.
     *
     * @oparam msg_list: the list holding message sending update
     *      trigger to server
     * @iparam msg_code: The message code to trigger
     * @iparam topic: the topic associated with the message.
     */
    static void addTriggerItem(CFdbMsgTriggerList &msg_list
                                , FdbMsgCode_t msg_code
                                , const char *topic = 0);

    /*
     * Build update list to trigger update manually.
     * All events in the group will be updated.
     *
     * @oparam msg_list: the list holding message sending update
     *      trigger to server
     * @iparam event_group: The event group to trigger
     * @iparam topic: the topic associated with the message.
     */
    static void addTriggerGroup(CFdbMsgTriggerList &msg_list
                                , FdbEventGroup_t event_group = FDB_DEFAULT_GROUP
                                , const char *topic = 0);
    /*
     * subscribe[1]
     * Subscribe a list of messages so that the messages can be received
     *      when the server broadcast a message.
     * The method doesn't block and is asynchronous. Once completed, onStatus()
     *      will be called with CBaseMessage::isSubscribe() being true.
     *
     *   server                                  client
     *     |                            /-----------|
     *     |                addNotifyItem(list,...) |
     *     |                            \---------->|
     *     |                            /-----------|
     *     |                addUpdateItem(list,...) |
     *     |                            \---------->|
     *     |<------------subscribe(list,...)--------|
     *     |---CFdbMessage::broadcast(event1)------>|
     *     |                            /-----------|
     *     |                       onBroadcast()    |
     *     |                            \---------->|
     *     |---CFdbMessage::broadcast(event2)------>|
     *     |                            /-----------|
     *     |                       onBroadcast()    |
     *     |                            \---------->|
     *     |------status()[auto reply]------------->|
     *     |                    /-------------------|
     *     |     onStatus()[isSubscribe()==true]    |
     *     |                    \------------------>|
     *     |                                        |
     *     |---CBaseServer::broadcast(event1)------>|
     *     |                            /-----------|
     *     |                       onBroadcast()    |
     *     |                            \---------->|
     *     |                                        |
     *
     * @iparam msg_list: list of messages to be subscribed
     * @iparam timeout: optional timeout
     */
    bool subscribe(CFdbMsgSubscribeList &msg_list
                   , int32_t timeout = 0
                   , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * subscribe[1.1]
     * Similiar to subscribe[1] but additional parameter msg is given. The
     *      msg is object of subclass of CBaseMessage allowing extra data to be
     *      attached with the transaction.
     * in onStatus() with CBaseMessage::isSubscribe() being true, the object
     *      can be retrieved with castToMessage().
     *
     * @iparam msg_ist: list of messages to be retrieved
     * @iparam msg: (subclass of) CBaseMessage containing transaction-
     *      specific data
     * @iparam timeout: a timer expires if response is not received from
     *      server with the specified time.
     */
    bool subscribe(CFdbMsgSubscribeList &msg_list
                   , CFdbMessage *msg
                   , int32_t timeout = 0);

    /*
     * subscribeSync
     * Similiar to subscribe[1] but is synchronous. It will block until the
     *      server has registered all messages in list.
     *     A key feature of subscribeSync() is that it can guarantee all
     *     onBroadcast() is called before returning. This means all events are
     *     initialized once return from the call.
     *
     * @iparam msg_ist: list of messages to be retrieved
     * @iparam timeout: timer of the transaction
     */
    bool subscribeSync(CFdbMsgSubscribeList &msg_list
                       , int32_t timeout = 0
                       , EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * Unsubscribe messages listed in msg_list
     */
    bool unsubscribe(CFdbMsgSubscribeList &msg_list
                     , EFdbQOS qos = FDB_QOS_DEFAULT);
    
    /*
     * Unsubscribe the whole object
     */
    bool unsubscribe(EFdbQOS qos = FDB_QOS_DEFAULT);

    /*
     * Bind a server object to endpoint.
     *      @endpoint - the endpoint bind to
     *      @oid - specify id of the object. If FDB_INVALID_ID, object id is
     *          allocated dynamically
     *      @return - id of the object
     */
    FdbObjectId_t bind(CBaseEndpoint *endpoint, FdbObjectId_t oid = FDB_INVALID_ID);
    /*
     * Connect a client object to endpoint.
     *      @endpoint - the endpoint connect to
     *      @sid - The session via which request are sent by the client object.
     *          If set to FDB_INVALID_ID, a default session is selected. Usually
     *          this argument is set when a client object is created by a server
     *          endpoint.
     *      @oid - specify id of the object. If FDB_INVALID_ID, object id is
     *          allocated dynamically
     *      @return - id of the object
     */
    FdbObjectId_t connect(CBaseEndpoint *endpoint, FdbObjectId_t oid = FDB_INVALID_ID,
                          FdbSessionId_t sid = FDB_INVALID_ID);

    void unbind();
    void disconnect();

    const std::string &name() const
    {
        return mName;
    }

    void name(const char *name)
    {
        if (name)
        {
            mName = name;
        }
    }

    /*
     * Get a list of subscribed messages.
     * Warning: not thread-safe!!! MUST be used within CONTEXT!!!
     */
    void getSubscribeTable(tFdbSubscribeMsgTbl &table);
    void getSubscribeTable(FdbMsgCode_t code, tFdbFilterSets &topics);
    void getSubscribeTable(FdbMsgCode_t code, const char *topic,
                            tSubscribedSessionSets &session_tbl);

    CBaseWorker *worker() const
    {
        return mWorker;
    }

    // Warning: Not thread safe and should be set before working!!
    void worker(CBaseWorker *worker)
    {
        mWorker = worker;
    }

    FdbObjectId_t objId() const
    {
        return mObjId;
    }

    EFdbEndpointRole role() const
    {
        return mRole;
    }

    FdbEndpointId_t epid() const;

    bool enableMigrate() const
    {
        return !!(mFlag & FDB_OBJ_ENABLED_MIGRATE);
    }

    void enableMigrate(bool enb)
    {
        if (enb)
        {
            mFlag |= FDB_OBJ_ENABLED_MIGRATE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLED_MIGRATE;
        }
    }

    void enableEventCache(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_ENABLE_EVENT_CACHE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLE_EVENT_CACHE;
        }
    }

    bool eventCacheEnabled() const
    {
        return !!(mFlag & FDB_OBJ_ENABLE_EVENT_CACHE);
    }

    void enableTimeStamp(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_ENABLE_TIMESTAMP;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLE_TIMESTAMP;
        }
    }

    bool timeStampEnabled() const
    {
        return !!(mFlag & FDB_OBJ_ENABLE_TIMESTAMP);
    }

    void enableEventRoute(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_ENABLE_EVENT_ROUTE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLE_EVENT_ROUTE;
        }
    }

    void enableWatchdog(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_ENABLE_WATCHDOG;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLE_WATCHDOG;
        }
    }

    void enableLog(bool active)
    {
        if (active)
        {
            mFlag |= FDB_OBJ_ENABLE_LOG;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ENABLE_LOG;
        }
    }

    bool logEnabled() const
    {
        return !!(mFlag & FDB_OBJ_ENABLE_LOG);
    }

    bool eventRouteEnabled() const
    {
        return !!(mFlag & FDB_OBJ_ENABLE_EVENT_ROUTE);
    }

    void autoRemove(bool enb)
    {
        if (enb)
        {
            mFlag |= FDB_OBJ_AUTO_REMOVE;
        }
        else
        {
            mFlag &= ~FDB_OBJ_AUTO_REMOVE;
        }
    }

    bool autoRemove() const
    {
        return !!(mFlag & FDB_OBJ_AUTO_REMOVE);
    }

    void onlineSeparateChannel(bool enable)
    {
        if (enable)
        {
            mFlag |= FDB_OBJ_ONLINE_SEPARATE_CHANNEL;
        }
        else
        {
            mFlag &= ~FDB_OBJ_ONLINE_SEPARATE_CHANNEL;
        }
    }
    bool onlineSeparateChannel() const
    {
        return !!(mFlag & FDB_OBJ_ONLINE_SEPARATE_CHANNEL);
    }

    void initEventCache(FdbMsgCode_t event,
                        const char *topic,
                        IFdbMsgBuilder &data,
                        bool allow_event_route = false,
                        bool allow_event_cache = true);

    void initEventCache(FdbMsgCode_t event,
                        const char *topic,
                        const void *buffer,
                        int32_t size,
                        bool allow_event_route = false,
                        bool allow_event_cache = true);

    // Internal use only!!!
    bool broadcast(FdbSessionId_t sid
                  , FdbMsgCode_t code
                  , IFdbMsgBuilder &data
                  , const char *topic = 0
                  , EFdbQOS qos = FDB_QOS_DEFAULT);

    // Internal use only!!!
    bool broadcast(FdbSessionId_t sid
                  , FdbMsgCode_t code
                  , const void *buffer = 0
                  , int32_t size = 0
                  , const char *topic = 0
                  , EFdbQOS qos = FDB_QOS_DEFAULT
                  , const char *log_data = 0);

    // Internal use only!!!
    bool broadcast(CFdbMessage *msg, CFdbSession *session);
    bool publish(CFdbMessage *msg, CFdbSession *session);

    /*
     * request-reply through side band.
     * Note: Only used internally for FDBus!!!
     */
    bool invokeSideband(FdbMsgCode_t code
                      , IFdbMsgBuilder &data
                      , int32_t timeout = 0
                      , EFdbQOS qos = FDB_QOS_DEFAULT);
    bool invokeSideband(FdbMsgCode_t code
                      , const void *buffer = 0
                      , int32_t size = 0
                      , int32_t timeout = 0
                      , EFdbQOS qos = FDB_QOS_DEFAULT);
    bool invokeSideband(FdbSessionId_t receiver
                       , CFdbMessage *msg
                       , const void *buffer = 0
                       , int32_t size = 0
                       , int32_t timeout = 0);

    virtual void prepareDestroy();

    void startWatchdog(int32_t interval = 0, int32_t max_retries = 0);
    void stopWatchdog();
    void removeWatchdog();

    void setOnlineChannelType(EFdbSecureType type)
    {
        mOnlineChannelType = type;
    }
     FdbSessionId_t getFixedSession()
    {
        return mFixedSid;
    }

    tRegEntryId registerConnNotification(tConnCallbackFn callback, CBaseWorker *worker = 0);

    bool registerEventHandle(const CEvtHandleTbl &evt_tbl,
                             tRegistryHandleTbl *reg_handle = 0);

    bool registerMsgHandle(const CMsgHandleTbl &msg_tbl);
    bool registerSetEvtHandle(const CEvtHandleTbl &setevt_tbl);
protected:
    std::string mName;
    CBaseEndpoint *mEndpoint; // Which endpoint the object belongs to
    uint32_t mFlag;
    CFdbWatchdog *mWatchdog;
    EFdbSecureType mOnlineChannelType;

    /*
     * Implemented by server: called when client subscribe a message
     *      IMPORTANT: in the callback, current value must sent to the
     *      client represented by sid using unicast version of
     *      broadcast() so that client can be updated asap.
     * @iparam sid - the session subscribing the message
     * @iparam msg_code - the message subscribed
     * @iparam topic - the topic subscribed
     */
    virtual void onSubscribe(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by client: called when server broadcast a message.
     * @iparam msg_ref: reference to a message. Using castToMessage()
     *      to convert it to CBaseMessage.
     */
    virtual void onBroadcast(CBaseJob::Ptr &msg_ref);
    /*
     * Implemented by either client or server: called at receiver when
     *      the sender call invoke() or send() upon the receiver.
     */
    virtual void onInvoke(CBaseJob::Ptr &msg_ref);

    /*
     * Implemented by either client or server: called when a session
     *      is destroyed.
     * @iparam sid: id of the session being destroyed
     * @iparam is_last: true when it is the last session to be destroyed
     */
    virtual void onOffline(const CFdbOnlineInfo &info);
    /*
     * Implemented by either client or server: called when a session
     *      is created.
     * @iparam sid: id of the session created
     * @iparam is_first: true when it is the first session to create
     */
    virtual void onOnline(const CFdbOnlineInfo &info);
    /*
     * Implemented by either client or server: called when
     *      CBaseMessage::reply() is called by the sender
     */
    virtual void onReply(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by client: response to get()
     *      method when event is returned from server
     */
    virtual void onGetEvent(CBaseJob::Ptr &msg_ref)
    {}
    /*
     * Implemented by server: called when get()
     *      method is invoked by client
     */
    virtual void onSetEvent(CBaseJob::Ptr &msg_ref);
    /*
     * Implemented by either client or server: called when
     *      CBaseMessage::status() is called by the sender
     */
    virtual void onStatus(CBaseJob::Ptr &msg_ref
                          , int32_t error_code
                          , const char *description)
    {}

    /*
     * Implemented by either client or server: called when
     *      an message (subscribe, invoke, send...) is
     *      received targeting an object which does not exist.
     *      The callback should create the associated object
     *      according to the message.
     *      =============== Warning!!! ===================
     *      1. It is running ONLY at CONTEXT thread even
     *      though worker thread is specified!!!
     *      2. If some tasks HAS to be done at other threads,
     *      use synchronous job. The synchronous job shall
     *      NOT call any FDBus API since the CONTEXT is
     *      blocked by the synchronous job.
     */
	virtual void onCreateObject(CBaseEndpoint *endpoint, CFdbMessage *msg)
    {}

    virtual void onKickDog(CBaseJob::Ptr &msg_ref);
    virtual void onBark(CFdbSession *session);

    void role(EFdbEndpointRole role)
    {
        mRole = role;
    }

    bool isPrimary()
    {
        return mObjId == FDB_OBJECT_MAIN;
    }

    virtual bool authentication(CFdbSession *session)
    {
        return true;
    }

    void objId(FdbObjectId_t obj_id)
    {
        mObjId = obj_id;
    }

    CBaseEndpoint *endpoint() const
    {
        return mEndpoint;
    }

    virtual void onSidebandInvoke(CBaseJob::Ptr &msg_ref);
    virtual void onSidebandReply(CBaseJob::Ptr &msg_ref)
    {}
    virtual void onPublish(CBaseJob::Ptr &msg_ref)
    {}

    // Warning: only used by FDBus internally!!!!!!!
    void broadcastLogNoQueue(FdbMsgCode_t code, const uint8_t *data, int32_t size,
                             const char *topic, EFdbQOS qos = FDB_QOS_TRY_SECURE_RELIABLE);

    bool publishNoQueue(FdbMsgCode_t code, const char *topic, const void *buffer, int32_t size,
                        const char *log_data, bool force_update, EFdbQOS qos);
    bool publishNoQueue(FdbMsgCode_t code, const char *topic, const void *buffer,
                        int32_t size, CFdbSession *session);
    bool publishNoQueue(CFdbMessage *msg);
    bool setNoQueue(CFdbMessage *msg);
    void publishCachedEvents(CFdbSession *session);
    void getDroppedProcesses(CFdbMsgProcessList &process_list);
private:
    struct CEventData
    {
        uint8_t *mBuffer;
        int32_t mSize;
        bool mAllowRoute;
        bool mAllowCache;

        bool setEventCache(const uint8_t *buffer, int32_t size);
        void replaceCacheData(uint8_t *buffer, int32_t size);
        CEventData();
        ~CEventData();
    };
    typedef std::map<std::string, CEventData> CacheDataTable_t;
    typedef std::map<FdbMsgCode_t, CacheDataTable_t> EventCacheTable_t;

    struct CEventCache
    {
        EventCacheTable_t mCacheData;

        bool add(FdbMsgCode_t event, const char *topic, IFdbMsgBuilder &data,
                 bool allow_event_route, bool allow_event_cache = true);
        bool add(FdbMsgCode_t event, const char *topic, const void *buffer, int32_t size,
                 bool allow_event_route, bool allow_event_cache = true);
        void erase(FdbMsgCode_t event, const char *topic);
        void replace(FdbMsgCode_t id, const char *topic, const uint8_t *buffer, int32_t size,
                     bool allow_event_route, bool allow_event_cache = true);
        CEventData *get(FdbMsgCode_t msg_code, const char *topic);
        int32_t update(CFdbMessage *msg, bool allow_event_route, EFdbEndpointRole role);
    };

    CBaseWorker *mWorker;
    CEventSubscribeHandle mEventSubscribeHandle;
    CEventSubscribeHandle mGroupSubscribeHandle;
    FdbObjectId_t mObjId;
    EFdbEndpointRole mRole;
    FdbSessionId_t mFixedSid;
    CEventCache mEventCache;

    CFdbEventDispatcher mEventDispather;
    CFdbMsgDispatcher mMsgDispather;
    CFdbSetEvtDispatcher mSetEvtDispather;

    tConnCallbackTbl mConnCallbackTbl;
    tRegEntryId mRegIdAllocator;

    void subscribe(CFdbSession *session,
                   FdbMsgCode_t msg,
                   FdbObjectId_t obj_id,
                   const char *topic);

    void unsubscribe(CFdbSession *session,
                     FdbMsgCode_t msg,
                     FdbObjectId_t obj_id,
                     const char *topic);
    void unsubscribe(CFdbSession *session);
    void unsubscribe(FdbObjectId_t obj_id);

    int32_t updateEventCache(CFdbMessage *msg);
    bool broadcastMsg(CFdbMessage *msg);
    bool broadcast(CFdbMessage *msg);

    void getSubscribeTable(FdbMsgCode_t code, CFdbSession *session, tFdbFilterSets &topic_tbl);

    void callOnOnline(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callBindObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callConnectObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callUnbindObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callDisconnectObject(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callPrepareDestroy(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);

    // migrate onXXX() callbacks to worker thread
    void migrateToWorker(CBaseJob::Ptr &msg_ref, tRemoteCallback callback, CBaseWorker *worker = 0);
    void migrateToWorker(const CFdbOnlineInfo &info, bool online, CBaseWorker *worker = 0);
    void migrateToWorker(const CFdbOnlineInfo &info, bool online, tConnCallbackFn &callback,
                         CBaseWorker *worker = 0);

    void doSubscribe(CBaseJob::Ptr &msg_ref);
    void doBroadcast(CBaseJob::Ptr &msg_ref);
    void doInvoke(CBaseJob::Ptr &msg_ref);
    void doGetEvent(CBaseJob::Ptr &msg_ref);
    void doSetEvent(CBaseJob::Ptr &msg_ref);
    void doReply(CBaseJob::Ptr &msg_ref);
    void doReturnEvent(CBaseJob::Ptr &msg_ref);
    void doStatus(CBaseJob::Ptr &msg_ref);
    void doPublish(CBaseJob::Ptr &msg_ref);

    FdbObjectId_t addToEndpoint(CBaseEndpoint *endpoint, FdbObjectId_t obj_id);
    void removeFromEndpoint();
    FdbObjectId_t doBind(CBaseEndpoint *endpoint, FdbObjectId_t obj_id); // Server object
    FdbObjectId_t doConnect(CBaseEndpoint *endpoint, FdbObjectId_t obj_id, FdbSessionId_t sid); // Client object
    void doUnbind();
    void doDisconnect();
    void notifyOnline(CFdbSession *session, bool is_first);
    void notifyOffline(CFdbSession *session, bool is_last);

    void broadcastCached(CBaseJob::Ptr &msg_ref);
    const CEventData *getCachedEventData(FdbMsgCode_t msg_code, const char *topic);

    void callDispOnOnline(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callInvoke(CBaseJob::Ptr &msg_ref);
    void callSetEvent(CBaseJob::Ptr &msg_ref);
    void callBroadcast(CBaseJob::Ptr &msg_ref);
    void callStatus(CBaseJob::Ptr &msg_ref);
    void callSubscribe(CBaseJob::Ptr &msg_ref);
    void callReply(CBaseJob::Ptr &msg_ref);
    void callReturnEvent(CBaseJob::Ptr &msg_ref);
    void callOnline(const CFdbOnlineInfo &info, bool online);

    bool sendSidebandNoQueue(CFdbSession *session, CFdbMessage &msg, bool expect_reply);
    bool sendSideband(FdbMsgCode_t code, IFdbMsgBuilder &data);
    bool sendSideband(FdbMsgCode_t code
                    , const void *buffer = 0
                    , int32_t size = 0);

    bool sendSideband(CFdbSession *session, FdbMsgCode_t code, IFdbMsgBuilder &data);
    bool sendSideband(CFdbSession *session
                    , FdbMsgCode_t code
                    , const void *buffer = 0
                    , int32_t size = 0);
    bool kickDog(CFdbSession *session);
    void createWatchdog(int32_t interval = 0, int32_t max_retries = 0);
    void callWatchdogAction(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);

    tRegEntryId doRegisterConnNotification(tConnCallbackFn callback, CBaseWorker *worker);
    bool doRegisterEventHandle(const CEvtHandleTbl &evt_tbl,
                               tRegistryHandleTbl *reg_handle);
    bool subscribeEvents(const tEvtHandleTbl &events,
                         tRegistryHandleTbl *reg_handle);

    bool doRegisterMsgHandle(const CMsgHandleTbl &msg_tbl);
    bool doRegisterSetEvtHandle(const CEvtHandleTbl &setevt_tbl);

    void callRegisterConnNotification(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callRegisterMsgHandle(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callRegisterEventHandle(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);
    void callRegisterSetEvtHandle(CBaseWorker *worker, CMethodJob<CFdbBaseObject> *job, CBaseJob::Ptr &ref);

    bool subscribeNoQueue(CFdbMsgSubscribeList &msg_list
                        , CFdbMessage *msg
                        , int32_t timeout = 0);

    void replaceEventCache(FdbMsgCode_t id,
                           const char *topic,
                           const uint8_t *buffer,
                           int32_t size,
                           bool allow_event_route);

    friend class COnOnlineJob;
    friend class CBindObjectJob;
    friend class CConnectObjectJob;
    friend class CUnbindObjectJob;
    friend class CDisconnectObjectJob;
    friend class CPrepareDestroyJob;
    friend class CDispOnOnlineJob;

    friend class CBaseClient;
    friend class CBaseServer;
    friend class CFdbSession;
    friend class CFdbUDPSession;
    friend class CBaseSession;
    friend class CFdbMessage;
    friend class CBaseEndpoint;
    friend class CLogProducer;
    friend class CFdbEventRouter;
    friend class CFdbWatchdog;
    friend class CWatchdogJob;
    friend class CSysFdWatch;
    friend class CFdbLogCache;
    friend class CLogServer;
    friend class CFdbAFComponent;
    friend class CRegisterEventHandleJob;
    friend class CRegisterMsgHandleJob;
    friend class CRegisterSetEvtHandleJob;
    friend class CRegisterConnNotificationJob;
    friend class CDpClient;
    friend class CDpServer;
};

}
}
#endif
