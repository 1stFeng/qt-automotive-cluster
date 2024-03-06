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

#ifndef __CGENERICSESSION_H__
#define __CGENERICSESSION_H__

#include "CBaseFdWatch.h"

namespace ipc {
namespace fdbus {
class CGenericSocket;
class CSocketImp;

#define FdbSessionDropped(_session) (!(_session).get() || ((_session).use_count() <= 1))

class CGenericSession : public CBaseFdWatch
{
public:
    CGenericSession(CGenericSocket *socket_handle, CSocketImp *sock_imp);
    CSocketImp *getSocket()
    {
        return mSocketImp;
    }
    // TCP: retry until data is sent;
    // UDP: send and forget; no retry
    virtual int32_t sendSync(const uint8_t *data, int32_t size)
    {return -1;}
    // TCP: retry until data is sent;
    // UDP: send and forget; no retry
    virtual int32_t sendSync(const uint8_t *data, int32_t size, tFdbIpV4 ip, int32_t port)
    {return -1;}
    // TCP: submit to output queue if jamed
    // UDP: send and forget; no retry
    virtual int32_t sendAsync(const uint8_t *data, int32_t size)
    {return -1;}
    // TCP: submit to output queue if jamed
    // UDP: send and forget; no retry
    virtual int32_t sendAsync(const uint8_t *data, int32_t size, tFdbIpV4 ip, int32_t port)
    {return -1;}
    // TCP: receive until all data is available;
    // UDP: receive as many as possible and return immediately
    virtual int32_t recvSync(uint8_t *data, int32_t size)
    {return -1;}
    // TCP: receive until all data is available;
    // UDP: receive as many as possible and return immediately
    virtual int32_t recvAsync(uint8_t *data, int32_t size)
    {return -1;}
    // TCP: receive until all data is available;
    // UDP: receive as many as possible and return immediately
    virtual int32_t recvSync(uint8_t *data, int32_t size, tFdbIpV4 &ip, int32_t &port)
    {return -1;}
    // TCP: receive until all data is available;
    // UDP: receive as many as possible and return immediately
    virtual int32_t recvAsync(uint8_t *data, int32_t size, tFdbIpV4 &ip, int32_t &port)
    {return -1;}
    
    virtual ~CGenericSession();
protected:
    CGenericSocket *mSocketHandle;
    CSocketImp *mSocketImp;
    void onInput();
    void onOutput();
    void onError();
    void onHup();
    int32_t readStream(uint8_t *data, int32_t size);
};
}
}
#endif
