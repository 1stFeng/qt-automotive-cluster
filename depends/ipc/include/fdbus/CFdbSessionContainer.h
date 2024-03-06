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

#ifndef _CFDBSESSIONCONTAINER_H_
#define _CFDBSESSIONCONTAINER_H_

#include <string>
#include <list>
#include "common_defs.h"
#include "CSocketImp.h"

namespace ipc {
namespace fdbus {
class CFdbSession;
class CBaseEndpoint;
class CFdbUDPSession;
class CFdbMessage;
struct CFdbSocketInfo
{
    CFdbSocketAddr const *mAddress;
};

class CFdbSessionContainer
{
public:
    CFdbSessionContainer(FdbSocketId_t skid, CBaseEndpoint *owner, CBaseSocket *tcp_socket,
                         int32_t udp_port = FDB_INET_PORT_INVALID, bool secure = false);
    virtual ~CFdbSessionContainer();
    FdbSocketId_t skid()
    {
        return mSkid;
    }

    bool getSocketInfo(CFdbSocketInfo &info);

    CBaseEndpoint *owner()
    {
        return mOwner;
    }
    CFdbSession *getDefaultSession();

    void enableSessionDestroyHook(bool enable)
    {
        mEnableSessionDestroyHook = enable;
    }

    bool bindUDPSocket(const char *ip_address = 0, int32_t udp_port = FDB_INET_PORT_INVALID);
    CFdbUDPSession *getUDPSession()
    {
        return mUDPSession;
    }
    bool getUDPSocketInfo(CFdbSocketInfo &info);

    CFdbSession *connectedSession(const CFdbSocketAddr &addr);
    CFdbSession *connectedSession(EFdbSecureType sec_type);
    CFdbSession *bound(const CFdbSocketAddr &addr);

    void pendingUDPPort(int32_t udp_port)
    {
        mPendingUDPPort = udp_port;
    }

    bool isSecure() const
    {
        return mSecure;
    }
    CFdbSession *findTcpSession(CFdbUDPSession *udp_session,
                                tFdbIpV4 peer_ip,
                                int32_t peer_port);
    int32_t interfaceId() const
    {
        return mInterfaceId;
    }
    void setInterfaceId(int32_t id)
    {
        mInterfaceId = id;
    }
protected:
    FdbSocketId_t mSkid;
    virtual void onSessionDeleted(CFdbSession *session) {}
    CBaseEndpoint *mOwner;
    CBaseSocket *mSocket;
    bool mSecure;
private:
    typedef std::list<CFdbSession *> ConnectedSessionTable_t;
    bool mEnableSessionDestroyHook;
    CBaseSocket *mUDPSocket;
    CFdbUDPSession *mUDPSession;
    int32_t mPendingUDPPort;
    int32_t mInterfaceId;

    ConnectedSessionTable_t mConnectedSessionTable;

    void addSession(CFdbSession *session);
    void removeSession(CFdbSession *session);
    void callSessionDestroyHook(CFdbSession *session);
    int32_t checkSecurityLevel();

    friend class CFdbSession;
    friend class CBaseSession;
    friend class CBaseEndpoint;
    friend class CFdbUDPSession;
    friend class CBaseServer;
    friend class CBaseClient;
    friend class CFdbBaseObject;
};

}
}
#endif
