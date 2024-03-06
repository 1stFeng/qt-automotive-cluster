/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXYBASE_HPP_
#define COMMONAPI_FDBUS_DBUSPROXYBASE_HPP_

#include <functional>
#include <memory>
#include <string>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/Proxy.hpp>
#include <CommonAPI/Types.hpp>

#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusAddress;

class COMMONAPI_EXPORT_CLASS_EXPLICIT FDBusProxyBase
        : public virtual CommonAPI::Proxy {
public:
    COMMONAPI_EXPORT FDBusProxyBase(const FDBusAddress &_address,
                  const std::shared_ptr<FDBusProxyConnection> &_connection);
    COMMONAPI_EXPORT virtual ~FDBusProxyBase() {}

    COMMONAPI_EXPORT const FDBusAddress &getFDBusAddress() const;
    COMMONAPI_EXPORT const std::shared_ptr<FDBusProxyConnection> &getFDBusConnection() const;

    COMMONAPI_EXPORT FDBusMessage createMethodCall(const std::string &_method,
                                 const std::string &_signature = "") const;

    typedef std::function<void(const AvailabilityStatus, const Timeout_t remaining)> isAvailableAsyncCallback;
    COMMONAPI_EXPORT virtual std::future<AvailabilityStatus> isAvailableAsync(
            isAvailableAsyncCallback _callback,
            const CallInfo *_info) const = 0;

    COMMONAPI_EXPORT virtual FDBusProxyConnection::FDBusSignalHandlerToken addSignalMemberHandler(
            const std::string &instance,
            const std::string &interfaceName,
            const std::string &signalName,
            const std::string &signalSignature,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
            const bool justAddFilter);

    COMMONAPI_EXPORT virtual FDBusProxyConnection::FDBusSignalHandlerToken addSignalMemberHandler(
            const std::string &instance,
            const std::string &interfaceName,
            const std::string &signalName,
            const std::string &signalSignature,
            const std::string &getMethodName,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusBroadCastHandler,
            const bool justAddFilter);

    COMMONAPI_EXPORT virtual bool removeSignalMemberHandler(
            const FDBusProxyConnection::FDBusSignalHandlerToken &_fdbusBroadCastHandlerToken,
            const FDBusProxyConnection::FDBusSignalHandler* fdbusBroadCastlHandler = NULL);

    COMMONAPI_EXPORT virtual void getCurrentValueForSignalListener(
            const std::string &_getMethodName,
            std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
            const uint32_t _subscription) {
        (void)_getMethodName;
        (void)_handler;
        (void)_subscription;
    }

    COMMONAPI_EXPORT virtual void init() = 0;

    COMMONAPI_EXPORT void addSignalStateHandler(
            std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
            const uint32_t _subscription);

    COMMONAPI_EXPORT void removeSignalStateHandler(
                std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
                const uint32_t _tag, bool _remove_all = false);

    COMMONAPI_EXPORT std::weak_ptr<FDBusProxyBase> getWeakPtr();

 protected:
    COMMONAPI_EXPORT FDBusProxyBase(const FDBusProxyBase &) = delete;

    FDBusAddress dbusAddress_;
    std::shared_ptr<FDBusProxyConnection> connection_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXYBASE_HPP_
