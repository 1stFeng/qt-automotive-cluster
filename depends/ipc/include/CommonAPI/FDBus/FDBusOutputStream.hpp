/*
 * China Entry Infotainment Project
 * Copyright (c) 2023 FAW-VW, P-VC & MOSI-Tech.
 * All Rights Reserved.
 */

#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#error "Only <CommonAPI/CommonAPI.hpp> can be included directly, this file may disappear or change contents."
#endif

#ifndef COMMONAPI_FDBUS_DBUSOUTPUTSTREAM_HPP_
#define COMMONAPI_FDBUS_DBUSOUTPUTSTREAM_HPP_

#include <cstring>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <CommonAPI/Export.hpp>
#include <CommonAPI/Logger.hpp>
#include <CommonAPI/OutputStream.hpp>
#include <CommonAPI/FDBus/FDBusDeployment.hpp>
#include <CommonAPI/FDBus/FDBusError.hpp>
#include <CommonAPI/FDBus/FDBusHelper.hpp>
#include <CommonAPI/FDBus/FDBusMessage.hpp>
#include <CommonAPI/FDBus/FDBusTypeOutputStream.hpp>

namespace CommonAPI {
namespace FDBus {

/**
 * @class DBusOutputMessageStream
 *
 * Used to serialize and write data into a #FDBusMessage. For all data types that may be written to a #FDBusMessage, a "<<"-operator should be defined to handle the writing
 * (this operator is predefined for all basic data types and for vectors). The signature that has to be written to the #FDBusMessage separately is assumed
 * to match the actual data that is inserted via the #DBusOutputMessageStream.
 */
class FDBusOutputStream: public OutputStream<FDBusOutputStream> {
public:

    /**
     * Creates a #DBusOutputMessageStream which can be used to serialize and write data into the given #FDBusMessage. Any data written is buffered within the stream.
     * Remember to call flush() when you are done with writing: Only then the data actually is written to the #FDBusMessage.
     *
     * @param dbusMessage The #FDBusMessage any data pushed into this stream should be written to.
     */
    COMMONAPI_EXPORT FDBusOutputStream(FDBusMessage dbusMessage);
    COMMONAPI_EXPORT FDBusOutputStream();

    COMMONAPI_EXPORT void beginWriteVectorOfSerializableStructs() {
        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        align(8);
        pushPosition();    // Start of map data
    }

    COMMONAPI_EXPORT void endWriteVector() {
        // Write number of written bytes to placeholder position
        const uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const bool &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        uint32_t tmp = (_value ? 1 : 0);
        return _writeValue(tmp);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const int8_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const int16_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const int32_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const int32_t &_value, const CommonAPI::FDBus::IntegerDeployment* _depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const int64_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const uint8_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const uint16_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const uint32_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const uint32_t &_value, const CommonAPI::FDBus::IntegerDeployment* _depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const uint64_t &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }
    COMMONAPI_EXPORT OutputStream &writeValue(const float &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(static_cast<double>(_value));
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const double &_value, const EmptyDeployment *_depl) {
        (void)_depl;
        return _writeValue(_value);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const std::string &_value, const EmptyDeployment *_depl = nullptr) {
        (void)_depl;
        return writeString(_value.c_str(), uint32_t(_value.length()));
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const std::string &_value, const CommonAPI::FDBus::StringDeployment* _depl) {
        (void)_depl;
        return writeString(_value.c_str(), uint32_t(_value.length()));
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const Version &_value, const EmptyDeployment *_depl = nullptr) {
        align(8);
        writeValue(_value.Major, _depl);
        writeValue(_value.Minor, _depl);
        return (*this);
    }

    template<int minimum, int maximum>
    COMMONAPI_EXPORT OutputStream &writeValue(const RangedInteger<minimum, maximum> &_value, const EmptyDeployment *) {
        if (_value.validate())
            writeValue(_value.value_, static_cast<EmptyDeployment *>(nullptr));
        else
            setError();
        return (*this);
    }

    template<class Deployment_, typename Base_>
    COMMONAPI_EXPORT OutputStream &writeValue(const Enumeration<Base_> &_value, const Deployment_ *_depl = nullptr) {
        return writeValue(static_cast<Base_>(_value), _depl);
    }

    template<class Deployment_, typename... Types_>
    COMMONAPI_EXPORT OutputStream &writeValue(const Struct<Types_...> &_value, const Deployment_ *_depl = nullptr) {
        align(8);

        const auto itsSize(std::tuple_size<std::tuple<Types_...>>::value);
        StructWriter<itsSize-1, FDBusOutputStream, Struct<Types_...>, Deployment_>{}((*this), _value, _depl);

        return (*this);
    }

