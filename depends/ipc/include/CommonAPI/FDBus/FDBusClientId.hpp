/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSCLIENTID_HPP_
#define COMMONAPI_FDBUS_DBUSCLIENTID_HPP_

#include <CommonAPI/Export.hpp>
#include <CommonAPI/Types.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusMessage;

/**
 * @brief Implementation of CommonAPI::ClientId for FDBus
 *
 * This class represents the DBus specific implementation of CommonAPI::ClientId.
 * It internally uses a string to identify clients. This string is the unique sender id used by dbus.
 */
class COMMONAPI_EXPORT_CLASS_EXPLICIT FDBusClientId
        : public CommonAPI::ClientId {
    friend struct std::hash<FDBusClientId>;

public:
    COMMONAPI_EXPORT FDBusClientId(std::string dbusId);

    COMMONAPI_EXPORT bool operator==(CommonAPI::ClientId& clientIdToCompare);
    COMMONAPI_EXPORT bool operator==(FDBusClientId& clientIdToCompare);
    COMMONAPI_EXPORT size_t hashCode();

    COMMONAPI_EXPORT const char * getDBusId();

    /**
     * @brief createMessage with instance, interface, signalName.
     * @param instance.
     * @param interface.
     * @param signalName.
    */
    COMMONAPI_EXPORT FDBusMessage createMessage(const std::string instance, const std::string interface, const std::string signalName) const;

    COMMONAPI_EXPORT uid_t getUid() const;
    COMMONAPI_EXPORT gid_t getGid() const;

protected:
    std::string dbusId_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // DBUSCLIENTID_HPP_
