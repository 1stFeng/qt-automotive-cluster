/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSSTUBADAPTERHELPER_HPP_
#define COMMONAPI_FDBUS_DBUSSTUBADAPTERHELPER_HPP_

#include <initializer_list>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <map>

#include <CommonAPI/Variant.hpp>
#include <CommonAPI/FDBus/FDBusStubAdapter.hpp>
#include <CommonAPI/FDBus/FDBusInputStream.hpp>
#include <CommonAPI/FDBus/FDBusOutputStream.hpp>
#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusSerializableArguments.hpp>
#include <CommonAPI/FDBus/FDBusClientId.hpp>

namespace CommonAPI {
namespace FDBus {

template <typename StubClass_>
class StubDispatcher {
public:

    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;

    virtual ~StubDispatcher() {}
    virtual bool dispatchFDBusMessage(const FDBusMessage &_message,
                                     const std::shared_ptr<StubClass_> &_stub,
                                     RemoteEventHandlerType* _remoteEventHandler,
                                     std::weak_ptr<FDBusProxyConnection> _connection_) = 0;
    virtual void appendGetAllReply(const FDBusMessage &_message,
                                   const std::shared_ptr<StubClass_> &_stub,
                                   FDBusOutputStream &_output) {
        (void)_message;
        (void)_stub;
        (void)_output;
    }
};

template <typename StubClass_>
struct FDBusAttributeDispatcherStruct {
    StubDispatcher<StubClass_>* getter;
    StubDispatcher<StubClass_>* setter;

