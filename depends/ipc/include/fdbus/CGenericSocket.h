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

#ifndef __CGENERICSOCKET_H__
#define __CGENERICSOCKET_H__

#include <fdbus/CSocketImp.h>
#include <memory>

namespace ipc {
namespace fdbus {
class CGenericSession;
typedef std::shared_ptr<CGenericSession> FdbScktSessionPtr;

class ISocketEventHandle
{
public:
    virtual ~ISocketEventHandle()
    {}
    virtual void onPeerInput(FdbScktSessionPtr &session)
    {}
    virtual void onPeerOutput(FdbScktSessionPtr &session)
    {}
    virtual void onPeerOnline(FdbScktSessionPtr &session)
    {}
    virtual void onPeerOffline(FdbScktSessionPtr &session)
    {}
    virtual void onPeerInputReady(FdbScktSessionPtr &session)
    {}
};

class CGenericSocket : public ISocketEventHandle
{
public:
    CGenericSocket(ISocketEventHandle *event_handle = 0)
        : mSocketEventHandle(event_handle)
    {}
    void setEventHandle(ISocketEventHandle *event_handle)
    {
        mSocketEventHandle = event_handle;
    }
    bool createSSL(const char *public_key,
                   const char *private_key,
                   const char *root_ca,
                   uint32_t verify_type);
    void config(bool block_mode, int32_t ka_interval, int32_t ka_retry);
    virtual FdbScktSessionPtr &findSession(CGenericSession *session)
    {
        return mEmptyPtr;
    }
    virtual void unregisterSession(CGenericSession *session)
    {}
    void onPeerInput(FdbScktSessionPtr &session)
    {
        if (mSocketEventHandle)
        {
            mSocketEventHandle->onPeerInput(session);
        }
    }
    void onPeerOutput(FdbScktSessionPtr &session)
    {
        if (mSocketEventHandle)
        {
            mSocketEventHandle->onPeerOutput(session);
        }
    }
    void onPeerOnline(FdbScktSessionPtr &session)
    {
        if (mSocketEventHandle)
        {
            mSocketEventHandle->onPeerOnline(session);
        }
    }
    void onPeerOffline(FdbScktSessionPtr &session)
    {
        if (mSocketEventHandle)
        {
            mSocketEventHandle->onPeerOffline(session);
        }
    }
    void onPeerInputReady(FdbScktSessionPtr &session)
    {
        if (mSocketEventHandle)
        {
            mSocketEventHandle->onPeerInputReady(session);
        }
    }
protected:
    CBaseSocket::CSocketParams mScktParams;
    static FdbScktSessionPtr mEmptyPtr;

    virtual CGenericSession *onCreateSession(CSocketImp *sock_imp)
    {
        return 0;
    }
private:
    ISocketEventHandle *mSocketEventHandle;
};

}
}
#endif
