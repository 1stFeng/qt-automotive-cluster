/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#ifndef COMMONAPI_FDBUS_FDBUSCLIENT_HPP_
#define COMMONAPI_FDBUS_FDBUSCLIENT_HPP_

#include<fdbus/fdbus.h>
#include <CommonAPI/FDBus/FDBusAddress.hpp>
#include <CommonAPI/FDBus/FDBusTypes.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>

using namespace ipc::fdbus;

namespace CommonAPI {
namespace FDBus {

class FDBusProxy;
class FDBusConnection;

class FDBusClient : public ipc::fdbus::CBaseClient
{
public:
    FDBusClient(const FDBusAddress& address, CBaseWorker *worker = 0);
    bool callServer(const FDBusMessage &fdbusMessage, int32_t timeout = 0, bool async = false);

    void setConnection(FDBusConnection* connection);
    void addSubscribeTopic(const char *topic);
    void removeSubscribeTopic(const char *topic);
    void doConnect(); // client must call this method to connect to server.
    void doDisconnect(); // client must call this method to disconnect from server.
    void registerAvailabilityHandler(availability_handler_t handler); // called when client is connected or disconnected from server.
    void unregisterAvailabilityHandler();

protected:
    void onOnline(const CFdbOnlineInfo &info);
    void onOffline(const CFdbOnlineInfo &info);
    void onBroadcast(CBaseJob::Ptr &msg_ref);
    void onKickDog(CBaseJob::Ptr &msg_ref);
    void onReply(CBaseJob::Ptr &msg_ref);
    void onStatus(CBaseJob::Ptr &msg_ref
                        , int32_t error_code
                        , const char *description);
                                                
private:
    FDBusProxy* proxy_;
    FDBusConnection*  connection_;
    EFdbQOS mQOS;
    FDBusAddress mAddress;
    availability_handler_t mHandler; 	// callback handler for client's connection/disconnection to server.
};

}
}
#endif
