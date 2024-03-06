/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_CONNECTION_HPP_
#define COMMONAPI_FDBUS_DBUS_CONNECTION_HPP_

#include <atomic>
#include <mutex>

#include <fdbus/fdbus.h>

#include <CommonAPI/FDBus/FDBusConfig.hpp>
#include <CommonAPI/FDBus/FDBusMainLoop.hpp>
#include <CommonAPI/FDBus/FDBusMainLoopContext.hpp>
#include <CommonAPI/FDBus/FDBusObjectManager.hpp>
#include <CommonAPI/FDBus/FDBusStubManager.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusServiceRegistry.hpp>
#include <CommonAPI/FDBus/Types.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusConnection;

struct QueueEntry {
    QueueEntry() { }
    virtual ~QueueEntry() { }

    virtual void process(std::shared_ptr<FDBusConnection> _connection) = 0;
    virtual void clear() = 0;
};

struct MsgQueueEntry : QueueEntry {
     MsgQueueEntry(FDBusMessage _message) :
                       mMessage(_message) { }
     virtual ~MsgQueueEntry() { }
     FDBusMessage mMessage;

     virtual void process(std::shared_ptr<FDBusConnection> _connection);
     virtual void clear();
 };

struct MsgReplyQueueEntry : MsgQueueEntry {
    MsgReplyQueueEntry(FDBusProxyConnection::FDBusMessageReplyAsyncHandler* _replyAsyncHandler,
                   FDBusMessage _reply) :
                   MsgQueueEntry(_reply),
                   replyAsyncHandler_(_replyAsyncHandler) { }
    virtual ~MsgReplyQueueEntry() { }

    FDBusProxyConnection::FDBusMessageReplyAsyncHandler* replyAsyncHandler_;

    void process(std::shared_ptr<FDBusConnection> _connection);
    void clear();
};

struct AvblQueueEntry : QueueEntry {

    AvblQueueEntry(std::string _interface,
                   std::string _instance,
                   bool _isAvailable) :
                       interface_(_interface),
                       instance_(_instance),
                       isAvailable_(_isAvailable) { }

    std::string interface_;
    std::string instance_;

    bool isAvailable_;

    void process(std::shared_ptr<FDBusConnection> _connection);
    void clear();
};

template<class Function, class... Arguments>
struct FunctionQueueEntry : QueueEntry {

    using bindType = decltype(std::bind(std::declval<Function>(),std::declval<Arguments>()...));

    FunctionQueueEntry(Function&& _function,
                       Arguments&& ... _args):
                           bind_(std::forward<Function>(_function), std::forward<Arguments>(_args)...) { }

    bindType bind_;

    void process(std::shared_ptr<FDBusConnection> _connection);
    void clear();
};

class FDBusMainLoop;
class FDBusObjectManager;
class FDBusClient;
class FDBusServer;

class FDBusConnectionStatusEvent
        : public FDBusProxyConnection::ConnectionStatusEvent {
public:
    FDBusConnectionStatusEvent(FDBusConnection* dbusConnection);
    virtual ~FDBusConnectionStatusEvent() {}

 protected:
    virtual void onListenerAdded(const Listener& listener, const Subscription subscription);

    // TODO: change to std::weak_ptr<DBusConnection> connection_;
    FDBusConnection* dbusConnection_;

friend class FDBusConnection;
};

struct WatchContext {
    WatchContext(std::weak_ptr<MainLoopContext> mainLoopContext, DispatchSource* dispatchSource,
                 std::weak_ptr<FDBusConnection> dbusConnection) :
            mainLoopContext_(mainLoopContext), dispatchSource_(dispatchSource), dbusConnection_(dbusConnection) {
    }

    std::weak_ptr<MainLoopContext> mainLoopContext_;
    DispatchSource* dispatchSource_;
    std::weak_ptr<FDBusConnection> dbusConnection_;
};

struct TimeoutContext {
    TimeoutContext(std::weak_ptr<MainLoopContext> mainLoopContext,
                   std::weak_ptr<FDBusConnection> dbusConnection) :
            mainLoopContext_(mainLoopContext), dbusConnection_(dbusConnection) {
    }

