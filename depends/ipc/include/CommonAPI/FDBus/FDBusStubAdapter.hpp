/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSSTUBADAPTER_HPP_
#define COMMONAPI_FDBUS_DBUSSTUBADAPTER_HPP_

#include <memory>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/Stub.hpp>
#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusInterfaceHandler.hpp>
#include <CommonAPI/FDBus/FDBusServer.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusProxyConnection;

class COMMONAPI_EXPORT_CLASS_EXPLICIT FDBusStubAdapter
        : virtual public CommonAPI::StubAdapter,
          public FDBusInterfaceHandler {
 public:
     COMMONAPI_EXPORT FDBusStubAdapter(const FDBusAddress &_dbusAddress,
                    const std::shared_ptr<FDBusProxyConnection> &_connection,
                    const bool isManagingInterface);

     COMMONAPI_EXPORT virtual ~FDBusStubAdapter();

     COMMONAPI_EXPORT virtual void init(std::shared_ptr<FDBusStubAdapter> _instance);
     COMMONAPI_EXPORT virtual void deinit();

    /**
     * @brief Get the dbusAddress object.
    */
     COMMONAPI_EXPORT const FDBusAddress &getFDBusAddress() const;
     /**
      * @brief Get the Connection object.
     */
     COMMONAPI_EXPORT const std::shared_ptr<FDBusProxyConnection> &getFDBusConnection() const;

     COMMONAPI_EXPORT bool isManaging() const;

     // COMMONAPI_EXPORT virtual const char* getMethodsDBusIntrospectionXmlData() const = 0;
     COMMONAPI_EXPORT virtual bool onInterfaceFDBusMessage(const FDBusMessage &_message) = 0;

     COMMONAPI_EXPORT virtual void deactivateManagedInstances() = 0;
     // COMMONAPI_EXPORT virtual bool hasFreedesktopProperties();
     // COMMONAPI_EXPORT virtual bool onInterfaceFDBusFreedesktopPropertiesMessage(const FDBusMessage &_message) = 0;
 protected:
    FDBusAddress dbusAddress_;
    const std::shared_ptr<FDBusProxyConnection> connection_;
    const bool isManaging_;
};

} // namespace dbus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSSTUBADAPTER_HPP_