    template<class Deployment_, class PolymorphicStruct_>
    COMMONAPI_EXPORT OutputStream &writeValue(const std::shared_ptr<PolymorphicStruct_> &_value, const Deployment_ *_depl = nullptr) {
        align(8);
        _writeValue(_value->getSerial());

        FDBusTypeOutputStream typeOutput;
        typeOutput.writeType(_value, _depl);
        writeSignature(typeOutput.getSignature());

        align(8);
        _value->template writeValue<>((*this), _depl);

        return (*this);
    }

    template<typename... Types_>
    COMMONAPI_EXPORT OutputStream &writeValue(const Variant<Types_...> &_value, const CommonAPI::EmptyDeployment *_depl = nullptr) {
        (void)_depl;

        align(8);
        writeValue(_value.getValueType(), static_cast<EmptyDeployment *>(nullptr));

        FDBusTypeOutputStream typeOutput;
        TypeOutputStreamWriteVisitor<FDBusTypeOutputStream> typeVisitor(typeOutput);
        ApplyVoidVisitor<TypeOutputStreamWriteVisitor<FDBusTypeOutputStream>,
            Variant<Types_...>, Types_...>::visit(typeVisitor, _value);
        writeSignature(typeOutput.getSignature());

        OutputStreamWriteVisitor<FDBusOutputStream> valueVisitor(*this);
        ApplyVoidVisitor<OutputStreamWriteVisitor<FDBusOutputStream>,
             Variant<Types_...>, Types_...>::visit(valueVisitor, _value);

        return (*this);
    }

    template<typename Deployment_, typename... Types_>
    COMMONAPI_EXPORT OutputStream &writeValue(const Variant<Types_...> &_value, const Deployment_ *_depl = nullptr) {
        if (_depl != nullptr && _depl->isDBus_) {
            align(1);
        } else {
            align(8);
            writeValue(_value.getValueType(), static_cast<EmptyDeployment *>(nullptr));
        }

        FDBusTypeOutputStream typeOutput;
        TypeOutputStreamWriteVisitor<FDBusTypeOutputStream> typeVisitor(typeOutput);
        ApplyStreamVisitor<TypeOutputStreamWriteVisitor<FDBusTypeOutputStream>,
            Variant<Types_...>, Deployment_, Types_...>::visit(typeVisitor, _value, _depl);
        writeSignature(typeOutput.getSignature());

        OutputStreamWriteVisitor<FDBusOutputStream> valueVisitor(*this);
        ApplyStreamVisitor<OutputStreamWriteVisitor<FDBusOutputStream>,
             Variant<Types_...>, Deployment_, Types_...>::visit(valueVisitor, _value, _depl);

        return (*this);
    }

