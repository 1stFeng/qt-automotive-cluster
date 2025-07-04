/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSMULTIEVENT_HPP_
#define COMMONAPI_FDBUS_DBUSMULTIEVENT_HPP_

#include <string>
#include <unordered_map>

#include <CommonAPI/Event.hpp>

namespace CommonAPI {
namespace FDBus {

template <typename... Arguments_>
class FDBusMultiEvent {
 public:
    typedef std::function<SubscriptionStatus(const std::string&, const Arguments_&...)> Listener;
    typedef std::unordered_multimap<std::string, Listener> ListenersMap;
    typedef typename ListenersMap::iterator Subscription;

    virtual ~FDBusMultiEvent() {}

    Subscription subscribeAll(const Listener& listener);
    Subscription subscribe(const std::string& eventName, const Listener& listener);

    void unsubscribe(Subscription listenerSubscription);

 protected:
    SubscriptionStatus notifyListeners(const std::string& name, const Arguments_&... eventArguments);

    virtual void onFirstListenerAdded(const std::string& name, const Listener& listener) { }
    virtual void onListenerAdded(const std::string& name, const Listener& listener) { }

    virtual void onListenerRemoved(const std::string& name, const Listener& listener) { }
    virtual void onLastListenerRemoved(const std::string& name, const Listener& listener) { }

 private:
    typedef std::pair<typename ListenersMap::iterator, typename ListenersMap::iterator> IteratorRange;
    SubscriptionStatus notifyListenersRange(const std::string& name, IteratorRange listenersRange, const Arguments_&... eventArguments);

    ListenersMap listenersMap_;
};

template <typename... Arguments_>
typename FDBusMultiEvent<Arguments_...>::Subscription
FDBusMultiEvent<Arguments_...>::subscribeAll(const Listener& listener) {
    return subscribe(std::string(), listener);
}

template <typename... Arguments_>
typename FDBusMultiEvent<Arguments_...>::Subscription
FDBusMultiEvent<Arguments_...>::subscribe(const std::string& eventName, const Listener& listener) {
    const bool firstListenerAdded = listenersMap_.empty();

    auto listenerSubscription = listenersMap_.insert({eventName, listener});

    if (firstListenerAdded) {
        onFirstListenerAdded(eventName, listener);
    }

    onListenerAdded(eventName, listener);

    return listenerSubscription;
}

template <typename... Arguments_>
void FDBusMultiEvent<Arguments_...>::unsubscribe(Subscription listenerSubscription) {
    const std::string name = listenerSubscription->first;
    const Listener listener = listenerSubscription->second;

    listenersMap_.erase(listenerSubscription);

    onListenerRemoved(name, listener);

    const bool lastListenerRemoved = listenersMap_.empty();
    if (lastListenerRemoved)
        onLastListenerRemoved(name, listener);
}

template <typename... Arguments_>
SubscriptionStatus FDBusMultiEvent<Arguments_...>::notifyListeners(const std::string& name, const Arguments_&... eventArguments) {
    const SubscriptionStatus subscriptionStatus = notifyListenersRange(name, listenersMap_.equal_range(name), eventArguments...);

    if (subscriptionStatus == SubscriptionStatus::CANCEL)
        return SubscriptionStatus::CANCEL;

    return notifyListenersRange(name, listenersMap_.equal_range(std::string()), eventArguments...);
}

template <typename... Arguments_>
SubscriptionStatus FDBusMultiEvent<Arguments_...>::notifyListenersRange(
        const std::string& name,
        IteratorRange listenersRange,
        const Arguments_&... eventArguments) {
    for (auto iterator = listenersRange.first; iterator != listenersRange.second; iterator++) {
        const Listener& listener = iterator->second;
        const SubscriptionStatus listenerSubcriptionStatus = listener(name, eventArguments...);

        if (listenerSubcriptionStatus == SubscriptionStatus::CANCEL) {
            auto listenerIterator = iterator;
            listenersMap_.erase(listenerIterator);
        }
    }

    return listenersMap_.empty() ? SubscriptionStatus::CANCEL : SubscriptionStatus::RETAIN;
}

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSMULTIEVENT_HPP_
