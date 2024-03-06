/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_DAEMON_PROXY_HPP_
#define COMMONAPI_FDBUS_DBUS_DAEMON_PROXY_HPP_

#include <functional>
#include <string>
#include <vector>

#include <CommonAPI/Address.hpp>

#include <CommonAPI/FDBus/FDBusAttribute.hpp>
#include <CommonAPI/FDBus/FDBusEvent.hpp>
#include <CommonAPI/FDBus/FDBusProxyBase.hpp>

namespace CommonAPI {
namespace FDBus {

class StaticInterfaceVersionAttribute: public InterfaceVersionAttribute {
 public:
    StaticInterfaceVersionAttribute(const uint32_t& majorValue, const uint32_t& minorValue);

    void getValue(CommonAPI::CallStatus& callStatus, Version &_version, const CommonAPI::CallInfo *_info) const;
    std::future<CommonAPI::CallStatus> getValueAsync(AttributeAsyncCallback _callback, const CommonAPI::CallInfo *_info);

 private:
    Version version_;
};

#ifndef DBUS_DAEMON_PROXY_DEFAULT_SEND_TIMEOUT
#define DBUS_DAEMON_PROXY_DEFAULT_SEND_TIMEOUT 10000
#endif

static const CommonAPI::CallInfo daemonProxyInfo(DBUS_DAEMON_PROXY_DEFAULT_SEND_TIMEOUT);

// TODO remove it
class FDBusDaemonProxy : public FDBusProxyBase,
                        public std::enable_shared_from_this<FDBusDaemonProxy> {
 public:
    typedef Event<std::string, std::string, std::string> NameOwnerChangedEvent;

    typedef std::unordered_map<std::string, int> PropertyDictStub;
    typedef std::unordered_map<std::string, PropertyDictStub> InterfaceToPropertyDict;
    typedef std::unordered_map<std::string, InterfaceToPropertyDict> DBusObjectToInterfaceDict;

    typedef std::function<void(const CommonAPI::CallStatus&, std::vector<std::string>)> ListNamesAsyncCallback;
    typedef std::function<void(const CommonAPI::CallStatus&, bool)> NameHasOwnerAsyncCallback;
    typedef std::function<void(const CommonAPI::CallStatus&, DBusObjectToInterfaceDict)> GetManagedObjectsAsyncCallback;
    typedef std::function<void(const CommonAPI::CallStatus&, std::string)> GetNameOwnerAsyncCallback;

    COMMONAPI_EXPORT FDBusDaemonProxy(const std::shared_ptr<FDBusProxyConnection>& dbusConnection);
    COMMONAPI_EXPORT virtual ~FDBusDaemonProxy();

    COMMONAPI_EXPORT virtual bool isAvailable() const;
    COMMONAPI_EXPORT virtual bool isAvailableBlocking() const;
    COMMONAPI_EXPORT virtual std::future<AvailabilityStatus> isAvailableAsync(
            isAvailableAsyncCallback _callback,
            const CallInfo *_info) const;

    COMMONAPI_EXPORT virtual ProxyStatusEvent& getProxyStatusEvent();

    COMMONAPI_EXPORT virtual InterfaceVersionAttribute& getInterfaceVersionAttribute();

    COMMONAPI_EXPORT void init();

    COMMONAPI_EXPORT static const char* getInterfaceId();

    COMMONAPI_EXPORT NameOwnerChangedEvent& getNameOwnerChangedEvent();

    COMMONAPI_EXPORT void listNames(CommonAPI::CallStatus& callStatus, std::vector<std::string>& busNames) const;

    template <typename DelegateObjectType>
    COMMONAPI_EXPORT std::future<CallStatus> listNamesAsync(typename FDBusProxyAsyncCallbackHandler<DelegateObjectType,
                                                            std::vector<std::string>>::Delegate& delegate) const {
        FDBusMessage dbusMessage = createMethodCall("ListNames", "");

        auto dbusMessageReplyAsyncHandler = std::move(FDBusProxyAsyncCallbackHandler<
                DelegateObjectType, std::vector< std::string > >::create(delegate, std::tuple<std::vector<std::string>>()));

        std::future<CallStatus> callStatusFuture;
        try {
            callStatusFuture = dbusMessageReplyAsyncHandler->getFuture();
        } catch (std::exception& e) {
            COMMONAPI_ERROR("getNameOwnerAsync: messageReplyAsyncHandler future failed(", e.what(), ")");
        }

        if(getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                        dbusMessage,
                        std::move(dbusMessageReplyAsyncHandler),
                        &daemonProxyInfo)) {
            return callStatusFuture;
        } else  {
            return std::future<CallStatus>();
        }
    }

