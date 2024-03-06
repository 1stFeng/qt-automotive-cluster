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

#ifndef __CGENERICCLIENTSOCKET_H__
#define __CGENERICCLIENTSOCKET_H__

#include <fdbus/CGenericSocket.h>
#include <fdbus/CMethodLoopTimer.h>

namespace ipc {
namespace fdbus {
class CGenericSession;
class CBaseWorker;
class CSocketImp;

class CGenericClientSocket : public CGenericSocket
{
public:
    CGenericClientSocket(ISocketEventHandle *event_handle = 0);
    FdbScktSessionPtr &connect(const char *url, CBaseWorker *worker = 0);
 
    FdbScktSessionPtr &findSession(CGenericSession *session);
    void unregisterSession(CGenericSession *session);
    void onPeerOffline(FdbScktSessionPtr &session);
    void enableReconnect(bool enb)
    {
        mEnableReconnect = enb;
    }
protected:
    CGenericSession *onCreateSession(CSocketImp *sock_imp);
private:
    FdbScktSessionPtr mSession;
    CClientSocketImp *mSocketImp;
    class CConnectTimer : public CMethodLoopTimer<CGenericClientSocket>
    {
    public:
        CConnectTimer(CGenericClientSocket *client);
    };
    CConnectTimer mConnectTimer;
    CBaseWorker *mWorker;
    bool mEnableReconnect;

    FdbScktSessionPtr &registerSession(CGenericSession *session);
    void onConnectTimer(CMethodLoopTimer<CGenericClientSocket> *timer);
    FdbScktSessionPtr &doConnect();
};

}
}
#endif
