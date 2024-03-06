/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXYCONNECTION_HPP_
#define COMMONAPI_FDBUS_DBUSPROXYCONNECTION_HPP_

#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <CommonAPI/Attribute.hpp>
#include <CommonAPI/Event.hpp>
#include <CommonAPI/Types.hpp>
#include <CommonAPI/FDBus/FDBusConfig.hpp>
#include <CommonAPI/FDBus/FDBusError.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusFunctionalHash.hpp>
#include <CommonAPI/FDBus/FDBusTypes.hpp>

namespace CommonAPI {
namespace FDBus {

typedef std::function<void(const FDBusMessage&)> DBusMessageHandler;

class FDBusObjectManager;
class FDBusStubManager;
class FDBusProxy;

/**
 * @brief FDBusProxyConnection
 * 
 */
class FDBusProxyConnection {
 public:
    class FDBusMessageReplyAsyncHandler {
     public:
       virtual ~FDBusMessageReplyAsyncHandler() {}
       virtual std::future<CallStatus> getFuture() = 0;
       virtual void onFDBusMessageReply(const CallStatus&, const FDBusMessage&) = 0;
       virtual void setExecutionStarted() = 0;
       virtual bool getExecutionStarted() = 0;
       virtual void setExecutionFinished() = 0;
       virtual bool getExecutionFinished() = 0;
       virtual void setTimeoutOccurred() = 0;
       virtual bool getTimeoutOccurred() = 0;
       virtual void setHasToBeDeleted() = 0;
       virtual bool hasToBeDeleted() = 0;
       virtual void lock() = 0;
       virtual void unlock() = 0;
    };

    class FDBusSignalHandler;

    // instance, interfaceName, interfaceMemberName, interfaceMemberSignature
    typedef std::tuple<std::string, std::string, std::string, std::string> FDBusSingalHandlerPath;

    typedef std::unordered_map<FDBusSingalHandlerPath,
                                std::map<const FDBusSignalHandler*,
                                    std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler>>> FDBusSignalHandlerTable;

    typedef std::unordered_multimap<std::string, std::pair<const FDBusSignalHandler*,
                                        std::weak_ptr<FDBusSignalHandler>>> FDBusOMBroadCastHandlerTable;

    typedef FDBusSingalHandlerPath FDBusSignalHandlerToken;

    class FDBusSignalHandler {
     public:
        virtual ~FDBusSignalHandler() {}
        virtual void onSignalFDBusMessage(const FDBusMessage&) = 0;
        //TODO remove it
        virtual void onInitialValueSignalDBusMessage(const FDBusMessage&, const uint32_t) {};
        virtual void onSpecificError(const CommonAPI::CallStatus, const uint32_t ) {};
        virtual void setSubscriptionToken(const FDBusSignalHandlerToken, const uint32_t) {};
    };

    typedef Event<AvailabilityStatus> ConnectionStatusEvent;

    virtual ~FDBusProxyConnection() {}

    virtual bool isConnected() const = 0;

    virtual ConnectionStatusEvent& getConnectionStatusEvent() = 0;

    virtual bool sendFDBusMessage(const FDBusMessage& dbusMessage) const = 0;

    virtual bool sendFDBusMessageWithReplyAsync(
            const FDBusMessage& dbusMessage,
            std::unique_ptr<FDBusMessageReplyAsyncHandler> dbusMessageReplyAsyncHandler,
            const CommonAPI::CallInfo *_info) const = 0;

    virtual FDBusMessage sendFDBusMessageWithReplyAndBlock(
            const FDBusMessage& dbusMessage,
            FDBusError& dbusError,
            const CommonAPI::CallInfo *_info) const = 0;

    virtual FDBusSignalHandlerToken addSignalMemberHandler(
            const std::string& instance,
            const std::string& interfaceName,
            const std::string& interfaceMemberName,
            const std::string& interfaceMemberSignature,
            std::weak_ptr<FDBusSignalHandler> fdbusBroadCastHandler,
            const bool justAddFilter = false) = 0;

