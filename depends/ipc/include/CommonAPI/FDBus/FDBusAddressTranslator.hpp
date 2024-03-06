/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_ADDRESSTRANSLATOR_HPP_
#define COMMONAPI_FDBUS_ADDRESSTRANSLATOR_HPP_

#include <map>
#include <memory>
#include <mutex>

#include <CommonAPI/Types.hpp>
#include <CommonAPI/Address.hpp>
#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusTypes.hpp>

namespace CommonAPI {
namespace FDBus {

#ifdef COMMONAPI_FDBUS_TEST
COMMONAPI_EXPORT extern const char *ENABLE_UDP_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_RECONNECT_KEY;
COMMONAPI_EXPORT extern const char *ACTIVATE_RECONNECT_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_TCP_BLOCKING_MODE_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_AYSNC_WRITE_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_AYSNC_READ_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_BLOCKING_MODE_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_TCP_KEY;
COMMONAPI_EXPORT extern const char *ENABLE_TCP_SECURE_KEY;
COMMONAPI_EXPORT extern const char *EXPORTABLE_LEVEL_KEY;
COMMONAPI_EXPORT extern const char *SSL_PUBLIC_KEY;
COMMONAPI_EXPORT extern const char *SSL_PRIVATE_KEY;
COMMONAPI_EXPORT extern const char *SSL_ROOT_CA_KEY;
COMMONAPI_EXPORT extern const char *SSL_VERIFY_TYPE_KEY;
COMMONAPI_EXPORT extern const char *SERVICE_URL_KEY;
#endif

class FDBusAddressTranslator {
public:
    COMMONAPI_EXPORT static std::shared_ptr<FDBusAddressTranslator> get();

    COMMONAPI_EXPORT FDBusAddressTranslator();

    COMMONAPI_EXPORT void init();

    /**
     * @brief Through_ Key Construction CommonAPI::Address, and translate CommonAPI::Address into the corresponding FDBusAddress.
     * @param[in] _key, used to Construction CommonAPI::Address.
     * @param[out] _value, FDBus's Address
     * @return Whether the translation was successful.
    */
    COMMONAPI_EXPORT bool translate(const std::string &_key, FDBusAddress &_value);
    /**
     * @brief Translate CommonAPI::Address into the corresponding FDBusAddress.
     * @param[in] _key CommonAPI's Address
     * @param[out] _value FDBus's Address
     * @return Whether the translation was successful.
    */
    COMMONAPI_EXPORT bool translate(const CommonAPI::Address &_key, FDBusAddress &_value);

    /**
     * @brief Translate FDBusAddress into the corresponding CommonAPI::Address and get CommonAPI's address.
     * @param[in] _key CommonAPI::Address
     * @param[out] _value, CommonAPI's address.
     * @return Whether the translation was successful.
    */
    COMMONAPI_EXPORT bool translate(const FDBusAddress &_key, std::string &_value);
    /**
     * @brief Translate FDBusAddress into the corresponding CommonAPI::Address.
     * @param CommonAPI's Address
     * @param FDBusAddress
     * @return Whether the translation was successful.
    */
    COMMONAPI_EXPORT bool translate(const FDBusAddress &_key, CommonAPI::Address &_value);

    /**
     * @brief Create CommonAPI::Address through _address, create FDBusAddress through _service, _path, and _interface, and establish the corresponding relationship between CommonAPI::Address and FDBusAddress.
     * @param _address
     * @param interface
     * @param instance
    */
    COMMONAPI_EXPORT void insert(const std::string &_address,
        const std::string &interface, const std::string &instance);

    COMMONAPI_EXPORT void setBooleanValue(const std::string &_address,
        const std::string &key, const bool &val);
    
    COMMONAPI_EXPORT void setStringValue(const std::string &_address,
        const std::string &key, const std::string &val);

    COMMONAPI_EXPORT void setIntValue(const std::string &_address,
        const std::string &key, const int& val);

    COMMONAPI_EXPORT void appendServiceUrls(const std::string &_address,
        const std::string &url);

    /**
     * @brief Returns whether or not org.freedesktop.DBus.Peer interface is used in a (valid) name mapping.
     * @return true in case any (valid) mapping of org.freedesktop.DBus.Peer is present, otherwise false
     */
    // TODO remove it
    COMMONAPI_EXPORT bool isOrgFreedesktopDBusPeerMapped() const;

    COMMONAPI_EXPORT void remove(const CommonAPI::Address &_address);

private:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif
    COMMONAPI_EXPORT bool readConfiguration();

    COMMONAPI_EXPORT bool isValid(const std::string &, const char,
                                  bool = false, bool = false, bool = false) const;

    COMMONAPI_EXPORT bool isValidVersion(const std::string &) const;
    void updateConfig(const CommonAPI::Address &_address);

private:
#ifdef COMMONAPI_FDBUS_TEST
 public:
#endif
    bool isDefault_;

    std::string defaultConfig_;
    std::string defaultDomain_;

    std::map<CommonAPI::Address, FDBusAddress> forwards_;
    std::map<FDBusAddress, CommonAPI::Address> backwards_;
    std::map<std::string, std::string> compatibility_;
    // tuple: service, instance
    std::map<CommonAPI::Address, std::tuple<std::string, std::string>> unversioned_;
    std::map<CommonAPI::Address, FDBusAddress> persistentAddresses_;
    std::map<CommonAPI::Address, std::map<std::string, bool>> mBoolConfig;
    std::map<CommonAPI::Address, std::map<std::string, std::string>> mStringConfig;
    std::map<CommonAPI::Address, std::map<std::string, int32_t>> mIntegerConfig;
    std::map<CommonAPI::Address, std::set<std::string>> mServiceUrls;

    std::mutex mutex_;

    bool orgFreedesktopDBusPeerMapped_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_ADDRESSTRANSLATOR_HPP_