    FDBusAttributeDispatcherStruct(StubDispatcher<StubClass_>* g, StubDispatcher<StubClass_>* s) {
        getter = g;
        setter = s;
    }
};

template <typename T>
struct identity { typedef T type; };

// interfaceMemberName, interfaceMemberSignature
typedef std::pair<const char*, const char*> FDBusInterfaceMemberPath;

/**
 * @brief FDBusStubAdapterHelper
 * 
 * @tparam Stubs_ 
 */
template <typename... Stubs_>
class FDBusStubAdapterHelper {
public:
  FDBusStubAdapterHelper(const FDBusAddress &_address,
                        const std::shared_ptr<FDBusProxyConnection> &_connection,
                        const bool _isManaging,
                        const std::shared_ptr<StubBase> &_stub) {
    (void)_address;
    (void)_connection;
    (void) _isManaging;
    (void) _stub;
  }
protected:
  bool findDispatcherAndHandle(const FDBusMessage& dbusMessage, FDBusInterfaceMemberPath& dbusInterfaceMemberPath) {
    (void) dbusMessage;
    (void) dbusInterfaceMemberPath;
    return false;
  }
  bool findAttributeGetDispatcherAndHandle(std::string interfaceName, std::string attributeName, const FDBusMessage &_message) {
    (void) interfaceName;
    (void) attributeName;
    (void) _message;
    return false;
  }
  bool findAttributeSetDispatcherAndHandle(std::string interfaceName, std::string attributeName, const FDBusMessage &_message) {
    (void) interfaceName;
    (void) attributeName;
    (void) _message;
    return false;
  }
  bool appendGetAllReply(const FDBusMessage& dbusMessage, FDBusOutputStream& dbusOutputStream) {
    (void) dbusMessage;
    (void) dbusOutputStream;
    return true;
  }
public:
  template <typename Stub_>
  void addStubDispatcher(FDBusInterfaceMemberPath _dbusInterfaceMemberPath,
                         StubDispatcher<Stub_>* _stubDispatcher) {
    (void) _dbusInterfaceMemberPath;
    (void) _stubDispatcher;
  }
  template <typename RemoteEventHandlerType>
  void setRemoteEventHandler(RemoteEventHandlerType * _remoteEventHandler) {
    (void) _remoteEventHandler;
  }

};

/**
 * @brief FDBusStubAdapterHelper
 * 
 * @tparam StubClass_ 
 * @tparam Stubs_ 
 */
template <typename StubClass_, typename... Stubs_>
class FDBusStubAdapterHelper<StubClass_, Stubs_...>:
 public virtual FDBusStubAdapter,
 public FDBusStubAdapterHelper<Stubs_...> {
 public:
    typedef typename StubClass_::StubAdapterType StubAdapterType;
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;

    typedef std::unordered_map<FDBusInterfaceMemberPath, StubDispatcher<StubClass_>*> StubDispatcherTable;
    typedef std::unordered_map<std::string, FDBusAttributeDispatcherStruct<StubClass_>> StubAttributeTable;

    FDBusStubAdapterHelper(const FDBusAddress &_address,
                          const std::shared_ptr<FDBusProxyConnection> &_connection,
                          const bool _isManaging,
                          const std::shared_ptr<StubBase> &_stub) :

                    FDBusStubAdapter(_address, _connection, _isManaging),
                    FDBusStubAdapterHelper<Stubs_...>(_address, _connection, _isManaging, _stub),
                    remoteEventHandler_(nullptr) {
                    stub_ = std::dynamic_pointer_cast<StubClass_>(_stub);
    }

    virtual ~FDBusStubAdapterHelper() {
        FDBusStubAdapter::deinit();
        stub_.reset();
    }

    virtual void init(std::shared_ptr<FDBusStubAdapter> instance) {
        FDBusStubAdapter::init(instance);
        std::shared_ptr<StubAdapterType> stubAdapter = std::dynamic_pointer_cast<StubAdapterType>(instance);
        remoteEventHandler_ = stub_->initStubAdapter(stubAdapter);
        FDBusStubAdapterHelper<Stubs_...>::setRemoteEventHandler(remoteEventHandler_);
    }

    void setRemoteEventHandler(RemoteEventHandlerType* _remoteEventHandler) {
      remoteEventHandler_ = _remoteEventHandler;
      FDBusStubAdapterHelper<Stubs_...>::setRemoteEventHandler(remoteEventHandler_);
    }

    virtual void deinit() {
        FDBusStubAdapter::deinit();
        stub_.reset();
    }

    inline RemoteEventHandlerType* getRemoteEventHandler() {
        return remoteEventHandler_;
    }

 protected:

    /**
     * @brief Callbacked when adapter reviced message from the connection.
     * 
     * @param dbusMessage 
     * @return true when find dispatcher
     * @return false 
     */
    virtual bool onInterfaceFDBusMessage(const FDBusMessage& dbusMessage) {
        const char* interfaceMemberName = dbusMessage.getMember();
        const char* interfaceMemberSignature = dbusMessage.getSignature();

        if (NULL == interfaceMemberName) {
            COMMONAPI_ERROR(std::string(__FUNCTION__), " member empty");
        }
        if (NULL == interfaceMemberSignature) {
            COMMONAPI_ERROR(std::string(__FUNCTION__), " signature empty");
        }

        FDBusInterfaceMemberPath dbusInterfaceMemberPath = {interfaceMemberName, interfaceMemberSignature};
        return findDispatcherAndHandle(dbusMessage, dbusInterfaceMemberPath);
    }

    /**
     * @brief Find dispatcherand handle
     * 
     * @param dbusMessage 
     * @param dbusInterfaceMemberPath 
     * @return true 
     * @return false 
     */
    bool findDispatcherAndHandle(const FDBusMessage& dbusMessage, FDBusInterfaceMemberPath& dbusInterfaceMemberPath) {
        auto findIterator = stubDispatcherTable_.find(dbusInterfaceMemberPath);
        const bool foundInterfaceMemberHandler = (findIterator != stubDispatcherTable_.end());
        bool dbusMessageHandled = false;
        if (foundInterfaceMemberHandler) {
            StubDispatcher<StubClass_>* stubDispatcher = findIterator->second;
            dbusMessageHandled = stubDispatcher->dispatchFDBusMessage(dbusMessage, stub_, getRemoteEventHandler(), getFDBusConnection());
            return dbusMessageHandled;
        }

        return FDBusStubAdapterHelper<Stubs_...>::findDispatcherAndHandle(dbusMessage, dbusInterfaceMemberPath);
    }

    // // TODO FDBus should support "Get"/"GetAll"/"Set" Method
    // virtual bool onInterfaceFDBusFreedesktopPropertiesMessage(const FDBusMessage &_message) {
    //     FDBusInputStream input(_message);

    //     if (_message.hasMemberName("Get")) {
    //         return handleFreedesktopGet(_message, input);
    //     } else if (_message.hasMemberName("Set")) {
    //         return handleFreedesktopSet(_message, input);
    //     } else if (_message.hasMemberName("GetAll")) {
    //         return handleFreedesktopGetAll(_message, input);
    //     }

    //     return false;
    // }

    template <typename Stub_>
    void addStubDispatcher(FDBusInterfaceMemberPath _dbusInterfaceMemberPath,
                           StubDispatcher<Stub_>* _stubDispatcher) {
        addStubDispatcher(_dbusInterfaceMemberPath, _stubDispatcher, identity<Stub_>());
    }

    template <typename Stub_>
    void addAttributeDispatcher(std::string _key,
                                StubDispatcher<Stub_>* _stubDispatcherGetter,
                                StubDispatcher<Stub_>* _stubDispatcherSetter) {
        addAttributeDispatcher(_key, _stubDispatcherGetter, _stubDispatcherSetter, identity<Stub_>());
    }

    std::shared_ptr<StubClass_> stub_;
    RemoteEventHandlerType* remoteEventHandler_;
    StubDispatcherTable stubDispatcherTable_;
    StubAttributeTable stubAttributeTable_;

protected:

    /**
     * @brief handle get method
     * 
     * @param _message 
     * @param _input 
     * @return true 
     * @return false 
     */
    bool handleFreedesktopGet(const FDBusMessage &_message, FDBusInputStream &_input) {
        std::string interfaceName;
        std::string attributeName;
        _input >> interfaceName;
        _input >> attributeName;

        if (_input.hasError()) {
            return false;
        }
        return findAttributeGetDispatcherAndHandle(interfaceName, attributeName, _message);
    }

    /**
     * @brief Find attribute get dispatcher and handle
     * 
     * @param interfaceName 
     * @param attributeName 
     * @param _message 
     * @return true 
     * @return false 
     */
    bool findAttributeGetDispatcherAndHandle(std::string interfaceName, std::string attributeName, const FDBusMessage &_message) {

        auto attributeDispatcherIterator = stubAttributeTable_.find(attributeName);
        if (attributeDispatcherIterator == stubAttributeTable_.end()) {
            // not found, try parent
            return FDBusStubAdapterHelper<Stubs_...>::findAttributeGetDispatcherAndHandle(interfaceName, attributeName, _message);
        }

        StubDispatcher<StubClass_>* getterDispatcher = attributeDispatcherIterator->second.getter;
        if (NULL == getterDispatcher) { // all attributes have at least a getter
            COMMONAPI_ERROR(std::string(__FUNCTION__), "getterDispatcher == NULL");
            return false;
        } else {
            return getterDispatcher->dispatchFDBusMessage(_message, stub_, getRemoteEventHandler(), getFDBusConnection());
        }
    }

    /**
     * @brief Handle set method
     * 
     * @param dbusMessage 
     * @param dbusInputStream 
     * @return true 
     * @return false 
     */
    bool handleFreedesktopSet(const FDBusMessage& dbusMessage, FDBusInputStream& dbusInputStream) {
        std::string interfaceName;
        std::string attributeName;
        dbusInputStream >> interfaceName;
        dbusInputStream >> attributeName;

        if(dbusInputStream.hasError()) {
            return false;
        }

        return findAttributeSetDispatcherAndHandle(interfaceName, attributeName, dbusMessage);
    }

    /**
     * @brief Find attribute set dispatcher and handle
     * 
     * @param interfaceName 
     * @param attributeName 
     * @param dbusMessage 
     * @return true 
     * @return false 
     */
    bool findAttributeSetDispatcherAndHandle(std::string interfaceName, std::string attributeName, const FDBusMessage& dbusMessage) {

        auto attributeDispatcherIterator = stubAttributeTable_.find(attributeName);
        if(attributeDispatcherIterator == stubAttributeTable_.end()) {
          // not found, try parent
          return FDBusStubAdapterHelper<Stubs_...>::findAttributeSetDispatcherAndHandle(interfaceName, attributeName, dbusMessage);

        }

        StubDispatcher<StubClass_> *setterDispatcher = attributeDispatcherIterator->second.setter;
        if (setterDispatcher == NULL) { // readonly attributes do not have a setter
            return false;
        }

        return setterDispatcher->dispatchFDBusMessage(dbusMessage, stub_, getRemoteEventHandler(), getFDBusConnection());
    }

    bool appendGetAllReply(const FDBusMessage& dbusMessage, FDBusOutputStream& dbusOutputStream)
    {
        for(auto attributeDispatcherIterator = stubAttributeTable_.begin(); attributeDispatcherIterator != stubAttributeTable_.end(); attributeDispatcherIterator++) {

            //To prevent the destruction of the stub whilst still handling a message
            if (stub_) {
                StubDispatcher<StubClass_>* getterDispatcher = attributeDispatcherIterator->second.getter;
                if (NULL == getterDispatcher) { // all attributes have at least a getter
                    COMMONAPI_ERROR(std::string(__FUNCTION__), "getterDispatcher == NULL");
                    return false;
                } else {
                    dbusOutputStream.align(8);
                    dbusOutputStream << attributeDispatcherIterator->first;
                    getterDispatcher->appendGetAllReply(dbusMessage, stub_, dbusOutputStream);
                }
            }
        }
        return FDBusStubAdapterHelper<Stubs_...>::appendGetAllReply(dbusMessage, dbusOutputStream);
     }

 private:

   template <typename Stub_>
   void addStubDispatcher(FDBusInterfaceMemberPath _dbusInterfaceMemberPath,
                          StubDispatcher<Stub_>* _stubDispatcher,
                          identity<Stub_>) {
       FDBusStubAdapterHelper<Stubs_...>::addStubDispatcher(_dbusInterfaceMemberPath, _stubDispatcher);

   }

   void addStubDispatcher(FDBusInterfaceMemberPath _dbusInterfaceMemberPath,
                          StubDispatcher<StubClass_>* _stubDispatcher,
                          identity<StubClass_>) {
       stubDispatcherTable_.insert({_dbusInterfaceMemberPath, _stubDispatcher});

   }

   template <typename Stub_>
   void addAttributeDispatcher(std::string _key,
                          StubDispatcher<Stub_>* _stubDispatcherGetter,
                          StubDispatcher<Stub_>* _stubDispatcherSetter,
                          identity<Stub_>) {
       FDBusStubAdapterHelper<Stubs_...>::addAttributeDispatcher(_key, _stubDispatcherGetter, _stubDispatcherSetter);

   }

   void addAttributeDispatcher(std::string _key,
                          StubDispatcher<StubClass_>* _stubDispatcherGetter,
                          StubDispatcher<StubClass_>* _stubDispatcherSetter,
                          identity<StubClass_>) {
       stubAttributeTable_.insert({_key, {_stubDispatcherGetter, _stubDispatcherSetter}});
   }

    /**
     * @brief Handle GetAll method.
     * 
     * @param dbusMessage 
     * @param dbusInputStream 
     * @return true 
     * @return false 
     */
   bool handleFreedesktopGetAll(const FDBusMessage& dbusMessage, FDBusInputStream& dbusInputStream) {
         std::string interfaceName;
         dbusInputStream >> interfaceName;

         if(dbusInputStream.hasError()) {
             return false;
         }

         FDBusMessage dbusMessageReply = dbusMessage.createMethodReturn("a{sv}");
         FDBusOutputStream dbusOutputStream(dbusMessageReply);

         dbusOutputStream.beginWriteMap();
         appendGetAllReply(dbusMessage, dbusOutputStream);
         dbusOutputStream.endWriteMap();
         dbusOutputStream.flush();

         return getFDBusConnection()->sendFDBusMessage(dbusMessageReply);
     }

};

template< class >
struct FDBusStubSignalHelper;

/**
 * @brief FDBusStubSignalHelper
 * 
 * @tparam In_ 
 * @tparam InArgs_ 
 */
template<template<class ...> class In_, class... InArgs_>
struct FDBusStubSignalHelper<In_<FDBusInputStream, FDBusOutputStream, InArgs_...>> {

