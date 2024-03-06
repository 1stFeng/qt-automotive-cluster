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

#ifndef _CBASECLIENT_H_
#define _CBASECLIENT_H_

#include <string>
#include <functional>
#include "CFdbSessionContainer.h"
#include "common_defs.h"
#include "CBaseEndpoint.h"
#include "CMethodJob.h"

namespace ipc {
namespace fdbus {
class CBaseClient;
class CBaseWorker;
class CFdbSession;
class CFdbBaseContext;
class FdbMsgAddressList;

class CClientSocket : public CFdbSessionContainer
{
public:
    CClientSocket(CBaseClient *owner
                  , FdbSocketId_t skid
                  , CClientSocketImp *socket
                  , const char *host_name
                  , int32_t udp_port
                  , bool secure);
    ~CClientSocket();
    CFdbSession *connect();
    void setSocket(CClientSocketImp *skt)
    {
        mSocket = skt;
    }
    CClientSocketImp *getSocket() const
    {
        return fdb_dynamic_cast_if_available<CClientSocketImp *>(mSocket);
    }

    const std::string &connectedHost() const
    {
        return mConnectedHost;
    }

    void connectedHost(const char *host_name)
    {
        mConnectedHost = host_name;
    }
    
    void disconnect();
protected:
    void onSessionDeleted(CFdbSession *session);
private:
    std::string mConnectedHost;
};

enum ConnectStatus
{
    CONNECTED,
    CONNECTING,
    TIMEOUT,
    FAILURE
};

struct CConnectionInfo
{
    const std::string *mServiceName;
    FdbInstanceId_t mInstanceId;
    const std::string *mEndpointName;
    const std::string *mHostName;
    const std::string *mDomainName;
    bool mIsLocal;
};

typedef std::function<bool(const CConnectionInfo &info)> tConnectionEnableFn;

class CBaseClient : public CBaseEndpoint
{
public:
    CBaseClient(const char *name = 0, CBaseWorker *worker = 0, CFdbBaseContext *context = 0);
    virtual ~CBaseClient();
    /*
     * Connect a client to an address
     * @iparam client: the client to be connected with the address
     * @iparam address: the address to be connected; if not specified,
     *     svc://server_name is used, where server_name is the string passed
     *     when creating CBaseClient
     * @return: the session the client is established with server. To check
     *     the return value, using isValidFdbId();
     *     Note that for "SVC://", isValidFdbId() always return false
     *
     * The supported address format is:
     * tcp://ip address:port number
     * ipc://directory to unix domain socket
     * svc://server name: own server name and get address dynamically
     *     allocated by name server
     */

    ConnectStatus connect(const char *url = 0, int32_t timeout = -1);
    /*
     * Disconnect the client with server.
     * @iparam sid: don't specify for now
     */
    void disconnect(FdbSessionId_t sid = FDB_INVALID_ID);

    bool local()
    {
        return mIsLocal;
    }

    bool hostConnected(const char *host_name);

    void prepareDestroy();

    const std::string *token() const
    {
        // for client, actually only one token is available
        auto it = mTokens.begin();
        if (it != mTokens.end())
        {
            return &(*it);
        }
        return 0;
    }
    void setConnectionEnableCallback(tConnectionEnableFn callback)
    {
        mConnectionEnableFn = callback;
    }
protected:
    CClientSocket *doConnect(const char *url, const char *host_name = 0, 
                             int32_t udp_port = FDB_INET_PORT_INVALID);
    void doDisconnect(FdbSessionId_t sid = FDB_INVALID_ID);
    /*
     * Check whether connection is allowed for the host.
     * Warning!!! It is running in the context of FDB_CONTEXT!!!
     */
    virtual bool connectionEnabled(const CConnectionInfo &conn_info)
    {
        return mConnectionEnableFn ? mConnectionEnableFn(conn_info) : true;
    }

private:
    bool mIsLocal;
    typedef std::list<CBaseJob::Ptr> tConnectJobList;
    tConnectJobList mConnectJobList;
    tConnectionEnableFn mConnectionEnableFn;

    void cbConnect(CBaseWorker *worker, CMethodJob<CBaseClient> *job, CBaseJob::Ptr &ref);
    void cbDisconnect(CBaseWorker *worker, CMethodJob<CBaseClient> *job, CBaseJob::Ptr &ref);

    void local(bool is_local)
    {
        mIsLocal = is_local;
    }
    void updateSecurityLevel(CFdbSession *session);
    void notifyConnectReady(ConnectStatus status);

    friend class CFdbContext;
    friend class CConnectClientJob;
    friend class CDisconnectClientJob;
    friend class CClientSocket;
    friend class CIntraNameProxy;
    friend class CBaseHostProxy;
    friend class CIntraHostProxy;
    friend class CNameServer;
    friend class CDataPool;
};

}
}
#endif
