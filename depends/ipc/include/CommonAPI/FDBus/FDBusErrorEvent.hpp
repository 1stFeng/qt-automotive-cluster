/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSERROREVENT_HPP_
#define COMMONAPI_FDBUS_DBUSERROREVENT_HPP_

#include <CommonAPI/Event.hpp>

#include <CommonAPI/FDBus/FDBusInputStream.hpp>

namespace CommonAPI {
namespace FDBus {

template <class...>
class FDBusErrorEvent;

template<>
class FDBusErrorEvent<> : public Event<std::string> {
public:
    FDBusErrorEvent(const std::string &_errorName) :
        errorName_(_errorName) {}

    FDBusErrorEvent(const FDBusErrorEvent &_source) :
        errorName_(_source.errorName_) {}

    virtual ~FDBusErrorEvent() {}

    inline const std::string & getErrorName() const {
        return errorName_;
    }

    void notifyErrorEventListeners(const FDBusMessage &_reply) {
        (void)_reply;
        this->notifyListeners(errorName_);
    }

private:
    std::string errorName_;
};

template <
    template <class...> class In_, class... InArgs_,
    template <class...> class DeplIn_, class... DeplIn_Args>
class FDBusErrorEvent<
        In_<InArgs_...>,
        DeplIn_<DeplIn_Args...>> : public Event<std::string, InArgs_...> {
public:
    FDBusErrorEvent(const std::string &_errorName,
                   const std::tuple<DeplIn_Args*...> &_in) :
        errorName_(_errorName) {
        initialize(typename make_sequence_range<sizeof...(DeplIn_Args), 0>::type(), _in);
    }

    FDBusErrorEvent(const FDBusErrorEvent &_source) :
        errorName_(_source.errorName_),
        in_(_source.in_) {}

    virtual ~FDBusErrorEvent() {}

    inline const std::string & getErrorName() const {
        return errorName_;
    }

    void notifyErrorEventListeners(const FDBusMessage &_reply) {
        deserialize(_reply, typename make_sequence_range<sizeof...(InArgs_), 0>::type());
    }

private:

    template <size_t... DeplIn_ArgIndices>
    inline void initialize(index_sequence<DeplIn_ArgIndices...>, const std::tuple<DeplIn_Args*...> &_in) {
        in_ = std::make_tuple(std::get<DeplIn_ArgIndices>(_in)...);
    }

    template <size_t... InArgIndices_>
    void deserialize(const FDBusMessage &_reply,
                     index_sequence<InArgIndices_...>) {
        if (sizeof...(InArgs_) > 0) {
            FDBusInputStream dbusInputStream(_reply);
            const bool success = FDBusSerializableArguments<CommonAPI::Deployable<InArgs_, DeplIn_Args>...>::deserialize(dbusInputStream, std::get<InArgIndices_>(in_)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusErrorEvent::", __func__, "(", errorName_, "): deserialization failed!");
                return;
            }
            this->notifyListeners(errorName_, std::move(std::get<InArgIndices_>(in_).getValue())...);
        }
    }

    std::string errorName_;
    std::tuple<CommonAPI::Deployable<InArgs_, DeplIn_Args>...> in_;
};

class FDBusErrorEventHelper {
public:
    template <size_t... ErrorEventsIndices_, class... ErrorEvents_>
    static void notifyListeners(const FDBusMessage &_reply,
                                const std::string &_errorName,
                                index_sequence<ErrorEventsIndices_...>,
                                const std::tuple<ErrorEvents_*...> &_errorEvents) {

        notifyListeners(_reply, _errorName, std::get<ErrorEventsIndices_>(_errorEvents)...);
    }

    template <class ErrorEvent_>
    static void notifyListeners(const FDBusMessage &_reply,
                                const std::string &_errorName,
                                ErrorEvent_ *_errorEvent) {
        if(_errorEvent->getErrorName() == _errorName)
            _errorEvent->notifyErrorEventListeners(_reply);

    }

    template <class ErrorEvent_, class... Rest_>
    static void notifyListeners(const FDBusMessage &_reply,
                                const std::string &_errorName,
                                ErrorEvent_ *_errorEvent,
                                Rest_&... _rest) {
        if(_errorEvent->getErrorName() == _errorName)
            _errorEvent->notifyErrorEventListeners(_reply);
        notifyListeners(_reply, _errorName, _rest...);
    }
};

} // namespace FDBus
} // namespace CommonAPI

#endif /* COMMONAPI_FDBUS_DBUSERROREVENT_HPP_ */
