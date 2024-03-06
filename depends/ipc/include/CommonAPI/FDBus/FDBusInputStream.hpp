/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSINPUTSTREAM_HPP_
#define COMMONAPI_FDBUS_DBUSINPUTSTREAM_HPP_

#include <iostream>
#include <iomanip>
#include <sstream>

#include <cstdint>
#include <stack>
#include <string>
#include <vector>
#include <cstring>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/InputStream.hpp>
#include <CommonAPI/Struct.hpp>
#include <CommonAPI/FDBus/FDBusDeployment.hpp>
#include <CommonAPI/FDBus/FDBusError.hpp>
#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusTypeOutputStream.hpp>

namespace CommonAPI {
namespace FDBus {

// Used to mark the position of a pointer within an array of bytes.
typedef uint32_t position_t;

template<class Visitor_, class Variant_, class Deployment_, typename ... Types_>
struct ApplyTypeCompareVisitor;

template<class Visitor_, class Variant_, class Deployment_>
struct ApplyTypeCompareVisitor<Visitor_, Variant_, Deployment_> {
    static const uint8_t index = 0;

    static bool visit(Visitor_&, const Variant_&, const Deployment_ *_depl, uint8_t &typeindex) {
        (void)_depl;
        // will be called only if the variant does not contain the requested type
        typeindex = index;
        return false;
    }
};

template<class Visitor_, class Variant_, class Deployment_, typename Type_, typename ... Types_>
struct ApplyTypeCompareVisitor<Visitor_, Variant_, Deployment_, Type_, Types_...> {
    static const uint8_t index
        = ApplyTypeCompareVisitor<Visitor_, Variant_, Deployment_, Types_...>::index + 1;

    static bool visit(Visitor_ &_visitor, const Variant_ &_variant, const Deployment_ *_depl, uint8_t &typeindex) {
        FDBusTypeOutputStream output;
        Type_ current;

        if ((0 < Deployment_::size_) && (_depl)) {
            output.writeType(current, std::get<Deployment_::size_-index>(_depl->values_));
           }
           else {
            output.writeType(current, static_cast<CommonAPI::EmptyDeployment*>(nullptr));
        }

#ifdef _WIN32
        if (_visitor.operator()<Type_>(output.getSignature())) {
#else
        if (_visitor.template operator()<Type_>(output.getSignature())) {
#endif
            typeindex = index;
            return true;
        } else {
            return ApplyTypeCompareVisitor<
                        Visitor_, Variant_, Deployment_, Types_...
                   >::visit(_visitor, _variant, _depl, typeindex);
        }
    }
};
template<typename ... Types_>
struct TypeCompareVisitor {
public:
    TypeCompareVisitor(const std::string &_type)
        : type_(_type) {
    }

