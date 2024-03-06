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

#ifndef __CGENERICUDPSOCKET_H__
#define __CGENERICUDPSOCKET_H__

#include <fdbus/CGenericSocket.h>

namespace ipc {
namespace fdbus {
class CBaseWorker;
class CGenericSession;
class CSocketImp;

class CGenericUdpSocket : public CGenericSocket
{
public:
    CGenericUdpSocket(ISocketEventHandle *event_handle = 0)
        : CGenericSocket(event_handle)
        , mSocketImp(0)
    {}
    bool connect(const char *url);
    FdbScktSessionPtr &bind(const char *url = 0, CBaseWorker *worker = 0);

    FdbScktSessionPtr &findSession(CGenericSession *session);
    void unregisterSession(CGenericSession *session);
protected:
    CGenericSession *onCreateSession(CSocketImp *sock_imp);
private:
    FdbScktSessionPtr mSession;
    CUDPSocketImp *mSocketImp;

    FdbScktSessionPtr &registerSession(CGenericSession *session);
};

}
}
#endif
