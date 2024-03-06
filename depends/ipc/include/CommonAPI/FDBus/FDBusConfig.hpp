/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSCONFIG_HPP_
#define COMMONAPI_FDBUS_DBUSCONFIG_HPP_

#include <CommonAPI/CallInfo.hpp>

namespace CommonAPI {
namespace FDBus {

static CommonAPI::CallInfo defaultCallInfo(CommonAPI::DEFAULT_SEND_TIMEOUT_MS);

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSCONFIG_HPP_