    static inline bool sendTopic(const char* instance,
                           const char* interfaceName,
                    const char* signalName,
                    const char* signalSignature,
                    const std::shared_ptr<FDBusProxyConnection>& dbusConnection,
                    const InArgs_&... inArgs) {
        FDBusMessage dbusMessage = FDBusMessage::createTopic(
                        instance,
                        interfaceName,
                        signalName,
                        signalSignature);

        if (sizeof...(InArgs_) > 0) {
            FDBusOutputStream outputStream(dbusMessage);
            const bool success = FDBusSerializableArguments<InArgs_...>::serialize(outputStream, inArgs...);
            if (!success) {
                COMMONAPI_ERROR("FDBusStubSignalHelper (dbus): serialization failed! [",
                                dbusMessage.getInstance(), " ",
                                dbusMessage.getInterface(), ".",
                                dbusMessage.getMember(), " ",
                                dbusMessage.getSerial());
                return false;
            }
            outputStream.flush();
        }

        const bool dbusMessageSent = dbusConnection->sendFDBusMessage(dbusMessage);
        return dbusMessageSent;
    }

    template <typename FDBusStub_ = FDBusStubAdapter>
    static bool sendTopic(const FDBusStub_ &_stub,
                    const char *_name,
                    const char *_signature,
                    const InArgs_&... inArgs) {
        return(sendTopic(_stub.getFDBusAddress().getInstance().c_str(),
                          _stub.getFDBusAddress().getInterface().c_str(),
                          _name,
                          _signature,
                          _stub.getFDBusConnection(),
                          inArgs...));
    }


    template <typename FDBusStub_ = FDBusStubAdapter>
       static bool sendTopic(const char *_target,
                                    const FDBusStub_ &_stub,
                                    const char *_name,
                                    const char *_signature,
                                    const InArgs_&... inArgs) {
           FDBusMessage dbusMessage
                  = FDBusMessage::createTopic(
                   _stub.getFDBusAddress().getInstance().c_str(),
                   _stub.getFDBusAddress().getInterface().c_str(),
                   _name,
                   _signature);

           dbusMessage.setDestination(_target);

           if (sizeof...(InArgs_) > 0) {
               FDBusOutputStream outputStream(dbusMessage);
               const bool success = FDBusSerializableArguments<InArgs_...>::serialize(outputStream, inArgs...);
               if (!success) {
                   COMMONAPI_ERROR("FDBusStubSignalHelper 2 (dbus): serialization failed! [",
                                   dbusMessage.getInstance(), " ",
                                   dbusMessage.getInterface(), ".",
                                   dbusMessage.getMember(), " ",
                                   dbusMessage.getSerial());
                   return false;
               }
               outputStream.flush();
           }

           return _stub.getFDBusConnection()->sendFDBusMessage(dbusMessage);
       }
};

template< class, class, class >
class FDBusMethodStubDispatcher;

/**
 * @brief FDBusMethodStubDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam DeplIn_ 
 * @tparam DeplIn_Args 
 */
template <
    typename StubClass_,
    template <class...> class In_, class... InArgs_,
    template <class...> class DeplIn_, class... DeplIn_Args>

class FDBusMethodStubDispatcher<StubClass_, In_<InArgs_...>, DeplIn_<DeplIn_Args...> >: public StubDispatcher<StubClass_> {
 public:

    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef void (StubClass_::*StubFunctor_)(std::shared_ptr<CommonAPI::ClientId>, InArgs_...);

