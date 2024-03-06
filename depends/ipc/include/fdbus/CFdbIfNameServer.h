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

#ifndef __CFDINTERFACENAMESERVER_H__
#define __CFDINTERFACENAMESERVER_H__

#include <string>
#include <list>
#include <fdbus/CFdbSimpleSerializer.h>
#include <fdbus/CFdbIfMsgTokens.h>
#include <fdbus/CBaseSysDep.h>
#include <fdbus/CFdbToken.h>
#include <fdbus/CSocketImp.h>

namespace ipc {
namespace fdbus {

enum FdbNsMsgCode
{
    REQ_ALLOC_SERVICE_ADDRESS = 0,
    REQ_REGISTER_SERVICE = 1,
    REQ_UNREGISTER_SERVICE = 2,

    REQ_QUERY_SERVICE = 3,
    REQ_QUERY_SERVICE_INTER_MACHINE = 4,

    REQ_QUERY_HOST_LOCAL = 5,
    REQ_NS_QUERY_EXPORTABLE_SERVICE = 6,

    NTF_HOST_ONLINE_LOCAL = 1,
    NTF_HOST_INFO = 2,

    NTF_WATCHDOG = 3
};

enum FdbHsMsgCode
{
    REQ_REGISTER_LOCAL_HOST = 0,
    REQ_UNREGISTER_LOCAL_HOST = 1,
    REQ_QUERY_HOST = 2,
    REQ_HEARTBEAT_OK = 3,
    REQ_LOCAL_HOST_READY = 4,
    REQ_EXTERNAL_HOST_READY = 5,
    REQ_POST_LOCAL_SVC_ADDRESS = 7,
    REQ_POST_DOMAIN_SVC_ADDRESS = 8,
    REQ_REGISTER_EXTERNAL_DOMAIN = 9,
    REQ_UNREGISTER_EXTERNAL_DOMAIN = 10,
    REQ_HS_QUERY_EXPORTABLE_SERVICE = 11,

