/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#ifndef COMMONAPI_FDBUS_DBUSFREEDESKTOPOBJECTMANAGERSTUB_HPP_
#define COMMONAPI_FDBUS_DBUSFREEDESKTOPOBJECTMANAGERSTUB_HPP_

#include <memory>
#include <mutex>
#include <string>

#include <CommonAPI/FDBus/FDBusInterfaceHandler.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusStubAdapter;

/**
 * Stub for standard <a href="http://dbus.freedesktop.org/doc/dbus-specification.html#standard-interfaces-objectmanager">org.freedesktop.dbus.ObjectManager</a> interface.
 *
 * Instantiated within a manager stub and it must hold reference to all registered objects.
 * Whenever the manager gets destroyed all references to registered objects are lost too.
 * This duplicates the semantic of the CommonAPI::ServicePublisher class.
 *
 * Only one FDBusStubAdapter instance could be registered per FDBusObjectManagerStub instance.
 *
 * The owner of the FDBusObjectManagerStub instance must take care of registering and unregistering it.
 *
 * Example stub life cycle:
 *  - create CommonAPI::ServicePublisher
 *  - create stub A
 *  - register stub A to CommonAPI::ServicePublisher
 *  - create stub B
 *  - register stub B with stub A as object manager
 *  - drop all references to stub B, stub A keeps a reference to stub B
 *  - drop all references to stub A, CommonAPI::ServicePublisher keeps a reference to stub A
 *  - reference overview: Application > CommonAPI::ServicePublisher > Stub A > Stub B
 *  - drop all references to CommonAPI::ServicePublisher causes all object references to be dropped
 */
class FDBusObjectManagerStub : public FDBusInterfaceHandler {
public:
   // serialization trick: use bool instead of variant since we never serialize it
    typedef std::unordered_map<std::string, bool> FDBusPropertiesChangedDict;
    typedef std::unordered_map<std::string, FDBusPropertiesChangedDict> FDBusInterfacesAndPropertiesDict;
    // instance
    typedef std::unordered_map<std::string, FDBusInterfacesAndPropertiesDict> FDBusObjectPathAndInterfacesDict;

public:
    COMMONAPI_EXPORT FDBusObjectManagerStub(const std::string& dbusObjectPath, const std::shared_ptr<FDBusProxyConnection>&);

    /**
     * Unregisters all currently registered FDBusStubAdapter instances from the DBusServicePublisher
     */
    COMMONAPI_EXPORT virtual ~FDBusObjectManagerStub();

    /**
     * Export FDBusStubAdapter instance with the current FDBusObjectManagerStub instance.
     *
     * The FDBusStubAdapter must be registered with the DBusServicePublisher!
     *
     * On registering a
     * <a href="http://dbus.freedesktop.org/doc/dbus-specification.html#standard-interfaces-objectmanager">InsterfaceAdded</a>
     * signal will be emitted with the FDBusObjectManagerStub instance's current D-Bus object path.
     *
     * @param dbusStubAdapter a refernce to FDBusStubAdapter instance
     *
     * @return false if the @a dbusStubAdapter instance was already registered
     * @return false if sending the InterfaceAdded signal fails
     *
     * @see ~FDBusObjectManagerStub()
     * @see CommonAPI::ServicePublisher
     * @see FDBusObjectManager
     */
    COMMONAPI_EXPORT bool exportManagedDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

    /**
     * Unexport FDBusStubAdapter instance from this FDBusObjectManagerStub instance.
     *
     * On unregistering a
     * <a href="http://dbus.freedesktop.org/doc/dbus-specification.html#standard-interfaces-objectmanager">InsterfaceRemoved</a>
     * signal will be emitted with the FDBusObjectManagerStub instance's current D-Bus object path.
     *
     * @param dbusStubAdapter
     *
     * @return false if @a dbusStubAdapter wasn't registered
     * @return true even if sending the InterfaceRemoved signal fails
     *
     * @see exportDBusStubAdapter()
     */
    COMMONAPI_EXPORT bool unexportManagedDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

    COMMONAPI_EXPORT bool isDBusStubAdapterExported(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

    COMMONAPI_EXPORT const std::string& getDBusObjectPath() const;
    COMMONAPI_EXPORT static const char* getInterfaceName();

    COMMONAPI_EXPORT virtual const char* getMethodsDBusIntrospectionXmlData() const;
    COMMONAPI_EXPORT virtual bool onInterfaceFDBusMessage(const FDBusMessage& dbusMessage);
    COMMONAPI_EXPORT virtual bool hasFreedesktopProperties();

 private:
     COMMONAPI_EXPORT bool registerDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);
     COMMONAPI_EXPORT bool unregisterDBusStubAdapter(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter);

     COMMONAPI_EXPORT bool emitInterfacesAddedSignal(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter,
                                   const std::shared_ptr<FDBusProxyConnection>& dbusConnection) const;

     COMMONAPI_EXPORT bool emitInterfacesRemovedSignal(std::shared_ptr<FDBusStubAdapter> dbusStubAdapter,
                                     const std::shared_ptr<FDBusProxyConnection>& dbusConnection) const;

    std::string dbusObjectPath_;
    std::weak_ptr<FDBusProxyConnection> dbusConnection_;

    typedef std::unordered_map<std::string, std::vector<std::shared_ptr<FDBusStubAdapter>>> FDBusInterfacesMap;
    typedef std::unordered_map<std::string, FDBusInterfacesMap> FDBusObjectPathsMap;
    FDBusObjectPathsMap registeredDBusObjectPathsMap_;

    std::mutex dbusObjectManagerStubLock_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSFREEDESKTOPOBJECTMANAGERSTUB_HPP_