    FDBusMethodStubDispatcher(StubFunctor_ stubFunctor, const bool _isImplemented, std::tuple<DeplIn_Args*...> _in):
            stubFunctor_(stubFunctor), isImplemented_(_isImplemented) {
            initialize(typename make_sequence_range<sizeof...(DeplIn_Args), 0>::type(), _in);
    }

    bool dispatchFDBusMessage(const FDBusMessage& dbusMessage, const std::shared_ptr<StubClass_>& stub,
          RemoteEventHandlerType* _remoteEventHandler,
          std::weak_ptr<FDBusProxyConnection> _connection) {
            (void) _remoteEventHandler;
            (void) _connection;

        if (!this->isImplemented_)
            return false;

        return handleFDBusMessage(dbusMessage, stub, typename make_sequence_range<sizeof...(InArgs_), 0>::type());
    }

 private:
    template <size_t... DeplIn_ArgIndices>
    inline void initialize(index_sequence<DeplIn_ArgIndices...>, std::tuple<DeplIn_Args*...> &_in) {
        in_ = std::make_tuple(std::get<DeplIn_ArgIndices>(_in)...);
    }

    template <size_t... InArgIndices_>
    inline bool handleFDBusMessage(const FDBusMessage& dbusMessage,
                                  const std::shared_ptr<StubClass_>& stub,
                                  index_sequence<InArgIndices_...>) {

        if (sizeof...(InArgs_) > 0) {
            FDBusInputStream dbusInputStream(dbusMessage);
            const bool success = FDBusSerializableArguments<CommonAPI::Deployable<InArgs_, DeplIn_Args>...>::deserialize(dbusInputStream, std::get<InArgIndices_>(in_)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusMethodStubDispatcher (dbus): deserialization failed! [",
                                dbusMessage.getInstance(), " ",
                                dbusMessage.getInterface(), ".",
                                dbusMessage.getMember(), " ",
                                dbusMessage.getSerial());
                return false;
            }
        }

        std::shared_ptr<FDBusClientId> clientId = std::make_shared<FDBusClientId>(std::string(dbusMessage.getSender()));

        (stub.get()->*stubFunctor_)(clientId, std::move(std::get<InArgIndices_>(in_).getValue())...);

        return true;
    }

    StubFunctor_ stubFunctor_;
    const bool isImplemented_;
    std::tuple<CommonAPI::Deployable<InArgs_, DeplIn_Args>...> in_;
};

template< class, class, class, class, class...>
class FDBusMethodWithReplyStubDispatcher;

/**
 * @brief FDBusMethodWithReplyStubDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam Out_ 
 * @tparam OutArgs_ 
 * @tparam DeplIn_ 
 * @tparam DeplIn_Args 
 * @tparam DeplOut_ 
 * @tparam DeplOutArgs_ 
 */
template <
    typename StubClass_,
    template <class...> class In_, class... InArgs_,
    template <class...> class Out_, class... OutArgs_,
    template <class...> class DeplIn_, class... DeplIn_Args,
    template <class...> class DeplOut_, class... DeplOutArgs_>

class FDBusMethodWithReplyStubDispatcher<
       StubClass_,
       In_<InArgs_...>,
       Out_<OutArgs_...>,
       DeplIn_<DeplIn_Args...>,
       DeplOut_<DeplOutArgs_...>>:
            public StubDispatcher<StubClass_> {
 public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef std::function<void (OutArgs_...)> ReplyType_t;

    typedef void (StubClass_::*StubFunctor_)(
                std::shared_ptr<CommonAPI::ClientId>, InArgs_..., ReplyType_t);

    FDBusMethodWithReplyStubDispatcher(StubFunctor_ _stubFunctor,
            const char* _dbusReplySignature,
            const bool _isImplemented,
            const std::tuple<DeplIn_Args*...> &_inDepArgs,
            const std::tuple<DeplOutArgs_*...> &_outDepArgs)
        : out_(_outDepArgs),
          currentCall_(0),
          isImplemented_(_isImplemented),
          stubFunctor_(_stubFunctor),
          dbusReplySignature_(_dbusReplySignature) {

        initialize(typename make_sequence_range<sizeof...(DeplIn_Args), 0>::type(), _inDepArgs);
    }

    bool dispatchFDBusMessage(const FDBusMessage& _dbusMessage,
                             const std::shared_ptr<StubClass_>& _stub,
                             RemoteEventHandlerType* _remoteEventHandler,
                             std::weak_ptr<FDBusProxyConnection> _connection) {
        (void) _remoteEventHandler;

        if (!this->isImplemented_)
            return false;

        connection_ = _connection;
        return handleFDBusMessage(
                _dbusMessage,
                _stub,
                typename make_sequence_range<sizeof...(InArgs_), 0>::type(),
                typename make_sequence_range<sizeof...(OutArgs_), 0>::type());
    }

    bool sendReply(const CommonAPI::CallId_t _call,
                   const std::tuple<CommonAPI::Deployable<OutArgs_, DeplOutArgs_>...> args = std::make_tuple()) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto dbusMessage = pending_.find(_call);
            if(dbusMessage != pending_.end()) {
                pending_[_call] = dbusMessage->second.createMethodReturn(dbusReplySignature_);;
            } else {
                return false;
            }
        }
        return sendReplyInternal(_call, typename make_sequence_range<sizeof...(OutArgs_), 0>::type(), args);
    }

protected:

    std::tuple<CommonAPI::Deployable<InArgs_, DeplIn_Args>...> in_;
    std::tuple<DeplOutArgs_*...> out_;
    CommonAPI::CallId_t currentCall_;
    std::map<CommonAPI::CallId_t, FDBusMessage> pending_;
    std::mutex mutex_; // protects pending_

    std::weak_ptr<FDBusProxyConnection> connection_;
    const bool isImplemented_;

private:

    template <size_t... DeplIn_ArgIndices>
    inline void initialize(index_sequence<DeplIn_ArgIndices...>, const std::tuple<DeplIn_Args*...>& _in) {
        in_ = std::make_tuple(std::get<DeplIn_ArgIndices>(_in)...);
    }

