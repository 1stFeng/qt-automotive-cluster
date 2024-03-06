/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_STUBMANAGER_HPP_
#define COMMONAPI_FDBUS_STUBMANAGER_HPP_

#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusStubAdapter.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusStubAdapter;

class FDBusStubManager
{
public:
    COMMONAPI_EXPORT FDBusStubManager(const std::shared_ptr<FDBusProxyConnection>&);
    COMMONAPI_EXPORT virtual ~FDBusStubManager();

    COMMONAPI_EXPORT bool registerStubAdapter(std::shared_ptr<FDBusStubAdapter> fdbusStubAdapter);
    COMMONAPI_EXPORT bool unregisterStubAdapter(std::shared_ptr<FDBusStubAdapter> fdbusStubAdapter);

    COMMONAPI_EXPORT bool handleMessage(const FDBusMessage&);

private:
    std::weak_ptr<FDBusProxyConnection> mConnection;
    //interface, instance
    std::map<std::string, std::map<std::string, std::shared_ptr<FDBusStubAdapter>>> mRegisteredStubAdapters;
    std::mutex mRegisteredStubAdaptersMutex;
};
}
}

#endif