    NTF_HOST_ONLINE = 0,
    NTF_HEART_BEAT = 1,
    NTF_EXPORT_SVC_ADDRESS_EXTERNAL = 2,
    NTF_EXPORT_SVC_ADDRESS_INTERNAL = 3,
};

class FdbMsgAddressItem : public IFdbParcelable
{
public:
    FdbMsgAddressItem()
        : mIsSecure(false)
        , mOptions(0)
    {
    }
    std::string &tcp_ipc_address()
    {
        return mTCPIPCAddress;
    }
    void set_tcp_ipc_address(const char *addr)
    {
        mTCPIPCAddress = addr;
    }
    void set_tcp_ipc_address(const std::string &addr)
    {
        mTCPIPCAddress = addr;
    }
    int32_t tcp_port() const
    {
        return mTCPPort;
    }
    void set_tcp_port(int32_t port)
    {
        mTCPPort = port;
    }
    EFdbSocketType address_type()
    {
        return mType;
    }
    void set_address_type(EFdbSocketType type)
    {
        mType = type;
    }
    std::string &tcp_ipc_url()
    {
        return mTCPIPCUrl;
    }
    void set_tcp_ipc_url(const char *url)
    {
        mTCPIPCUrl = url;
    }
    void set_tcp_ipc_url(const std::string &url)
    {
        mTCPIPCUrl = url;
    }
    int32_t udp_port() const
    {
        return mUDPPort;
    }
    void set_udp_port(int32_t port)
    {
        mUDPPort = port;
        mOptions |= mMaskHasUDPPort;
    }
    bool has_udp_port() const
    {
        return !!(mOptions & mMaskHasUDPPort);
    }
    bool is_secure() const
    {
        return mIsSecure;
    }
    void set_secure(bool secure)
    {
        mIsSecure = secure;
    }
    void fromSocketAddress(const CFdbSocketAddr &sckt_addr)
    {
        mTCPIPCAddress = sckt_addr.mAddr;
        mTCPPort = sckt_addr.mPort;
        mType = sckt_addr.mType;
        mTCPIPCUrl = sckt_addr.mUrl;
        mIsSecure = sckt_addr.mSecure;
    }
    void toSocketAddress(CFdbSocketAddr &sckt_addr)
    {
        sckt_addr.mAddr = mTCPIPCAddress;
        sckt_addr.mPort = mTCPPort;
        sckt_addr.mType = mType;
        sckt_addr.mUrl = mTCPIPCUrl;
        sckt_addr.mSecure = mIsSecure;
    }
    int32_t exportable_level()
    {
        return mExportableLevel;
    }
    void set_exportable_level(int32_t level)
    {
        mExportableLevel = level;
    }
    int32_t interface_id()
    {
        return mInterfaceId;
    }
    void set_interface_id(int32_t level)
    {
        mInterfaceId = level;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mTCPIPCAddress
                   << mTCPPort
                   << (uint8_t)mType
                   << mTCPIPCUrl
                   << mIsSecure
                   << mExportableLevel
                   << mInterfaceId
                   << mOptions;
        if (mOptions & mMaskHasUDPPort)
        {
            serializer << mUDPPort;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        uint8_t type;
        deserializer >> mTCPIPCAddress
                     >> mTCPPort
                     >> type
                     >> mTCPIPCUrl
                     >> mIsSecure
                     >> mExportableLevel
                     >> mInterfaceId
                     >> mOptions;
        mType = (EFdbSocketType)type;
        if (mOptions & mMaskHasUDPPort)
        {
            deserializer >> mUDPPort;
        }
    }
private:
    std::string mTCPIPCAddress;
    int32_t mTCPPort;
    EFdbSocketType mType;
    std::string mTCPIPCUrl;
    int32_t mUDPPort;
    bool mIsSecure;
    int32_t mExportableLevel;
    int32_t mInterfaceId;
    uint8_t mOptions;
        static const uint8_t mMaskHasUDPPort = 1 << 0;
};

class FdbMsgAddressList : public IFdbParcelable
{
public:
    FdbMsgAddressList()
        : mIsLocal(true)
        , mOptions(0)
    {}
    std::string &service_name()
    {
        return mServiceName;
    }
    void set_service_name(const std::string &name)
    {
        mServiceName = name;
    }
    void set_service_name(const char *name)
    {
        mServiceName = name;
    }
    std::string &host_name()
    {
        return mHostName;
    }
    void set_host_name(const std::string &name)
    {
        mHostName = name;
    }
    void set_host_name(const char *name)
    {
        mHostName = name;
    }
    std::string &endpoint_name()
    {
        return mEndpointName;
    }
    void set_endpoint_name(const std::string &name)
    {
        mEndpointName = name;
    }
    void set_endpoint_name(const char *name)
    {
        mEndpointName = name;
    }
    std::string &domain_name()
    {
        return mDomainName;
    }
    void set_domain_name(const std::string &name)
    {
        mDomainName = name;
    }
    void set_domain_name(const char *name)
    {
        mDomainName = name;
    }
    FdbInstanceId_t instance_id()
    {
        return mInstanceId;
    }
    void set_instance_id(FdbInstanceId_t id)
    {
        mInstanceId = id;
    }
    bool is_local() const
    {
        return mIsLocal;
    }
    void set_is_local(bool local)
    {
        mIsLocal = local;
    }
    CBASE_tProcId pid() const
    {
        return (CBASE_tProcId)mPid;
    }
    void set_pid(CBASE_tProcId pid)
    {
        mPid = pid;
    }
    CFdbParcelableArray<FdbMsgAddressItem> &address_list()
    {
        return mAddressList;
    }
    FdbMsgAddressItem *add_address_list()
    {
        return mAddressList.Add();
    }
    FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }

    void populateTokens(const CFdbToken::tTokenList &tokens)
    {
        mTokenList.clear_tokens();
        mOptions |= mMaskTokenList;
        for (auto it = tokens.begin(); it != tokens.end(); ++it)
        {
            mTokenList.add_tokens(*it);
        }
    }