    template <size_t... InArgIndices_, size_t... OutArgIndices_>
    inline bool handleFDBusMessage(const FDBusMessage& _dbusMessage,
                                  const std::shared_ptr<StubClass_>& _stub,
                                  index_sequence<InArgIndices_...>,
                                  index_sequence<OutArgIndices_...>) {
        if (sizeof...(DeplIn_Args) > 0) {
            FDBusInputStream dbusInputStream(_dbusMessage);
            const bool success = FDBusSerializableArguments<CommonAPI::Deployable<InArgs_, DeplIn_Args>...>::deserialize(dbusInputStream, std::get<InArgIndices_>(in_)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusMethodWithReplyStubDispatcher (dbus): deserialization failed! [",
                                _dbusMessage.getInstance(), " ",
                                _dbusMessage.getInterface(), ".",
                                _dbusMessage.getMember(), " ",
                                _dbusMessage.getSerial());
                return false;
            }
        }

        std::shared_ptr<FDBusClientId> clientId
            = std::make_shared<FDBusClientId>(std::string(_dbusMessage.getSender()));

        CommonAPI::CallId_t call;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            call = currentCall_++;
            pending_[call] = _dbusMessage;
        }

        (_stub.get()->*stubFunctor_)(
            clientId,
            std::move(std::get<InArgIndices_>(in_).getValue())...,
            [call, this](OutArgs_... _args){
                this->sendReply(call, std::make_tuple(CommonAPI::Deployable<OutArgs_, DeplOutArgs_>(
                            _args, std::get<OutArgIndices_>(out_)
                        )...));
            }
        );

        return true;
    }

    template<size_t... OutArgIndices_>
    bool sendReplyInternal(const CommonAPI::CallId_t _call,
                           index_sequence<OutArgIndices_...>,
                           const std::tuple<CommonAPI::Deployable<OutArgs_, DeplOutArgs_>...>& _args) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto reply = pending_.find(_call);
        if (reply != pending_.end()) {
            if (sizeof...(DeplOutArgs_) > 0) {
                FDBusOutputStream output(reply->second);
                if (!FDBusSerializableArguments<CommonAPI::Deployable<OutArgs_, DeplOutArgs_>...>::serialize(
                        output, std::get<OutArgIndices_>(_args)...)) {
                    (void)_args;
                    pending_.erase(_call);
                    COMMONAPI_ERROR("FDBusMethodWithReplyStubDispatcher (dbus): serialization failed! [",
                                    reply->second.getInstance(), " ",
                                    reply->second.getInterface(), ".",
                                    reply->second.getMember(), " ",
                                    reply->second.getSerial());
                    return false;
                }
                output.flush();
            }
            if (std::shared_ptr<FDBusProxyConnection> connection = connection_.lock()) {
                bool isSuccessful = connection->sendFDBusMessage(reply->second);
                pending_.erase(_call);
                return isSuccessful;
            }
            else {
                return false;
            }
        }
        return false;
    }

    StubFunctor_ stubFunctor_;
    const char* dbusReplySignature_;
};

/**
 * @brief FDBusMethodWithReplyStubDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam Out_ 
 * @tparam OutArgs_ 
 * @tparam DeplIn_ 
 * @tparam DeplIn_Args 
 * @tparam DeplOut_ 
 * @tparam DeplOutArgs_ 
 * @tparam ErrorReplies_ 
 */
template <
    typename StubClass_,
    template <class...> class In_, class... InArgs_,
    template <class...> class Out_, class... OutArgs_,
    template <class...> class DeplIn_, class... DeplIn_Args,
    template <class...> class DeplOut_, class... DeplOutArgs_,
    class... ErrorReplies_>

class FDBusMethodWithReplyStubDispatcher<
       StubClass_,
       In_<InArgs_...>,
       Out_<OutArgs_...>,
       DeplIn_<DeplIn_Args...>,
       DeplOut_<DeplOutArgs_...>,
       ErrorReplies_...> :
            public FDBusMethodWithReplyStubDispatcher<
                StubClass_,
                In_<InArgs_...>,
                Out_<OutArgs_...>,
                DeplIn_<DeplIn_Args...>,
                DeplOut_<DeplOutArgs_...>> {
 public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef std::function<void (OutArgs_...)> ReplyType_t;

    typedef void (StubClass_::*StubFunctor_)(
                std::shared_ptr<CommonAPI::ClientId>, CommonAPI::CallId_t, InArgs_..., ReplyType_t, ErrorReplies_...);

    FDBusMethodWithReplyStubDispatcher(StubFunctor_ _stubFunctor, const char* _dbusReplySignature,
            const bool _isImplemented,
            const std::tuple<DeplIn_Args*...> &_inDepArgs,
            const std::tuple<DeplOutArgs_*...> &_outDepArgs,
            const ErrorReplies_... _errorReplies)
        : FDBusMethodWithReplyStubDispatcher<StubClass_, In_<InArgs_...>, Out_<OutArgs_...>,
              DeplIn_<DeplIn_Args...>, DeplOut_<DeplOutArgs_...>>(NULL,
                      _dbusReplySignature, _isImplemented, _inDepArgs, _outDepArgs),
          stubFunctor_(_stubFunctor),
          errorReplies_(std::make_tuple(_errorReplies...)) { }

    bool dispatchFDBusMessage(const FDBusMessage& _dbusMessage,
                             const std::shared_ptr<StubClass_>& _stub,
                             RemoteEventHandlerType* _remoteEventHandler,
                             std::weak_ptr<FDBusProxyConnection> _connection) {
        (void) _remoteEventHandler;

        if (!this->isImplemented_)
            return false;

        this->connection_ = _connection;
        return handleFDBusMessage(
                _dbusMessage,
                _stub,
                typename make_sequence_range<sizeof...(InArgs_), 0>::type(),
                typename make_sequence_range<sizeof...(OutArgs_), 0>::type(),
                typename make_sequence_range<sizeof...(ErrorReplies_), 0>::type());
    }

    template <class... ErrorReplyOutArgs_, class... ErrorReplyDeplOutArgs_>
    bool sendErrorReply(const CommonAPI::CallId_t _call,
                        const std::string &_signature,
                        const std::string &_errorName,
                        const std::tuple<CommonAPI::Deployable<ErrorReplyOutArgs_, ErrorReplyDeplOutArgs_>...>& _args) {
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            auto dbusMessage = this->pending_.find(_call);
            if(dbusMessage != this->pending_.end()) {
                this->pending_[_call] = dbusMessage->second.createMethodError(_errorName, _signature);;
            } else {
                return false;
            }
        }
        return sendErrorReplyInternal(_call, typename make_sequence_range<sizeof...(ErrorReplyOutArgs_), 0>::type(), _args);
    }

