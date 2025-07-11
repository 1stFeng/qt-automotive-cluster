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

#ifndef _CBASEENDPOINT_H_
#define _CBASEENDPOINT_H_

#include <string>
#include <vector>
#include "common_defs.h"
#include "CEntityContainer.h"
#include "CFdbBaseObject.h"
#include "CMethodJob.h"
#include "CFdbToken.h"
#include "CFdbEventRouter.h"

namespace ipc {
namespace fdbus {
class CBaseWorker;
class CFdbSessionContainer;
class CFdbMessage;
struct CFdbSocketAddr;
class CFdbSession;
class CApiSecurityConfig;
class CFdbBaseContext;

class CBaseEndpoint : public CEntityContainer<FdbSocketId_t, CFdbSessionContainer *>
                    , public CFdbBaseObject
{
public:
    typedef CEntityContainer<FdbObjectId_t, CFdbBaseObject *> tObjectContainer;

#define FDB_EP_RECONNECT_ENABLED        (1 << 16)
#define FDB_EP_RECONNECT_ACTIVATED      (1 << 17)
#define FDB_EP_ENABLE_UDP               (1 << 18)
#define FDB_EP_TCP_BLOCKING_MODE        (1 << 19)
#define FDB_EP_IPC_BLOCKING_MODE        (1 << 20)
#define FDB_EP_READ_ASYNC               (1 << 21)
#define FDB_EP_WRITE_ASYNC              (1 << 22)
#define FDB_EP_ALLOW_TCP_NORMAL         (1 << 23)
#define FDB_EP_ALLOW_TCP_SECURE         (1 << 24)
#define FDB_EP_VALID_SVC_NAME           (1 << 25)

    CBaseEndpoint(const char *name = 0, CBaseWorker *worker = 0, CFdbBaseContext *context = 0,
                  EFdbEndpointRole role = FDB_OBJECT_ROLE_UNKNOWN);
    ~CBaseEndpoint();

    const std::string &nsName() const
    {
        return mNsName;
    }

    /*
     * Kick out a session (usually called by server to kick out a client)
     */
    void kickOut(FdbSessionId_t sid);
    uint32_t getSessionCount() const
    {
        return mSecureSessionCnt + mInsecureSessionCnt;
    }
    uint32_t getNormalSessionCount() const
    {
        return mInsecureSessionCnt;
    }
    uint32_t getSecureSessionCount() const
    {
        return mSecureSessionCnt;
    }

    bool connected(EFdbSecureType sec_type = FDB_SEC_TRY_SECURE)
    {
        if (sec_type == FDB_SEC_TRY_SECURE)
        {
            return !!(mSecureSessionCnt + mInsecureSessionCnt);
        }
        else
        {
            return (sec_type == FDB_SEC_SECURE) ? !!mSecureSessionCnt : !!mInsecureSessionCnt;
        }
    }

    FdbEndpointId_t epid() const
    {
        return mEpid;
    }

    void enableReconnect(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_RECONNECT_ENABLED;
        }
        else
        {
            mFlag &= ~FDB_EP_RECONNECT_ENABLED;
        }
    }

    bool reconnectEnabled() const
    {
        return !!(mFlag & FDB_EP_RECONNECT_ENABLED);
    }

    void activateReconnect(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_RECONNECT_ACTIVATED;
        }
        else
        {
            mFlag &= ~FDB_EP_RECONNECT_ACTIVATED;
        }
    }

    bool reconnectActivated() const
    {
        return !!(mFlag & FDB_EP_RECONNECT_ACTIVATED);
    }

    bool UDPEnabled() const
    {
        return !!(mFlag & FDB_EP_ENABLE_UDP);
    }

