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
#ifndef __CFDBMSGDISPATCHER_H__
#define __CFDBMSGDISPATCHER_H__

#include <map>
#include <functional>
#include <string>
#include <vector>
#include "CBaseJob.h"
#include "CFdbMessage.h"
#include "CBaseWorker.h"

namespace ipc {
namespace fdbus {
class CFdbMsgSubscribeItem;
class CFdbBaseObject;

typedef std::function<void(CBaseJob::Ptr &, CFdbBaseObject *)> tDispatcherCallbackFn;

void fdbMigrateCallback(CBaseJob::Ptr &msg_ref, CFdbMessage *msg, tDispatcherCallbackFn &fn,
                               CBaseWorker *worker, CFdbBaseObject *obj);

template<typename T>
class CFdbMessageHandle
{
private:
    typedef void (T::*tCallbackFn)(CBaseJob::Ptr &msg_ref);
    typedef std::map<FdbMsgCode_t, tCallbackFn> tRegistryTbl;

public:
    void registerCallback(FdbMsgCode_t code, tCallbackFn callback)
    {
        mRegistryTbl[code] = callback;
    }

    bool processMessage(T *instance, CBaseJob::Ptr &msg_ref)
    {
        CFdbMessage *msg = castToMessage<CFdbMessage *>(msg_ref);
        typename tRegistryTbl::iterator it = mRegistryTbl.find(msg->code());
        if (it == mRegistryTbl.end())
        {
            return false;
        }
        (instance->*(it->second))(msg_ref);
        return true;
    }
private:
    tRegistryTbl mRegistryTbl;
};

template<typename T>
class CFdbSubscribeHandle
{
private:
    typedef void (T::*tCallbackFn)(CBaseJob::Ptr &msg_ref, const CFdbMsgSubscribeItem *sub_item);
    typedef std::map<FdbMsgCode_t, tCallbackFn> tRegistryTbl;

public:
    void registerCallback(FdbMsgCode_t code, tCallbackFn callback)
    {
        mRegistryTbl[code] = callback;
    }

    bool processMessage(T *instance, CBaseJob::Ptr &msg_ref, const CFdbMsgSubscribeItem *sub_item, 
                        FdbMsgCode_t code)
    {
        typename tRegistryTbl::iterator it = mRegistryTbl.find(code);
        if (it == mRegistryTbl.end())
        {
            return false;
        }
        (instance->*(it->second))(msg_ref, sub_item);
        return true;
    }
private:
    tRegistryTbl mRegistryTbl;
};

class CMsgHandleTbl
{
public:
    struct CMsgHandleItem
    {
        FdbMsgCode_t mCode;
        tDispatcherCallbackFn mCallback;
        CBaseWorker *mWorker;
    };
    typedef std::vector<CMsgHandleItem> tMsgHandleTbl;
    bool add(FdbMsgCode_t code, tDispatcherCallbackFn callback, CBaseWorker *worker = 0);
    const tMsgHandleTbl &getMsgHandleTbl() const
    {
        return mTable;
    }

    tMsgHandleTbl mTable;
};
class CFdbMsgDispatcher
{
public:
    typedef std::map<FdbMsgCode_t, CMsgHandleTbl::CMsgHandleItem> tRegistryTbl;

    bool registerCallback(const CMsgHandleTbl &msg_tbl);
    bool unregisterCallback(FdbMsgCode_t code)
    {return true;}
    bool processMessage(CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj);
    bool enabled() const
    {
        return !mRegistryTbl.empty();
    }
private:
    tRegistryTbl mRegistryTbl;
};

struct CEvtHandleItem
{
    FdbMsgCode_t mCode;
    tDispatcherCallbackFn mCallback;
    std::string mTopic;
    CBaseWorker *mWorker;
};
typedef std::vector<CEvtHandleItem> tEvtHandleTbl;
typedef std::vector<CEvtHandleItem *>tEvtHandlePtrTbl;

typedef uint32_t tRegEntryId;
typedef std::map<tRegEntryId, CEvtHandleItem> tEvtCallbackList;
typedef std::map<std::string, tEvtCallbackList> tTopicList;
typedef std::map<FdbMsgCode_t, tTopicList> tRegistryTbl;
typedef std::vector<tRegEntryId> tRegistryHandleTbl;
class CEvtHandleTbl
{
public:
    bool add(FdbMsgCode_t code, tDispatcherCallbackFn callback,
             CBaseWorker *worker = 0, const char *topic = "");
    bool addGroup(FdbMsgCode_t group, tDispatcherCallbackFn callback,
             CBaseWorker *worker = 0, const char *topic = "");
    const tEvtHandleTbl &getEvtHandleTbl() const
    {
        return mTable;
    }
    tEvtHandleTbl mTable;
};

class CBaseEventDispatcher 
{
public:
    CBaseEventDispatcher()
        : mRegIdAllocator(0)
    {}
    virtual ~CBaseEventDispatcher(){};
    bool registerCallback(const CEvtHandleTbl &evt_tbl,
                          tRegistryHandleTbl *registered_evt_tbl = 0);
    bool unregisterCallback(tRegEntryId reg_id)
    {return true;}
    bool processMessage(CBaseJob::Ptr &msg_ref,
                        FdbMsgCode_t code,
                        CFdbBaseObject *obj,
                        tEvtHandlePtrTbl &handles_to_invoke,
                        const tRegistryHandleTbl *registered_evt_tbl = 0);
    void dispatchEvents(tEvtHandlePtrTbl &handles_to_invoke,
                        CBaseJob::Ptr &msg_ref, CFdbMessage *msg,
                        CFdbBaseObject *obj);
    void dumpEvents(tEvtHandleTbl &event_table);
    void dumpEventDetails(tEvtHandleTbl &event_table);
    bool enabled() const
    {
        return !mRegistryTbl.empty();
    }
protected:
    virtual void migrateCallback(CBaseJob::Ptr &msg_ref,
                                 CFdbMessage *msg,
                                 tDispatcherCallbackFn &fn,
                                 CBaseWorker *worker,
                                 CFdbBaseObject *obj) = 0;
private:
    tRegistryTbl mRegistryTbl;
    tRegEntryId mRegIdAllocator;
};

class CFdbSetEvtDispatcher : public CBaseEventDispatcher 
{
public:
    void migrateCallback(CBaseJob::Ptr &msg_ref,
                         CFdbMessage *msg,
                         tDispatcherCallbackFn &fn,
                         CBaseWorker *worker,
                         CFdbBaseObject *obj);
};

class CFdbEventDispatcher : public CBaseEventDispatcher 
{
public:
    void migrateCallback(CBaseJob::Ptr &msg_ref,
                         CFdbMessage *msg,
                         tDispatcherCallbackFn &fn,
                         CBaseWorker *worker,
                         CFdbBaseObject *obj);
};
}
}
#endif