private:

    template <size_t... InArgIndices_, size_t... OutArgIndices_, size_t... ErrorRepliesIndices_>
    inline bool handleFDBusMessage(const FDBusMessage& _dbusMessage,
                                  const std::shared_ptr<StubClass_>& _stub,
                                  index_sequence<InArgIndices_...>,
                                  index_sequence<OutArgIndices_...>,
                                  index_sequence<ErrorRepliesIndices_...>) {
        if (sizeof...(DeplIn_Args) > 0) {
            FDBusInputStream dbusInputStream(_dbusMessage);
            const bool success = FDBusSerializableArguments<CommonAPI::Deployable<InArgs_, DeplIn_Args>...>::deserialize(dbusInputStream, std::get<InArgIndices_>(this->in_)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusMethodWithReplyStubDispatcher w/ error replies (dbus): deserialization failed! [",
                                _dbusMessage.getInstance(), " ",
                                _dbusMessage.getInterface(), ".",
                                _dbusMessage.getMember(), " ",
                                _dbusMessage.getSerial());
                return false;
            }
        }

        std::shared_ptr<FDBusClientId> clientId
            = std::make_shared<FDBusClientId>(std::string(_dbusMessage.getSender()));

        CommonAPI::CallId_t call;
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            call = this->currentCall_++;
            this->pending_[call] = _dbusMessage;
        }

        (_stub.get()->*stubFunctor_)(
            clientId,
            call,
            std::move(std::get<InArgIndices_>(this->in_).getValue())...,
            [call, this](OutArgs_... _args){
                this->sendReply(call, std::make_tuple(CommonAPI::Deployable<OutArgs_, DeplOutArgs_>(
                            _args, std::get<OutArgIndices_>(this->out_)
                        )...));
            },
            std::get<ErrorRepliesIndices_>(errorReplies_)...
        );

        return true;
    }

    template<size_t... ErrorReplyOutArgIndices_, class... ErrorReplyOutArgs_, class ...ErrorReplyDeplOutArgs_>
    bool sendErrorReplyInternal(CommonAPI::CallId_t _call,
                           index_sequence<ErrorReplyOutArgIndices_...>,
                           const std::tuple<CommonAPI::Deployable<ErrorReplyOutArgs_, ErrorReplyDeplOutArgs_>...>& _args) {
        std::lock_guard<std::mutex> lock(this->mutex_);
        auto reply = this->pending_.find(_call);
        if (reply != this->pending_.end()) {
            if (sizeof...(ErrorReplyDeplOutArgs_) > 0) {
                FDBusOutputStream output(reply->second);
                if (!FDBusSerializableArguments<CommonAPI::Deployable<ErrorReplyOutArgs_, ErrorReplyDeplOutArgs_>...>::serialize(
                        output, std::get<ErrorReplyOutArgIndices_>(_args)...)) {
                    (void)_args;
                    this->pending_.erase(_call);
                    COMMONAPI_ERROR("FDBusMethodWithReplyStubDispatcher w/ error replies 2 (dbus): serialization failed! [",
                                    reply->second.getInstance(), " ",
                                    reply->second.getInterface(), ".",
                                    reply->second.getMember(), " ",
                                    reply->second.getSerial());
                    return false;
                }
                output.flush();
            }
            if (std::shared_ptr<FDBusProxyConnection> connection = this->connection_.lock()) {
                bool isSuccessful = connection->sendFDBusMessage(reply->second);
                this->pending_.erase(_call);
                return isSuccessful;
            }
            else {
                return false;
            }
        }
        return false;
    }

    StubFunctor_ stubFunctor_;
    std::tuple<ErrorReplies_...> errorReplies_;
};

template< class, class, class, class >
class FDBusMethodWithReplyAdapterDispatcher;

/**
 * @brief FDBusMethodWithReplyAdapterDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam StubAdapterClass_ 
 * @tparam In_ 
 * @tparam InArgs_ 
 * @tparam Out_ 
 * @tparam OutArgs_ 
 */
template <
    typename StubClass_,
    typename StubAdapterClass_,
    template <class...> class In_, class... InArgs_,
    template <class...> class Out_, class... OutArgs_>
class FDBusMethodWithReplyAdapterDispatcher<StubClass_, StubAdapterClass_, In_<InArgs_...>, Out_<OutArgs_...> >
    : public StubDispatcher<StubClass_> {
 public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef void (StubAdapterClass_::*StubFunctor_)(std::shared_ptr<CommonAPI::ClientId>, InArgs_..., OutArgs_&...);
    typedef typename CommonAPI::Stub<typename StubClass_::StubAdapterType, typename StubClass_::RemoteEventType> StubType;

    FDBusMethodWithReplyAdapterDispatcher(StubFunctor_ stubFunctor, const char* dbusReplySignature, const bool _isImplemented)
        : stubFunctor_(stubFunctor),
          dbusReplySignature_(dbusReplySignature),
          isImplemented_(_isImplemented) {
    }

    bool dispatchFDBusMessage(const FDBusMessage& dbusMessage, const std::shared_ptr<StubClass_>& stub,
        RemoteEventHandlerType *_remoteEventHandler, std::weak_ptr<FDBusProxyConnection> _connection) {

        (void)_remoteEventHandler;

        if (!this->isImplemented_)
            return false;

        std::tuple<InArgs_..., OutArgs_...> argTuple;
        return handleFDBusMessage(
                        dbusMessage,
                        stub,
                        _connection,
                        typename make_sequence_range<sizeof...(InArgs_), 0>::type(),
                        typename make_sequence_range<sizeof...(OutArgs_), sizeof...(InArgs_)>::type(),argTuple);
    }

 private:
    template <size_t... InArgIndices_, size_t... OutArgIndices_>
    inline bool handleFDBusMessage(const FDBusMessage& dbusMessage,
                                  const std::shared_ptr<StubClass_>& stub,
                                  std::weak_ptr<FDBusProxyConnection> _connection,
                                  index_sequence<InArgIndices_...>,
                                  index_sequence<OutArgIndices_...>,
                                  std::tuple<InArgs_..., OutArgs_...> argTuple) const {
        (void)argTuple;

        if (sizeof...(InArgs_) > 0) {
            FDBusInputStream dbusInputStream(dbusMessage);
            const bool success = FDBusSerializableArguments<InArgs_...>::deserialize(dbusInputStream, std::get<InArgIndices_>(argTuple)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusMethodWithReplyAdapterDispatcher (dbus): deserialization failed! [",
                                dbusMessage.getInstance(), " ",
                                dbusMessage.getInterface(), ".",
                                dbusMessage.getMember(), " ",
                                dbusMessage.getSerial());
                return false;
            }
        }

        std::shared_ptr<FDBusClientId> clientId = std::make_shared<FDBusClientId>(std::string(dbusMessage.getSender()));

        (stub->StubType::getStubAdapter().get()->*stubFunctor_)(clientId, std::move(std::get<InArgIndices_>(argTuple))..., std::get<OutArgIndices_>(argTuple)...);
        FDBusMessage dbusMessageReply = dbusMessage.createMethodReturn(dbusReplySignature_);

        if (sizeof...(OutArgs_) > 0) {
            FDBusOutputStream dbusOutputStream(dbusMessageReply);
            const bool success = FDBusSerializableArguments<OutArgs_...>::serialize(dbusOutputStream, std::get<OutArgIndices_>(argTuple)...);
            if (!success) {
                COMMONAPI_ERROR("FDBusMethodWithReplyAdapterDispatcher (dbus): serialization failed! [",
                                dbusMessageReply.getInstance(), " ",
                                dbusMessageReply.getInterface(), ".",
                                dbusMessageReply.getMember(), " ",
                                dbusMessageReply.getSerial());
                return false;
            }

            dbusOutputStream.flush();
        }
        if (std::shared_ptr<FDBusProxyConnection> connection = _connection.lock()) {
            bool isSuccessful = connection->sendFDBusMessage(dbusMessageReply);
            return isSuccessful;
        }
        else {
            return false;
        }
    }

    StubFunctor_ stubFunctor_;
    const char* dbusReplySignature_;
    const bool isImplemented_;
};

