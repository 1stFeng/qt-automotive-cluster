/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXYASYNCCALLBACKHANDLER_HPP_
#define COMMONAPI_FDBUS_DBUSPROXYASYNCCALLBACKHANDLER_HPP_

#include <functional>
#include <future>
#include <memory>

#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusSerializableArguments.hpp>
#include <CommonAPI/FDBus/FDBusErrorEvent.hpp>

namespace CommonAPI {
namespace FDBus {

template <class, class...>
class FDBusProxyAsyncCallbackHandler;

template <class DelegateObjectType_, class ... ArgTypes_>
class FDBusProxyAsyncCallbackHandler :
        public FDBusProxyConnection::FDBusMessageReplyAsyncHandler {
 public:

    struct Delegate {
        typedef std::function<void(CallStatus, ArgTypes_...)> FunctionType;

        Delegate(std::shared_ptr<DelegateObjectType_> _object, FunctionType _function) :
            function_(std::move(_function)) {
            object_ = _object;
        }
        std::weak_ptr<DelegateObjectType_> object_;
        FunctionType function_;
    };

    static std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler> create(
            Delegate& _delegate, const std::tuple<ArgTypes_...>& _args) {
        return std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler>(
                new FDBusProxyAsyncCallbackHandler<DelegateObjectType_, ArgTypes_...>(std::move(_delegate), _args));
    }

    FDBusProxyAsyncCallbackHandler() = delete;
    FDBusProxyAsyncCallbackHandler(Delegate&& _delegate, const std::tuple<ArgTypes_...>& _args)
        : delegate_(std::move(_delegate)),
          args_(_args),
          executionStarted_(false),
          executionFinished_(false),
          timeoutOccurred_(false),
          hasToBeDeleted_(false) {
    }
    virtual ~FDBusProxyAsyncCallbackHandler() {
        // free assigned std::function<> immediately
        delegate_.function_ = [](CallStatus, ArgTypes_...) {};
    }

    virtual std::future<CallStatus> getFuture() {
        return promise_.get_future();
    }

    virtual void onFDBusMessageReply(const CallStatus& _dbusMessageCallStatus,
            const FDBusMessage& _dbusMessage) {
        promise_.set_value(handleFDBusMessageReply(_dbusMessageCallStatus,
                _dbusMessage,
                typename make_sequence<sizeof...(ArgTypes_)>::type(), args_));
    }

    virtual void setExecutionStarted() {
        executionStarted_ = true;
    }

    virtual bool getExecutionStarted() {
        return executionStarted_;
    }

    virtual void setExecutionFinished() {
        executionFinished_ = true;
    }

    virtual bool getExecutionFinished() {
        return executionFinished_;
    }

    virtual void setTimeoutOccurred() {
        timeoutOccurred_ = true;
    }

    virtual bool getTimeoutOccurred() {
        return timeoutOccurred_;
    }

    virtual void setHasToBeDeleted() {
        hasToBeDeleted_ = true;
    }

    virtual bool hasToBeDeleted() {
        return hasToBeDeleted_;
    }

    virtual void lock() {
        asyncHandlerMutex_.lock();
    }

    virtual void unlock() {
        asyncHandlerMutex_.unlock();
    }

 protected:
    Delegate delegate_;
    std::promise<CallStatus> promise_;
    std::tuple<ArgTypes_...> args_;

 private:
    template <size_t... ArgIndices_>
    inline CallStatus handleFDBusMessageReply(
            const CallStatus _dbusMessageCallStatus,
            const FDBusMessage& _dbusMessage,
            index_sequence<ArgIndices_...>,
            std::tuple<ArgTypes_...>& _argTuple) const {
        (void)_argTuple; // this suppresses warning "set but not used" in case of empty _ArgTypes
                        // Looks like: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57560 - Bug 57650

        CallStatus callStatus = _dbusMessageCallStatus;

        if (_dbusMessageCallStatus == CallStatus::SUCCESS) {
            FDBusInputStream dbusInputStream(_dbusMessage);
            if(!FDBusSerializableArguments<ArgTypes_...>::deserialize(dbusInputStream,
                    std::get<ArgIndices_>(_argTuple)...)) {
                callStatus = CallStatus::SERIALIZATION_ERROR;
            }
        }

        //ensure that delegate object (e.g. Proxy) survives
        if(auto itsDelegateObject = delegate_.object_.lock())
            delegate_.function_(callStatus, std::move(std::get<ArgIndices_>(_argTuple))...);

        return callStatus;
    }

    bool executionStarted_;
    bool executionFinished_;
    bool timeoutOccurred_;
    bool hasToBeDeleted_;

    std::mutex asyncHandlerMutex_;
};

template<
    template <class...> class ErrorEventsTuple_, class... ErrorEvents_,
    class DelegateObjectType_, class... ArgTypes_>
class FDBusProxyAsyncCallbackHandler<
    ErrorEventsTuple_<ErrorEvents_...>,
    DelegateObjectType_,
    ArgTypes_...>:
        public FDBusProxyAsyncCallbackHandler<DelegateObjectType_, ArgTypes_...> {

public:

    static std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler> create(
            typename FDBusProxyAsyncCallbackHandler<DelegateObjectType_, ArgTypes_...>::Delegate& _delegate,
            const std::tuple<ArgTypes_...>& _args,
            const ErrorEventsTuple_<ErrorEvents_...> &_errorEvents) {
        return std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler>(
                new FDBusProxyAsyncCallbackHandler<ErrorEventsTuple_<ErrorEvents_...>, DelegateObjectType_, ArgTypes_...>(std::move(_delegate), _args, _errorEvents));
    }

    FDBusProxyAsyncCallbackHandler() = delete;
    FDBusProxyAsyncCallbackHandler(typename FDBusProxyAsyncCallbackHandler<DelegateObjectType_, ArgTypes_...>::Delegate&& _delegate,
                                  const std::tuple<ArgTypes_...>& _args,
                                  const ErrorEventsTuple_<ErrorEvents_...> &_errorEvents) :
                                      FDBusProxyAsyncCallbackHandler<DelegateObjectType_, ArgTypes_...>(std::move(_delegate), _args),
                                      errorEvents_(_errorEvents) {}

    virtual ~FDBusProxyAsyncCallbackHandler() {
        // free assigned std::function<> immediately
        this->delegate_.function_ = [](CallStatus, ArgTypes_...) {};
    }

    virtual void onFDBusMessageReply(const CallStatus& _dbusMessageCallStatus,
            const FDBusMessage& _dbusMessage) {
        this->promise_.set_value(handleFDBusMessageReply(
                _dbusMessageCallStatus,
                _dbusMessage,
                typename make_sequence<sizeof...(ArgTypes_)>::type(), this->args_));
    }

private:

    template <size_t... ArgIndices_>
    inline CallStatus handleFDBusMessageReply(
            const CallStatus _dbusMessageCallStatus,
            const FDBusMessage& _dbusMessage,
            index_sequence<ArgIndices_...>,
            std::tuple<ArgTypes_...>& _argTuple) const {
        (void)_argTuple; // this suppresses warning "set but not used" in case of empty _ArgTypes
                        // Looks like: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57560 - Bug 57650

        CallStatus callStatus = _dbusMessageCallStatus;

        if (_dbusMessageCallStatus == CallStatus::SUCCESS) {
            FDBusInputStream dbusInputStream(_dbusMessage);
            if (!FDBusSerializableArguments<ArgTypes_...>::deserialize(dbusInputStream,
                    std::get<ArgIndices_>(_argTuple)...)) {
                callStatus = CallStatus::SERIALIZATION_ERROR;
            }
        } else {
            if(_dbusMessage.isErrorType()) {
                //ensure that delegate object (e.g. Proxy and its error events) survives
                if(auto itsDelegateObject = this->delegate_.object_.lock()) {
                    FDBusErrorEventHelper::notifyListeners(_dbusMessage,
                                                          _dbusMessage.getError(),
                                                          typename make_sequence_range<sizeof...(ErrorEvents_), 0>::type(),
                                                          errorEvents_);
                }
            }
        }

        //ensure that delegate object (i.e Proxy) is not destroyed while callback function is invoked
        if(auto itsDelegateObject = this->delegate_.object_.lock())
            this->delegate_.function_(callStatus, std::move(std::get<ArgIndices_>(_argTuple))...);

        return callStatus;
    }

    ErrorEventsTuple_<ErrorEvents_...> errorEvents_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXYASYNCCALLBACKHANDLER_HPP_
