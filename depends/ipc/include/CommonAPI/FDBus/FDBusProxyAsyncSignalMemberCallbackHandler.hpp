/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSPROXYASYNCSIGNALMEMBERCALLBACKHANDLER_HPP_
#define COMMONAPI_FDBUS_DBUSPROXYASYNCSIGNALMEMBERCALLBACKHANDLER_HPP_

#include <functional>
#include <future>
#include <memory>

//#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>

namespace CommonAPI {
namespace FDBus {

template <typename DelegateObjectType_>
class FDBusProxyAsyncSignalMemberCallbackHandler: public FDBusProxyConnection::FDBusMessageReplyAsyncHandler {
 public:

    struct Delegate {
        typedef std::function<void(CallStatus, FDBusMessage, std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler>, uint32_t)> FunctionType;

        Delegate(std::shared_ptr<DelegateObjectType_> object, FunctionType function) :
            function_(std::move(function)) {
            object_ = object;
        }
        std::weak_ptr<DelegateObjectType_> object_;
        FunctionType function_;
    };

    static std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler> create(
            Delegate& delegate, std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusSignalHandler,
            const uint32_t tag) {
        return std::unique_ptr<FDBusProxyConnection::FDBusMessageReplyAsyncHandler>(
                new FDBusProxyAsyncSignalMemberCallbackHandler<DelegateObjectType_>(std::move(delegate), fdbusSignalHandler, tag));
    }

    FDBusProxyAsyncSignalMemberCallbackHandler() = delete;
    FDBusProxyAsyncSignalMemberCallbackHandler(Delegate&& delegate,
                                              std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusSignalHandler,
                                              const uint32_t tag):
        delegate_(std::move(delegate)), fdbusSignalHandler_(fdbusSignalHandler), tag_(tag),
        executionStarted_(false), executionFinished_(false),
        timeoutOccurred_(false), hasToBeDeleted_(false) {
    }
    virtual ~FDBusProxyAsyncSignalMemberCallbackHandler() {}

    virtual std::future<CallStatus> getFuture() {
        return promise_.get_future();
    }

    virtual void onFDBusMessageReply(const CallStatus& dbusMessageCallStatus, const FDBusMessage& dbusMessage) {
        promise_.set_value(handleFDBusMessageReply(dbusMessageCallStatus, dbusMessage));
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

 private:
    inline CallStatus handleFDBusMessageReply(const CallStatus dbusMessageCallStatus, const FDBusMessage& dbusMessage) const {
        CallStatus callStatus = dbusMessageCallStatus;

        //ensure that delegate object (i.e Proxy) is not destroyed while callback function is invoked
        if(auto itsDelegateObject = this->delegate_.object_.lock())
            this->delegate_.function_(callStatus, dbusMessage, fdbusSignalHandler_, tag_);

        return callStatus;
    }

    std::promise<CallStatus> promise_;
    const Delegate delegate_;
    std::weak_ptr<FDBusProxyConnection::FDBusSignalHandler> fdbusSignalHandler_;
    const uint32_t tag_;
    bool executionStarted_;
    bool executionFinished_;
    bool timeoutOccurred_;
    bool hasToBeDeleted_;

    std::mutex asyncHandlerMutex_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSPROXYASYNCSIGNALMEMBERCALLBACKHANDLER_HPP_
