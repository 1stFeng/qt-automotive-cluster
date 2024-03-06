// Copyright (C) 2013-2020 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_RUNTIME_HPP_
#define COMMONAPI_RUNTIME_HPP_

#include <map>
#include <memory>
#include <mutex>
#include <set>

#include <CommonAPI/AttributeExtension.hpp>
#include <CommonAPI/Export.hpp>
#include <CommonAPI/Factory.hpp>
#include <CommonAPI/Types.hpp>

namespace CommonAPI {

static const ConnectionId_t DEFAULT_CONNECTION_ID = "";

class MainLoopContext;
class Proxy;
class ProxyManager;
class StubBase;

/**
 * @brief The `Runtime` class is responsible for managing the CommonAPI runtime environment. 
 *        It provides methods for creating proxies and registering stubs, as well as loading and unloading libraries.
 *        It also reads and stores configuration settings, and manages factories for different IPC mechanisms. 
 *        The `Runtime` class is a singleton, and can be accessed through the `get()` method.
 */
class Runtime {
public:
    COMMONAPI_EXPORT static std::string getProperty(const std::string &_name);
    COMMONAPI_EXPORT static void setProperty(const std::string &_name, const std::string &_value);

    COMMONAPI_EXPORT static std::shared_ptr<Runtime> get();

    COMMONAPI_EXPORT Runtime();
    COMMONAPI_EXPORT virtual ~Runtime();

    /**
     * @brief `buildProxy` is a template function that creates a proxy object for a given domain, instance, and connection ID or main loop context. 
     *         It takes a template parameter `ProxyClass_` which specifies the type of the proxy to be created, 
     *         and a variadic template parameter `AttributeExtensions_` which specifies any attribute extensions to be used with the proxy. 
     *          The function returns a shared pointer to the created proxy object.
     * 
     * @tparam ProxyClass_ 
     * @tparam AttributeExtensions_ 
     * @param _domain 
     * @param _instance 
     * @param _connectionId 
     * @return COMMONAPI_EXPORT 
     */
    template<template<typename ...> class ProxyClass_, typename ... AttributeExtensions_>
    COMMONAPI_EXPORT std::shared_ptr<
        ProxyClass_<AttributeExtensions_...>
    >
    buildProxy(const std::string &_domain,
               const std::string &_instance,
               const ConnectionId_t &_connectionId = DEFAULT_CONNECTION_ID) {
        std::shared_ptr<Proxy> proxy
            = createProxy(_domain,
                          ProxyClass_<AttributeExtensions_...>::getInterface(),
                          _instance,
                          _connectionId);

        if (proxy) {
            return std::make_shared<ProxyClass_<AttributeExtensions_...>>(proxy);
        }
        return nullptr;
    }

    template<template<typename ...> class ProxyClass_, typename ... AttributeExtensions_>
    COMMONAPI_EXPORT std::shared_ptr<
        ProxyClass_<AttributeExtensions_...>
    >
    buildProxy(const std::string &_domain,
               const std::string &_instance,
               std::shared_ptr<MainLoopContext> _context) {
        std::shared_ptr<Proxy> proxy
            = createProxy(_domain,
                          ProxyClass_<AttributeExtensions_...>::getInterface(),
                          _instance,
                          _context);
        if (proxy) {
            return std::make_shared<ProxyClass_<AttributeExtensions_...>>(proxy);
        }
        return nullptr;
    }

    template <template<typename ...> class ProxyClass_, template<typename> class AttributeExtension_>
    COMMONAPI_EXPORT std::shared_ptr<typename DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t>
    buildProxyWithDefaultAttributeExtension(const std::string &_domain,
                                            const std::string &_instance,
                                            const ConnectionId_t &_connectionId = DEFAULT_CONNECTION_ID) {
        std::shared_ptr<Proxy> proxy
            = createProxy(_domain,
                           DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t::getInterface(),
                          _instance,
                          _connectionId);
        if (proxy) {
            return std::make_shared<typename DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t>(proxy);
        }
        return nullptr;
    }

    template <template<typename ...> class ProxyClass_, template<typename> class AttributeExtension_>
    COMMONAPI_EXPORT std::shared_ptr<typename DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t>
    buildProxyWithDefaultAttributeExtension(const std::string &_domain,
                                            const std::string &_instance,
                                            std::shared_ptr<MainLoopContext> _context) {
        std::shared_ptr<Proxy> proxy
            = createProxy(_domain,
                           DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t::getInterface(),
                          _instance,
                          _context);
        if (proxy) {
            return std::make_shared<typename DefaultAttributeProxyHelper<ProxyClass_, AttributeExtension_>::class_t>(proxy);
        }
        return nullptr;
    }

