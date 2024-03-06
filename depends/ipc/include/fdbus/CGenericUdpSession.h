/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CGENERICUDPSESSION_H__
#define __CGENERICUDPSESSION_H__

#include "CGenericSession.h"

namespace ipc {
namespace fdbus {
class CGenericUdpSession : public CGenericSession
{
public:
    CGenericUdpSession(CGenericSocket *socket_handle, CSocketImp *sock_imp)
        : CGenericSession(socket_handle, sock_imp)
    {}
    int32_t sendSync(const uint8_t *data, int32_t size);
    int32_t sendSync(const uint8_t *data, int32_t size, tFdbIpV4 ip, int32_t port);
    int32_t sendAsync(const uint8_t *data, int32_t size);
    int32_t sendAsync(const uint8_t *data, int32_t size, tFdbIpV4 ip, int32_t port);

    int32_t recvSync(uint8_t *data, int32_t size);
    int32_t recvAsync(uint8_t *data, int32_t size);
    int32_t recvSync(uint8_t *data, int32_t size, tFdbIpV4 &ip, int32_t &port);
    int32_t recvAsync(uint8_t *data, int32_t size, tFdbIpV4 &ip, int32_t &port);
};

}
}
#endif

