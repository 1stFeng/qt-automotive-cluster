/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSDEPLOYMENTS_HPP_
#define COMMONAPI_FDBUS_DBUSDEPLOYMENTS_HPP_

#include <string>
#include <unordered_map>

#include <CommonAPI/Deployment.hpp>
#include <CommonAPI/Export.hpp>

namespace CommonAPI {
namespace FDBus {

template<typename... Types_>
struct VariantDeployment : CommonAPI::Deployment<Types_...> {
    static const size_t size_ = std::tuple_size<std::tuple<Types_...>>::value;
    VariantDeployment(bool _isDBus, Types_*... _t)
          : CommonAPI::Deployment<Types_...>(_t...),
            isDBus_(_isDBus) {};
    bool isDBus_;
};

struct StringDeployment : CommonAPI::Deployment<> {
    StringDeployment(bool _isObjectPath)
    : isObjectPath_(_isObjectPath) {};

    bool isObjectPath_;
};

struct IntegerDeployment : CommonAPI::Deployment<> {
    IntegerDeployment(bool _isUnixFD)
    : isUnixFD_(_isUnixFD) {};

    bool isUnixFD_;
};

template<typename... Types_>
struct StructDeployment : CommonAPI::Deployment<Types_...> {
    StructDeployment(Types_*... t)
    : CommonAPI::Deployment<Types_...>(t...) {};
};

template<typename ElementDepl_>
struct ArrayDeployment : CommonAPI::ArrayDeployment<ElementDepl_> {
    ArrayDeployment(ElementDepl_ *_element)
    : CommonAPI::ArrayDeployment<ElementDepl_>(_element) {}
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSDEPLOYMENTS_HPP_