    void enableUDP(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_ENABLE_UDP;
        }
        else
        {
            mFlag &= ~FDB_EP_ENABLE_UDP;
        }
    }

    void enableTcpBlockingMode(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_TCP_BLOCKING_MODE;
        }
        else
        {
            mFlag &= ~FDB_EP_TCP_BLOCKING_MODE;
        }
    }

    bool enableTcpBlockingMode() const
    {
        return !!(mFlag & FDB_EP_TCP_BLOCKING_MODE);
    }

    void enableIpcBlockingMode(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_IPC_BLOCKING_MODE;
        }
        else
        {
            mFlag &= ~FDB_EP_IPC_BLOCKING_MODE;
        }
    }

    bool enableIpcBlockingMode() const
    {
        return !!(mFlag & FDB_EP_IPC_BLOCKING_MODE);
    }

    void enableAysncWrite(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_WRITE_ASYNC;
        }
        else
        {
            mFlag &= ~FDB_EP_WRITE_ASYNC;
        }
    }

    bool enableAysncWrite() const
    {
        return !!(mFlag & FDB_EP_WRITE_ASYNC);
    }

    void enableAysncRead(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_READ_ASYNC;
        }
        else
        {
            mFlag &= ~FDB_EP_READ_ASYNC;
        }
    }
    bool enableAysncRead() const
    {
        return !!(mFlag & FDB_EP_READ_ASYNC);
    }

    void enableBlockingMode(bool active)
    {
        if (active)
        {
            mFlag |= FDB_EP_IPC_BLOCKING_MODE | FDB_EP_TCP_BLOCKING_MODE;
            mFlag &= ~(FDB_EP_WRITE_ASYNC | FDB_EP_READ_ASYNC);
        }
        else
        {
            mFlag &= ~(FDB_EP_IPC_BLOCKING_MODE | FDB_EP_TCP_BLOCKING_MODE);
            mFlag |= FDB_EP_WRITE_ASYNC | FDB_EP_READ_ASYNC;
        }
    }

    bool TCPEnabled() const
    {
        return !!(mFlag & FDB_EP_ALLOW_TCP_NORMAL);
    }
    void enableTCP(bool enable)
    {
        if (enable)
        {
            mFlag |= FDB_EP_ALLOW_TCP_NORMAL;
        }
        else
        {
            mFlag &= ~FDB_EP_ALLOW_TCP_NORMAL;
        }
    }

    void enableTCPSecure(bool enable)
    {
        if (enable)
        {
            mFlag |= FDB_EP_ALLOW_TCP_SECURE;
        }
        else
        {
            mFlag &= ~FDB_EP_ALLOW_TCP_SECURE;
        }
    }
    bool TCPSecureEnabled() const
    {
        return !!(mFlag & FDB_EP_ALLOW_TCP_SECURE) && mSSLContext;
    }

    int32_t exportableLevel() const
    {
        return mExportableLevel;
    }

    void setExportableLevel(int32_t level)
    {
        mExportableLevel = level;
    }

    bool validSvcName() const
    {
        return !!(mFlag & FDB_EP_VALID_SVC_NAME);
    }
    void validSvcName(bool valid)
    {
        if (valid)
        {
            mFlag |= FDB_EP_VALID_SVC_NAME;
        }
        else
        {
            mFlag &= ~FDB_EP_VALID_SVC_NAME;
        }
    }

    void prepareDestroy();

    void addPeerRouter(const char *peer_router_name)
    {
        mEventRouter.addPeer(peer_router_name);
    }

    CFdbBaseContext *context() const
    {
        return mContext;
    }

    tObjectContainer &getObjectContainer()
    {
        return mObjectContainer;
    }

    // Internal use only!!!
    CFdbSession *getSession(FdbSessionId_t session_id);
    void registerSession(CFdbSession *session);
    void unregisterSession(FdbSessionId_t session_id);
    void deleteSession(FdbSessionId_t session_id);
    void deleteSession(CFdbSessionContainer *container);

    void keepAlive(int32_t interval, int32_t retries)
    {
        mKeepAliveInterval = interval;
        mKeepAliveRetries = retries;
    }

    int32_t getKeepAliveInterval() const
    {
        return mKeepAliveInterval;
    }

    int32_t getKeepAliveRetries() const
    {
        return mKeepAliveRetries;
    }
    bool createSSL(const char *public_key,
                   const char *private_key,
                   const char *root_ca,
                   uint32_t verify_type);
    bool createSSL(const char *public_key,
                   const char *private_key,
                   const char *root_ca);
    bool createSSL(const char *public_key,
                   const char *private_key);
    bool createSSL(const char *root_ca,
                   uint32_t verify_type);
    bool createSSL(const char *root_ca);

    static EFdbSecureType QosToSecureType(EFdbQOS qos);

    bool checkOnlineChannel(EFdbSecureType type, const CFdbOnlineInfo &channel_info);
    void regularHouseKeeping();

    void setContext(CFdbBaseContext *ctx = 0);

    FdbInstanceId_t instanceId() const
    {
        return mInstanceId;
    }

    void setInstanceId(FdbInstanceId_t id)
    {
        mInstanceId = id;
    }

