/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#ifndef COMMONAPI_FDBUS_DBUSINTERFACEHANDLER_HPP_
#define COMMONAPI_FDBUS_DBUSINTERFACEHANDLER_HPP_

#include <memory>

#include <CommonAPI/FDBus/FDBusProxyConnection.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusInterfaceHandler {
 public:
    virtual ~FDBusInterfaceHandler() {}

    // virtual const char* getMethodsDBusIntrospectionXmlData() const = 0;

    virtual bool onInterfaceFDBusMessage(const FDBusMessage& dbusMessage) = 0;

    // virtual bool hasFreedesktopProperties() = 0;
};

} // namespace dbus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSINTERFACEHANDLER_HPP_
