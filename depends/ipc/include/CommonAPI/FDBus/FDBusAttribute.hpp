/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUS_ATTRIBUTE_HPP_
#define COMMONAPI_FDBUS_DBUS_ATTRIBUTE_HPP_

#include <cstdint>
#include <tuple>

#include <CommonAPI/FDBus/FDBusConfig.hpp>
#include <CommonAPI/FDBus/FDBusEvent.hpp>
#include <CommonAPI/FDBus/FDBusProxyHelper.hpp>

namespace CommonAPI {
namespace FDBus {

/**
 * @brief Readonly attribute
 * 
 * @tparam AttributeType_ 
 * @tparam AttributeDepl_ 
 */
template <typename AttributeType_, typename AttributeDepl_ = EmptyDeployment>
class FDBusReadonlyAttribute: public AttributeType_ {
public:
    typedef typename AttributeType_::ValueType ValueType;
    typedef AttributeDepl_ ValueTypeDepl;
    typedef typename AttributeType_::AttributeAsyncCallback AttributeAsyncCallback;

    FDBusReadonlyAttribute(FDBusProxy &_proxy,
                          const char *setMethodSignature, const char *getMethodName,
                          AttributeDepl_ *_depl = nullptr)
        : proxy_(_proxy),
          getMethodName_(getMethodName),
          setMethodSignature_(setMethodSignature),
          depl_(_depl)    {
        if (NULL == getMethodName) {
            COMMONAPI_ERROR(std::string(__FUNCTION__) + ": getMethodName is NULL");
        }
    }

    void getValue(CommonAPI::CallStatus &_status, ValueType &_value, const CommonAPI::CallInfo *_info) const {
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedValue(depl_);
        FDBusProxyHelper<
            FDBusSerializableArguments<
            >,
            FDBusSerializableArguments<
                CommonAPI::Deployable<
                    ValueType,
                    AttributeDepl_
                >
            >
        >::callMethodWithReply(proxy_, getMethodName_, "", (_info ? _info : &defaultCallInfo), _status, deployedValue);
        _value = deployedValue.getValue();
    }

    std::future<CallStatus> getValueAsync(AttributeAsyncCallback _callback, const CommonAPI::CallInfo *_info) {
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedValue(depl_);
        return FDBusProxyHelper<
                    FDBusSerializableArguments<>,
                    FDBusSerializableArguments<CommonAPI::Deployable<ValueType, AttributeDepl_>>
               >::callMethodAsync(proxy_, getMethodName_, "", (_info ? _info : &defaultCallInfo),
                    [_callback](CommonAPI::CallStatus _status, CommonAPI::Deployable<ValueType, AttributeDepl_> _response) {
                        _callback(_status, _response.getValue());
                    },
                    std::make_tuple(deployedValue));
    }

 protected:
    FDBusProxy &proxy_;
    const char *getMethodName_;
    const char *setMethodSignature_;
    AttributeDepl_ *depl_;
};

/**
 * @brief FDBusAttribute
 * 
 * @tparam AttributeType_ 
 * @tparam AttributeDepl_ 
 */
template <typename AttributeType_, typename AttributeDepl_ = EmptyDeployment>
class FDBusAttribute: public FDBusReadonlyAttribute<AttributeType_, AttributeDepl_> {
public:
    typedef typename AttributeType_::ValueType ValueType;
    typedef typename AttributeType_::AttributeAsyncCallback AttributeAsyncCallback;

    FDBusAttribute(FDBusProxy &_proxy,
                  const char *_setMethodName, const char *_setMethodSignature, const char *_getMethodName,
                  AttributeDepl_ *_depl = nullptr)
        : FDBusReadonlyAttribute<AttributeType_, AttributeDepl_>(_proxy, _setMethodSignature, _getMethodName, _depl),
            setMethodName_(_setMethodName),
            setMethodSignature_(_setMethodSignature) {
        if (NULL == _setMethodName) {
            COMMONAPI_ERROR(std::string(__FUNCTION__) + ": _setMethodName is NULL");
        }
        if (NULL == _setMethodSignature) {
            COMMONAPI_ERROR(std::string(__FUNCTION__) + ": _setMethodSignature is NULL");
        }
    }

    void setValue(const ValueType &_request, CommonAPI::CallStatus &_status, ValueType &_response, const CommonAPI::CallInfo *_info) {
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedRequest(_request, this->depl_);
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedResponse(this->depl_);
        FDBusProxyHelper<FDBusSerializableArguments<CommonAPI::Deployable<ValueType, AttributeDepl_>>,
                        FDBusSerializableArguments<CommonAPI::Deployable<ValueType, AttributeDepl_>> >::callMethodWithReply(
                                this->proxy_,
                                setMethodName_,
                                setMethodSignature_,
                                (_info ? _info : &defaultCallInfo),
                                deployedRequest,
                                _status,
                                deployedResponse);
        _response = deployedResponse.getValue();
    }


    std::future<CallStatus> setValueAsync(const ValueType &_request, AttributeAsyncCallback _callback, const CommonAPI::CallInfo *_info) {
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedRequest(_request, this->depl_);
        CommonAPI::Deployable<ValueType, AttributeDepl_> deployedResponse(this->depl_);
        return FDBusProxyHelper<FDBusSerializableArguments<CommonAPI::Deployable<ValueType, AttributeDepl_>>,
                               FDBusSerializableArguments<CommonAPI::Deployable<ValueType, AttributeDepl_>> >::callMethodAsync(
                                       this->proxy_,
                                       setMethodName_,
                                       setMethodSignature_,
                                       (_info ? _info : &defaultCallInfo),
                                       deployedRequest,
                                       [_callback](CommonAPI::CallStatus _status, CommonAPI::Deployable<ValueType, AttributeDepl_> _response) {
                                            _callback(_status, _response.getValue());
                                       },
                                       std::make_tuple(deployedResponse));
    }

 protected:
    const char* setMethodName_;
    const char* setMethodSignature_;
};

/**
 * @brief Observable Attribute
 * 
 * @tparam AttributeType_ 
 */
template <typename AttributeType_>
class FDBusObservableAttribute: public AttributeType_ {
public:
    typedef typename AttributeType_::ValueType ValueType;
    typedef typename AttributeType_::ValueTypeDepl ValueTypeDepl;
    typedef typename AttributeType_::AttributeAsyncCallback AttributeAsyncCallback;
    typedef typename AttributeType_::ChangedEvent ChangedEvent;

    template <typename... AttributeType_Arguments>
    FDBusObservableAttribute(FDBusProxy &_proxy,
                            const char *_changedEventName,
                            AttributeType_Arguments... arguments)
         : AttributeType_(_proxy, arguments...),
           changedEvent_(_proxy, _changedEventName, this->setMethodSignature_, this->getMethodName_,
                               std::make_tuple(CommonAPI::Deployable<ValueType, ValueTypeDepl>(this->depl_))) {
    }

    ChangedEvent &getChangedEvent() {
        return changedEvent_;
    }

 protected:
    FDBusEvent<ChangedEvent, CommonAPI::Deployable<ValueType, ValueTypeDepl> > changedEvent_;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_ATTRIBUTE_HPP_