    template<typename Type_>
    bool operator()(const std::string &_type) const {
        return (_type == type_);
    }

private:
    const std::string type_;
};

/**
 * @class DBusInputMessageStream
 *
 * Used to deserialize and read data from a #FDBusMessage. For all data types that can be read from a #FDBusMessage, a ">>"-operator should be defined to handle the reading
 * (this operator is predefined for all basic data types and for vectors).
 */
class FDBusInputStream
    : public InputStream<FDBusInputStream> {
public:
    COMMONAPI_EXPORT bool hasError() const {
        return isErrorSet();
    }

    COMMONAPI_EXPORT InputStream &readValue(bool &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT InputStream &readValue(int8_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(int16_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(int32_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(int64_t &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT InputStream &readValue(uint8_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(uint16_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(uint32_t &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(uint64_t &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT InputStream &readValue(float &_value, const EmptyDeployment *_depl);
    COMMONAPI_EXPORT InputStream &readValue(double &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT InputStream &readValue(std::string &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT InputStream &readValue(std::string &_value, const CommonAPI::FDBus::StringDeployment* _depl) {
        (void)_depl;
        return readValue(_value, static_cast<EmptyDeployment *>(nullptr));
    }
    COMMONAPI_EXPORT InputStream &readValue(int32_t &_value, const CommonAPI::FDBus::IntegerDeployment* _depl) {
        (void)_depl;
        return readValue(_value, static_cast<EmptyDeployment *>(nullptr));
    }
    COMMONAPI_EXPORT InputStream &readValue(uint32_t &_value, const CommonAPI::FDBus::IntegerDeployment* _depl) {
        (void)_depl;
        return readValue(_value, static_cast<EmptyDeployment *>(nullptr));
    }
    COMMONAPI_EXPORT InputStream &readValue(Version &_value, const EmptyDeployment *_depl);

    COMMONAPI_EXPORT void beginReadMapOfSerializableStructs() {
        uint32_t itsSize(0);
        _readValue(itsSize);
        pushSize(itsSize);
        align(8); /* correct alignment for first DICT_ENTRY */
        pushPosition();
    }

    COMMONAPI_EXPORT bool readMapCompleted() {
        return (sizes_.back() <= (current_ - positions_.back()));
    }

    COMMONAPI_EXPORT void endReadMapOfSerializableStructs() {
        (void)popSize();
        (void)popPosition();
    }

    COMMONAPI_EXPORT InputStream &skipMap() {
        uint32_t itsSize(0);
        _readValue(itsSize);
        align(8); /* skip padding (if any) */
        if (itsSize > (sizes_.back() + positions_.back() - current_)) {
            COMMONAPI_ERROR(std::string(__FUNCTION__) + ": size ", itsSize, " exceeds remaining ", (sizes_.back() + positions_.back() - current_));
        }
        _readRaw(itsSize);
        return (*this);
    }

    template<int minimum, int maximum>
    COMMONAPI_EXPORT InputStream &readValue(RangedInteger<minimum, maximum> &_value, const EmptyDeployment *) {
        int tmpValue;
        readValue(tmpValue, static_cast<EmptyDeployment *>(nullptr));
        if(!_value.validate()) {
            setError();
        }
        if (!hasError()) {
            _value = tmpValue;
        }
        return (*this);
    }

    template<class Deployment_, typename Base_>
    COMMONAPI_EXPORT InputStream &readValue(Enumeration<Base_> &_value, const Deployment_ *_depl) {
        Base_ tmpValue;
        readValue(tmpValue, _depl);
        _value = tmpValue;

        if(!_value.validate()) {
            setError();
        }
        return (*this);
    }

    template<class Deployment_, typename... Types_>
    COMMONAPI_EXPORT InputStream &readValue(Struct<Types_...> &_value, const Deployment_ *_depl) {
        align(8);
        const auto itsSize(std::tuple_size<std::tuple<Types_...>>::value);
        StructReader<itsSize-1, FDBusInputStream, Struct<Types_...>, Deployment_>{}(
            (*this), _value, _depl);
        return (*this);
    }

    template<class Deployment_, class PolymorphicStruct_>
    COMMONAPI_EXPORT InputStream &readValue(std::shared_ptr<PolymorphicStruct_> &_value,
                           const Deployment_ *_depl) {
        uint32_t serial(0);
        align(8);
        _readValue(serial);
        skipSignature();
        align(8);
        if (!hasError()) {
            _value = PolymorphicStruct_::create(serial);
            _value->template readValue<>(*this, _depl);
        }

        return (*this);
    }

    template<typename... Types_>
    COMMONAPI_EXPORT InputStream &readValue(Variant<Types_...> &_value, const CommonAPI::EmptyDeployment *_depl = nullptr) {
        (void)_depl;
        if(_value.hasValue()) {
#if _MSC_VER < 1900
            const auto maxSize = Variant<Types_...>::maxSize;
#else
            constexpr auto maxSize = Variant<Types_...>::maxSize;
#endif
            DeleteVisitor<maxSize> visitor(_value.valueStorage_);
            ApplyVoidVisitor<DeleteVisitor<maxSize>,
                Variant<Types_...>, Types_... >::visit(visitor, _value);
        }

        align(8);
        readValue(_value.valueType_, static_cast<EmptyDeployment *>(nullptr));
        skipSignature();

        InputStreamReadVisitor<FDBusInputStream, Types_...> visitor((*this), _value);
        ApplyVoidVisitor<InputStreamReadVisitor<FDBusInputStream, Types_... >,
            Variant<Types_...>, Types_...>::visit(visitor, _value);

        return (*this);
    }

    template<typename Deployment_, typename... Types_>
    COMMONAPI_EXPORT InputStream &readValue(Variant<Types_...> &_value, const Deployment_ *_depl) {
        if(_value.hasValue()) {
#if _MSC_VER < 1900
            const auto maxSize = Variant<Types_...>::maxSize;
#else
            constexpr auto maxSize = Variant<Types_...>::maxSize;
#endif
            DeleteVisitor<maxSize> visitor(_value.valueStorage_);
            ApplyVoidVisitor<DeleteVisitor<maxSize>,
                Variant<Types_...>, Types_... >::visit(visitor, _value);
        }

        if (_depl != nullptr && _depl->isDBus_) {
            // Read signature
            uint8_t signatureLength(0);
            readValue(signatureLength, static_cast<EmptyDeployment *>(nullptr));
            char * raw = _readRaw(signatureLength+1);
            if (hasError()) {
                return (*this);
            } else  {
                std::string signature(raw, signatureLength);

                // Determine index (value type) from signature
                TypeCompareVisitor<Types_...> visitor(signature);
                bool success = ApplyTypeCompareVisitor<
                                        TypeCompareVisitor<Types_...>,
                                        Variant<Types_...>,
                                        Deployment_,
                                        Types_...
                                    >::visit(visitor, _value, _depl, _value.valueType_);
                /*
                 * It is possible that this ApplyTypeCompareVisitor fails on purpose,
                 * if the data type of the variant does not match the data type in the stream.
                 * This can happen for instance with the Freedesktop messages that return
                 * data in variants, but we don't have any idea _which variant_ until we've
                 * tried to stream the value in the ApplyTypeComputerVisitor.
                 */
                if (!success) {
                    _value.valueType_ = 0; // Invalid index signifying 'no value'
                    setError();
                    return (*this);
                }
            }
        } else {
            align(8);
            readValue(_value.valueType_, static_cast<EmptyDeployment *>(nullptr));
            skipSignature();
        }


        InputStreamReadVisitor<FDBusInputStream, Types_...> visitor((*this), _value);
        ApplyStreamVisitor<InputStreamReadVisitor<FDBusInputStream, Types_... >,
            Variant<Types_...>, Deployment_, Types_...>::visit(visitor, _value, _depl);

        return (*this);
    }

    template<typename ElementType_>
    COMMONAPI_EXPORT InputStream &readValue(std::vector<ElementType_> &_value, const EmptyDeployment *_depl) {
        (void)_depl;

        uint32_t itsSize(0);
        _readValue(itsSize);
        pushSize(itsSize);

        alignVector<ElementType_>();

        pushPosition();

        _value.clear();
        while (sizes_.back() > current_ - positions_.back()) {
            ElementType_ itsElement;
            readValue(itsElement, static_cast<EmptyDeployment *>(nullptr));

            if (hasError()) {
                break;
            }

            _value.push_back(std::move(itsElement));
        }

        popSize();
        popPosition();

        return (*this);
    }

    COMMONAPI_EXPORT InputStream &readValue(std::vector<uint8_t> &_value, const EmptyDeployment *_depl) {
        (void)_depl;

        uint32_t itsSize(0);
        _readValue(itsSize);

        alignVector<uint8_t>();

        uint8_t *data = reinterpret_cast<uint8_t *>(_readRaw(itsSize));
        if (!hasError()) {
            _value.resize(itsSize);
            std::memcpy(_value.data(), data, itsSize);
        }

        return (*this);
    }


    template<class Deployment_, typename ElementType_>
    COMMONAPI_EXPORT InputStream &readValue(std::vector<ElementType_> &_value, const Deployment_ *_depl) {
        uint32_t itsSize(0);
        _readValue(itsSize);
        pushSize(itsSize);

        alignVector<ElementType_>();

        pushPosition();

        _value.clear();
        while (sizes_.back() > current_ - positions_.back()) {
            ElementType_ itsElement;
            readValue(itsElement, (_depl ? _depl->elementDepl_ : nullptr));

            if (hasError()) {
                break;
            }

            _value.push_back(std::move(itsElement));
        }

        popSize();
        popPosition();

        return (*this);
    }

    template<typename KeyType_, typename ValueType_, typename HasherType_>
    COMMONAPI_EXPORT InputStream &readValue(std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                           const EmptyDeployment *_depl) {

        typedef typename std::unordered_map<KeyType_, ValueType_, HasherType_>::value_type MapElement;

        uint32_t itsSize(0);
        _readValue(itsSize);
        pushSize(itsSize);

        align(8);
        pushPosition();

        _value.clear();
        while (sizes_.back() > current_ - positions_.back()) {
            KeyType_ itsKey;
            ValueType_ itsValue;

            align(8);
            readValue(itsKey, _depl);
            readValue(itsValue, _depl);

            if (hasError()) {
                break;
            }

            _value.insert(MapElement(std::move(itsKey), std::move(itsValue)));
        }

        (void)popSize();
        (void)popPosition();

        return (*this);
    }

    template<class Deployment_, typename KeyType_, typename ValueType_, typename HasherType_>
    COMMONAPI_EXPORT InputStream &readValue(std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                           const Deployment_ *_depl) {

        typedef typename std::unordered_map<KeyType_, ValueType_, HasherType_>::value_type MapElement;

        uint32_t itsSize(0);
        _readValue(itsSize);
        pushSize(itsSize);

        align(8);
        pushPosition();

        _value.clear();
        while (sizes_.back() > current_ - positions_.back()) {
            KeyType_ itsKey;
            ValueType_ itsValue;

            align(8);
            readValue(itsKey, (_depl ? _depl->key_ : nullptr));
            readValue(itsValue, (_depl ? _depl->value_ : nullptr));

            if (hasError()) {
                break;
            }

            _value.insert(MapElement(std::move(itsKey), std::move(itsValue)));
        }

        (void)popSize();
        (void)popPosition();

        return (*this);
    }

    /**
     * Creates a #DBusInputMessageStream which can be used to deserialize and read data from the given #FDBusMessage.
     * As no message-signature is checked, the user is responsible to ensure that the correct data types are read in the correct order.
     *
     * @param message the #FDBusMessage from which data should be read.
     */
    COMMONAPI_EXPORT FDBusInputStream(const CommonAPI::FDBus::FDBusMessage &_message);
    COMMONAPI_EXPORT FDBusInputStream(char *begin, size_t current, size_t size);
    COMMONAPI_EXPORT FDBusInputStream(const FDBusInputStream &_stream) = delete;

    /**
     * Destructor; does not call the destructor of the referred #FDBusMessage. Make sure to maintain a reference to the
     * #FDBusMessage outside of the stream if you intend to make further use of the message.
     */
    COMMONAPI_EXPORT ~FDBusInputStream();

    // Marks the stream as erroneous.
    COMMONAPI_EXPORT void setError();

    /**
     * @return An instance of #DBusError if this stream is in an erroneous state, NULL otherwise
     */
    COMMONAPI_EXPORT const FDBusError &getError() const;

    /**
     * @return true if this stream is in an erroneous state, false otherwise.
     */
    COMMONAPI_EXPORT bool isErrorSet() const;

    // Marks the state of the stream as cleared from all errors. Further reading is possible afterwards.
    // The stream will have maintained the last valid position from before its state became erroneous.
    COMMONAPI_EXPORT void clearError();

    /**
     * Aligns the stream to the given byte boundary, i.e. the stream skips as many bytes as are necessary to execute the next read
     * starting from the given boundary.
     *
     * @param _boundary the byte boundary to which the stream needs to be aligned.
     */
    COMMONAPI_EXPORT void align(const size_t _boundary);

    /**
     * Reads the given number of bytes and returns them as an array of characters.
     *
     * Actually, for performance reasons this command only returns a pointer to the current position in the stream,
     * and then increases the position of this pointer by the number of bytes indicated by the given parameter.
     * It is the user's responsibility to actually use only the number of bytes he indicated he would use.
     * It is assumed the user knows what kind of value is stored next in the #FDBusMessage the data is streamed from.
     * Using a reinterpret_cast on the returned pointer should then restore the original value.
     *
     * Example use case:
     * @code
     * ...
     * inputMessageStream.alignForBasicType(sizeof(int32_t));
     * char* const dataPtr = inputMessageStream.read(sizeof(int32_t));
     * int32_t val = *(reinterpret_cast<int32_t*>(dataPtr));
     * ...
     * @endcode
     */
    COMMONAPI_EXPORT char *_readRaw(const size_t _size);

    /**
     * Handles all reading of basic types from a given #DBusInputMessageStream.
     * Basic types in this context are: uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, double.
     * Any types not listed here (especially all complex types, e.g. structs, unions etc.) need to provide a
     * specialized implementation of this operator.
     *
     * @tparam Type_ The type of the value that is to be read from the given stream.
     * @param _value The variable in which the retrieved value is to be stored
     * @return The given inputMessageStream to allow for successive reading
     */
    template<typename Type_>
    COMMONAPI_EXPORT FDBusInputStream &_readValue(Type_ &_value) {
        if (sizeof(_value) > 1)
            align(sizeof(Type_));

        char * raw = _readRaw(sizeof(Type_));
        if (!hasError()) {
            _value = *(reinterpret_cast<Type_ *>(raw));
        }

        return (*this);
    }

    COMMONAPI_EXPORT FDBusInputStream &_readValue(float &_value) {
        align(sizeof(double));

        char * raw = _readRaw(sizeof(double));
        if (!hasError()) {
            _value = float(*(reinterpret_cast<double*>(raw)));
        }
        return (*this);
    }

private:
    COMMONAPI_EXPORT void pushPosition();
    COMMONAPI_EXPORT size_t popPosition();

    COMMONAPI_EXPORT void pushSize(size_t _size);
    COMMONAPI_EXPORT size_t popSize();

    inline void skipSignature() {
        uint8_t length(0);
        _readValue(length);
        _readRaw(length + 1);
    }

    template<typename Type_,
        typename std::enable_if<(!std::is_class<Type_>::value &&
                                 !is_std_vector<Type_>::value &&
                                 !is_std_unordered_map<Type_>::value), int>::type = 0>
    COMMONAPI_EXPORT void alignVector() {
        if (4 < sizeof(Type_)) align(8);
    }

    template<typename Type_,
        typename std::enable_if<(!std::is_same<Type_, std::string>::value &&
                                std::is_class<Type_>::value && 
                                !is_std_vector<Type_>::value &&
                                !is_std_unordered_map<Type_>::value &&
                                !std::is_base_of<Enumeration<uint8_t>, Type_>::value &&
                                !std::is_base_of<Enumeration<uint16_t>, Type_>::value &&
                                !std::is_base_of<Enumeration<uint32_t>, Type_>::value &&
                                !std::is_base_of<Enumeration<int8_t>, Type_>::value &&
                                !std::is_base_of<Enumeration<int16_t>, Type_>::value &&
                                !std::is_base_of<Enumeration<int32_t>, Type_>::value), int>::type = 0>
    COMMONAPI_EXPORT void alignVector() {
        align(8);
    }

    template<typename Type_,
        typename std::enable_if<(std::is_same<Type_, std::string>::value ||
                                 is_std_vector<Type_>::value ||
                                 std::is_base_of<Enumeration<uint8_t>, Type_>::value ||
                                 std::is_base_of<Enumeration<int8_t>, Type_>::value), int>::type = 0>
    COMMONAPI_EXPORT void alignVector() {
        // Intentionally do nothing
    }

    template<typename Type_,
        typename std::enable_if<(is_std_unordered_map<Type_>::value ||
                                 std::is_base_of<Enumeration<uint32_t>, Type_>::value ||
                                 std::is_base_of<Enumeration<int32_t>, Type_>::value), int>::type = 0>
    COMMONAPI_EXPORT void alignVector() {
        align(4);
    }

    template<typename Type_,
        typename std::enable_if<(std::is_base_of<Enumeration<uint16_t>, Type_>::value ||
                                 std::is_base_of<Enumeration<int16_t>, Type_>::value), int>::type = 0>
    COMMONAPI_EXPORT void alignVector() {
        align(2);
    }

    char *begin_;
    size_t current_;
    size_t size_;
    CommonAPI::FDBus::FDBusError* exception_;
    CommonAPI::FDBus::FDBusMessage mMessage;

    std::vector<uint32_t> sizes_;
    std::vector<size_t> positions_;

    friend class FDBusMessage;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUS_INPUTSTREAM_HPP_
