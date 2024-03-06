/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXYHELPER_HPP_
#define COMMONAPI_FDBUS_DBUSPROXYHELPER_HPP_

#include <functional>
#include <future>
#include <memory>
#include <string>

#include <CommonAPI/Logger.hpp>

#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusConfig.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusSerializableArguments.hpp>
#include <CommonAPI/FDBus/FDBusProxyAsyncCallbackHandler.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusErrorEvent.hpp>
#include <CommonAPI/FDBus/Types.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusProxy;

template< class, class... >
struct FDBusProxyHelper;

#ifdef _WIN32
// Visual Studio 2013 does not support 'magic statics' yet.
// Change back when switched to Visual Studio 2015 or higher.
static std::mutex callMethod_mutex_;
static std::mutex callMethodWithReply_mutex_;
static std::mutex callMethodAsync_mutex_;
#endif

/**
 * @brief FDBusProxyHelper
 * 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam Out_ 
 * @tparam OutArgs_ 
 */
template<
    template<class ...> class In_, class... InArgs_,
    template <class...> class Out_, class... OutArgs_>
struct FDBusProxyHelper<In_<FDBusInputStream, FDBusOutputStream, InArgs_...>,
                           Out_<FDBusInputStream, FDBusOutputStream, OutArgs_...>> {

    /**
     * @brief call method
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _method 
     * @param _signature 
     * @param _in 
     * @param _status 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethod(const FDBusProxy_ &_proxy,
                           const std::string &_method,
                           const std::string &_signature,
                           const InArgs_&... _in,
                           CommonAPI::CallStatus &_status) {
#ifndef _WIN32
        static std::mutex callMethod_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethod_mutex_);

        if (_proxy.isAvailable()) {
            FDBusMessage message = _proxy.createMethodCall(_method, _signature);
            message.setNoReplyExpected(1);
            if (sizeof...(InArgs_) > 0) {
                FDBusOutputStream output(message);
                if (!FDBusSerializableArguments<InArgs_...>::serialize(output, _in...)) {
                    COMMONAPI_ERROR("MethodSync(dbus): serialization failed: [",
                                    message.getInstance(), " ",
                                    message.getInterface(), " ",
                                    message.getMember(), " ",
                                    message.getSerial(), "]");

                    _status = CallStatus::SERIALIZATION_ERROR;
                    return;
                }
                output.flush();
            }

            const bool isSent = _proxy.getFDBusConnection()->sendFDBusMessage(message);
            _status = (isSent ? CallStatus::SUCCESS : CallStatus::OUT_OF_MEMORY);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _message 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                    const FDBusProxy_ &_proxy,
                    const FDBusMessage &_message,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    CommonAPI::CallStatus &_status,
                    OutArgs_&... _out) {

        if (sizeof...(InArgs_) > 0) {
            FDBusOutputStream output(_message);
            if (!FDBusSerializableArguments<InArgs_...>::serialize(output, _in...)) {
                COMMONAPI_ERROR("MethodSync w/ reply (dbus): serialization failed: [",
                                _message.getInstance(), " ",
                                _message.getInterface(), " ",
                                _message.getMember(), " ",
                                _message.getSerial(), "]");

                _status = CallStatus::SERIALIZATION_ERROR;
                return;
            }
            output.flush();
        }

        FDBusError error;
        FDBusMessage reply = _proxy.getFDBusConnection()->sendFDBusMessageWithReplyAndBlock(_message, error, _info);

        if (error || !reply.isMethodReturnType()) {
            _status = CallStatus::REMOTE_ERROR;
            return;
        }

        if (sizeof...(OutArgs_) > 0) {
            FDBusInputStream input(reply);
            if (!FDBusSerializableArguments<OutArgs_...>::deserialize(input, _out...)) {
                COMMONAPI_ERROR("MethodSync w/ reply (dbus): reply deserialization failed: [",
                                reply.getInstance(), " ",
                                reply.getInterface(), " ",
                                reply.getMember(), " ",
                                reply.getSerial(), "]");

                _status = CallStatus::SERIALIZATION_ERROR;
                return;
            }
        }
        _status = CallStatus::SUCCESS;
    }
    
    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _address 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                const FDBusProxy_ &_proxy,
                const FDBusAddress &_address,
                const char *_method,
                const char *_signature,
                const CommonAPI::CallInfo *_info,
                const InArgs_&... _in,
                CommonAPI::CallStatus &_status,
                OutArgs_&... _out) {
#ifndef _WIN32
        static std::mutex callMethodWithReply_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodWithReply_mutex_);

        if (_proxy.isAvailable()) {
            FDBusMessage message = FDBusMessage::createMethodCall(_address, _method, _signature);
            callMethodWithReply(_proxy, message, _info, _in..., _status, _out...);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _interface 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                const FDBusProxy_ &_proxy,
                const std::string &_interface,
                const std::string &_method,
                const std::string &_signature,
                const CommonAPI::CallInfo *_info,
                const InArgs_&... _in,
                CommonAPI::CallStatus &_status,
                OutArgs_&... _out) {
        FDBusAddress itsAddress(_proxy.getFDBusAddress());
        itsAddress.setInterface(_interface);
        callMethodWithReply(
                _proxy, itsAddress,
                _method.c_str(), _signature.c_str(),
                _info,
                _in..., _status, _out...);
    }

    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                    const FDBusProxy_ &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    CommonAPI::CallStatus &_status,
                    OutArgs_&... _out) {
#ifndef _WIN32
        static std::mutex callMethodWithReply_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodWithReply_mutex_);

        if (_proxy.isAvailable()) {
            FDBusMessage message = _proxy.createMethodCall(_method, _signature);
            callMethodWithReply(_proxy, message, _info, _in..., _status, _out...);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    /**
     * @brief call method async
     * 
     * @tparam FDBusProxy_ 
     * @tparam DelegateFunction_ 
     * @param _proxy 
     * @param _message 
     * @param _info 
     * @param _in 
     * @param _function 
     * @param _out 
     * @return std::future<CallStatus> 
     */
    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                    FDBusProxy_ &_proxy,
                    const FDBusMessage &_message,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    DelegateFunction_ _function,
                    std::tuple<OutArgs_...> _out) {
        if (sizeof...(InArgs_) > 0) {
            FDBusOutputStream output(_message);
            const bool success = FDBusSerializableArguments<
                                    InArgs_...
                                 >::serialize(output, _in...);
            if (!success) {
                COMMONAPI_ERROR("MethodAsync (dbus): serialization failed: [",
                                _message.getInstance(), " ",
                                _message.getInterface(), " ",
                                _message.getMember(), " ",
                                _message.getSerial(), "]");

                std::promise<CallStatus> promise;
                promise.set_value(CallStatus::SERIALIZATION_ERROR);
                return promise.get_future();
            }
            output.flush();
        }

        typename FDBusProxyAsyncCallbackHandler<
                                    FDBusProxy, OutArgs_...
                                >::Delegate delegate(_proxy.shared_from_this(), _function);
        auto dbusMessageReplyAsyncHandler = std::move(FDBusProxyAsyncCallbackHandler<
                                                                FDBusProxy, OutArgs_...
                                                                >::create(delegate, _out));

        std::future<CallStatus> callStatusFuture;
        try {
            callStatusFuture = dbusMessageReplyAsyncHandler->getFuture();
        } catch (std::exception& e) {
            COMMONAPI_ERROR("MethodAsync(dbus): messageReplyAsyncHandler future failed(",
                            e.what(), ") [", _message.getInstance(), " ",
                            _message.getInterface(), " ",
                            _message.getMember(), " ",
                            _message.getSerial(), "]");
        }

        if(_proxy.isAvailable()) {
            if (_proxy.getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                            _message,
                            std::move(dbusMessageReplyAsyncHandler),
                            _info)){
            COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy available -> sendMessageWithReplyAsync");
                return callStatusFuture;
            } else {
                return std::future<CallStatus>();
            }
        } else {
            std::shared_ptr< std::unique_ptr< FDBusProxyConnection::FDBusMessageReplyAsyncHandler > > sharedDbusMessageReplyAsyncHandler(
                    new std::unique_ptr< FDBusProxyConnection::FDBusMessageReplyAsyncHandler >(std::move(dbusMessageReplyAsyncHandler)));
            //async isAvailable call with timeout
            COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy not available -> register callback");
            _proxy.isAvailableAsync([&_proxy, _message, sharedDbusMessageReplyAsyncHandler](
                                             const AvailabilityStatus _status,
                                             const Timeout_t remaining) {
                if(_status == AvailabilityStatus::AVAILABLE) {
                    //create new call info with remaining timeout. Minimal timeout is 100 ms.
                    Timeout_t newTimeout = remaining;
                    if(remaining < 100)
                        newTimeout = 100;
                    CallInfo newInfo(newTimeout);
                    if(*sharedDbusMessageReplyAsyncHandler) {
                    _proxy.getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                                    _message,
                                    std::move(*sharedDbusMessageReplyAsyncHandler),
                                    &newInfo);
                    COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy callback available -> sendMessageWithReplyAsync");
                    } else {
                        COMMONAPI_ERROR("MethodAsync(dbus): Proxy callback available but callback taken");
                    }
                } else {
                    //create error message and push it directly to the connection
                    unsigned int dummySerial = 999;
                    _message.setSerial(dummySerial);   //set dummy serial
                    if (*sharedDbusMessageReplyAsyncHandler) {
                        FDBusMessage errorMessage = _message.createMethodError(DBUS_ERROR_UNKNOWN_METHOD);
                        _proxy.getFDBusConnection()->pushFDBusMessageReplyToMainLoop(errorMessage,
                        std::move(*sharedDbusMessageReplyAsyncHandler));
                        COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy callback not reachable -> sendMessageWithReplyAsync");
                    } else {
                        COMMONAPI_ERROR("MethodAsync(dbus): Proxy callback not reachable but callback taken");
                    }
                }
            }, _info);
            return callStatusFuture;
        }
    }

    /**
     * @brief call method async
     * 
     * @tparam FDBusProxy_ 
     * @tparam DelegateFunction_ 
     * @param _proxy 
     * @param _address 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _function 
     * @param _out 
     * @return std::future<CallStatus> 
     */
    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                        FDBusProxy_ &_proxy,
                        const FDBusAddress &_address,
                        const std::string &_method,
                        const std::string &_signature,
                        const CommonAPI::CallInfo *_info,
                        const InArgs_&... _in,
                        DelegateFunction_ _function,
                        std::tuple<OutArgs_...> _out) {
#ifndef _WIN32
        static std::mutex callMethodAsync_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodAsync_mutex_);

        FDBusMessage message = FDBusMessage::createMethodCall(_address, _method, _signature);
        return callMethodAsync(_proxy, message, _info, _in..., _function, _out);
    }

    /**
     * @brief call method async
     * 
     * @tparam FDBusProxy_ 
     * @tparam DelegateFunction_ 
     * @param _proxy 
     * @param _interface 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _function 
     * @param _out 
     * @return std::future<CallStatus> 
     */
    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                FDBusProxy_ &_proxy,
                const std::string &_interface,
                const std::string &_method,
                const std::string &_signature,
                const CommonAPI::CallInfo *_info,
                const InArgs_&... _in,
                DelegateFunction_ _function,
                std::tuple<OutArgs_...> _out) {
        FDBusAddress itsAddress(_proxy.getFDBusAddress());
        itsAddress.setInterface(_interface);
        return callMethodAsync(
                    _proxy, itsAddress,
                    _method, _signature,
                    _info,
                    _in..., _function,
                    _out);
    }

    /**
     * @brief call method async
     * 
     * @tparam FDBusProxy_ 
     * @tparam DelegateFunction_ 
     * @param _proxy 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _function 
     * @param _out 
     * @return std::future<CallStatus> 
     */
    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                    FDBusProxy_ &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    DelegateFunction_ _function,
                    std::tuple<OutArgs_...> _out) {
#ifndef _WIN32
        static std::mutex callMethodAsync_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodAsync_mutex_);

        FDBusMessage message = _proxy.createMethodCall(_method, _signature);
        return callMethodAsync(_proxy, message, _info, _in..., _function, _out);
    }

    template <size_t... ArgIndices_>
    static void callCallbackOnNotAvailable(std::function<void(CallStatus, OutArgs_&...)> _callback,
                                           index_sequence<ArgIndices_...>, std::tuple<OutArgs_...> _out) {
        const CallStatus status(CallStatus::NOT_AVAILABLE);
       _callback(status, std::get<ArgIndices_>(_out)...);
       (void)_out;
    }
};

