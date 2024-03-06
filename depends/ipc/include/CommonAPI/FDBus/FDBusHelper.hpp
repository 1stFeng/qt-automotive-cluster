/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_HELPER_HPP_
#define COMMONAPI_FDBUS_DBUS_HELPER_HPP_

#include <unordered_map>
#include <vector>

namespace CommonAPI {
namespace FDBus {

template <size_t ...>
struct index_sequence {};


template <size_t N_, size_t ...S_>
struct make_sequence : make_sequence<N_-1, N_-1, S_...> {};

template <size_t ...S_>
struct make_sequence<0, S_...> {
    typedef index_sequence<S_...> type;
};


template <size_t N_, size_t Offset_, size_t ...S_>
struct make_sequence_range : make_sequence_range<N_-1, Offset_, N_-1+Offset_, S_...> {};

template <size_t Offset_, size_t ...S_>
struct make_sequence_range<0, Offset_, S_...> {
    typedef index_sequence<S_...> type;
};

template<typename Type>
struct is_std_vector { static const bool value = false; };

template<typename Type_>
struct is_std_vector<std::vector<Type_> > { static const bool value = true; };

template<typename Type_>
struct is_std_unordered_map { static const bool value = false; };

template<typename KeyType_, typename ValueType_, typename HasherType_>
struct is_std_unordered_map<std::unordered_map<KeyType_, ValueType_, HasherType_>> { static const bool value = true; };

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_HELPER_HPP_