    template<typename ElementType_>
    COMMONAPI_EXPORT OutputStream &writeValue(const std::vector<ElementType_> &_value,
                             const EmptyDeployment *_depl) {
        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        alignVector<ElementType_>();
        pushPosition(); // Start of vector data

        for (auto i : _value) {
            writeValue(i, _depl);
            if (hasError()) {
                break;
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);

        return (*this);
    }

    COMMONAPI_EXPORT OutputStream &writeValue(const std::vector<uint8_t> &_value,
                                              const EmptyDeployment *_depl) {
        (void)_depl;
        align(sizeof(uint32_t));
        writeByteBuffer(_value.data(), static_cast<uint32_t>(_value.size()));
        return (*this);
    }

    template<class Deployment_, typename ElementType_>
    COMMONAPI_EXPORT OutputStream &writeValue(const std::vector<ElementType_> &_value,
                             const Deployment_ *_depl) {
        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        alignVector<ElementType_>();
        pushPosition(); // Start of vector data

        for (auto i : _value) {
            writeValue(i, (_depl ? _depl->elementDepl_ : nullptr));
            if (hasError()) {
                break;
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);

        return (*this);
    }

    template<typename KeyType_, typename ValueType_, typename HasherType_>
    COMMONAPI_EXPORT OutputStream &writeValue(const std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                             const EmptyDeployment *_depl) {
        (void)_depl;

        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        align(8);
        pushPosition();    // Start of map data

        for (auto v : _value) {
            align(8);
            writeValue(v.first, static_cast<EmptyDeployment *>(nullptr));
            writeValue(v.second, static_cast<EmptyDeployment *>(nullptr));

            if (hasError()) {
                return (*this);
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);
        return (*this);
    }

    template<class Deployment_, typename KeyType_, typename ValueType_, typename HasherType_>
    COMMONAPI_EXPORT OutputStream &writeValue(const std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                             const Deployment_ *_depl) {
        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        align(8);
        pushPosition();    // Start of map data

        for (auto v : _value) {
            align(8);
            writeValue(v.first, (_depl ? _depl->key_ : nullptr));
            writeValue(v.second, (_depl ? _depl->value_ : nullptr));

            if (hasError()) {
                return (*this);
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);
        return (*this);
    }

    /**
     * Fills the stream with 0-bytes to make the next value be aligned to the boundary given.
     * This means that as many 0-bytes are written to the buffer as are necessary
     * to make the next value start with the given alignment.
     *
     * @param alignBoundary The byte-boundary to which the next value should be aligned.
     */
    COMMONAPI_EXPORT void align(const size_t _boundary);

    /**
     * Writes the data that was buffered within this #DBusOutputMessageStream to the #FDBusMessage that was given to the constructor. Each call to flush()
     * will completely override the data that currently is contained in the #FDBusMessage. The data that is buffered in this #DBusOutputMessageStream is
     * not deleted by calling flush().
     */
    COMMONAPI_EXPORT void flush();

    COMMONAPI_EXPORT bool hasError() const;

    // Helper for serializing Freedesktop properties
    COMMONAPI_EXPORT void beginWriteMap() {
        align(sizeof(uint32_t));
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder

        align(8);
        pushPosition(); // Start of map data
    }

    COMMONAPI_EXPORT void endWriteMap() {
        // Write number of written bytes to placeholder position
        const uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(popPosition(), length);
    }

private:
#ifdef COMMONAPI_FDBUS_TEST
public:
#endif
    COMMONAPI_EXPORT size_t getPosition();
    COMMONAPI_EXPORT void pushPosition();
    COMMONAPI_EXPORT size_t popPosition();

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

    COMMONAPI_EXPORT void setError();

    /**
     * Reserves the given number of bytes for writing, thereby negating the need to dynamically allocate memory while writing.
     * Use this method for optimization: If possible, reserve as many bytes as you need for your data before doing any writing.
     *
     * @param numOfBytes The number of bytes that should be reserved for writing.
     */
    COMMONAPI_EXPORT void reserveMemory(size_t numOfBytes);

    template<typename Type_>
    COMMONAPI_EXPORT FDBusOutputStream &_writeValue(const Type_ &_value) {
        if (sizeof(Type_) > 1)
            align(sizeof(Type_));

        _writeRaw(reinterpret_cast<const char*>(&_value), sizeof(Type_));
        return (*this);
    }

    template<typename Type_>
    COMMONAPI_EXPORT void _writeValueAt(size_t _position, const Type_ &_value) {
        if ((_position + sizeof(Type_)) > payload_.size()) {
            COMMONAPI_ERROR(std::string(__FUNCTION__) + ": payload size ", payload_.size(), " too small ", (_position + sizeof(Type_)));
        }
        _writeRawAt(reinterpret_cast<const char *>(&_value),
                      sizeof(Type_), _position);
    }

    COMMONAPI_EXPORT FDBusOutputStream &writeString(const char *_data, const uint32_t &_length);

    COMMONAPI_EXPORT FDBusOutputStream &writeByteBuffer(const uint8_t *_date, const uint32_t &_length);

    /**
     * Takes sizeInByte characters, starting from the character which val points to, and stores them for later writing.
     * When calling flush(), all values that were written to this stream are copied into the payload of the #FDBusMessage.
     *
     * The array of characters might be created from a pointer to a given value by using a reinterpret_cast. Example:
     * @code
     * ...
     * int32_t val = 15;
     * outputMessageStream.alignForBasicType(sizeof(int32_t));
     * const char* const reinterpreted = reinterpret_cast<const char*>(&val);
     * outputMessageStream.writeValue(reinterpreted, sizeof(int32_t));
     * ...
     * @endcode
     *
     * @param _data The array of chars that should serve as input
     * @param _size The number of bytes that should be written
     * @return true if writing was successful, false otherwise.
     *
     * @see DBusOutputMessageStream()
     * @see flush()
     */
    COMMONAPI_EXPORT void _writeRaw(const char *_data, const size_t _size);
    COMMONAPI_EXPORT void _writeRawAt(const char *_data, const size_t _size, size_t _position);

protected:
    std::string payload_;

private:
#ifdef COMMONAPI_FDBUS_TEST
public:
#endif
    COMMONAPI_EXPORT void writeSignature(const std::string& signature);

    COMMONAPI_EXPORT size_t getCurrentStreamPosition();

    bool errorOccurred_;
    FDBusMessage dbusMessage_;

    std::vector<size_t> positions_;
    
    friend class FDBusMessage;
};

} // namespace FDBus
} // namespace CommonAPI

#endif // COMMONAPI_FDBUS_DBUSOUTPUTSTREAM_PPH_