    std::weak_ptr<MainLoopContext> mainLoopContext_;
    std::weak_ptr<FDBusConnection> dbusConnection_;
};

/**
 * 
 * @class FDBusConnection
 * @brief 
 * 
 */
class FDBusConnection
        : public FDBusProxyConnection,
          public std::enable_shared_from_this<FDBusConnection> {
public:
    COMMONAPI_EXPORT FDBusConnection(const ConnectionId_t& _connectionId);
    COMMONAPI_EXPORT FDBusConnection(const FDBusConnection&) = delete;
    COMMONAPI_EXPORT virtual ~FDBusConnection();

    COMMONAPI_EXPORT FDBusConnection& operator=(const FDBusConnection&) = delete;

    /**
     * @brief Create a Client object
     * 
     * @return COMMONAPI_EXPORT 
     */
    COMMONAPI_EXPORT void createClient(const FDBusAddress& fdbusAddress);
    COMMONAPI_EXPORT void doConnect(const FDBusAddress& address);

    /**
     * @brief Create a Server object
     * 
     * @return COMMONAPI_EXPORT 
     */
    COMMONAPI_EXPORT void createServer(const FDBusAddress& address);
    
    std::shared_ptr<FDBusClient> getClient(const std::string& interface, const std::string& instance) const;
    std::shared_ptr<FDBusServer> getServer(const std::string& interface, const std::string& instance) const;
    std::shared_ptr<FDBusClient> getClient(const FDBusAddress& address) const;
    std::shared_ptr<FDBusServer> getServer(const FDBusAddress& address) const;

    /**
     * @brief Start connection.
     * @param startDispatchThread Whether to start the mainloop thread
     * @return Whether the connection was started successfully
    */
    COMMONAPI_EXPORT bool connect(const FDBusType_t type, const FDBusAddress& fdbusAddress, bool startDispatchThread = true);

    COMMONAPI_EXPORT void disconnect();

    COMMONAPI_EXPORT virtual bool isConnected() const;

    COMMONAPI_EXPORT virtual ConnectionStatusEvent& getConnectionStatusEvent();

    COMMONAPI_EXPORT virtual bool requestServiceNameAndBlock(const std::string& serviceName) const;
    COMMONAPI_EXPORT virtual bool releaseServiceName(const std::string& serviceName) const;

    COMMONAPI_EXPORT bool sendFDBusMessage(const FDBusMessage& dbusMessage/*, uint32_t* allocatedSerial = NULL*/) const;

    COMMONAPI_EXPORT bool sendFDBusMessageWithReplyAsync(
            const FDBusMessage& dbusMessage,
            std::unique_ptr<FDBusMessageReplyAsyncHandler> dbusMessageReplyAsyncHandler,
            const CommonAPI::CallInfo *_info) const;

    COMMONAPI_EXPORT FDBusMessage sendFDBusMessageWithReplyAndBlock(const FDBusMessage& dbusMessage,
                                                 FDBusError& dbusError,
                                                 const CommonAPI::CallInfo *_info) const;

    virtual bool isAvailable(const FDBusAddress &_address);

    virtual AvailabilityHandlerId_t registerAvailabilityHandler(
            const FDBusAddress &_address, AvailabilityHandler_t _handler,
            std::weak_ptr<FDBusProxy> _proxy, void* _data);
    virtual void unregisterAvailabilityHandler(const FDBusAddress &_address,
            AvailabilityHandlerId_t _handlerId);

    virtual void registerService(const FDBusAddress &_address);

    virtual void unregisterService(const FDBusAddress &_address);

    virtual void requestService(const FDBusAddress &_address);

    virtual void releaseService(const FDBusAddress &_address);

    virtual void getAvailableInstances(std::string _interface, std::vector<std::string> *_instances);

    /**
     * @brief Establish correspondence between dbusBusName and handlers in the handler table.
     * @param dbusBusName.
     * @param fdbusSignalHandler, message's handler.
    */
    COMMONAPI_EXPORT virtual bool addObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                                      std::weak_ptr<FDBusSignalHandler> fdbusSignalHandler);

    /**
     * @brief Remove the correspondence between dbusBusName and handlers in the handler table.
     * @param dbusBusName.
     * @param fdbusSignalHandler, message's handler.
    */
    COMMONAPI_EXPORT virtual bool removeObjectManagerSignalMemberHandler(const std::string& dbusBusName,
                                                                         const FDBusSignalHandler* fdbusSignalHandler);

    /**
     * @brief Establish correspondence between matching fields and handlers in the handler table.
     * @param instance, Used to generate matching fields.
     * @param interfaceName, Used to generate matching fields.
     * @param interfaceMemberName, Used to generate matching fields.
     * @param interfaceMemberSignature, Used to generate matching fields.
     * @param fdbusSignalHandler, message's Handler.
     * @param justAddFilter.
    */
    COMMONAPI_EXPORT FDBusSignalHandlerToken addSignalMemberHandler(const std::string& instance,
                                                  const std::string& interfaceName,
                                                  const std::string& interfaceMemberName,
                                                  const std::string& interfaceMemberSignature,
                                                  std::weak_ptr<FDBusSignalHandler> fdbusSignalHandler,
                                                  const bool justAddFilter = false);

    COMMONAPI_EXPORT void subscribeForSelectiveBroadcast(const std::string& instance,
                                                   const std::string& interfaceName,
                                                   const std::string& interfaceMemberName,
                                                   const std::string& interfaceMemberSignature,
                                                   std::weak_ptr<FDBusSignalHandler> fdbusSignalHandler,
                                                   FDBusProxy* callingProxy,
                                                   uint32_t tag);

    COMMONAPI_EXPORT void unsubscribeFromSelectiveBroadcast(const std::string& eventName,
                                          FDBusSignalHandlerToken subscription,
                                          FDBusProxy* callingProxy,
                                          const FDBusSignalHandler* fdbusSignalHandler);

    /**
     * @brief Remove the correspondence between matching fields and handlers in the handler table.
     * @param fdbusBroadCastHandlerToken, consisting of instance, interfaceName, interfaceMemberName, interfaceMemberSignature.
     * @param fdbusSignalHandler, message's handler.
    */
    COMMONAPI_EXPORT bool removeSignalMemberHandler(const FDBusSignalHandlerToken& fdbusBroadCastHandlerToken,
                                                    const FDBusSignalHandler* fdbusSignalHandler = NULL);

    COMMONAPI_EXPORT bool readWriteDispatch(int timeoutMilliseconds = -1);

    COMMONAPI_EXPORT virtual const std::shared_ptr<FDBusObjectManager> getFDBusObjectManager();
    COMMONAPI_EXPORT virtual const std::shared_ptr<FDBusStubManager> getFDBusStubManager();
    /**
     * @brief Set the message handler for the current connection.
     * @param Handler for processing messages
    */
    COMMONAPI_EXPORT void setObjectPathMessageHandler(FDBusObjectPathMessageHandler);
    /**
     * @brief Determine whether the message handler has been set.
    */
    COMMONAPI_EXPORT bool isObjectPathMessageHandlerSet();

    COMMONAPI_EXPORT virtual bool attachMainLoopContext(std::weak_ptr<MainLoopContext>);

    COMMONAPI_EXPORT bool isDispatchReady();
    COMMONAPI_EXPORT bool singleDispatch();
    COMMONAPI_EXPORT void dispatchDBusMessageReply(const FDBusMessage& _reply,
                                                   FDBusMessageReplyAsyncHandler* _dbusMessageReplyAsyncHandler);

    /**
     * @brief Determine whether the DispatchThread is set.
    */
    COMMONAPI_EXPORT virtual bool hasDispatchThread();

    COMMONAPI_EXPORT virtual const ConnectionId_t& getConnectionId() const;

    /**
     * @brief Increase the count of connections.
    */
    COMMONAPI_EXPORT void incrementConnection();
    /**
     * @brief Reduce the count of connections. If the count equals 0, call disconnect.
    */
    COMMONAPI_EXPORT void decrementConnection();

    COMMONAPI_EXPORT bool setDispatching(bool isDispatching);

    template<class Function, class... Arguments>
    COMMONAPI_EXPORT void processFunctionQueueEntry(FunctionQueueEntry<Function, Arguments ...> &_functionQueueEntry);

    COMMONAPI_EXPORT void pushFDBusMessageReplyToMainLoop(const FDBusMessage& _reply,
                                      std::unique_ptr<FDBusMessageReplyAsyncHandler> _dbusMessageReplyAsyncHandler);

    template<class Function, class... Arguments>
    COMMONAPI_EXPORT void proxyPushFunctionToMainLoop(Function&& _function, Arguments&& ... _args);

