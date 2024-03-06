/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_ERROR_HPP_
#define COMMONAPI_FDBUS_DBUS_ERROR_HPP_

#include <string>
#include <fdbus/fdbus.h>

#include <CommonAPI/Export.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusConnection;


class COMMONAPI_EXPORT FDBusError {
 public:
    FDBusError();
    ~FDBusError();

    operator bool() const;

    void clear();

    std::string getName() const;
    std::string getMessage() const;

 private:
    //::DBusError libdbusError_;

    friend class FDBusConnection;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_ERROR_HPP_
