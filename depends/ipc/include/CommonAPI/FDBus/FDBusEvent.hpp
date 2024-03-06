/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_EVENT_HPP_
#define COMMONAPI_FDBUS_DBUS_EVENT_HPP_

#include <CommonAPI/Event.hpp>
#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusProxyBase.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusSerializableArguments.hpp>

namespace CommonAPI {
namespace FDBus {

template <typename Event_, typename... Arguments_>
class FDBusEvent: public Event_ {
public:
    typedef typename Event_::Subscription Subscription;
    typedef typename Event_::Listener Listener;

    FDBusEvent(FDBusProxyBase &_proxy,
              const std::string &_name, const std::string &_signature,
              std::tuple<Arguments_...> _arguments)
        : proxy_(_proxy),
          signalHandler_(),
          name_(_name), signature_(_signature),
          getMethodName_(""),
          arguments_(_arguments) {

        mInstance = proxy_.getFDBusAddress().getInstance();
        mInterface = proxy_.getFDBusAddress().getInterface();
    }

    FDBusEvent(FDBusProxyBase &_proxy,
              const std::string &_name, const std::string &_signature,
              const std::string &_instance, const std::string &_interface,
              std::tuple<Arguments_...> _arguments)
        : proxy_(_proxy),
          signalHandler_(),
          name_(_name), signature_(_signature),
          mInstance(_instance), mInterface(_interface),
          getMethodName_(""),
          arguments_(_arguments) {
    }

    FDBusEvent(FDBusProxyBase &_proxy,
              const std::string &_name,
              const std::string &_signature,
              const std::string &_getMethodName,
              std::tuple<Arguments_...> _arguments)
        : proxy_(_proxy),
          signalHandler_(),
          name_(_name),
          signature_(_signature),
          getMethodName_(_getMethodName),
          arguments_(_arguments) {

        mInstance = proxy_.getFDBusAddress().getInstance();
        mInterface = proxy_.getFDBusAddress().getInterface();
    }

    virtual ~FDBusEvent() {
        proxy_.removeSignalStateHandler(signalHandler_, 0, true);
        proxy_.removeSignalMemberHandler(subscription_, signalHandler_.get());
    }

    virtual void onError(const CommonAPI::CallStatus status) {
        (void)status;
    }

    virtual void onSpecificError(const CommonAPI::CallStatus status, const uint32_t tag) {
        this->notifySpecificError(tag, status);
    }

    virtual void setSubscriptionToken(const FDBusProxyConnection::FDBusSignalHandlerToken token, const uint32_t tag) {
        (void)token;
        (void)tag;
    }

 protected:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif

    class SignalHandler : public FDBusProxyConnection::FDBusSignalHandler,
                    public std::enable_shared_from_this<SignalHandler> {
    public:
        SignalHandler(FDBusProxyBase&_proxy,
                FDBusEvent<Event_, Arguments_ ...>* _dbusEvent) :
            proxy_(_proxy.getWeakPtr()),
            dbusEvent_(_dbusEvent) {

        }

        virtual void onSignalFDBusMessage(const FDBusMessage &_message) {
            if(auto itsProxy = proxy_.lock()) {
                dbusEvent_->handleSignalFDBusMessage(_message, typename make_sequence<sizeof...(Arguments_)>::type());
            }
        }

        virtual void onInitialValueSignalDBusMessage(const FDBusMessage&_message, const uint32_t tag) {
            if(auto itsProxy = proxy_.lock()) {
                dbusEvent_->handleSignalFDBusMessage(tag, _message, typename make_sequence<sizeof...(Arguments_)>::type());
            }
        }

        virtual void onSpecificError(const CommonAPI::CallStatus status, const uint32_t tag) {
            if(auto itsProxy = proxy_.lock()) {
                dbusEvent_->onSpecificError(status, tag);
            }
        }

        virtual void setSubscriptionToken(const FDBusProxyConnection::FDBusSignalHandlerToken token, const uint32_t tag) {
            if(auto itsProxy = proxy_.lock()) {
                dbusEvent_->setSubscriptionToken(token, tag);
            }
        }

    private :
        std::weak_ptr<FDBusProxyBase> proxy_;
        FDBusEvent<Event_, Arguments_ ...>* dbusEvent_;
    };

    virtual void onFirstListenerAdded(const Listener &_listener) {
        (void)_listener;
        init();
        subscription_ = proxy_.addSignalMemberHandler(
                            mInstance, mInterface, name_, signature_, getMethodName_, signalHandler_, false);
    }

    virtual void onListenerAdded(const Listener &_listener, const Subscription subscription) {
        (void)_listener;
        if ("" != getMethodName_) {
            proxy_.getCurrentValueForSignalListener(getMethodName_, signalHandler_, subscription);
        }
        proxy_.addSignalStateHandler(signalHandler_, subscription);
    }

    virtual void onListenerRemoved(const Listener &_listener, const Subscription _subscription) {
        (void)_listener;
        proxy_.removeSignalStateHandler(signalHandler_, _subscription);
    }

    virtual void onLastListenerRemoved(const Listener&) {
        proxy_.removeSignalMemberHandler(subscription_, signalHandler_.get());
        std::get<0>(subscription_) = "";
        std::get<1>(subscription_) = "";
        std::get<2>(subscription_) = "";
        std::get<3>(subscription_) = "";
    }

    template<size_t ... Indices_>
    inline void handleSignalFDBusMessage(const FDBusMessage &_message, index_sequence<Indices_...>) {
        FDBusInputStream input(_message);
        if (FDBusSerializableArguments<
                Arguments_...
            >::deserialize(input, std::get<Indices_>(arguments_)...)) {
            this->notifyListeners(std::get<Indices_>(arguments_)...);
        }
    }

    template<size_t ... Indices_>
    inline void handleSignalFDBusMessage(const uint32_t tag, const FDBusMessage &_message, index_sequence<Indices_...>) {
        FDBusInputStream input(_message);
        if (FDBusSerializableArguments<
                Arguments_...
            >::deserialize(input, std::get<Indices_>(arguments_)...)) {
            this->notifySpecificListener(tag, std::get<Indices_>(arguments_)...);
        }
    }

    virtual void init() {
        if (!signalHandler_) {
            signalHandler_ = std::make_shared<SignalHandler>(proxy_, this);
        }
    }

    FDBusProxyBase &proxy_;
    std::shared_ptr<SignalHandler> signalHandler_;

    std::string name_;
    std::string signature_;
    std::string mInstance;
    std::string mInterface;
    std::string getMethodName_;

    FDBusProxyConnection::FDBusSignalHandlerToken subscription_;
    std::tuple<Arguments_...> arguments_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_EVENT_HPP_
