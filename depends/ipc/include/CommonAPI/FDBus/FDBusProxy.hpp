/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXY_HPP_
#define COMMONAPI_FDBUS_DBUSPROXY_HPP_

#include <functional>
#include <memory>
#include <string>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/FDBus/FDBusAttribute.hpp>
#include <CommonAPI/FDBus/FDBusServiceRegistry.hpp>
#include <CommonAPI/FDBus/FDBusClient.hpp>

namespace CommonAPI {
namespace FDBus {

class DBusProxyStatusEvent
        : public ProxyStatusEvent {
    friend class FDBusProxy;

 public:
    DBusProxyStatusEvent(FDBusProxy* dbusProxy);
    virtual ~DBusProxyStatusEvent() {}

 protected:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif
    virtual void onListenerAdded(const Listener& _listener, const Subscription _subscription);
    virtual void onListenerRemoved(const Listener &_listener, const Subscription _subscription);

    FDBusProxy* dbusProxy_;

    std::recursive_mutex listenersMutex_;
    std::vector<std::pair<ProxyStatusEvent::Subscription, ProxyStatusEvent::Listener>> listeners_;
};


class COMMONAPI_EXPORT_CLASS_EXPLICIT FDBusProxy
        : public FDBusProxyBase,
          public std::enable_shared_from_this<FDBusProxy> {
public:
    COMMONAPI_EXPORT FDBusProxy(const FDBusAddress &_address,
              const std::shared_ptr<FDBusProxyConnection> &_connection);
    COMMONAPI_EXPORT virtual ~FDBusProxy();

    COMMONAPI_EXPORT AvailabilityStatus getAvailabilityStatus() const;
  
    COMMONAPI_EXPORT virtual ProxyStatusEvent& getProxyStatusEvent();
    COMMONAPI_EXPORT virtual InterfaceVersionAttribute& getInterfaceVersionAttribute();

    COMMONAPI_EXPORT virtual bool isAvailable() const;
    COMMONAPI_EXPORT virtual bool isAvailableBlocking() const;
    COMMONAPI_EXPORT virtual std::future<AvailabilityStatus> isAvailableAsync(
                isAvailableAsyncCallback _callback,
                const CallInfo *_info) const;

    COMMONAPI_EXPORT void subscribeForSelectiveBroadcastOnConnection(
              const std::string& instance,
              const std::string& interfaceName,
              const std::string& interfaceMemberName,
              const std::string& interfaceMemberSignature,
              std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
              uint32_t tag);

    COMMONAPI_EXPORT void insertSelectiveSubscription(
            const std::string& interfaceMemberName,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> dbusBroadCastHandler,
            uint32_t tag, std::string interfaceMemberSignature);

    COMMONAPI_EXPORT void unsubscribeFromSelectiveBroadcast(const std::string& eventName,
                                           FDBusProxyConnection::FDBusSignalHandlerToken subscription,
                                           const FDBusProxyConnection::FDBusSignalHandler* fdbusBroadCastHandler);

    COMMONAPI_EXPORT void init();

    /**
     * @brief Establish correspondence between matching fields and handlers in the handler table.
     * @param instance, Used to generate matching fields.
     * @param interfaceName, Used to generate matching fields.
     * @param Name, Used to generate matching fields.
     * @param Signature, Used to generate matching fields.
     * @param fdbusBroadCastHandler, message's Handler.
     * @param justAddFilter.
    */
    COMMONAPI_EXPORT virtual FDBusProxyConnection::FDBusSignalHandlerToken addSignalMemberHandler(
            const std::string &instance,
            const std::string &interfaceNameice,
            const std::string &Name,
            const std::string &Signature,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
            const bool justAddFilter);

    /**
     * @brief Establish correspondence between matching fields and handlers in the handler table.
     * @param instance, Used to generate matching fields.
     * @param interfaceName, Used to generate matching fields.
     * @param Name, Used to generate matching fields.
     * @param Signature, Used to generate matching fields.
     * @param getMethodName, if not empty, add the handler to queue.
     * @param fdbusBroadCastHandler, message's Handler.
     * @param justAddFilter.
    */
    COMMONAPI_EXPORT virtual FDBusProxyConnection::FDBusSignalHandlerToken addSignalMemberHandler(
            const std::string &instance,
            const std::string &interfaceName,
            const std::string &Name,
            const std::string &Signature,
            const std::string &getMethodName,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
            const bool justAddFilter);

    /**
     * @brief Remove the correspondence between matching fields and handlers in the handler table.
     * @param fdbusBroadCastHandlerToken, consisting of instance, interfaceName, interfaceMemberName, interfaceMemberSignature.
     * @param fdbusBroadCastHandler, message's handler.
    */
    COMMONAPI_EXPORT virtual bool removeSignalMemberHandler(
            const FDBusProxyConnection::FDBusSignalHandlerToken &_fdbusBroadCastHandlerToken,
            const FDBusProxyConnection::FDBusSignalHandler* _fdbusBroadCastHandler = NULL);

    COMMONAPI_EXPORT virtual void getCurrentValueForSignalListener(
            const std::string &getMethodName,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
            const uint32_t subscription);

    COMMONAPI_EXPORT static void notifySpecificListener(std::weak_ptr<FDBusProxy> _dbusProxy,
                                                         const ProxyStatusEvent::Listener &_listener,
                                                         const ProxyStatusEvent::Subscription _subscription);

private:
#ifdef COMMONAPI_FDBUS_TEST
public:
#endif
    typedef std::tuple<
        const std::string,
        const std::string,
        const std::string,
        const std::string,
        const std::string,
        std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler>,
        const bool,
        bool
        > SignalMemberHandlerTuple;

    COMMONAPI_EXPORT FDBusProxy(const FDBusProxy &) = delete;

    COMMONAPI_EXPORT void onDBusServiceInstanceStatus(std::shared_ptr<FDBusProxy> _proxy,
                                             std::string interface,
                                                std::string instance,
                                                bool isAvailable,
                                                void* _data);

COMMONAPI_EXPORT void onFDBusInstanceStatus(std::shared_ptr<FDBusProxy> _proxy,
                                                      const AvailabilityStatus& availabilityStatus);

    COMMONAPI_EXPORT void signalMemberCallback(const CallStatus dbusMessageCallStatus,
            const FDBusMessage& dbusMessage,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandlers,
            const uint32_t tag);
    COMMONAPI_EXPORT void signalInitialValueCallback(const CallStatus dbusMessageCallStatus,
            const FDBusMessage& dbusMessage,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandlers,
            const uint32_t tag);
    COMMONAPI_EXPORT void addSignalMemberHandlerToQueue(SignalMemberHandlerTuple& _signalMemberHandler);

    COMMONAPI_EXPORT void availabilityTimeoutThreadHandler() const;

    DBusProxyStatusEvent dbusProxyStatusEvent_;
    FDBusServiceRegistry::FDBusServiceSubscription dbusServiceRegistrySubscription_;
    AvailabilityStatus availabilityStatus_;
    AvailabilityHandlerId_t availabilityHandlerId_;

    FDBusReadonlyAttribute<InterfaceVersionAttribute> interfaceVersionAttribute_;

    std::shared_ptr<FDBusServiceRegistry> dbusServiceRegistry_;

    mutable std::mutex availabilityMutex_;
    mutable std::condition_variable availabilityCondition_;

    std::list<SignalMemberHandlerTuple> signalMemberHandlerQueue_;
    mutable std::mutex signalMemberHandlerQueueMutex_;

    std::map<std::string,
            std::tuple<std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler>, uint32_t,
                    std::string>> selectiveBroadcastHandlers;
    mutable std::mutex selectiveBroadcastHandlersMutex_;

    mutable std::shared_ptr<std::thread> availabilityTimeoutThread_;
    mutable std::mutex availabilityTimeoutThreadMutex_;
    mutable std::mutex timeoutsMutex_;
    mutable std::condition_variable availabilityTimeoutCondition_;

    typedef std::tuple<
                std::chrono::steady_clock::time_point,
                isAvailableAsyncCallback,
                std::promise<AvailabilityStatus>
                > AvailabilityTimeout_t;
    mutable std::list<AvailabilityTimeout_t> timeouts_;

    std::atomic<bool> everAvailable_;
    mutable std::atomic<bool> cancelAvailabilityTimeoutThread_;
};


} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXY_HPP_
