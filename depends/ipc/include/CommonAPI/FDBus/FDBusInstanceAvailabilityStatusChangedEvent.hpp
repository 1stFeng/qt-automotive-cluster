/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSINSTANCEAVAILABILITYSTATUSCHANGED_EVENT_HPP_
#define COMMONAPI_FDBUS_DBUSINSTANCEAVAILABILITYSTATUSCHANGED_EVENT_HPP_

#include <functional>
#include <future>
#include <string>
#include <vector>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/ProxyManager.hpp>
#include <CommonAPI/FDBus/FDBusProxy.hpp>
#include <CommonAPI/FDBus/FDBusObjectManagerStub.hpp>
#include <CommonAPI/FDBus/FDBusTypes.hpp>

namespace CommonAPI {
namespace FDBus {

// TODO Check to move logic to FDBusServiceRegistry, now every proxy will deserialize the messages!
class FDBusInstanceAvailabilityStatusChangedEvent:
                public ProxyManager::InstanceAvailabilityStatusChangedEvent {
 public:

    typedef std::function<void(const CallStatus &, const std::vector<FDBusAddress> &)> GetAvailableServiceInstancesCallback;

    COMMONAPI_EXPORT FDBusInstanceAvailabilityStatusChangedEvent(FDBusProxy &_proxy,
                                               const std::string &_dbusInterfaceName,
                                               const std::string &_capiInterfaceName);

    COMMONAPI_EXPORT virtual ~FDBusInstanceAvailabilityStatusChangedEvent();

    COMMONAPI_EXPORT void getAvailableServiceInstances(CommonAPI::CallStatus &_status, std::vector<FDBusAddress> &_availableServiceInstances);
    COMMONAPI_EXPORT std::future<CallStatus> getAvailableServiceInstancesAsync(GetAvailableServiceInstancesCallback _callback);

    COMMONAPI_EXPORT void getServiceInstanceAvailabilityStatus(const std::string &_instance,
                                       CallStatus &_callStatus,
                                       AvailabilityStatus &_availabilityStatus);
    COMMONAPI_EXPORT std::future<CallStatus> getServiceInstanceAvailabilityStatusAsync(const std::string& _instance,
            ProxyManager::GetInstanceAvailabilityStatusCallback _callback);

 protected:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif

    class SignalHandler : public FDBusProxyConnection::FDBusSignalHandler {
    public:
        COMMONAPI_EXPORT SignalHandler(FDBusInstanceAvailabilityStatusChangedEvent* _instanceAvblStatusEvent);
        COMMONAPI_EXPORT virtual void onSignalFDBusMessage(const FDBusMessage& dbusMessage);
    private:
        FDBusInstanceAvailabilityStatusChangedEvent* instanceAvblStatusEvent_;
    };

    virtual void onFirstListenerAdded(const Listener&);
    virtual void onLastListenerRemoved(const Listener&);

 private:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif

    void onInterfacesAddedBroadcast(const FDBusMessage &_message);

    void onInterfacesRemovedBroadcast(const FDBusMessage &_message);

    void notifyInterfaceStatusChanged(const std::string &_objectPath,
                                      const std::string &_interfaceName,
                                      const AvailabilityStatus &_availability);

    bool addInterface(const std::string &_dbusObjectPath,
                      const std::string &_dbusInterfaceName);
    bool removeInterface(const std::string &_dbusObjectPath,
                         const std::string &_dbusInterfaceName);

    void serviceInstancesAsyncCallback(std::shared_ptr<Proxy> _proxy,
                                       const FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict _dict,
                                       GetAvailableServiceInstancesCallback &_call,
                                       std::shared_ptr<std::promise<CallStatus> > &_promise);

    void translate(const FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict &_dict,
                                     std::vector<FDBusAddress> &_serviceInstances);

    std::shared_ptr<SignalHandler> signalHandler_;
    FDBusProxy &proxy_;
    std::weak_ptr<FDBusProxy> proxyWeakPtr_;
    std::string observedDbusInterfaceName_;
    std::string observedCapiInterfaceName_;
    FDBusProxyConnection::FDBusSignalHandlerToken interfacesAddedSubscription_;
    FDBusProxyConnection::FDBusSignalHandlerToken interfacesRemovedSubscription_;
    std::mutex interfacesMutex_;
    std::map<std::string, std::set<std::string>> interfaces_;
    const std::shared_ptr<FDBusServiceRegistry> registry_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSINSTANCEAVAILABILITYSTATUSCHANGEDEVENT_HPP_