    virtual void subscribeForSelectiveBroadcast(const std::string& instance,
                                                                  const std::string& interfaceName,
                                                                  const std::string& interfaceMemberName,
                                                                  const std::string& interfaceMemberSignature,
                                                                  std::weak_ptr<FDBusSignalHandler> fdbusBroadCastHandler,
                                                                  FDBusProxy* callingProxy,
                                                                  uint32_t tag) = 0;

    virtual void unsubscribeFromSelectiveBroadcast(const std::string& eventName,
                                                  FDBusProxyConnection::FDBusSignalHandlerToken subscription,
                                                  FDBusProxy* callingProxy,
                                                  const FDBusSignalHandler* fdbusBroadCastHandler) = 0;

    virtual bool removeSignalMemberHandler(const FDBusSignalHandlerToken& fdbusBroadCastHandlerToken,
                                              const FDBusSignalHandler* fdbusBroadCastHandler = NULL) = 0;

    virtual bool addObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                     std::weak_ptr<FDBusSignalHandler> fdbusBroadCastHandler) = 0;
    virtual bool removeObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                        const FDBusSignalHandler* fdbusBroadCastHandler) = 0;

    virtual const std::shared_ptr<FDBusObjectManager> getFDBusObjectManager() = 0;
    virtual const std::shared_ptr<FDBusStubManager> getFDBusStubManager() = 0;

    virtual bool requestServiceNameAndBlock(const std::string& serviceName) const = 0;
    virtual bool releaseServiceName(const std::string& serviceName) const = 0;

    typedef std::function<bool(const FDBusMessage&)> FDBusObjectPathMessageHandler;

    virtual void setObjectPathMessageHandler(FDBusObjectPathMessageHandler) = 0;
    virtual bool isObjectPathMessageHandlerSet() = 0;

    virtual bool hasDispatchThread() = 0;

    virtual void sendPendingSelectiveSubscription(
            FDBusProxy* proxy, std::string interfaceMemberName,
            std::weak_ptr<FDBusSignalHandler> fdbusBroadCastHandler, uint32_t tag,
            std::string interfaceMemberSignature) = 0;

    virtual void pushFDBusMessageReplyToMainLoop(const FDBusMessage& reply,
                             std::unique_ptr<FDBusMessageReplyAsyncHandler> dbusMessageReplyAsyncHandler) = 0;

    template<class DBusConnection, class Function, class... Arguments>
    void proxyPushFunctionToMainLoop(Function&& _function, Arguments&& ... _args) {
        static_cast<DBusConnection*>(this)->proxyPushFunctionToMainLoop(std::forward<Function>(_function), std::forward<Arguments>(_args) ...);
    }

    virtual void addSignalStateHandler(
            std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
            const uint32_t _subscription) = 0;

    virtual void removeSignalStateHandler(
                std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
                const uint32_t _tag, bool _remove_all) = 0;

    virtual void handleSignalStates() = 0;

    virtual bool isAvailable(const FDBusAddress &_address) = 0;

    virtual AvailabilityHandlerId_t registerAvailabilityHandler(
            const FDBusAddress &_address, AvailabilityHandler_t _handler,
            std::weak_ptr<FDBusProxy> _proxy, void* _data) = 0;
    virtual void unregisterAvailabilityHandler(const FDBusAddress &_address,
            AvailabilityHandlerId_t _handlerId) = 0;

    virtual void registerService(const FDBusAddress &_address) = 0;
    virtual void unregisterService(const FDBusAddress &_address) = 0;

    virtual void requestService(const FDBusAddress &_address) = 0;

    virtual void releaseService(const FDBusAddress &_address) = 0;
    
    virtual void getAvailableInstances(std::string _interface, std::vector<std::string> *_instances) = 0;

};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXYCONNECTION_HPP_