    void dumpTokens(CFdbToken::tTokenList &tokens)
    {
        const auto &t = mTokenList.tokens();
        for (auto it = t.pool().begin(); it != t.pool().end(); ++it)
        {
            tokens.push_back(*it);
        }
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceName
                   << mHostName
                   << mEndpointName
                   << mDomainName
                   << mInstanceId
                   << mIsLocal
                   << (uint32_t)mPid
                   << mAddressList
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        uint32_t pid;
        deserializer >> mServiceName
                     >> mHostName
                     >> mEndpointName
                     >> mDomainName
                     >> mInstanceId
                     >> mIsLocal
                     >> pid
                     >> mAddressList
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
        mPid = (CBASE_tProcId)pid;
    }
private:
    std::string mServiceName;
    std::string mHostName;
    std::string mEndpointName;
    std::string mDomainName;
    FdbInstanceId_t mInstanceId;
    bool mIsLocal;
    CBASE_tProcId mPid;
    CFdbParcelableArray<FdbMsgAddressItem> mAddressList;
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbAddrBindStatus : public IFdbParcelable
{
public:
    const std::string &request_address() const
    {
        return mRequestAddr;
    }
    void request_address(const std::string &addr)
    {
        mRequestAddr = addr;
    }
    const std::string &bind_address() const
    {
        return mBindAddr;
    }
    void bind_address(const std::string &addr)
    {
        mBindAddr = addr;
    }
    int32_t udp_port() const
    {
        return mUDPPort;
    }
    void set_udp_port(int32_t port)
    {
        mUDPPort = port;
    }
protected:
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mRequestAddr << mBindAddr << mUDPPort;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mRequestAddr >> mBindAddr >> mUDPPort;
    }

private:
    std::string mRequestAddr;
    std::string mBindAddr;
    int32_t mUDPPort;
};

class FdbMsgAddrBindResults : public IFdbParcelable 
{
public:
    const std::string &service_name() const
    {
        return mServiceName;
    }
    void set_service_name(const std::string &name)
    {
        mServiceName = name;
    }
    CFdbParcelableArray<FdbAddrBindStatus> &address_list()
    {
        return mAddrList;
    }
    FdbAddrBindStatus *add_address_list()
    {
        return mAddrList.Add();
    }
    FdbInstanceId_t instance_id() const
    {
        return mInstanceId;
    }

    void set_instance_id(FdbInstanceId_t id)
    {
        mInstanceId = id;
    }
protected:
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceName
                   << mAddrList
                   << mInstanceId;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceName
                     >> mAddrList
                     >> mInstanceId;
    }
private:
    std::string mServiceName;
    CFdbParcelableArray<FdbAddrBindStatus> mAddrList;
    FdbInstanceId_t mInstanceId;
};

class FdbMsgServerName : public IFdbParcelable
{
public:
    FdbMsgServerName()
        : mFlag(0)
        , mExportableLevel(FDB_EXPORTABLE_NODE_INTERNAL)
        , mInstanceId(FDB_DEFAULT_INSTANCE)
    {
    }
#define FDB_NS_LOCAL_ONLY               (1 << 0)
#define FDB_NS_ALLOW_TCP_NORMAL         (1 << 1)
#define FDB_NS_ALLOW_TCP_SECURE         (1 << 2)
    std::string &name()
    {
        return mName;
    }
    void set_name(const std::string &n)
    {
        mName = n;
    }
    void set_name(const char *n)
    {
        mName = n;
    }
    std::string &endpoint_name()
    {
        return mEndpointName;
    }
    void set_endpoint_name(const std::string &n)
    {
        mEndpointName = n;
    }
    void set_endpoint_name(const char *n)
    {
        mEndpointName = n;
    }

    bool local_only() const
    {
        return !!(mFlag & FDB_NS_LOCAL_ONLY);
    }
    void set_local_only(bool local)
    {
        if (local)
        {
            mFlag |= FDB_NS_LOCAL_ONLY;
        }
        else
        {
            mFlag &= ~FDB_NS_LOCAL_ONLY;
        }
    }

    bool allow_tcp_normal() const
    {
        return !!(mFlag & FDB_NS_ALLOW_TCP_NORMAL);
    }
    void set_allow_tcp_normal(bool enable)
    {
        if (enable)
        {
            mFlag |= FDB_NS_ALLOW_TCP_NORMAL;
        }
        else
        {
            mFlag &= ~FDB_NS_ALLOW_TCP_NORMAL;
        }
    }

    bool allow_tcp_secure() const
    {
        return !!(mFlag & FDB_NS_ALLOW_TCP_SECURE);
    }
    void set_allow_tcp_secure(bool enable)
    {
        if (enable)
        {
            mFlag |= FDB_NS_ALLOW_TCP_SECURE;
        }
        else
        {
            mFlag &= ~FDB_NS_ALLOW_TCP_SECURE;
        }
    }
    int32_t exportable_level() const
    {
        return mExportableLevel;
    }
    void set_exportable_level(int32_t level)
    {
        mExportableLevel = level;
    }

    uint8_t flag() const
    {
        return mFlag;
    }

    FdbInstanceId_t instance_id() const
    {
        return mInstanceId;
    }