#ifdef COMMONAPI_FDBUS_TEST
    COMMONAPI_EXPORT inline std::weak_ptr<FDBusMainLoop> getLoop() { return loop_; }
#endif

    COMMONAPI_EXPORT virtual void addSignalStateHandler(
            std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
            const uint32_t _subscription);

    COMMONAPI_EXPORT virtual void removeSignalStateHandler(
            std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler> _handler,
            const uint32_t _tag, bool _remove_all);

    COMMONAPI_EXPORT virtual void handleSignalStates();

    typedef std::tuple<std::string, std::string, std::string> FDBusSignalMatchRuleTuple;
    typedef std::pair<uint32_t, std::string> FDBusSignalMatchRuleMapping;
    typedef std::unordered_map<FDBusSignalMatchRuleTuple, FDBusSignalMatchRuleMapping> FDBusSignalMatchRulesMap;
    typedef std::unordered_map<std::string, size_t> FDBusOMSingalMatchRulesMap;
 //private:

    struct PendingCallNotificationData {
        PendingCallNotificationData(const FDBusConnection* _dbusConnection,
                                    FDBusMessageReplyAsyncHandler* _replyAsyncHandler) :
                                    dbusConnection_(_dbusConnection),
                                    replyAsyncHandler_(_replyAsyncHandler) { }

        const FDBusConnection* dbusConnection_;
        FDBusMessageReplyAsyncHandler* replyAsyncHandler_;
    };

    COMMONAPI_EXPORT void dispatch();
    COMMONAPI_EXPORT void suspendDispatching() const;
    COMMONAPI_EXPORT void resumeDispatching() const;

    std::thread* dispatchThread_;

    std::weak_ptr<MainLoopContext> mainLoopContext_;
    FDBusQueueWatch* queueWatch_;
    FDBusQueueDispatchSource* queueDispatchSource_;
    DispatchSource* dispatchSource_;
    WatchContext* watchContext_;

    COMMONAPI_EXPORT void addLibfdbusSignalMatchRule(const std::string& instance,
            const std::string& interfaceName,
            const std::string& interfaceMemberName,
            const bool justAddFilter = false);

    COMMONAPI_EXPORT void removeLibfdbusSignalMatchRule(const std::string& instance,
            const std::string& interfaceName,
            const std::string& interfaceMemberName);

    //TODO to delete
    COMMONAPI_EXPORT void initLibdbusSignalFilterAfterConnect();

    //COMMONAPI_EXPORT void initLibfdbusSubscribeBraodCastAfterConnect();

    COMMONAPI_EXPORT ::DBusHandlerResult onLibdbusSignalFilter(ipc::fdbus::CFdbMessage* libdbusMessage);

    /**
     * @brief Callback function for message, Both input and output parameters need to be changed.
     * @param libfdbusMessage libfdbus's Message
     * @return Results of processing
    */
    COMMONAPI_EXPORT ::DBusHandlerResult onLibdbusObjectPathMessage(ipc::fdbus::CFdbMessage* libdbusMessage);

    COMMONAPI_EXPORT void onLibdbusPendingCallNotifyThunk(CFdbMessage* libfdbusMessage, FDBusMessageReplyAsyncHandler* userData);

    void onLibfdbusPendingCall(::DBusPendingCall* _libdbusPendingCall,
                              const FDBusMessage& _reply,
                              FDBusMessageReplyAsyncHandler* _dbusMessageReplyAsyncHandler) const;

    /**
     * @brief Callback function for broadcast message.
     * @param libdbusMessage, libfdbus's Message, Broadcast messages that need to be handle.
    */
    bool onLibfdbusBroadcast(CFdbMessage* libdbusMessage);

    COMMONAPI_EXPORT void onClientStatus(const FDBusAddress& address, const CFdbOnlineInfo &info, const AvailabilityStatus& availabilityStatus);

    COMMONAPI_EXPORT void onServerStatus(const FDBusAddress& address, const CFdbOnlineInfo &info, const AvailabilityStatus& availabilityStatus);
    void processAvblQueueEntry(AvblQueueEntry &_avblQueueEntry);

    COMMONAPI_EXPORT void onAvailabilityChange(std::string _interface, std::string _instance,
            bool _is_available);
    void handleAvailabilityChange(const std::string _interface, const std::string _instance,
                                  bool _is_available);

    COMMONAPI_EXPORT bool broadCastMessage(const FDBusMessage &_message) const;

    COMMONAPI_EXPORT void enforceAsynchronousTimeouts();

    COMMONAPI_EXPORT void sendPendingSelectiveSubscription(FDBusProxy* proxy,
                                                           std::string interfaceMemberName,
                                                           std::weak_ptr<FDBusSignalHandler> fdbusSignalHandler,
                                                           uint32_t tag,
                                                           std::string interfaceMemberSignature);

    /**
     * @brief Notify the handle of the broadcast to process the message.
     * @param handlerPath, Matching fields used to find a specific handler.
     * @param dbusMessage, messages to be handle.
     * @param dbusHandlerResult, results of message processing.
    */
    COMMONAPI_EXPORT void notifyFDBusSignalHandlers(FDBusSingalHandlerPath handlerPath,
                                        const FDBusMessage& dbusMessage,
                                        ::DBusHandlerResult& dbusHandlerResult);

    COMMONAPI_EXPORT void notifyFDBusOMSignalHandlers(const char* dbusSenderName,
                                    const FDBusMessage& dbusMessage,
                                    ::DBusHandlerResult& dbusHandlerResult);

    /**
     * @brief Delete handlers from the queue
    */
    void deleteAsyncHandlers();

    mutable std::recursive_mutex connectionGuard_;

    std::mutex signalHandlersGuard_;

    std::mutex objectManagerGuard_;
    std::mutex serviceRegistryGuard_;

    std::shared_ptr<FDBusObjectManager> dbusObjectManager_;
    std::mutex mStubManagerGuard;
    std::shared_ptr<FDBusStubManager> mStubManager;

    FDBusConnectionStatusEvent dbusConnectionStatusEvent_;

    FDBusSignalMatchRulesMap fdbusSignalMatchRulesMap_;

    FDBusSignalHandlerTable fdbusSignalHandlers_;

    std::mutex fdbusOMSignalHandlersGuard_;

    FDBusOMSingalMatchRulesMap fdbusOMSignalMatchRulesMap_;

    FDBusOMBroadCastHandlerTable fdbusOMSignalHandlers_;

    COMMONAPI_EXPORT bool addLibfdbusSubscribe();
    COMMONAPI_EXPORT bool removeLibfdbusSubscribe();

    COMMONAPI_EXPORT bool addObjectManagerSignalMatchRule(const std::string& dbusBusName);
    COMMONAPI_EXPORT bool removeObjectManagerSignalMatchRule(const std::string& dbusBusName);

    COMMONAPI_EXPORT bool addLibfdbusSignalMatchRule(const std::string& dbusMatchRule);
    COMMONAPI_EXPORT bool removeLibfdbusSignalMatchRule(const std::string& dbusMatchRule);

    std::atomic_size_t libfdbusSignalMatchRulesCount_;

    // instance, referenceCount
    typedef std::unordered_map<std::string, uint32_t> LibdbusRegisteredObjectPathHandlersTable;
    LibdbusRegisteredObjectPathHandlersTable libdbusRegisteredObjectPaths_;

    FDBusObjectPathMessageHandler dbusObjectMessageHandler_;

    mutable std::unordered_map<std::string, uint16_t> connectionNameCount_;

    typedef std::pair<
            DBusPendingCall*,
            std::tuple<
                std::chrono::steady_clock::time_point,
                FDBusMessageReplyAsyncHandler*,
                FDBusMessage
            >
        > TimeoutMapElement;
    mutable std::map<
            DBusPendingCall*,
            std::tuple<
                std::chrono::steady_clock::time_point,
                FDBusMessageReplyAsyncHandler*,
                FDBusMessage
            >
        > timeoutMap_;

    typedef std::tuple<
                FDBusMessageReplyAsyncHandler *,
                FDBusMessage,
                CommonAPI::CallStatus,
                ::DBusPendingCall*
            > MainloopTimeout_t;
    mutable std::list<MainloopTimeout_t> mainloopTimeouts_;
    mutable std::mutex mainloopTimeoutsMutex_;

    mutable std::mutex enforceTimeoutMutex_;
    mutable std::condition_variable_any enforceTimeoutCondition_;

    mutable std::thread* enforcerThread_;
    mutable std::recursive_mutex enforcerThreadMutex_;
    bool enforcerThreadCancelled_;
    ConnectionId_t connectionId_;

    std::shared_ptr<FDBusMainLoop> loop_;

    // set contains asyncHandlers with infinite timeout
    mutable std::set<FDBusMessageReplyAsyncHandler*> timeoutInfiniteAsyncHandlers_;
    mutable std::mutex timeoutInfiniteAsyncHandlersMutex_;

    int activeConnections_;
    mutable std::mutex activeConnectionsMutex_;

    bool isDisconnecting_;
    bool isDispatching_;
    bool isWaitingOnFinishedDispatching_;

    std::set<std::thread::id> dispatchThreads_;
    std::condition_variable_any dispatchCondition_;

    std::vector<FDBusMessageReplyAsyncHandler*> asyncHandlersToDelete_;
    std::mutex asyncHandlersToDeleteMutex_;

    std::map<std::shared_ptr<FDBusProxyConnection::FDBusSignalHandler>, std::set<uint32_t>> signalStateHandlers_;
    std::mutex signalStateHandlersMutex_;

    // interface, instance
    std::map<std::string, std::map<std::string, std::shared_ptr<FDBusClient>>> mClientMap;
    std::map<std::string, std::map<std::string, std::shared_ptr<FDBusServer>>> mServerMap;
    CBaseWorker* mServerWorker;
    CBaseWorker* mClientWorker;

    //TODO
    mutable std::map<int32_t, std::shared_ptr<PendingCallNotificationData>> mAsyncReplyHandlerMap;
    mutable std::mutex mAsyncReplyHandlerMapMutex;

    mutable std::mutex availabilityMutex_;

    // interface, instance
    typedef std::map<std::string,
            std::map<std::string,
                    std::map<AvailabilityHandlerId_t, std::tuple<AvailabilityHandler_t,
                                                          std::weak_ptr<FDBusProxy>,
                                                          void*>> > > availability_map_t;
    availability_map_t availabilityHandlers_;
    std::map<std::string, std::map<std::string, bool>> availabilityCalled_;
};

template<class Function, class... Arguments>
void FunctionQueueEntry<Function, Arguments ...>::process(std::shared_ptr<FDBusConnection> _connection) {
    _connection->processFunctionQueueEntry(*this);
}

template<class Function, class... Arguments>
void FunctionQueueEntry<Function, Arguments ...>::clear() {
}

template<class Function, class... Arguments>
void FDBusConnection::processFunctionQueueEntry(FunctionQueueEntry<Function, Arguments ...> &_functionQueueEntry) {
    _functionQueueEntry.bind_();
}

template<class Function, class... Arguments>
void FDBusConnection::proxyPushFunctionToMainLoop(Function&& _function, Arguments&& ... _args) {
    if (auto lockedContext = mainLoopContext_.lock()) {
        std::shared_ptr<FunctionQueueEntry<Function, Arguments ...>> functionQueueEntry = std::make_shared<FunctionQueueEntry<Function, Arguments ...>>(
                std::forward<Function>(_function), std::forward<Arguments>(_args) ...);
        queueWatch_->pushQueue(functionQueueEntry);
    }
}


} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_CONNECTION_HPP_
