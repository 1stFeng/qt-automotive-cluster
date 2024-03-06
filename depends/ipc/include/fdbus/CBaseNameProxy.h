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

#ifndef _CBASENAMEPROXY_H_
#define _CBASENAMEPROXY_H_

#include <fdbus/common_defs.h>
#include <fdbus/CBaseClient.h>
#include <fdbus/CFdbIfNameServer.h>
#include <fdbus/CBaseJob.h>
#include <fdbus/CMethodLoopTimer.h>

namespace ipc {
namespace fdbus {
class CFdbSession;

enum SubscribeType
{
    INTRA_NORMAL = FDB_CUSTOM_OBJECT_BEGIN,   //NTF_SERVICE_ONLINE
    INTRA_MONITOR,  //NTF_SERVICE_ONLINE_MONITOR
    INTER_NORMAL,   //NTF_SERVICE_ONLINE_INTER_MACHINE
    INTER_MONITOR,   //NTF_SERVICE_ONLINE_MONITOR_INTER_MACHINE

    MORE_ADDRESS, //NTF_MORE_ADDRESS
};

class CBaseNameProxy : public CBaseClient
{
public:
    CBaseNameProxy();

    void enableNsMonitor(bool enb)
    {
        mEnableReconnectToNS = enb;
    }

    const std::string &getNsUrl() const
    {
        return mNsUrl;
    }

    bool connectToNameServer();

protected:
    bool mEnableReconnectToNS;
    class CConnectTimer : public CMethodLoopTimer<CBaseNameProxy>
    {
    public:
        CConnectTimer(CBaseNameProxy *proxy);
        void fire();
    };
    CConnectTimer mConnectTimer;

    virtual void validateUrl(FdbMsgAddressList &msg_addr_list, CFdbSession *session) {}

    void onConnectTimer(CMethodLoopTimer<CBaseNameProxy> *timer);

    void onOnline(const CFdbOnlineInfo &info);
    void onOffline(const CFdbOnlineInfo &info);

    std::string mNsUrl;
};
}
}
#endif