    void set_instance_id(FdbInstanceId_t id)
    {
        mInstanceId = id;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mName
                   << mFlag
                   << mEndpointName
                   << mExportableLevel
                   << mInstanceId;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mName
                     >> mFlag
                     >> mEndpointName
                     >> mExportableLevel
                     >> mInstanceId;
    }
private:
    std::string mName;
    std::string mEndpointName;
    uint8_t mFlag;
    int32_t mExportableLevel;
    FdbInstanceId_t mInstanceId;
};

class FdbMsgExportableSvcAddress : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgAddressList> &svc_address_list()
    {
        return mSvcAddrList;
    }
    FdbMsgAddressList *add_svc_address_list()
    {
        return mSvcAddrList.Add();
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mSvcAddrList;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mSvcAddrList;
    }
private:
    CFdbParcelableArray<FdbMsgAddressList> mSvcAddrList;
};

class FdbMsgHostRegisterReq : public IFdbParcelable
{
public:
    FdbMsgHostRegisterReq()
        : mOptions(0)
    {}
    std::string &ip_address()
    {
        return mIpAddress;
    }
    void set_ip_address(const std::string &address)
    {
        mIpAddress = address;
    }
    void set_ip_address(const char *address)
    {
        mIpAddress = address;
    }
    std::string &host_name()
    {
        return mHostName;
    }
    void set_host_name(const std::string &name)
    {
        mHostName = name;
    }
    void set_host_name(const char *name)
    {
        mHostName = name;
    }
    void set_cred(const char *cred)
    {
        mCred = cred;
        mOptions |= mMaskCred;
    }
    std::string &cred()
    {
        return mCred;
    }
    bool has_cred() const
    {
        return !!(mOptions & mMaskCred);
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mIpAddress
                   << mHostName
                   << mOptions;
        if (mOptions & mMaskCred)
        {
            serializer << mCred;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mIpAddress
                     >> mHostName
                     >> mOptions;
        if (mOptions & mMaskCred)
        {
            deserializer >> mCred;
        }
    }
private:
    std::string mIpAddress;
    std::string mHostName;
    std::string mCred;
    uint8_t mOptions;
        static const uint8_t mMaskCred = 1 << 0;
};

class FdbMsgHostAddress : public IFdbParcelable
{
public:
    FdbMsgHostAddress()
        : mOptions(0)
    {}
    std::string &ip_address()
    {
        return mIpAddress;
    }
    void set_ip_address(const std::string &address)
    {
        mIpAddress = address;
    }
    void set_ip_address(const char *address)
    {
        mIpAddress = address;
    }
    CFdbParcelableArray<FdbMsgAddressItem> &address_list()
    {
        return mAddressList;
    }
    FdbMsgAddressItem *add_address_list()
    {
        return mAddressList.Add();
    }
    std::string &host_name()
    {
        return mHostName;
    }
    void set_host_name(const std::string &name)
    {
        mHostName = name;
    }
    void set_host_name(const char *name)
    {
        mHostName = name;
    }
    FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }
    FdbMsgExportableSvcAddress &export_svc_address()
    {
        return mSvcAddrList;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mIpAddress
                   << mHostName
                   << mAddressList
                   << mSvcAddrList
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mIpAddress
                     >> mHostName
                     >> mAddressList
                     >> mSvcAddrList
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    std::string mIpAddress;
    std::string mHostName;
    CFdbParcelableArray<FdbMsgAddressItem> mAddressList;
    FdbMsgTokens mTokenList;
    FdbMsgExportableSvcAddress mSvcAddrList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgDomainRegisterReq : public IFdbParcelable
{
public:
    FdbMsgDomainRegisterReq()
        : mOptions(0)
    {}
    std::string &domain_name()
    {
        return mDomainName;
    }
    void set_domain_name(const std::string &name)
    {
        mDomainName = name;
    }
    void set_domain_name(const char *name)
    {
        mDomainName = name;
    }
    void set_cred(const char *cred)
    {
        mCred = cred;
        mOptions |= mMaskCred;
    }
    std::string &cred()
    {
        return mCred;
    }
    bool has_cred() const
    {
        return !!(mOptions & mMaskCred);
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mDomainName
                   << mOptions;
        if (mOptions & mMaskCred)
        {
            serializer << mCred;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mDomainName 
                     >> mOptions;
        if (mOptions & mMaskCred)
        {
            deserializer >> mCred;
        }
    }
private:
    std::string mDomainName;
    std::string mCred;
    uint8_t mOptions;
        static const uint8_t mMaskCred = 1 << 0;
};


class FdbMsgDomainAddress : public IFdbParcelable
{
public:
    FdbMsgDomainAddress()
        : mOptions(0)
    {}
    FdbMsgExportableSvcAddress &export_svc_address()
    {
        return mSvcAddrList;
    }
    std::string &domain_name()
    {
        return mDomainName;
    }
    void set_domain_name(const std::string &name)
    {
        mDomainName = name;
    }
    void set_domain_name(const char *name)
    {
        mDomainName = name;
    }
    FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mDomainName
                   << mSvcAddrList
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mDomainName 
                     >> mSvcAddrList
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    std::string mDomainName;
    FdbMsgExportableSvcAddress mSvcAddrList;
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgHostRegisterAck : public IFdbParcelable
{
public:
    FdbMsgHostRegisterAck()
        : mOptions(0)
    {}
    std::string &domain_name()
    {
        return mDomainName;
    }
    void set_domain_name(const char *dn)
    {
        if (dn)
        {
            mDomainName = dn;
        }
    }
    void set_domain_name(const std::string &dn)
    {
        mDomainName = dn;
    }
    FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mDomainName
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mDomainName
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    std::string mDomainName;
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgHostInfo : public IFdbParcelable
{
public:
    std::string &name()
    {
        return mName;
    }
    void set_name(const std::string &n)
    {
        mName = n;
    }
    void set_name(const char *n)
    {
        mName = n;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mName;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mName;
    }
private:
    std::string mName;
};

class FdbMsgHostAddressList : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgHostAddress> &address_list()
    {
        return mAddressList;
    }
    FdbMsgHostAddress *add_address_list()
    {
        return mAddressList.Add();
    }
    FdbMsgExportableSvcAddress &export_svc_address()
    {
        return mSvcAddrList;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mAddressList
                   << mSvcAddrList;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mAddressList
                     >> mSvcAddrList;
    }
private:
    CFdbParcelableArray<FdbMsgHostAddress> mAddressList;
    FdbMsgExportableSvcAddress mSvcAddrList;
};

class FdbMsgServiceInfo : public IFdbParcelable
{
public:
    FdbMsgAddressList &service_addr()
    {
        return mServiceAddr;
    }
    FdbMsgHostAddress &host_addr()
    {
        return mHostAddr;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceAddr
                   << mHostAddr;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceAddr
                     >> mHostAddr;
    }
private:
   FdbMsgAddressList mServiceAddr;
   FdbMsgHostAddress mHostAddr;
};

class FdbMsgServiceTable : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgServiceInfo> &service_tbl()
    {
        return mServiceTbl;
    }
    FdbMsgServiceInfo *add_service_tbl()
    {
        return mServiceTbl.Add();
    }

    FdbMsgExportableSvcAddress &exported_service_tbl()
    {
        return mExportedSvcTbl;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceTbl
                   << mExportedSvcTbl
                   ;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceTbl
                     >> mExportedSvcTbl
                     ;
    }
private:
    CFdbParcelableArray<FdbMsgServiceInfo> mServiceTbl;
    FdbMsgExportableSvcAddress mExportedSvcTbl;
};

class FdbMsgStatisticsData : public IFdbParcelable
{
public:
    FdbMsgStatisticsData()
        : mSyncRequest(0)
        , mSyncReply(0)
        , mAsyncRequest(0)
        , mAsyncReply(0)
        , mBroadcast(0)
        , mPublish(0)
        , mSend(0)
        , mGetEvent(0)
        , mReturnEvent(0)
        , mSyncStatus(0)
        , mAsyncStatus(0)
    {}
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mSyncRequest
                   << mSyncReply
                   << mAsyncRequest
                   << mAsyncReply
                   << mBroadcast
                   << mPublish
                   << mSend
                   << mGetEvent
                   << mReturnEvent
                   << mSyncStatus
                   << mAsyncStatus
                   ;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mSyncRequest
                     >> mSyncReply
                     >> mAsyncRequest
                     >> mAsyncReply
                     >> mBroadcast
                     >> mPublish
                     >> mSend
                     >> mGetEvent
                     >> mReturnEvent
                     >> mSyncStatus
                     >> mAsyncStatus
                     ;
    }
    uint32_t mSyncRequest;
    uint32_t mSyncReply;
    uint32_t mAsyncRequest;
    uint32_t mAsyncReply;
    uint32_t mBroadcast;
    uint32_t mPublish;
    uint32_t mSend;
    uint32_t mGetEvent;
    uint32_t mReturnEvent;
    uint32_t mSyncStatus;
    uint32_t mAsyncStatus;
};

enum FdbMsgDogStatus
{
    FDB_DOG_ST_DIE = -1,
    FDB_DOG_ST_OK = 0,
    FDB_DOG_ST_NON_EXIST = 1
};

class FdbMsgClientInfo : public IFdbParcelable
{
public:
    const std::string &peer_name() const
    {
        return mPeerName;
    }
    void set_peer_name(const char *name)
    {
        mPeerName = name;
    }
    const std::string &peer_address() const
    {
        return mPeerAddress;
    }
    void set_peer_address(const char *address)
    {
        mPeerAddress = address;
    }
    int32_t security_level() const
    {
        return mSecurityLevel;
    }
    void set_security_level(int32_t sec_level)
    {
        mSecurityLevel = sec_level;
    }
    int32_t udp_port() const
    {
        return mUDPPort;
    }
    void set_udp_port(int32_t port)
    {
        mUDPPort = port;
    }
    FdbMsgDogStatus dog_status() const
    {
        return mDogStatus;
    }
    void set_dog_status(FdbMsgDogStatus dog_status)
    {
        mDogStatus = dog_status;
    }
    CBASE_tProcId pid() const
    {
        return (CBASE_tProcId)mPid;
    }
    void set_pid(CBASE_tProcId pid)
    {
        mPid = pid;
    }
    FdbMsgStatisticsData &tcp_tx_statistics()
    {
        return mTcpTxStatistics;
    }
    FdbMsgStatisticsData &tcp_rx_statistics()
    {
        return mTcpRxStatistics;
    }
    EFdbQOS qos() const
    {
        return mQOS;
    }
    void set_qos(EFdbQOS qos)
    {
        mQOS = qos;
    }
    FdbSessionId_t sid() const
    {
        return mSid;
    }
    void set_sid(FdbSessionId_t id)
    {
        mSid = id;
    }
    uint32_t tx_queue_size() const
    {
        return mTxQueueSize;
    }
    void set_tx_queue_size(uint32_t size)
    {
        mTxQueueSize = size;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mPeerName
                   << mPeerAddress
                   << mSecurityLevel
                   << mUDPPort
                   << (int8_t)mDogStatus
                   << (uint32_t)mPid
                   << mTcpTxStatistics
                   << mTcpRxStatistics
                   << (uint8_t)mQOS
                   << (int32_t)mSid
                   << mTxQueueSize
                   ;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        int8_t dog_status;
        uint8_t qos;
        int32_t sid;
        uint32_t pid;
        deserializer >> mPeerName
                     >> mPeerAddress
                     >> mSecurityLevel
                     >> mUDPPort
                     >> dog_status
                     >> pid
                     >> mTcpTxStatistics
                     >> mTcpRxStatistics
                     >> qos
                     >> sid
                     >> mTxQueueSize
                     ;
        mDogStatus = (FdbMsgDogStatus)dog_status;
        mQOS = (EFdbQOS)qos;
        mSid = (FdbSessionId_t)sid;
        mPid = (CBASE_tProcId)pid;
    }
private:
    std::string mPeerName;
    std::string mPeerAddress;
    int32_t mSecurityLevel;
    int32_t mUDPPort;
    FdbMsgDogStatus mDogStatus;
    CBASE_tProcId mPid;
    FdbMsgStatisticsData mTcpTxStatistics;
    FdbMsgStatisticsData mTcpRxStatistics;
    EFdbQOS mQOS;
    FdbSessionId_t mSid;
    uint32_t mTxQueueSize;
};

class FdbMsgClientTable : public IFdbParcelable
{
public:
    FdbMsgClientTable()
        : mContextJobQueueSize(0)
        , mWorkerJobQueueSize(0)
        , mOption(0)
    {
    }
    const std::string &endpoint_name() const
    {
        return mEndpointName;
    }
    void set_endpoint_name(const char *endpoint)
    {
        mEndpointName = endpoint;
    }
    const std::string &server_name() const
    {
        return mServerName;
    }
    void set_server_name(const char *name)
    {
        mServerName = name;
    }
    CFdbParcelableArray<FdbMsgClientInfo> &client_tbl()
    {
        return mClientTbl;
    }
    FdbMsgClientInfo *add_client_tbl()
    {
        return mClientTbl.Add();
    }
    uint32_t context_job_queue_size() const
    {
        return mContextJobQueueSize;
    }
    void set_context_job_queue_size(uint32_t size)
    {
        mContextJobQueueSize = size;
        mOption |= mMaskHasContext;
    }
    bool has_context_job_queue_size() const
    {
        return !!(mOption & mMaskHasContext);
    }
    uint32_t worker_job_queue_size() const
    {
        return mWorkerJobQueueSize;
    }
    void set_worker_job_queue_size(uint32_t size)
    {
        mWorkerJobQueueSize = size;
        mOption |= mMaskHasWorker;
    }
    bool has_worker_job_queue_size() const
    {
        return !!(mOption & mMaskHasWorker);
    }
    FdbInstanceId_t instance_id()
    {
        return mInstanceId;
    }
    void set_instance_id(FdbInstanceId_t id)
    {
        mInstanceId = id;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mEndpointName
                   << mServerName 
                   << mClientTbl
                   << mInstanceId
                   << mOption;
        if (mOption & mMaskHasContext)
        {
            serializer << mContextJobQueueSize;
        }
        if (mOption & mMaskHasWorker)
        {
            serializer << mWorkerJobQueueSize;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mEndpointName
                     >> mServerName 
                     >> mClientTbl
                     >> mInstanceId
                     >> mOption;
        if (mOption & mMaskHasContext)
        {
            deserializer >> mContextJobQueueSize;
        }
        if (mOption & mMaskHasWorker)
        {
            deserializer >> mWorkerJobQueueSize;
        }
    }
private:
    std::string mEndpointName;
    std::string mServerName;
    uint32_t mContextJobQueueSize;
    uint32_t mWorkerJobQueueSize;
    CFdbParcelableArray<FdbMsgClientInfo> mClientTbl;
    FdbInstanceId_t mInstanceId;
    uint8_t mOption;
        static const uint8_t mMaskHasContext = 1 << 0;
        static const uint8_t mMaskHasWorker = 1 << 1;
};

class FdbMsgQueryPeerInfo : public IFdbParcelable
{
public:
    FdbSessionId_t sid() const
    {
        return mSid;
    }
    void set_sid(FdbSessionId_t id)
    {
        mSid = id;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << (int32_t)mSid;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        int32_t sid;
        deserializer >> sid;
        mSid = (FdbSessionId_t)sid;
    }
private:
    FdbSessionId_t mSid;
};

class FdbMsgEventCacheItem : public IFdbParcelable
{
public:
    int32_t event() const
    {
        return mEvent;
    }
    void set_event(int32_t event)
    {
        mEvent = event;
    }
    const std::string &topic() const
    {
        return mTopic;
    }
    void set_topic(const char *topic)
    {
        mTopic = topic;
    }
    int32_t size() const
    {
        return mSize;
    }
    void set_size(int32_t size)
    {
        mSize = size;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mEvent 
                   << mTopic 
                   << mSize;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mEvent 
                     >> mTopic 
                     >> mSize;
    }
private:
    int32_t mEvent;
    std::string mTopic;
    int32_t mSize;
};

class FdbMsgEventCache : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgEventCacheItem> &cache()
    {
        return mCache;
    }
    FdbMsgEventCacheItem *add_cache()
    {
        return mCache.Add();
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mCache; 
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mCache; 
    }
    
private:
    CFdbParcelableArray<FdbMsgEventCacheItem> mCache;
};

class FdbMsgAllExportableService : public IFdbParcelable
{
public:
    FdbMsgExportableSvcAddress &upstream_table()
    {
        return mUpstreamTable;
    }
    FdbMsgExportableSvcAddress &local_table()
    {
        return mLocalTable;
    }
    FdbMsgExportableSvcAddress &downstream_table()
    {
        return mDownstreamTable;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mUpstreamTable
                   << mLocalTable
                   << mDownstreamTable
                   ; 
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mUpstreamTable
                     >> mLocalTable
                     >> mDownstreamTable
                     ; 
    }
private:
    FdbMsgExportableSvcAddress mUpstreamTable;
    FdbMsgExportableSvcAddress mLocalTable;
    FdbMsgExportableSvcAddress mDownstreamTable;
};

}
}

#endif

