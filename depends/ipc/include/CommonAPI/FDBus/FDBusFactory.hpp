/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_FACTORY_HPP_
#define COMMONAPI_FDBUS_FACTORY_HPP_

#include <list>
#include <map>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/Factory.hpp>
#include <CommonAPI/FDBus/FDBusTypes.hpp>
#include <CommonAPI/FDBus/FDBusConnection.hpp>
namespace CommonAPI {

class Runtime;

namespace FDBus {

class FDBusAddress;
class FDBusProxy;
class FDBusProxyConnection;
class FDBusStubAdapter;

typedef void (*InterfaceInitFunction)(void);

typedef std::shared_ptr<FDBusProxy>
(*ProxyCreateFunction)(const FDBusAddress &_address,
                       const std::shared_ptr<FDBusProxyConnection> &_connection);

typedef std::shared_ptr<FDBusStubAdapter>
(*StubAdapterCreateFunction) (const FDBusAddress &_address,
                              const std::shared_ptr<FDBusProxyConnection> &_connection,
                              const std::shared_ptr<StubBase> &_stub);

class Factory : public CommonAPI::Factory {
public:
    COMMONAPI_EXPORT static std::shared_ptr<Factory> get();

    COMMONAPI_EXPORT Factory();
    COMMONAPI_EXPORT virtual ~Factory();

    COMMONAPI_EXPORT void init();

    /**
     * @brief Establish an interface and corresponding proxy creation method.
     * @param _address is interface.
     * @param function, Corresponding proxy creation method.
    */
    COMMONAPI_EXPORT void registerProxyCreateMethod(const std::string &_address,
                                    ProxyCreateFunction _function);
    /**
     * @brief Establish an interface and corresponding stub adapter creation method.
     * @param _address is interface.
     * @param function, Corresponding stub adapter creation method
    */
    COMMONAPI_EXPORT void registerStubAdapterCreateMethod(const std::string &_address,
                                         StubAdapterCreateFunction _function);


    /**
     * @brief Create proxies based on _domain、_interface、_instance and _connectionId.
     * @param _domain, Used to generate CommonAPI::Address
     * @param _interface, Used to generate CommonAPI::Address
     * @param _instance, Used to generate CommonAPI::Address
     * @param _connectionId, Used to get connection.
    */
    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxy(const std::string &_domain,
                                       const std::string &_interface,
                                       const std::string &_instance,
                                       const ConnectionId_t &_connectionId);

    /**
     * @brief Create proxies based on _domain、_interface、_instance and _connectionId.
     * @param _domain, Used to generate CommonAPI::Address
     * @param _interface, Used to generate CommonAPI::Address
     * @param _instance, Used to generate CommonAPI::Address
     * @param _connectionId, Used to get connection.
    */
    COMMONAPI_EXPORT std::shared_ptr<Proxy> createProxy(const std::string &_domain,
                                       const std::string &_interface,
                                       const std::string &_instance,
                                       std::shared_ptr<MainLoopContext> _context);

    /**
     * @brief Creating a stubAdapter using a stub inside the function.
     * @param _domain, used to create an address.
     * @param _interface, used to create an address.
     * @param _instance, used to create an address.
     * @param _stub, used to create stubAdapter.
     * @param _connectionId, used to getConnection.
     * @return Returns whether registration succeeded.
    */
    COMMONAPI_EXPORT bool registerStub(const std::string &_domain,
                                const std::string &_interface,
                                const std::string &_instance,
                          std::shared_ptr<StubBase> _stub,
                          const ConnectionId_t &_connectionId);

    /**
     * @brief Creating a stubAdapter using a stub inside the function.
     * @param _domain, used to create an address.
     * @param _interface, used to create an address.
     * @param _instance, used to create an address.
     * @param _stub, used to create stubAdapter.
     * @param _context, used to getConnection.
     * @return Returns whether registration succeeded.
    */
    COMMONAPI_EXPORT bool registerStub(const std::string &_domain,
                            const std::string &_interface,
                            const std::string &_instance,
                      std::shared_ptr<StubBase> _stub,
                      std::shared_ptr<MainLoopContext> _context);

    /**
     * @brief unregisterStubAdapter.
     * @param _domain, used to create an address.
     * @param _interface, used to create an address.
     * @param _instance, used to create an address.
    */
    COMMONAPI_EXPORT bool unregisterStub(const std::string &_domain,
                        const std::string &_interface, 
                        const std::string &_instance);

    // Services
    COMMONAPI_EXPORT std::shared_ptr<FDBusStubAdapter> getRegisteredService(const std::string &_address);

    // Managed services
    COMMONAPI_EXPORT std::shared_ptr<FDBusStubAdapter> createFDBusStubAdapter(const std::shared_ptr<StubBase> &_stub,
                                                           const std::string &_interface,
                                                           const FDBusAddress &_address,
                                                           const std::shared_ptr<FDBusProxyConnection> &_connection);
    COMMONAPI_EXPORT bool registerManagedService(const std::shared_ptr<FDBusStubAdapter> &_adapter);
    COMMONAPI_EXPORT bool unregisterManagedService(const std::string &_address);

    /**
     * @brief Reduce the count of connections. If the count equals 0, call disconnect.
     * @param Connections that need to be counted down.
    */
    COMMONAPI_EXPORT void decrementConnection(std::shared_ptr<FDBusProxyConnection>);
    /**
     * @brief Release the connection corresponding to the connectionid.
    */
    COMMONAPI_EXPORT void releaseConnection(const ConnectionId_t&);

    // Initialization
    COMMONAPI_EXPORT void registerInterface(InterfaceInitFunction _function);

    static std::weak_ptr<CommonAPI::Runtime> runtime_;

private:
    COMMONAPI_EXPORT void incrementConnection(std::shared_ptr<FDBusProxyConnection>);
    COMMONAPI_EXPORT std::shared_ptr<FDBusConnection> getConnection(const ConnectionId_t &, FDBusType_t type, const FDBusAddress& address);
    COMMONAPI_EXPORT std::shared_ptr<FDBusConnection> getConnection(std::shared_ptr<MainLoopContext>, FDBusType_t type, const FDBusAddress& address);
    COMMONAPI_EXPORT bool registerStubAdapter(std::shared_ptr<FDBusStubAdapter>);
    COMMONAPI_EXPORT bool unregisterStubAdapter(std::shared_ptr<FDBusStubAdapter>);

    // Managed services
    typedef std::unordered_map<std::string, std::shared_ptr<FDBusStubAdapter>> ServicesMap;

private:
    static std::shared_ptr<Factory> theFactory;

    std::recursive_mutex connectionsMutex_;
    std::map<ConnectionId_t, std::shared_ptr<FDBusConnection>> connections_;

    std::map<std::string, ProxyCreateFunction> proxyCreateFunctions_;
    std::map<std::string, StubAdapterCreateFunction> stubAdapterCreateFunctions_;

    ServicesMap services_;
    std::recursive_mutex servicesMutex_;

    std::list<InterfaceInitFunction> initializers_;
    std::mutex initializerMutex_;
    bool isInitialized_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_FACTORY_HPP_
