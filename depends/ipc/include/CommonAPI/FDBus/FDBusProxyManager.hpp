/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_PROXYMANAGER_HPP_
#define COMMONAPI_FDBUS_PROXYMANAGER_HPP_

#include <functional>
#include <future>
#include <string>
#include <vector>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/ProxyManager.hpp>
#include <CommonAPI/FDBus/FDBusProxy.hpp>
#include <CommonAPI/FDBus/FDBusFactory.hpp>
#include <CommonAPI/FDBus/FDBusObjectManagerStub.hpp>
#include <CommonAPI/FDBus/FDBusInstanceAvailabilityStatusChangedEvent.hpp>
#include <CommonAPI/FDBus/FDBusConnection.hpp>

namespace CommonAPI {
namespace FDBus {

/**
 * @brief FDBusProxyManager
 * 
 */
class COMMONAPI_EXPORT_CLASS_EXPLICIT FDBusProxyManager: public ProxyManager {
public:
    COMMONAPI_EXPORT FDBusProxyManager(FDBusProxy &_proxy,
                     const std::string &_dbusInterfaceName,
                     const std::string &_capiInterfaceName);

    COMMONAPI_EXPORT const std::string &getDomain() const;
    COMMONAPI_EXPORT const std::string &getInterface() const;
    COMMONAPI_EXPORT const ConnectionId_t &getConnectionId() const;

    COMMONAPI_EXPORT virtual void getAvailableInstances(CommonAPI::CallStatus &_status, std::vector<std::string> &_instances);
    COMMONAPI_EXPORT virtual std::future<CallStatus> getAvailableInstancesAsync(GetAvailableInstancesCallback _callback);

    COMMONAPI_EXPORT virtual void getInstanceAvailabilityStatus(const std::string &_instance,
                                               CallStatus &_callStatus,
                                               AvailabilityStatus &_availabilityStatus);

    COMMONAPI_EXPORT  virtual std::future<CallStatus> getInstanceAvailabilityStatusAsync(
                                        const std::string& _instance,
                                        GetInstanceAvailabilityStatusCallback _callback);

    COMMONAPI_EXPORT virtual InstanceAvailabilityStatusChangedEvent& getInstanceAvailabilityStatusChangedEvent();

private:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif
    COMMONAPI_EXPORT void instancesAsyncCallback(std::shared_ptr<Proxy> _proxy,
                                                 const CommonAPI::CallStatus &_status,
                                                 const std::vector<FDBusAddress> &_availableServiceInstances,
                                                 GetAvailableInstancesCallback &_call);

    COMMONAPI_EXPORT void translate(const std::vector<FDBusAddress> &_serviceInstances,
                                     std::vector<std::string> &_instances);

    FDBusProxy &proxy_;
    FDBusInstanceAvailabilityStatusChangedEvent instanceAvailabilityStatusEvent_;
    const std::string dbusInterfaceId_;
    const std::string capiInterfaceId_;
    ConnectionId_t connectionId_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_PROXYMANAGER_HPP_