/**
 * @brief FDBusProxyHelper
 * 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam Out_ 
 * @tparam OutArgs_ 
 * @tparam ErrorEvents_ 
 */
template<
    template <class ...> class In_, class... InArgs_,
    template <class...> class Out_, class... OutArgs_,
    class... ErrorEvents_>
struct FDBusProxyHelper<In_<FDBusInputStream, FDBusOutputStream, InArgs_...>,
                           Out_<FDBusInputStream, FDBusOutputStream, OutArgs_...>,
                           ErrorEvents_...> {
    
    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _message 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     * @param _errorEvents 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                    const FDBusProxy_ &_proxy,
                    const FDBusMessage &_message,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    CommonAPI::CallStatus &_status,
                    OutArgs_&... _out,
                    const std::tuple<ErrorEvents_*...> &_errorEvents) {

        if (sizeof...(InArgs_) > 0) {
            FDBusOutputStream output(_message);
            if (!FDBusSerializableArguments<InArgs_...>::serialize(output, _in...)) {
                COMMONAPI_ERROR("MethodSync w/ reply and error events (dbus): serialization failed: [",
                                _message.getInstance(), " ",
                                _message.getInterface(), " ",
                                _message.getMember(), " ",
                                _message.getSerial(), "]");

                _status = CallStatus::SERIALIZATION_ERROR;
                return;
            }
            output.flush();
        }

        FDBusError error;
        FDBusMessage reply = _proxy.getFDBusConnection()->sendFDBusMessageWithReplyAndBlock(_message, error, _info);
        if (error) {
            FDBusErrorEventHelper::notifyListeners(reply,
                                                  error.getName(),
                                                  typename make_sequence_range<sizeof...(ErrorEvents_), 0>::type(),
                                                  _errorEvents);
            _status = CallStatus::REMOTE_ERROR;
            return;
        }

        if (sizeof...(OutArgs_) > 0) {
            FDBusInputStream input(reply);
            if (!FDBusSerializableArguments<OutArgs_...>::deserialize(input, _out...)) {
                COMMONAPI_ERROR("MethodSync w/ reply and error events (dbus): reply deserialization failed: [",
                                reply.getInstance(), " ",
                                reply.getInterface(), " ",
                                reply.getMember(), " ",
                                reply.getSerial(), "]");

                _status = CallStatus::SERIALIZATION_ERROR;
                return;
            }
        }
        _status = CallStatus::SUCCESS;
    }

    /**
     * @brief call method with reply
     * 
     * @tparam FDBusProxy_ 
     * @param _proxy 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _status 
     * @param _out 
     * @param _errorEvents 
     */
    template <typename FDBusProxy_ = FDBusProxy>
    static void callMethodWithReply(
                    const FDBusProxy_ &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    CommonAPI::CallStatus &_status,
                    OutArgs_&... _out,
                    const std::tuple<ErrorEvents_*...> &_errorEvents){
#ifndef _WIN32
        static std::mutex callMethodWithReply_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodWithReply_mutex_);

        if (_proxy.isAvailable()) {
            FDBusMessage message = _proxy.createMethodCall(_method, _signature);
            callMethodWithReply(_proxy, message, _info, _in..., _status, _out..., _errorEvents);
        } else {
            _status = CallStatus::NOT_AVAILABLE;
        }
    }

    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                    FDBusProxy_ &_proxy,
                    const FDBusMessage &_message,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    DelegateFunction_ _function,
                    std::tuple<OutArgs_...> _out,
                    const std::tuple<ErrorEvents_*...> &_errorEvents) {
        if (sizeof...(InArgs_) > 0) {
            FDBusOutputStream output(_message);
            const bool success = FDBusSerializableArguments<
                                    InArgs_...
                                 >::serialize(output, _in...);
            if (!success) {
                COMMONAPI_ERROR("MethodAsync w/ reply and error events (dbus): serialization failed: [",
                                _message.getInstance(), " ",
                                _message.getInterface(), " ",
                                _message.getMember(), " ",
                                _message.getSerial(), "]");

                std::promise<CallStatus> promise;
                promise.set_value(CallStatus::SERIALIZATION_ERROR);
                return promise.get_future();
            }
            output.flush();
        }

        typename FDBusProxyAsyncCallbackHandler<FDBusProxy, OutArgs_...>::Delegate delegate(
                _proxy.shared_from_this(), _function);
        auto dbusMessageReplyAsyncHandler = std::move(FDBusProxyAsyncCallbackHandler<
                                                      std::tuple<ErrorEvents_*...>,
                                                      FDBusProxy,
                                                      OutArgs_...>::create(delegate, _out, _errorEvents));

        std::future<CallStatus> callStatusFuture;
        try {
            callStatusFuture = dbusMessageReplyAsyncHandler->getFuture();
        } catch (std::exception& e) {
            COMMONAPI_ERROR("MethodAsync(dbus): messageReplyAsyncHandler future failed(",
                            e.what(), ") [", _message.getInstance(), " ",
                            _message.getInterface(), " ",
                            _message.getMember(), " ",
                            _message.getSerial(), "]");
        }

        if(_proxy.isAvailable()) {
            if (_proxy.getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                            _message,
                            std::move(dbusMessageReplyAsyncHandler),
                            _info)){
            COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy available -> sendMessageWithReplyAsync");
                return callStatusFuture;
            } else {
                return std::future<CallStatus>();
            }
        } else {
            std::shared_ptr< std::unique_ptr< FDBusProxyConnection::FDBusMessageReplyAsyncHandler > > sharedDbusMessageReplyAsyncHandler(
                    new std::unique_ptr< FDBusProxyConnection::FDBusMessageReplyAsyncHandler >(std::move(dbusMessageReplyAsyncHandler)));
            //async isAvailable call with timeout
            COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy not available -> register callback");
            _proxy.isAvailableAsync([&_proxy, _message, sharedDbusMessageReplyAsyncHandler](
                                             const AvailabilityStatus _status,
                                             const Timeout_t remaining) {
                if(_status == AvailabilityStatus::AVAILABLE) {
                    //create new call info with remaining timeout. Minimal timeout is 100 ms.
                    Timeout_t newTimeout = remaining;
                    if(remaining < 100)
                        newTimeout = 100;
                    CallInfo newInfo(newTimeout);
                    if(*sharedDbusMessageReplyAsyncHandler) {
                    _proxy.getFDBusConnection()->sendFDBusMessageWithReplyAsync(
                                    _message,
                                    std::move(*sharedDbusMessageReplyAsyncHandler),
                                    &newInfo);
                    COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy callback available -> sendMessageWithReplyAsync");
                    } else {
                        COMMONAPI_ERROR("MethodAsync(dbus): Proxy callback available but callback taken");
                    }
                } else {
                    //create error message and push it directly to the connection
                    unsigned int dummySerial = 999;
                    _message.setSerial(dummySerial);   //set dummy serial
                    if (*sharedDbusMessageReplyAsyncHandler) {
                        FDBusMessage errorMessage = _message.createMethodError(DBUS_ERROR_UNKNOWN_METHOD);
                        _proxy.getFDBusConnection()->pushFDBusMessageReplyToMainLoop(errorMessage,
                        std::move(*sharedDbusMessageReplyAsyncHandler));
                        COMMONAPI_VERBOSE("MethodAsync(dbus): Proxy callback not reachable -> sendMessageWithReplyAsync");
                    } else {
                        COMMONAPI_ERROR("MethodAsync(dbus): Proxy callback not reachable but callback taken");
                    }
                }
            }, _info);
            return callStatusFuture;
        }
    }

    /**
     * @brief call method async
     * 
     * @tparam FDBusProxy_ 
     * @tparam DelegateFunction_ 
     * @param _proxy 
     * @param _method 
     * @param _signature 
     * @param _info 
     * @param _in 
     * @param _function 
     * @param _out 
     * @param _errorEvents 
     * @return std::future<CallStatus> 
     */
    template <typename FDBusProxy_ = FDBusProxy, typename DelegateFunction_>
    static std::future<CallStatus> callMethodAsync(
                    FDBusProxy_ &_proxy,
                    const std::string &_method,
                    const std::string &_signature,
                    const CommonAPI::CallInfo *_info,
                    const InArgs_&... _in,
                    DelegateFunction_ _function,
                    std::tuple<OutArgs_...> _out,
                    const std::tuple<ErrorEvents_*...> &_errorEvents) {
#ifndef _WIN32
        static std::mutex callMethodAsync_mutex_;
#endif
        std::lock_guard<std::mutex> lock(callMethodAsync_mutex_);

        FDBusMessage message = _proxy.createMethodCall(_method, _signature);
        return callMethodAsync(_proxy, message, _info, _in..., _function, _out, _errorEvents);
    }
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXYHELPER_HPP_
