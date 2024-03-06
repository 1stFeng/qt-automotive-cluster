/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSMESSAGE_HPP_
#define COMMONAPI_FDBUS_DBUSMESSAGE_HPP_

#include <string>
#include <atomic>

#include <fdbus/fdbus.h>

#ifdef _WIN32
#include <stdint.h>
#endif

#include <CommonAPI/Export.hpp>
#include <CommonAPI/FDBus/Types.hpp>

namespace CommonAPI {
namespace FDBus {

class FDBusAddress;
class FDBusConnection;

/*
     *=========================================================
     * FDBusMessage payload structure:
     * +-----+-----+-------+
     * |  1  |  2  |   3   |
     * +-----+-----+-------+
     *
     * 1: type
     * 2: interface/path/method ...
     * 3: body
     */

class COMMONAPI_EXPORT FDBusMessage {
public:
    /**
     * @brief This enum type defines the type of FDBusMessage, which needs to be adapted according to fdbus.
    */
    // TODO
    enum class Type: int {
        Invalid = 0,
        MethodCall = 1,
        MethodReturn = 2,
        Error = 3,
        Signal = 4
    };

    FDBusMessage();
    /**
     * @brief Construct a new FDBusMessage object
     * 
     * @param libFDBusMessage 
     */
    FDBusMessage(ipc::fdbus::CFdbMessage* libFDBusMessage);
    FDBusMessage(std::shared_ptr<ipc::fdbus::CFdbMessage> libFDBusMessage);

    FDBusMessage(const FDBusMessage &_source);
    FDBusMessage(FDBusMessage &&_source);

    ~FDBusMessage();

    FDBusMessage &operator=(const FDBusMessage &_source);
    FDBusMessage &operator=(FDBusMessage &&_source);
    operator bool() const;

    /**
     * @brief Create a Method Call object
     * 
     * @param _address 
     * @param _method 
     * @param _signature 
     * @return FDBusMessage 
     */
    static FDBusMessage createMethodCall(const FDBusAddress &_address,
                                        const std::string &_method, const std::string &_signature = "");

    /**
     * @brief Create a Method Return object
     * 
     * @param _signature 
     * @return FDBusMessage 
     */
    FDBusMessage createMethodReturn(const std::string &_signature) const;

    /**
     * @brief Create a Method Error object
     * 
     * @param _name 
     * @param _signature 
     * @param _reason 
     * @return FDBusMessage 
     */
    FDBusMessage createMethodError(const std::string &_name, const std::string &_signature = "s", const std::string &_reason = "") const;

    /**
     * @brief Create a Topic object
     * 
     * @param instance 
     * @param interfaceName 
     * @param signalName 
     * @param signature 
     * @return FDBusMessage 
     */
    static FDBusMessage createTopic(const std::string& instance,
                                    const std::string& interfaceName,
                                    const std::string& signalName,
                                    const std::string& signature = "");

    /**
     * @brief Get the Sender object
     * 
     * @return const char* 
     */
    const char* getSender() const;
    
    /**
     * @brief Get the Object Path object
     * 
     * @return const char* 
     */
    const char* getInstance() const;
    
    /**
     * @brief Get the Interface object
     * 
     * @return const char* 
     */
    const char* getInterface() const;
    
    /**
     * @brief Get the Member object
     * 
     * @return const char* 
     */
    const char* getMember() const;

    /**
     * @brief Get the Signature object
     * 
     * @return const char* 
     */
    const char* getSignature() const;

    /**
     * @brief Get the Error object
     * 
     * @return const char* 
     */
    const char* getError() const;

    /**
     * @brief Get the Destination object
     * 
     * @return const char* 
     */
    const char* getDestination() const;

    /**
     * @brief Get the Serial object
     * 
     * @return uint32_t 
     */
    uint32_t getSerial() const;

    /**
     * @brief Get the Topic object
     * 
     * @return const char* 
     */
    const char* getTopic() const;

    /**
     * @brief Get the Service object
     * 
     * @return const char* 
     */
    const char* getService() const;
    /**
     * @brief 
     * 
     * @param objectPath 
     * @return true 
     * @return false 
     */
    bool hasObjectPath(const std::string& objectPath) const;

    bool hasObjectPath(const char* objectPath) const;
    bool hasInterfaceName(const char* interfaceName) const;
    bool hasMemberName(const char* memberName) const;
    bool hasSignature(const char* signature) const;

    Type getType() const;
    bool isInvalidType() const;
    bool isMethodCallType() const;
    bool isMethodReturnType() const;
    bool isErrorType() const;
    bool isSignalType() const;

    char* getBodyData() const;
    int getBodyLength() const;
    int getBodySize() const;

    bool setBodyLength(const int bodyLength);
    bool setDestination(const char* destination);
    bool setDestination(FdbObjectId_t dest);

    void setSerial(const unsigned int serial) const;

    void setNoReplyExpected(const uint32_t replyNotExpected) const;

#ifdef COMMONAPI_FDBUS_TEST
    std::shared_ptr<ipc::fdbus::CFdbMessage> getMessage() {return mMessage;};
#endif

private:

    bool parseLibFDBusMessage(ipc::fdbus::CFdbMessage*  libFDBusMessage);
    void copy(const FDBusMessage &_message);

private:

    std::shared_ptr<ipc::fdbus::CFdbMessage> mMessage;
    Type mType = Type::Invalid;
    std::string mService;
    std::string mInstance;
    std::string mSender;
    std::string mInterface;
    std::string mMember; // method
    std::string mSignature;
    std::string mError;
    std::string mDestination;
    int32_t mBodyOffset;
    static std::atomic<int32_t> sMsgNo;

    std::string mTopic;
    friend class FDBusConnection;
    friend class FDBusClient;
    friend class FDBusServer;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSMESSAGE_HPP_