/**
 * @brief FDBusGetAttributeStubDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam AttributeType_ 
 * @tparam AttributeDepl_ 
 */
template <typename StubClass_, typename AttributeType_, typename AttributeDepl_ = EmptyDeployment>
class FDBusGetAttributeStubDispatcher: public virtual StubDispatcher<StubClass_> {
 public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef void (StubClass_::*LockStubFunctor)(bool);
    typedef const AttributeType_& (StubClass_::*GetStubFunctor)(std::shared_ptr<CommonAPI::ClientId>);
    typedef typename StubClass_::StubAdapterType StubAdapterType;
    typedef typename CommonAPI::Stub<StubAdapterType, typename StubClass_::RemoteEventType> StubType;

    FDBusGetAttributeStubDispatcher(LockStubFunctor _lockStubFunctor, GetStubFunctor _getStubFunctor, const char *_signature,
            const bool _isImplemented,
            AttributeDepl_ *_depl = nullptr)
        : lockStubFunctor_(_lockStubFunctor), 
          getStubFunctor_(_getStubFunctor),
          signature_(_signature),
          isImplemented_(_isImplemented),
          depl_(_depl) {
    }

    virtual ~FDBusGetAttributeStubDispatcher() {};

    bool dispatchFDBusMessage(const FDBusMessage& dbusMessage, const std::shared_ptr<StubClass_>& stub,
            RemoteEventHandlerType* _remoteEventHandler, std::weak_ptr<FDBusProxyConnection> _connection) {
        (void) _remoteEventHandler;

        if (!this->isImplemented_)
            return false;

        return sendAttributeValueReply(dbusMessage, stub, _connection);
    }

    void appendGetAllReply(const FDBusMessage& dbusMessage, const std::shared_ptr<StubClass_>& stub, FDBusOutputStream &_output) {

        std::shared_ptr<FDBusClientId> clientId = std::make_shared<FDBusClientId>(std::string(dbusMessage.getSender()));
        auto varDepl = CommonAPI::FDBus::VariantDeployment<AttributeDepl_>(true, depl_); // presuming FreeDesktop variant deployment, as support for "legacy" service only

        (stub.get()->*lockStubFunctor_)(true);
        auto deployable = CommonAPI::Deployable<CommonAPI::Variant<AttributeType_>, CommonAPI::FDBus::VariantDeployment<AttributeDepl_>>((stub.get()->*getStubFunctor_)(clientId), &varDepl);
        (stub.get()->*lockStubFunctor_)(false);

        _output << deployable;
        _output.flush();
    }

 protected:
    virtual bool sendAttributeValueReply(const FDBusMessage& dbusMessage, const std::shared_ptr<StubClass_>& stub, std::weak_ptr<FDBusProxyConnection> connection_) {
        FDBusMessage dbusMessageReply = dbusMessage.createMethodReturn(signature_);
        FDBusOutputStream dbusOutputStream(dbusMessageReply);

        std::shared_ptr<FDBusClientId> clientId = std::make_shared<FDBusClientId>(std::string(dbusMessage.getSender()));

        (stub.get()->*lockStubFunctor_)(true);
        auto deployable = CommonAPI::Deployable<AttributeType_, AttributeDepl_>((stub.get()->*getStubFunctor_)(clientId), depl_);
        (stub.get()->*lockStubFunctor_)(false);

        dbusOutputStream << deployable;
        dbusOutputStream.flush();
        if (std::shared_ptr<FDBusProxyConnection> connection = connection_.lock()) {
            bool isSuccessful = connection->sendFDBusMessage(dbusMessageReply);
            return isSuccessful;
        }
        else {
            return false;
        }
    }

    LockStubFunctor lockStubFunctor_;
    GetStubFunctor getStubFunctor_;
    const char* signature_;
    const bool isImplemented_;
    AttributeDepl_ *depl_;
};

/**
 * @brief FDBusSetAttributeStubDispatcher
 * 
 * @tparam StubClass_ 
 * @tparam AttributeType_ 
 * @tparam AttributeDepl_ 
 */
template <typename StubClass_, typename AttributeType_, typename AttributeDepl_ = EmptyDeployment>
class FDBusSetAttributeStubDispatcher: public virtual FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_> {
 public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;

    typedef typename FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::LockStubFunctor LockStubFunctor;
    typedef typename FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::GetStubFunctor GetStubFunctor;
    typedef bool (RemoteEventHandlerType::*OnRemoteSetFunctor)(std::shared_ptr<CommonAPI::ClientId>, AttributeType_);
    typedef void (RemoteEventHandlerType::*OnRemoteChangedFunctor)();

