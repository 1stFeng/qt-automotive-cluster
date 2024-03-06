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

#ifndef __CGENERICSERVERSOCKET_H__
#define __CGENERICSERVERSOCKET_H__

#include <mutex>
#include <list>
#include <vector>
#include "CBaseFdWatch.h"
#include <fdbus/CGenericSocket.h>

namespace ipc {
namespace fdbus {

class CGenericSession;
class CBaseWorker;
class CSocketImp;

class CGenericServerSocket : public CBaseFdWatch, public CGenericSocket
{
public:
    typedef std::vector<FdbScktSessionPtr> tSessionContainer;

    CGenericServerSocket(ISocketEventHandle *event_handle = 0);
    bool bind(const char *url, CBaseWorker *worker = 0);
    void getConnectedSessions(CGenericServerSocket::tSessionContainer &sessions);

    FdbScktSessionPtr &findSession(CGenericSession *session);
    void unregisterSession(CGenericSession *session);
protected:
    void onInput();
    CGenericSession *onCreateSession(CSocketImp *sock_imp);
private:
    CServerSocketImp *mSocketImp;
    typedef std::list<FdbScktSessionPtr> tSessionTbl;
    tSessionTbl mSessionTbl;
    std::mutex mSessionTblLock;

    FdbScktSessionPtr &registerSession(CGenericSession *session);
};

}
}

#endif