    COMMONAPI_EXPORT void nameHasOwner(const std::string& busName, CommonAPI::CallStatus& callStatus, bool& hasOwner) const;

    template <typename DelegateObjectType>
    COMMONAPI_EXPORT std::future<CallStatus> nameHasOwnerAsync(const std::string& busName,
                                                               typename FDBusProxyAsyncCallbackHandler<DelegateObjectType,
                                                               bool>::Delegate& delegate) const {
        FDBusMessage dbusMessage = createMethodCall("NameHasOwner", "s");

        FDBusOutputStream outputStream(dbusMessage);
        const bool success = FDBusSerializableArguments<std::string>::serialize(outputStream, busName);
        if (!success) {
            std::promise<CallStatus> promise;
            promise.set_value(CallStatus::SERIALIZATION_ERROR);
            return promise.get_future();
        }
        outputStream.flush();

        auto dbusMessageReplyAsyncHandler = std::move(FDBusProxyAsyncCallbackHandler<
                DelegateObjectType, bool >::create(delegate, std::tuple< bool >()));

        std::future<CallStatus> callStatusFuture;
        try {
            callStatusFuture = dbusMessageReplyAsyncHandler->getFuture();
        } catch (std::exception& e) {
            COMMONAPI_ERROR("getNameOwnerAsync: messageReplyAsyncHandler future failed(", e.what(), ")");
        }

        if (getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                        dbusMessage,
                        std::move(dbusMessageReplyAsyncHandler),
                        &daemonProxyInfo)) {
            return callStatusFuture;
        } else {
            return std::future<CallStatus>();
        }
    }

    template <typename DelegateObjectType>
    COMMONAPI_EXPORT std::future<CallStatus> getManagedObjectsAsync(const std::string& forDBusServiceName,
                                                                    typename FDBusProxyAsyncCallbackHandler<DelegateObjectType,
                                                                    DBusObjectToInterfaceDict>::Delegate& delegate) const {
        static FDBusAddress address("org.freedesktop.DBus.ObjectManager", "org.freedesktop.DBus.ObjectManager");
        auto dbusMethodCallMessage = FDBusMessage::createMethodCall(address, "GetManagedObjects", "");

        return getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                        dbusMethodCallMessage,
                        FDBusProxyAsyncCallbackHandler<DelegateObjectType, DBusObjectToInterfaceDict>::create(
                            delegate, std::tuple<DBusObjectToInterfaceDict>()
                        ),
                        &daemonProxyInfo);
    }

    /**
     * Get the unique connection/bus name of the primary owner of the name given
     *
     * @param busName Name to get the owner of
     * @param getNameOwnerAsyncCallback callback functor
     *
     * @return CallStatus::REMOTE_ERROR if the name is unknown, otherwise CallStatus::SUCCESS and the uniq name of the owner
     */
    template <typename DelegateObjectType>
    std::future<CallStatus> getNameOwnerAsync(const std::string& busName,
                                              typename FDBusProxyAsyncCallbackHandler<DelegateObjectType,
                                              std::string>::Delegate& delegate) const {
        FDBusMessage dbusMessage = createMethodCall("GetNameOwner", "s");

        FDBusOutputStream outputStream(dbusMessage);
        const bool success = FDBusSerializableArguments<std::string>::serialize(outputStream, busName);
        if (!success) {
            std::promise<CallStatus> promise;
            promise.set_value(CallStatus::SERIALIZATION_ERROR);
            return promise.get_future();
        }
        outputStream.flush();

        auto dbusMessageReplyAsyncHandler = std::move(FDBusProxyAsyncCallbackHandler<
               DelegateObjectType, std::string>::create(delegate, std::tuple<std::string>()));

        std::future<CallStatus> callStatusFuture;
        try {
            callStatusFuture = dbusMessageReplyAsyncHandler->getFuture();
        } catch (std::exception& e) {
            COMMONAPI_ERROR("getNameOwnerAsync: messageReplyAsyncHandler future failed(", e.what(), ")");
        }

        if (getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                        dbusMessage,
                        std::move(dbusMessageReplyAsyncHandler),
                        &daemonProxyInfo)) {
            return callStatusFuture;
        } else {
            return std::future<CallStatus>();
        }
    }

 private:
    FDBusEvent<NameOwnerChangedEvent, std::string, std::string, std::string> nameOwnerChangedEvent_;
    StaticInterfaceVersionAttribute interfaceVersionAttribute_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_DAEMON_PROXY_HPP_
