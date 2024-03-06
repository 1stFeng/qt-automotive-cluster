/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSOBJECTMANAGER_HPP_
#define COMMONAPI_FDBUS_DBUSOBJECTMANAGER_HPP_

#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusObjectManagerStub.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusStubAdapter;
class FDBusInterfaceHandler;

class FDBusObjectManager {
 public:
     COMMONAPI_EXPORT FDBusObjectManager(const std::shared_ptr<FDBusProxyConnection>&);
     COMMONAPI_EXPORT ~FDBusObjectManager();

    /**
     * @brief Register the stub adapter in the manager.
     * @param StubAdapter that needs to be registered.
     * @return Whether the registration was successful.
    */
     COMMONAPI_EXPORT bool registerDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);
     /**
     * @brief Unregister the stub adapter in the manager.
     * @param StubAdapter that needs to be unregistered.
     * @return Whether the unregistration was successful.
    */
     COMMONAPI_EXPORT bool unregisterDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

    //Zusammenfassbar mit "registerDBusStubAdapter"?
     COMMONAPI_EXPORT bool exportManagedDBusStubAdapter(const std::string& parentObjectPath, std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);
     COMMONAPI_EXPORT bool unexportManagedDBusStubAdapter(const std::string& parentObjectPath, std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

    /**
     * @brief Process message
     * @param messages to be processed.
     * @return Whether the processing was successful.
    */
     COMMONAPI_EXPORT bool handleMessage(const FDBusMessage&);

     COMMONAPI_EXPORT std::shared_ptr<FDBusObjectManagerStub> getRootDBusObjectManagerStub();

 private:
    // instance, interfaceName
    typedef std::pair<std::string, std::string> FDBusInterfaceHandlerPath;

    COMMONAPI_EXPORT bool addDBusInterfaceHandler(const FDBusInterfaceHandlerPath& dbusInterfaceHandlerPath,
                                 std::shared_ptr<FDBusInterfaceHandler> dbusInterfaceHandler);

    COMMONAPI_EXPORT bool removeDBusInterfaceHandler(const FDBusInterfaceHandlerPath& dbusInterfaceHandlerPath,
                                    std::shared_ptr<FDBusInterfaceHandler> dbusInterfaceHandler);

    COMMONAPI_EXPORT bool onFreedesktopPropertiesDBusMessage(const FDBusMessage& callMessage);


    typedef std::unordered_map<FDBusInterfaceHandlerPath, std::vector<std::shared_ptr<FDBusInterfaceHandler>>> DBusRegisteredObjectsTable;
    DBusRegisteredObjectsTable dbusRegisteredObjectsTable_;
    COMMONAPI_EXPORT bool addToRegisteredObjectsTable(FDBusInterfaceHandlerPath ifpath, std::shared_ptr<FDBusInterfaceHandler> handler);

    std::shared_ptr<FDBusObjectManagerStub> rootDBusObjectManagerStub_;

    typedef std::pair<std::shared_ptr<FDBusObjectManagerStub>, uint32_t> ReferenceCountedDBusObjectManagerStub;
    typedef std::unordered_map<std::string, ReferenceCountedDBusObjectManagerStub> RegisteredObjectManagersTable;
    RegisteredObjectManagersTable managerStubs_;

    std::weak_ptr<FDBusProxyConnection> dbusConnection_;
    std::recursive_mutex objectPathLock_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSOBJECTMANAGER_HPP_