protected:
    std::string mNsName;
    CFdbToken::tTokenList mTokens;
    CFdbBaseContext *mContext;

    void deleteSocket(FdbSocketId_t skid = FDB_INVALID_ID);
    void addSocket(CFdbSessionContainer *container);
    void getDefaultSvcUrl(std::string &url);
    void epid(FdbEndpointId_t epid)
    {
        mEpid = epid;
    }

    bool requestServiceAddress(const char *server_name = 0, FdbInstanceId_t instance_id = FDB_DEFAULT_INSTANCE);
    bool releaseServiceAddress();
    void onSidebandInvoke(CBaseJob::Ptr &msg_ref);
    void onSidebandReply(CBaseJob::Ptr &msg_ref);

    virtual bool onMessageAuthentication(CFdbMessage *msg, CBaseSession *session);
    virtual bool onGetEventAuthentication(CFdbMessage *msg, CBaseSession *session);
    virtual bool onSetEventAuthentication(CFdbMessage *msg, CBaseSession *session);
    virtual bool onPublishAuthentication(CFdbMessage *msg, CBaseSession *session);

    void setNsName(const char *name)
    {
        if (name)
        {
            mNsName = name;
            if (mName.empty())
            {
                mName = mNsName;
            }
        }
    }
    void onPublish(CBaseJob :: Ptr &msg_ref);
    void *getSSLContext() const
    {
        return mSSLContext;
    }
private:
    typedef CEntityContainer<FdbSessionId_t, CFdbSession *> tSessionContainer;
    tSessionContainer mSessionContainer;

    tObjectContainer mObjectContainer;

    uint32_t mSessionCnt;
    uint32_t mInsecureSessionCnt;
    uint32_t mSecureSessionCnt;
    FdbObjectId_t mSnAllocator;
    FdbEndpointId_t mEpid;
    CFdbEventRouter mEventRouter;

    int32_t mKeepAliveInterval;
    int32_t mKeepAliveRetries;
    std::string mRootCA;
    std::string mPublicKey;
    std::string mPrivateKey;
    void *mSSLContext;
    int32_t mExportableLevel;
    FdbInstanceId_t mInstanceId;
    
    CFdbSession *preferredPeer(EFdbQOS qos);
    void checkAutoRemove();
    void getUrlList(std::vector<std::string> &url_list);
    FdbObjectId_t addObject(CFdbBaseObject *obj);
    void removeObject(CFdbBaseObject *obj);
    CFdbBaseObject *getObject(CFdbMessage *msg, bool server_only);
    void unsubscribeSession(CFdbSession *session);
    bool addConnectedSession(CFdbSessionContainer *socket, CFdbSession *session);
    void deleteConnectedSession(CFdbSession *session);

    FdbEndpointId_t registerSelf();
    void unregisterSelf();
    void destroySelf(bool prepare);
    void callRegisterEndpoint(CBaseWorker *worker, CMethodJob<CBaseEndpoint> *job, CBaseJob::Ptr &ref);
    void callDestroyEndpoint(CBaseWorker *worker, CMethodJob<CBaseEndpoint> *job, CBaseJob::Ptr &ref);
    CFdbBaseObject *findObject(FdbObjectId_t obj_id, bool server_only);
    bool importTokens(const CFdbParcelableArray<std::string> &in_tokens);
    int32_t checkSecurityLevel(const char *token);
    void updateSecurityLevel();
    void updateSessionInfo(CFdbSession *session);
    CFdbSession *connectedSession(const CFdbSocketAddr &addr);
    CFdbSession *connectedSession(EFdbSecureType sec_type);
    CFdbSession *bound(const CFdbSocketAddr &addr);
    virtual const CApiSecurityConfig *getApiSecurityConfig()
    {
        return 0;
    }

    void callKickOutSession(CBaseWorker *worker, CMethodJob<CBaseEndpoint> *job, CBaseJob::Ptr &ref);

    friend class CFdbSession;
    friend class CFdbUDPSession;
    friend class CBaseSession;
    friend class CFdbMessage;
    friend class CFdbBaseContext;
    friend class CBaseServer;
    friend class CBaseClient;
    friend class CIntraNameProxy;
    friend class CIntraHostProxy;
    friend class CInterHostProxy;
    friend class CFdbEventRouter;
    friend class CFdbBaseObject;
    friend class CServerSocket;
    friend class CClientSocket;
    friend class CRegisterJob;
    friend class CDestroyJob;
    friend class CLogProducer;
    friend class CKickOutSessionJob;
};

}
}
#endif
