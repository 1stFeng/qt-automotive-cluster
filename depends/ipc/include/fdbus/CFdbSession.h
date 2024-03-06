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

#ifndef _CFDBSESSION_
#define _CFDBSESSION_

#include <string>
#include "CBaseSession.h"
#include "common_defs.h"
//#include "CFdbMessage.h"
#include "CFdbMessage.h"

namespace ipc {
namespace fdbus {
class CFdbMessage;
class CFdbMessageHeader;
class CFdbSessionContainer;
class CFdbSession : public CBaseSession
{
public:
    CFdbSession(FdbSessionId_t sid, CFdbSessionContainer *container, CSocketImp *socket);
    virtual ~CFdbSession();

    bool sendMessage(const uint8_t *buffer, int32_t size);
    bool sendMessage(CFdbMessage *msg);
    bool sendMessageRef(CBaseJob::Ptr &ref);
    bool sendMessageRef(CBaseJob::Ptr &ref, bool* useUdpOut = nullptr);
    void securityLevel(int32_t level);
    int32_t securityLevel() const
    {
        return mSecurityLevel;
    }
    void token(const char *token);
    const std::string &token()
    {
        return mToken;
    }
    CBASE_tProcId pid() const
    {
        return mPid;
    }
    void pid(CBASE_tProcId pid)
    {
        mPid = pid;
    }
    bool hostIp(std::string &host_ip);
    bool peerIp(std::string &host_ip);

    const std::string &getEndpointName() const;

    bool connected(const CFdbSocketAddr &addr);
    bool bound(const CFdbSocketAddr &addr);
    EFdbQOS qos();
    void scanLifeTime();
    const std::string &getUDPDestAddr(int32_t &port) const
    {
        port = mPeerUDPPort;
        return mPeerUDPIp;
    }
    void getUDPDestAddr(tFdbIpV4 &peer_ip, int32_t &peer_port) const
    {
        peer_ip = mPeerUDPIpDigit;
        peer_port = mPeerUDPPort;
    }
    void setUDPDestAddr(const char *ip_addr, int32_t port);
protected:
    void onInput();
    int32_t onInputReady();
    void onError();
    void onHup();
    void doSubscribeReq(CFdbMessageHeader &head, bool subscribe);
    int32_t writeStream(const uint8_t *data, int32_t size);
private:
    int32_t mSecurityLevel;
    std::string mToken;
    int32_t mRecursiveDepth;
    CBASE_tProcId mPid;
    std::string mPeerUDPIp;
    tFdbIpV4 mPeerUDPIpDigit;
    int32_t mPeerUDPPort;

    bool receiveData(uint8_t *buf, int32_t size);
};

}
}
#endif
