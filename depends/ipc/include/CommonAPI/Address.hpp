// Copyright (C) 2015-2020 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_ADDRESS_HPP_
#define COMMONAPI_ADDRESS_HPP_

#include <iostream>
#include <string>

#include <CommonAPI/Export.hpp>

namespace CommonAPI {

/**
 * @brief The `class Address` is defining a class that represents an address for a CommonAPI service.
 *        It has several member functions to set and get the domain, interface, and instance of the address
 * 
 */
class Address {
public:
    COMMONAPI_EXPORT Address();
    COMMONAPI_EXPORT Address(const std::string &_address);
    COMMONAPI_EXPORT Address(const std::string &_domain,
            const std::string &_interface,
            const std::string &_instance);
    COMMONAPI_EXPORT Address(const Address &_source);
    COMMONAPI_EXPORT virtual ~Address() = default;

    COMMONAPI_EXPORT Address &operator=(const Address &_other);

    COMMONAPI_EXPORT bool operator==(const Address &_other) const;
    COMMONAPI_EXPORT bool operator!=(const Address &_other) const;
    COMMONAPI_EXPORT bool operator<(const Address &_other) const;

    COMMONAPI_EXPORT std::string getAddress() const;
    COMMONAPI_EXPORT void setAddress(const std::string &_address);

    /**
     * @brief Get the Domain object
     * 
     * @return COMMONAPI_EXPORT const& 
     */
    COMMONAPI_EXPORT const std::string &getDomain() const;
    COMMONAPI_EXPORT void setDomain(const std::string &_domain);

    COMMONAPI_EXPORT const std::string &getInterface() const;
    COMMONAPI_EXPORT void setInterface(const std::string &_interface);

    COMMONAPI_EXPORT const std::string &getInstance() const;
    COMMONAPI_EXPORT void setInstance(const std::string &_instance);

private:
    std::string domain_;
    std::string interface_;
    std::string instance_;

    friend COMMONAPI_EXPORT std::ostream &operator<<(std::ostream &_out, const Address &_address);
};

} // namespace CommonAPI

#endif // COMMONAPI_ADDRESS_HPP_
