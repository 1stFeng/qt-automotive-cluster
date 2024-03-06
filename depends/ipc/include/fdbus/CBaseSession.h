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
#ifndef __CBASESESSION_H__
#define __CBASESESSION_H__

#include <string>
#include "common_defs.h"
#include "CBaseFdWatch.h"
#include "CFdbMessage.h"
#include "CFdbSessionContainer.h"
#include "CEntityContainer.h"
#include "CBaseSysDep.h"

namespace ipc {
namespace fdbus {
class CSocketImp;
class CFdbMessage;
struct CFdbSessionInfo
{
    CFdbSocketInfo mContainerSocket;
    CFdbSocketCredentials const *mCred;
    CFdbSocketConnInfo const *mConn;
};

class CBaseSession : public CBaseFdWatch
{
public:
    struct CStatisticsData
    {
        uint32_t mSyncRequest;
        uint32_t mSyncReply;
        uint32_t mAsyncRequest;
        uint32_t mAsyncReply;
        uint32_t mBroadcast;
        uint32_t mPublish;
        uint32_t mSend;
        uint32_t mGetEvent;
        uint32_t mSetEvent;
        uint32_t mReturnEvent;
        uint32_t mSyncStatus;
        uint32_t mAsyncStatus;
        CStatisticsData()
            : mSyncRequest(0)
            , mSyncReply(0)
            , mAsyncRequest(0)
            , mAsyncReply(0)
            , mBroadcast(0)
            , mPublish(0)
            , mSend(0)
            , mGetEvent(0)
            , mSetEvent(0)
            , mReturnEvent(0)
            , mSyncStatus(0)
            , mAsyncStatus(0)
        {
        }
    };
    struct CStatistics
    {
        CStatisticsData mTx;
        CStatisticsData mRx;
    };
    CBaseSession(FdbSessionId_t sid, CFdbSessionContainer *container, CSocketImp *socket);
    ~CBaseSession();
    CFdbSessionContainer *container()
    {
        return mContainer;
    }
    CSocketImp *getSocket()
    {
        return mSocket;
    }
    const CFdbMsgPrefix &msgPrefix() const
    {
        return mMsgPrefix;
    }
    uint8_t *payloadBuffer()
    {
        auto buffer = mPayloadBuffer;
        mPayloadBuffer = 0;
        return buffer;
    }
    FdbSessionId_t sid() const
    {
        return mSid;
    }
    void sid(FdbSessionId_t sid)
    {
        mSid = sid;
    }
    const std::string &senderName() const
    {
        return mSenderName;
    }
    void senderName(const char *name)
    {
        mSenderName = name;
    }

    virtual EFdbQOS qos() {return FDB_QOS_INVALID;}

    bool isSecure() const;

    const CStatistics &getStatistics() const
    {
        return mStatistics;
    }
    
    virtual int32_t securityLevel() const
    {
        return FDB_SECURITY_LEVEL_UNKNOWN;
    }

    virtual bool sendMessageRef(CBaseJob::Ptr &ref) { return false; }
    virtual bool sendMessage(CFdbMessage* msg) { return false; }
    void terminateMessage(FdbMsgSn_t msg, int32_t status, const char *reason = 0);
    CFdbMessage *peepPendingMessage(FdbMsgSn_t sn);
    void getSessionInfo(CFdbSessionInfo &info);
    virtual const std::string &getUDPDestAddr(int32_t &port) const
    {
        static const std::string dummy;
        port = FDB_INET_PORT_INVALID;
        return dummy;
    }
    virtual void getUDPDestAddr(tFdbIpV4 &peer_ip, int32_t &peer_port) const
    {
        peer_ip = 0;
        peer_port = FDB_INET_PORT_INVALID;
    }
    virtual void setUDPDestAddr(const char *ip_addr, int32_t port)
    {}
    EFdbSocketType getSocketType() const;
    int32_t checkSecurityLevel();
protected:
    CFdbSessionContainer *mContainer;
    CSocketImp *mSocket;
    uint8_t *mPayloadBuffer;
    CFdbMsgPrefix mMsgPrefix;
    std::string mSenderName;
    CStatistics mStatistics;
    FdbSessionId_t mSid;
    typedef CEntityContainer<FdbMsgSn_t, CBaseJob::Ptr> PendingMsgTable_t;
    PendingMsgTable_t mPendingMsgTable;

    int32_t readStream(uint8_t *data, int32_t size);
    void terminateMessage(CBaseJob::Ptr &job, int32_t status, const char *reason);
    virtual void doRequest(CFdbMessageHeader &head);
    virtual void doResponse(CFdbMessageHeader &head);
    virtual void doBroadcast(CFdbMessageHeader &head);
    virtual void doSubscribeReq(CFdbMessageHeader &head, bool subscribe) {}

    void doStatistics(EFdbMessageType type, uint32_t flag, CStatisticsData &stat_data);

    void parsePrefix(const uint8_t *data);
    void processPayload();
    bool sendMessageSync(CBaseJob::Ptr &ref, CFdbMessage *msg);
};

}
}
#endif

