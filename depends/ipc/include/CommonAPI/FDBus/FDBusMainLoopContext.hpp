/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSMAINLOOPCONTEXT_HPP_
#define COMMONAPI_FDBUS_DBUSMAINLOOPCONTEXT_HPP_

#include <list>
#include <memory>
#include <queue>
#include <atomic>

#include <fdbus/fdbus.h>

#include <CommonAPI/MainLoopContext.hpp>

#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusConnection;

class FDBusDispatchSource: public DispatchSource {
 public:
    FDBusDispatchSource(std::weak_ptr<FDBusConnection> dbusConnection);
    ~FDBusDispatchSource();

    bool prepare(int64_t& timeout);
    bool check();
    bool dispatch();

 private:
    std::weak_ptr<FDBusConnection> dbusConnection_;
};

class FDBusQueueWatch;
class FDBusQueueDispatchSource: public DispatchSource {
 public:
    FDBusQueueDispatchSource(FDBusQueueWatch* watch);
    virtual ~FDBusQueueDispatchSource();

    bool prepare(int64_t& timeout);
    bool check();
    bool dispatch();

 private:
    FDBusQueueWatch* watch_;
};

class FDBusWatch: public Watch {
 public:
    FDBusWatch(/*::FDBusWatch* libdbusWatch,*/ std::weak_ptr<MainLoopContext>& mainLoopContext,
              std::weak_ptr<FDBusConnection>& dbusConnection);

    bool isReadyToBeWatched();
    void startWatching();
    void stopWatching();

    void dispatch(unsigned int eventFlags);

    const pollfd& getAssociatedFileDescriptor();

#ifdef _WIN32
    const HANDLE& getAssociatedEvent();
#endif

    const std::vector<DispatchSource*>& getDependentDispatchSources();
    void addDependentDispatchSource(DispatchSource* dispatchSource);
 private:
    bool isReady();

   // ::FDBusWatch* libdbusWatch_;
    pollfd pollFileDescriptor_;
    std::vector<DispatchSource*> dependentDispatchSources_;
    std::mutex dependentDispatchSourcesMutex_;

    std::weak_ptr<MainLoopContext> mainLoopContext_;
    std::weak_ptr<FDBusConnection> dbusConnection_;

#ifdef _WIN32
    HANDLE wsaEvent_;
#endif
};

struct QueueEntry;

class FDBusQueueWatch : public Watch {
public:

    FDBusQueueWatch(std::shared_ptr<FDBusConnection> _connection);
    virtual ~FDBusQueueWatch();

    void dispatch(unsigned int eventFlags);

    const pollfd& getAssociatedFileDescriptor();

#ifdef _WIN32
    const HANDLE& getAssociatedEvent();
#endif

    const std::vector<DispatchSource*>& getDependentDispatchSources();

    void addDependentDispatchSource(CommonAPI::DispatchSource* _dispatchSource);

    void removeDependentDispatchSource(CommonAPI::DispatchSource* _dispatchSource);

    void pushQueue(std::shared_ptr<QueueEntry> _queueEntry);

    void popQueue();

    std::shared_ptr<QueueEntry> frontQueue();

    bool emptyQueue();

    void processQueueEntry(std::shared_ptr<QueueEntry> _queueEntry);

private:
#ifdef _WIN32
    int pipeFileDescriptors_[2];
#else
    int eventFd_;
#endif

    pollfd pollFileDescriptor_;

    std::vector<CommonAPI::DispatchSource*> dependentDispatchSources_;
    std::queue<std::shared_ptr<QueueEntry>> queue_;

    std::mutex queueMutex_;
    std::mutex dependentDispatchSourcesMutex_;

    std::weak_ptr<FDBusConnection> connection_;

#ifdef _WIN32
    HANDLE wsaEvent_;
    const int pipeValue_;
#else
    const std::uint64_t eventFdValue_;
#endif

};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSMAINLOOPCONTEXT_HPP_
