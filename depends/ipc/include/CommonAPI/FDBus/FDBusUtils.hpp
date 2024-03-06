/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSUTILS_HPP_
#define COMMONAPI_FDBUS_DBUSUTILS_HPP_

#include <future>

namespace CommonAPI {
namespace FDBus {

#define UNUSED_VAR(VAR) (void*)(VAR);

//In gcc 4.4.1, the enumeration "std::future_status" is defined, but the return values of some functions
//are bool where the same functions in gcc 4.6. return a value from this enum. This template is a way
//to ensure compatibility for this issue.
template<typename FutureWaitType_>
inline bool checkReady(FutureWaitType_&);

template<>
inline bool checkReady<bool>(bool& returnedValue) {
    return returnedValue;
}

template<>
inline bool checkReady<std::future_status>(std::future_status& returnedValue) {
    return returnedValue == std::future_status::ready;
}

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSUTILS_HPP_
