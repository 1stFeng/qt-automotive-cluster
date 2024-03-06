/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSTYPES_HPP_
#define COMMONAPI_FDBUS_DBUSTYPES_HPP_

#include <string>
#include <unordered_map>

#include <fdbus/fdbus.h>

namespace CommonAPI {
namespace FDBus {

class FDBusProxy;

typedef std::unordered_map<std::string, bool> FDBusPropertiesChangedDict;
typedef std::unordered_map<std::string,
            FDBusPropertiesChangedDict> FDBusInterfacesAndPropertiesDict;
typedef std::unordered_map<std::string,
            FDBusInterfacesAndPropertiesDict> FDBusObjectPathAndInterfacesDict;

typedef uint32_t AvailabilityHandlerId_t;
// interface, instance, avaibility
typedef std::function<void (std::string, std::string, bool)> availability_handler_t;
// interface, instance, avaibility
typedef std::function<void (std::shared_ptr<FDBusProxy>, std::string, std::string, bool, void*)> AvailabilityHandler_t;

//TODO remove it
enum class FDBusType_t {
    SERVER = 0,
    CLIENT = 1
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSTYPES_HPP_
