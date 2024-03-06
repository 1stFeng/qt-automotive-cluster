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

#ifndef _CBASESOCKETFACTORY_H_
#define _CBASESOCKETFACTORY_H_

#include <list>
#include <string>
#include "CSocketImp.h"

namespace ipc {
namespace fdbus {
struct CFdbInterfaceTable
{
public:
    struct IfDesc
    {
        std::string mIpAddr;
        std::string mMask;
        std::string mIfName;
    };
    typedef std::list<IfDesc> tIpAddressTbl;
    tIpAddressTbl mAddrTbl;
    const IfDesc *findByIp(const char *ip_addr) const
    {
        for (auto it = mAddrTbl.begin(); it != mAddrTbl.end(); ++it)
        {
            if (it->mIpAddr == ip_addr)
            {
                return &(*it);
            }
        }
        return 0;
    }
    const IfDesc *findByName(const char *name) const
    {
        for (auto it = mAddrTbl.begin(); it != mAddrTbl.end(); ++it)
        {
            if (it->mIfName == name)
            {
                return &(*it);
            }
        }
        return 0;
    }
};

class CBaseSocketFactory
{
public:
    static CClientSocketImp *createClientSocket(CFdbSocketAddr &addr);
    static CClientSocketImp *createClientSocket(const char *url);
    static CServerSocketImp *createServerSocket(CFdbSocketAddr &addr);
    static CServerSocketImp *createServerSocket(const char *url);
    static CUDPSocketImp *createUDPSocket(CFdbSocketAddr &addr);
    static CUDPSocketImp *createUDPSocket(const char *url);
    static bool parseUrl(const char *url, CFdbSocketAddr &addr);
    static bool getIpAddress(CFdbInterfaceTable &addr_tbl);
    static void buildUrl(std::string &url, const char *ip_addr, const char *port, bool secure);
    static void buildUrl(std::string &url, const char *ip_addr, int32_t port, bool secure);
    static void buildUrl(std::string &url, tFdbIpV4 ip_addr, int32_t port, bool secure, EFdbSocketType type);
    static void buildUrl(std::string &url, uint32_t uds_id, const char *ipc_path = 0);
    static void buildUrl(std::string &url, const char *svc_name,
                         FdbInstanceId_t instance_id = FDB_DEFAULT_INSTANCE);
    static bool sameSubnet(const char *ip1, const char *ip2, const char *mask);
    static void *createSSL(const char *public_key,
                           const char *private_key,
                           const char *root_ca,
                           uint32_t verify_type,
                           bool is_client);
    static bool parseIp(const char* ip, tFdbIpV4 &int_ip);
    static bool isValidIpAddr(const char *addr);

private:
    static int32_t buildINetAddress(const char *host_addr, CFdbSocketAddr &addr);
    static int32_t buildIPCAddress(const char *addr_str, CFdbSocketAddr &addr);
    static int32_t buildSvcAddress(const char *host_name, CFdbSocketAddr &addr);
};

}
}
#endif