    /**
     * @brief The `registerService` function is a template function that registers a service with the CommonAPI runtime environment. 
     *        It takes a template parameter `Stub_` which specifies the type of the stub to be registered, 
     *        and a variadic template parameter `AttributeExtensions_` which specifies any attribute extensions to be used with the stub. 
     *        The function returns a boolean value indicating whether the registration was successful or not.
     * 
     * @tparam Stub_ 
     * @param _domain 
     * @param _instance 
     * @param _service 
     * @param _connectionId 
     * @return bool 
     */
    template<typename Stub_>
    COMMONAPI_EXPORT bool registerService(const std::string &_domain,
                         const std::string &_instance,
                         std::shared_ptr<Stub_> _service,
                         const ConnectionId_t &_connectionId = DEFAULT_CONNECTION_ID) {
        return registerStub(_domain, Stub_::StubInterface::getInterface(), _instance, _service, _connectionId);
    }

    template<typename Stub_>
    COMMONAPI_EXPORT bool registerService(const std::string &_domain,
                         const std::string &_instance,
                         std::shared_ptr<Stub_> _service,
                         std::shared_ptr<MainLoopContext> _context) {
        return registerStub(_domain, Stub_::StubInterface::getInterface(), _instance, _service, _context);
    }

    COMMONAPI_EXPORT bool unregisterService(const std::string &_domain,
                            const std::string &_interface,
                            const std::string &_instance) {
        return unregisterStub(_domain, _interface, _instance);
    }

    COMMONAPI_EXPORT bool registerFactory(const std::string &_ipc, std::shared_ptr<Factory> _factory);
    COMMONAPI_EXPORT bool unregisterFactory(const std::string &_ipc);

    inline const std::string &getDefaultBinding() const { return defaultBinding_; };

    COMMONAPI_EXPORT void initFactories();
    COMMONAPI_EXPORT Timeout_t getDefaultCallTimeout() const;

private:
    COMMONAPI_EXPORT void init();
    COMMONAPI_EXPORT bool readConfiguration();
    COMMONAPI_EXPORT bool splitAddress(const std::string &, std::string &, std::string &, std::string &);

    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxy(const std::string &, const std::string &, const std::string &,
                                       const ConnectionId_t &);
    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxy(const std::string &, const std::string &, const std::string &,
                                       std::shared_ptr<MainLoopContext>);

    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxyHelper(const std::string &, const std::string &, const std::string &,
                                             const ConnectionId_t &, bool);
    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxyHelper(const std::string &, const std::string &, const std::string &,
                                             std::shared_ptr<MainLoopContext>, bool);


    COMMONAPI_EXPORT bool registerStub(const std::string &, const std::string &, const std::string &,
                      std::shared_ptr<StubBase>, const ConnectionId_t &);
    COMMONAPI_EXPORT bool registerStub(const std::string &, const std::string &, const std::string &,
                      std::shared_ptr<StubBase>, std::shared_ptr<MainLoopContext>);
    COMMONAPI_EXPORT bool registerStubHelper(const std::string &, const std::string &, const std::string &,
                            std::shared_ptr<StubBase>, const ConnectionId_t &, bool);
    COMMONAPI_EXPORT bool registerStubHelper(const std::string &, const std::string &, const std::string &,
                            std::shared_ptr<StubBase>, std::shared_ptr<MainLoopContext>, bool);

    COMMONAPI_EXPORT bool unregisterStub(const std::string &, const std::string &, const std::string &);

    COMMONAPI_EXPORT std::string getLibrary(const std::string &, const std::string &, const std::string &, bool);
    COMMONAPI_EXPORT bool loadLibrary(const std::string &);

private:
    std::string usedConfig_;

    std::string defaultBinding_;
    std::string defaultFolder_;
    std::string defaultConfig_;
    Timeout_t defaultCallTimeout_ = 0;

    std::map<std::string, std::shared_ptr<Factory>> factories_;
    std::shared_ptr<Factory> defaultFactory_;
    std::map<std::string, std::map<bool, std::string>> libraries_;
    std::set<std::string> loadedLibraries_; // Library name

    std::mutex mutex_;
    std::mutex factoriesMutex_;
    std::mutex loadMutex_;

    bool isConfigured_;
    bool isInitialized_;

    static std::map<std::string, std::string> properties_;

friend class ProxyManager;
};

} // namespace CommonAPI

#endif // COMMONAPI_RUNTIME_HPP_
