/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_SERIALIZABLEARGUMENTS_HPP_
#define COMMONAPI_FDBUS_DBUS_SERIALIZABLEARGUMENTS_HPP_

#include <CommonAPI/SerializableArguments.hpp>
#include <CommonAPI/FDBus/FDBusInputStream.hpp>
#include <CommonAPI/FDBus/FDBusOutputStream.hpp>

namespace CommonAPI {
namespace FDBus {

template<typename... Arguments_>
using FDBusSerializableArguments = CommonAPI::SerializableArguments<
                                        FDBusInputStream,
                                        FDBusOutputStream,
                                        Arguments_...
                                  >;

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSSERIALIZABLEARGUMENTS_HPP_
