/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_ADDRESS_HPP_
#define COMMONAPI_FDBUS_ADDRESS_HPP_

#include <iostream>
#include <map>

#include <CommonAPI/Export.hpp>
#include <fdbus/fdbus.h>

namespace CommonAPI {
namespace FDBus {

#define FDB_EP_RECONNECT_ENABLED        (1 << 16)
#define FDB_EP_RECONNECT_ACTIVATED      (1 << 17)
#define FDB_EP_ENABLE_UDP               (1 << 18)
#define FDB_EP_TCP_BLOCKING_MODE        (1 << 19)
#define FDB_EP_IPC_BLOCKING_MODE        (1 << 20)
#define FDB_EP_READ_ASYNC               (1 << 21)
#define FDB_EP_WRITE_ASYNC              (1 << 22)
#define FDB_EP_ALLOW_TCP_NORMAL         (1 << 23)
#define FDB_EP_ALLOW_TCP_SECURE         (1 << 24)
#define FDB_EP_VALID_SVC_NAME           (1 << 25)

class FDBusAddress {
public:
    COMMONAPI_EXPORT FDBusAddress(const std::string &interface = "",
                const std::string &instance = "");
    COMMONAPI_EXPORT FDBusAddress(const FDBusAddress &_source);
    COMMONAPI_EXPORT virtual ~FDBusAddress();

    COMMONAPI_EXPORT FDBusAddress &operator=(const FDBusAddress &_other);

    COMMONAPI_EXPORT bool operator==(const FDBusAddress &_other) const;
    COMMONAPI_EXPORT bool operator!=(const FDBusAddress &_other) const;
    COMMONAPI_EXPORT bool operator<(const FDBusAddress &_other) const;

    /**
     * @brief Get the interface of DBusAddress.
    */
    COMMONAPI_EXPORT const std::string &getInterface() const;
    /**
     * @brief Set the interface of DBusAddress.
    */
    COMMONAPI_EXPORT void setInterface(const std::string &_interface);

    /**
     * @brief Get the instance of DBusAddress.
    */
    COMMONAPI_EXPORT const std::string &getInstance() const;
    /**
     * @brief Set the instance of DBusAddress.
    */
    COMMONAPI_EXPORT void setInstance(const std::string &instance);

    /**
     * @brief Get the Service of DBusAddress.
    */
    COMMONAPI_EXPORT const std::string &getService() const;
    /**
     * @brief Set the Service of DBusAddress.
    */
    COMMONAPI_EXPORT void setService(const std::string &_service);

    /**
     * @brief Get the Name object
     * 
     * @return COMMONAPI_EXPORT const& 
     */
    COMMONAPI_EXPORT const std::string &getName() const;

    /**
     * @brief Set the Name object
     * 
     * @param _name 
     * @return COMMONAPI_EXPORT 
     */
    COMMONAPI_EXPORT void setName(const std::string &_name);

    /**
     * @brief Get the Obj Id object
     * 
     * @return COMMONAPI_EXPORT const& 
     */
    COMMONAPI_EXPORT const FdbObjectId_t &getObjId() const;

    /**
     * @brief Set the Obj Id object
     * 
     * @param _objId 
     * @return COMMONAPI_EXPORT 
     */
    COMMONAPI_EXPORT void setObjId(const FdbObjectId_t &_objId);

    COMMONAPI_EXPORT void enableReconnect(bool active);
    COMMONAPI_EXPORT bool reconnectEnabled() const;

    COMMONAPI_EXPORT void activateReconnect(bool active);
    COMMONAPI_EXPORT bool reconnectActivated() const;
    
    COMMONAPI_EXPORT bool UDPEnabled() const;
    COMMONAPI_EXPORT void enableUDP(bool active);

    COMMONAPI_EXPORT void enableTcpBlockingMode(bool active);
    COMMONAPI_EXPORT bool tcpBlockingModeEnabled() const;

    COMMONAPI_EXPORT void enableIpcBlockingMode(bool active);
    COMMONAPI_EXPORT bool ipcBlockingModeEnabled() const;
    
    COMMONAPI_EXPORT void enableAsyncWrite(bool active);
    COMMONAPI_EXPORT bool asyncWriteEnabled() const;

    COMMONAPI_EXPORT void enableAsyncRead(bool active);
    COMMONAPI_EXPORT bool asyncReadEnabled() const;
    
    COMMONAPI_EXPORT bool blockingModeEnabled() const;
    COMMONAPI_EXPORT void enableBlockingMode(bool active);

    COMMONAPI_EXPORT bool TCPEnabled() const;
    COMMONAPI_EXPORT void enableTCP(bool enable);

    COMMONAPI_EXPORT void enableTCPSecure(bool enable);
    COMMONAPI_EXPORT bool TCPSecureEnabled() const;

    COMMONAPI_EXPORT int32_t exportableLevel() const;
    COMMONAPI_EXPORT void setExportableLevel(int32_t level);

    COMMONAPI_EXPORT std::string publicKeyFile() const;
    COMMONAPI_EXPORT void setPublicKeyFile(const std::string& publicKeyFile);

    COMMONAPI_EXPORT std::string privateKeyFile() const;
    COMMONAPI_EXPORT void setPrivateKeyFile(const std::string& privateKeyFile);

    COMMONAPI_EXPORT std::string rootCaFile() const;
    COMMONAPI_EXPORT void setRootCaFile(const std::string& rootCaFile);

    COMMONAPI_EXPORT uint32_t verifyType() const;
    COMMONAPI_EXPORT void setVerifyType(int32_t type);

    COMMONAPI_EXPORT const std::set<std::string>& serviceUrls() const;
    COMMONAPI_EXPORT void appendServiceUrls(const std::string& url);

    COMMONAPI_EXPORT const std::string& serviceUrl() const;
    COMMONAPI_EXPORT void setServiceUrl(const std::string& url);

    COMMONAPI_EXPORT EFdbQOS qos() const;
    COMMONAPI_EXPORT void setQos(EFdbQOS qos);

private:
    std::string service_;
    std::string mInstance;
    std::string mInterface;
    uint32_t mFlag;
    int32_t mExportableLevel;
    std::string mPublicKeyFile;
    std::string mPrivateKeyFile;
    std::string mRootCaFile;
    uint32_t mVerifyType;
    EFdbQOS mQos;
    std::set<std::string> mServiceUrls; // server to bind
    std::string mServiceUrl; // client to connect

    COMMONAPI_EXPORT friend std::ostream &operator<<(std::ostream &_out, const FDBusAddress &_dbusAddress);
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_ADDRESS_HPP_