    FDBusSetAttributeStubDispatcher(LockStubFunctor _lockStubFunctor, GetStubFunctor _getStubFunctor,
            OnRemoteSetFunctor onRemoteSetFunctor, OnRemoteChangedFunctor onRemoteChangedFunctor,
            const char *_signature,
            const bool _isImplemented,
            AttributeDepl_ *_depl = nullptr)
        : FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>(_lockStubFunctor, _getStubFunctor, _signature, _isImplemented, _depl),
          onRemoteSetFunctor_(onRemoteSetFunctor),
          onRemoteChangedFunctor_(onRemoteChangedFunctor) {
    }

    virtual ~FDBusSetAttributeStubDispatcher() {};

    bool dispatchFDBusMessage(const FDBusMessage &_message, const std::shared_ptr<StubClass_> &_stub,
            RemoteEventHandlerType *_remoteEventHandler, std::weak_ptr<FDBusProxyConnection> _connection) {
        bool attributeValueChanged;

        if (!this->isImplemented_)
            return false;

        if (!setAttributeValue(_message, _stub, _remoteEventHandler, _connection, attributeValueChanged))
            return false;

        if (attributeValueChanged)
            notifyOnRemoteChanged(_remoteEventHandler);

        return true;
    }

 protected:
    virtual AttributeType_ retrieveAttributeValue(const FDBusMessage &_message, bool &_errorOccured) {
        _errorOccured = false;

        FDBusInputStream itsInput(_message);
        CommonAPI::Deployable<AttributeType_, AttributeDepl_> itsValue(this->depl_);
        itsInput >> itsValue;

        if (itsInput.hasError()) {
            _errorOccured = true;
        }

        return itsValue.getValue();
    }

    inline bool setAttributeValue(const FDBusMessage &_message,
            const std::shared_ptr<StubClass_> &_stub,
            RemoteEventHandlerType *_remoteEventHandler,
            std::weak_ptr<FDBusProxyConnection> _connection,
            bool &_hasChanged) {
        bool errorOccured;
        CommonAPI::Deployable<AttributeType_, AttributeDepl_> itsValue(
             retrieveAttributeValue(_message, errorOccured), this->depl_);

        if (errorOccured) {
            return false;
        }

        std::shared_ptr<FDBusClientId> clientId = std::make_shared<FDBusClientId>(std::string(_message.getSender()));

        _hasChanged = (_remoteEventHandler->*onRemoteSetFunctor_)(clientId, std::move(itsValue.getValue()));

        return this->sendAttributeValueReply(_message, _stub, _connection);
    }

    inline void notifyOnRemoteChanged(RemoteEventHandlerType *_remoteEventHandler) {
        (_remoteEventHandler->*onRemoteChangedFunctor_)();
    }

    inline AttributeType_ getAttributeValue(std::shared_ptr<CommonAPI::ClientId> _client,
            const std::shared_ptr<StubClass_> &_stub) {
        return (_stub.get()->*(this->getStubFunctor_))(_client);
    }

    const OnRemoteSetFunctor onRemoteSetFunctor_;
    const OnRemoteChangedFunctor onRemoteChangedFunctor_;
};

/**
 * @brief Set oberservable attribute stub dispatcher
 * 
 * @tparam StubClass_ 
 * @tparam AttributeType_ 
 * @tparam AttributeDepl_ 
 */
template <typename StubClass_, typename AttributeType_, typename AttributeDepl_ = EmptyDeployment>
class FDBusSetObservableAttributeStubDispatcher: public virtual FDBusSetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_> {
public:
    typedef typename StubClass_::RemoteEventHandlerType RemoteEventHandlerType;
    typedef typename StubClass_::StubAdapterType StubAdapterType;
    typedef typename FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::LockStubFunctor LockStubFunctor;
    typedef typename FDBusSetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::GetStubFunctor GetStubFunctor;
    typedef typename FDBusSetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::OnRemoteSetFunctor OnRemoteSetFunctor;
    typedef typename FDBusSetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::OnRemoteChangedFunctor OnRemoteChangedFunctor;
    typedef typename CommonAPI::Stub<StubAdapterType, typename StubClass_::RemoteEventType> StubType;
    typedef void (StubAdapterType::*FireChangedFunctor)(const AttributeType_&);

    FDBusSetObservableAttributeStubDispatcher(LockStubFunctor _lockStubFunctor, GetStubFunctor _getStubFunctor,
            OnRemoteSetFunctor onRemoteSetFunctor, OnRemoteChangedFunctor onRemoteChangedFunctor,
            FireChangedFunctor fireChangedFunctor,
            const char* dbusSignature,
            const bool _isImplemented,
            AttributeDepl_ *_depl = nullptr)
        : FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>(
                _lockStubFunctor, _getStubFunctor, dbusSignature, _isImplemented, _depl),
          FDBusSetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>(
                _lockStubFunctor, _getStubFunctor, onRemoteSetFunctor, onRemoteChangedFunctor, dbusSignature, _depl),
          fireChangedFunctor_(fireChangedFunctor) {
    }

    virtual ~FDBusSetObservableAttributeStubDispatcher() {};

    bool dispatchFDBusMessage(const FDBusMessage &_message, const std::shared_ptr<StubClass_> &_stub,
        RemoteEventHandlerType *_remoteEventHandler, std::weak_ptr<FDBusProxyConnection> _connection) {

        if (!this->isImplemented_)
            return false;

        bool hasChanged;
        if (!this->setAttributeValue(_message, _stub, _remoteEventHandler, _connection, hasChanged))
            return false;

        if (hasChanged) {
            std::shared_ptr<FDBusClientId> itsClient = std::make_shared<FDBusClientId>(std::string(_message.getSender()));
            fireAttributeValueChanged(itsClient, _remoteEventHandler, _stub);
            this->notifyOnRemoteChanged(_remoteEventHandler);
        }
        return true;
    }

protected:
    virtual void fireAttributeValueChanged(std::shared_ptr<CommonAPI::ClientId> _client,
            RemoteEventHandlerType *_remoteEventHandler, const std::shared_ptr<StubClass_> _stub) {
        (void)_remoteEventHandler;

        auto stubAdapter = _stub->StubType::getStubAdapter();
        (_stub.get()->*FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::lockStubFunctor_)(true);
        (stubAdapter.get()->*fireChangedFunctor_)(this->getAttributeValue(_client, _stub));
        (_stub.get()->*FDBusGetAttributeStubDispatcher<StubClass_, AttributeType_, AttributeDepl_>::lockStubFunctor_)(false);
    }

    const FireChangedFunctor fireChangedFunctor_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSSTUBADAPTERHELPER_HPP_
