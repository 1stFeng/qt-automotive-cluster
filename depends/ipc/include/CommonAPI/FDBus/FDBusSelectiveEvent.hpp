/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSSELECTIVEEVENT_HPP_
#define COMMONAPI_FDBUS_DBUSSELECTIVEEVENT_HPP_

#include <CommonAPI/FDBus/FDBusEvent.hpp>

namespace CommonAPI {
namespace FDBus {

template<typename EventType_, typename... Arguments_>
class FDBusSelectiveEvent: public FDBusEvent<EventType_, Arguments_...> {
public:
    typedef typename FDBusEvent<EventType_, Arguments_...>::Listener Listener;
    typedef FDBusEvent<EventType_, Arguments_...> FDBusEventBase;

    FDBusSelectiveEvent(FDBusProxy &_proxy,
                       const char *_name, const char *_signature,
                       std::tuple<Arguments_...> _arguments)
        : FDBusEventBase(_proxy, _name, _signature, _arguments) {
    }

    FDBusSelectiveEvent(FDBusProxy &_proxy,
                       const char *_name, const char *_signature,
                       const char *_path, const char *_interface,
                       std::tuple<Arguments_...> _arguments)
        : FDBusEventBase(_proxy, _name, _signature, _path, _interface, _arguments) {
    }

    virtual ~FDBusSelectiveEvent() {}

    virtual void onSpecificError(const CommonAPI::CallStatus status, uint32_t tag) {
        this->notifySpecificError(tag, status);
    }

    virtual void setSubscriptionToken(const FDBusProxyConnection::FDBusSignalHandlerToken _subscriptionToken, uint32_t tag) {
        this->subscription_ = _subscriptionToken;
        static_cast<FDBusProxy&>(this->proxy_).insertSelectiveSubscription(this->name_, this->signalHandler_, tag, this->signature_);
    }

protected:
    void onFirstListenerAdded(const Listener &) {
        this->init();
    }

    void onListenerAdded(const Listener &_listener, const uint32_t subscription) {
        (void) _listener;
        static_cast<FDBusProxy&>(this->proxy_).subscribeForSelectiveBroadcastOnConnection(
                this->mInstance, this->mInterface, this->name_, this->signature_, this->signalHandler_, subscription);
    }

    void onLastListenerRemoved(const Listener &) {
        static_cast<FDBusProxy&>(this->proxy_).unsubscribeFromSelectiveBroadcast(
                        this->name_, this->subscription_, this->signalHandler_.get());
    }
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSSELECTIVEEVENT_HPP_
