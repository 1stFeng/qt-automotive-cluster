/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSSERVICEREGISTRY_HPP_
#define COMMONAPI_FDBUS_DBUSSERVICEREGISTRY_HPP_

#include <algorithm>
#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <list>

#include <CommonAPI/Attribute.hpp>
#include <CommonAPI/Proxy.hpp>
#include <CommonAPI/Types.hpp>
#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusFactory.hpp>
#include <CommonAPI/FDBus/FDBusObjectManagerStub.hpp>

namespace CommonAPI {
namespace FDBus {

typedef Event<std::string, std::string, std::string> NameOwnerChangedEvent;
typedef Event<std::string, std::string, std::string>::Subscription NameOwnerChangedEventSubscription;

// Connection name, Object path
typedef std::pair<std::string, std::string> DBusInstanceId;

class FDBusAddress;
class FDBusAddressTranslator;
class FDBusDaemonProxy;

class COMMONAPI_EXPORT FDBusServiceRegistry: public std::enable_shared_from_this<FDBusServiceRegistry>,
                           public FDBusProxyConnection::FDBusSignalHandler {
 public:
    enum class FDBusRecordState {
        UNKNOWN,
        AVAILABLE,
        RESOLVING,
        RESOLVED,
        NOT_AVAILABLE
    };

    // template class DBusServiceListener<> { typedef functor; typedef list; typedef subscription }
    typedef std::function<void(std::shared_ptr<FDBusProxy>, const AvailabilityStatus& availabilityStatus)> DBusServiceListener;
    typedef long int FDBusServiceSubscription;
    struct FDBusServiceListenerInfo {
        DBusServiceListener listener;
        std::weak_ptr<FDBusProxy> proxy;
    };
    typedef std::map<FDBusServiceSubscription, std::shared_ptr<FDBusServiceListenerInfo>> FDBusServiceListenerList;

    typedef std::function<void(const std::vector<std::string>& interfaces,
                               const AvailabilityStatus& availabilityStatus)> FDBusManagedInterfaceListener;
    typedef std::list<FDBusManagedInterfaceListener> DBusManagedInterfaceListenerList;
    typedef DBusManagedInterfaceListenerList::iterator DBusManagedInterfaceSubscription;

    typedef std::function<void(const FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict)> GetAvailableServiceInstancesCallback;

    static std::shared_ptr<FDBusServiceRegistry> get(const std::shared_ptr<FDBusProxyConnection> &_connection, bool _insert=true);
    static void remove(const std::shared_ptr<FDBusProxyConnection> &_connection);

    FDBusServiceRegistry(std::shared_ptr<FDBusProxyConnection> dbusProxyConnection);

    FDBusServiceRegistry(const FDBusServiceRegistry&) = delete;
    FDBusServiceRegistry& operator=(const FDBusServiceRegistry&) = delete;

    virtual ~FDBusServiceRegistry();

    void init();

    FDBusServiceSubscription subscribeAvailabilityListener(const std::string &_address,
                                                          DBusServiceListener _listener,
                                                          std::weak_ptr<FDBusProxy> _proxy);

    void unsubscribeAvailabilityListener(const std::string &_address,
                                         FDBusServiceSubscription &_listener);


    bool isServiceInstanceAlive(const std::string &_dbusInterfaceName,
                                const std::string &_dbusConnectionName,
                                const std::string &_dbusObjectPath);


    virtual void getAvailableServiceInstances(const std::string& dbusServiceName,
            const std::string& dbusObjectPath,
            FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict& availableServiceInstances);

    virtual void getAvailableServiceInstancesAsync(GetAvailableServiceInstancesCallback callback,
                                                   const std::string& dbusServiceName,
                                                   const std::string& dbusObjectPath);

    virtual void onSignalFDBusMessage(const FDBusMessage&);

    void setFDBusServicePredefined(const std::string& _serviceName);
    
    void onGetNameOwnerCallback(const CallStatus& status,
                                std::string dbusServiceUniqueName,
                                const std::string& dbusServiceName);

 private:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif
    struct FDBusInterfaceNameListenersRecord {
        FDBusInterfaceNameListenersRecord()
            : state(FDBusRecordState::UNKNOWN),
              nextSubscriptionKey(0) {
        }

        FDBusRecordState state;
        FDBusServiceListenerList listenerList;
        std::list<FDBusServiceSubscription> listenersToRemove;
        FDBusServiceSubscription nextSubscriptionKey;
    };

    typedef std::unordered_map<std::string, FDBusInterfaceNameListenersRecord> FDBusInterfaceNameListenersMap;

    struct FDBusServiceListenersRecord {
        FDBusServiceListenersRecord()
            : uniqueBusNameState(FDBusRecordState::UNKNOWN),
              promiseOnResolve(std::make_shared<std::promise<FDBusRecordState>>()),
              mutexOnResolve() {
        }

        ~FDBusServiceListenersRecord() { 
            if(uniqueBusNameState == FDBusRecordState::RESOLVING && futureOnResolve.valid())
                promiseOnResolve->set_value(FDBusRecordState::NOT_AVAILABLE);
        };

        FDBusRecordState uniqueBusNameState;
        std::string uniqueBusName;

        std::shared_ptr<std::promise<FDBusRecordState>> promiseOnResolve;
        std::shared_future<FDBusRecordState> futureOnResolve;
        std::unique_lock<std::mutex>* mutexOnResolve;

        std::unordered_map<std::string, FDBusInterfaceNameListenersMap> dbusObjectPathListenersMap;
    };

    std::unordered_map<std::string, FDBusServiceListenersRecord> dbusServiceListenersMap;


    struct FDBusObjectPathCache {
        FDBusObjectPathCache()
            : referenceCount(0),
              state(FDBusRecordState::UNKNOWN),
              promiseOnResolve(std::make_shared<std::promise<FDBusRecordState>>()),
              pendingObjectManagerCalls(0) {
        }

        ~FDBusObjectPathCache() {
            if(state == FDBusRecordState::RESOLVING && futureOnResolve.valid())
                promiseOnResolve->set_value(FDBusRecordState::NOT_AVAILABLE);
        }

        size_t referenceCount;
        FDBusRecordState state;
        std::shared_ptr<std::promise<FDBusRecordState>> promiseOnResolve;
        std::shared_future<FDBusRecordState> futureOnResolve;
        std::string serviceName;

        std::unordered_set<std::string> dbusInterfaceNamesCache;
        uint8_t pendingObjectManagerCalls;
    };

    struct FDBusUniqueNameRecord {
        FDBusUniqueNameRecord()
            : objectPathsState(FDBusRecordState::UNKNOWN) {
        }

        std::string uniqueName;
        FDBusRecordState objectPathsState;
        std::unordered_set<std::string> ownedBusNames;
        std::unordered_map<std::string, FDBusObjectPathCache> dbusObjectPathsCache;
    };

    std::unordered_map<std::string, FDBusUniqueNameRecord> dbusUniqueNamesMap_;
    typedef std::unordered_map<std::string, FDBusUniqueNameRecord>::iterator DBusUniqueNamesMapIterator;

    // mapping service names (well-known names) to service instances
    std::unordered_map<std::string, FDBusUniqueNameRecord*> dbusServiceNameMap_;

    // protects the dbus service maps
    std::recursive_mutex dbusServicesMutex_;

    void resolveDBusServiceName(const std::string& dbusServiceName,
                                FDBusServiceListenersRecord& dbusServiceListenersRecord);


    FDBusRecordState resolveDBusInterfaceNameState(const FDBusAddress &_address,
                                                  FDBusServiceListenersRecord &_record);


    FDBusObjectPathCache& getDBusObjectPathCacheReference(const std::string& dbusObjectPath,
                                                         const std::string& dbusServiceName,
                                                         const std::string& dbusServiceUniqueName,
                                                         FDBusUniqueNameRecord& dbusUniqueNameRecord);

    bool resolveObjectPathWithObjectManager(FDBusObjectPathCache& dbusObjectPathRecord,
                                            const std::string& dbusServiceUniqueName,
                                            const std::string& dbusObjectPath);

    typedef std::function<void(const CallStatus&,
                               const FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict,
                               const std::string&,
                               const std::string&)> GetManagedObjectsCallback;

    bool getManagedObjects(const std::string& dbusServiceName,
                           const std::string& dbusObjectPath,
                           FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict& availableServiceInstances);

    bool getManagedObjectsAsync(const std::string& dbusServiceName,
                                const std::string& dbusObjectPath,
                                GetManagedObjectsCallback callback);

    void onGetManagedObjectsCallbackResolve(const CallStatus& callStatus,
                                     const FDBusObjectManagerStub::FDBusObjectPathAndInterfacesDict availableServiceInstances,
                                     const std::string& dbusServiceUniqueName,
                                     const std::string& dbusObjectPath);

    void onDBusDaemonProxyNameOwnerChangedEvent(const std::string& name,
                                                const std::string& oldOwner,
                                                const std::string& newOwner);

    std::shared_ptr<FDBusDaemonProxy> dbusDaemonProxy_;
    bool initialized_;

    ProxyStatusEvent::Subscription dbusDaemonProxyStatusEventSubscription_;
    NameOwnerChangedEvent::Subscription dbusDaemonProxyNameOwnerChangedEventSubscription_;


    void checkFDBusServiceWasAvailable(const std::string& dbusServiceName, const std::string& dbusServiceUniqueName);

    void onFDBusServiceAvailable(const std::string& dbusServiceName, const std::string& dbusServiceUniqueName);

    void onFDBusServiceNotAvailable(FDBusServiceListenersRecord& dbusServiceListenersRecord, const std::string &_serviceName = "");

    void notifyFDBusServiceListenersLocked(const FDBusUniqueNameRecord _dbusUniqueNameRecord,
                                    const std::string _dbusObjectPath,
                                    const std::unordered_set<std::string> _dbusInterfaceNames,
                                    const FDBusRecordState _dbusInterfaceNamesState);


    void notifyDBusServiceListeners(const FDBusUniqueNameRecord& dbusUniqueNameRecord,
                                    const std::string& dbusObjectPath,
                                    const std::unordered_set<std::string>& dbusInterfaceNames,
                                    const FDBusRecordState& dbusInterfaceNamesState);

    void notifyFDBusObjectPathResolved(FDBusInterfaceNameListenersMap& dbusInterfaceNameListenersMap,
                                      const std::unordered_set<std::string>& dbusInterfaceNames);

    void notifyFDBusObjectPathChanged(FDBusInterfaceNameListenersMap& dbusInterfaceNameListenersMap,
                                     const std::unordered_set<std::string>& dbusInterfaceNames,
                                     const FDBusRecordState& dbusInterfaceNamesState);

    void notifyFDBusInterfaceNameListeners(FDBusInterfaceNameListenersRecord& dbusInterfaceNameListenersRecord,
                                          const bool& isDBusInterfaceNameAvailable);


    void removeUniqueName(const DBusUniqueNamesMapIterator &_dbusUniqueName, const std::string &_serviceName);
    FDBusUniqueNameRecord* insertServiceNameMapping(const std::string& dbusUniqueName, const std::string& dbusServiceName);
    bool findCachedDbusService(const std::string& dbusServiceName, FDBusUniqueNameRecord** uniqueNameRecord);
    bool findCachedObjectPath(const std::string& dbusObjectPathName, const FDBusUniqueNameRecord* uniqueNameRecord, FDBusObjectPathCache* objectPathCache);

    void fetchAllServiceNames();

    inline bool isFDBusServiceName(const std::string &_name) {
        return (_name.length() > 0 && _name[0] != ':');
    };


    inline bool isOrgFreedesktopDBusInterface(const std::string& dbusInterfaceName) {
        return dbusInterfaceName.find("org.freedesktop.DBus.") == 0;
    }

    std::thread::id notificationThread_;

    std::unordered_set<std::string> dbusPredefinedServices_;

private:
    typedef std::map<FDBusProxyConnection*, std::shared_ptr<FDBusServiceRegistry>> RegistryMap_t;
    static std::shared_ptr<RegistryMap_t> getRegistryMap() {
        static std::shared_ptr<RegistryMap_t> registries(new RegistryMap_t);
        return registries;
    }
    static std::mutex registriesMutex_;
    std::shared_ptr<RegistryMap_t> registries_;
    std::shared_ptr<FDBusAddressTranslator> translator_;
    std::weak_ptr<FDBusServiceRegistry> selfReference_;

    friend class FDBusProxy;
};


} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSSERVICEREGISTRY_HPP_
