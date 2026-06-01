#pragma once
#include "Types.hpp"
#include <vector>
#include <cstring>
#include <stdexcept>


namespace rcf {

class Serializer {
public:
    static std::vector<uint8_t> encode(const void* data, size_t len);
    static std::vector<uint8_t> encodeVec(const std::vector<uint8_t>& v);
    static int32_t              peekLength(const uint8_t* hdr);          // -1 on bad magic
    static std::vector<uint8_t> decode(const uint8_t* buf, size_t len);

    template<typename T>
    static std::vector<uint8_t> encodeStruct(const T& obj) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "encodeStruct requires a trivially copyable type");
        return encode(&obj, sizeof(T));
    }

    template<typename T>
    static T decodeStruct(const std::vector<uint8_t>& payload) {
        static_assert(std::is_trivially_copyable_v<T>,
                      "decodeStruct requires a trivially copyable type");
        if (payload.size() < sizeof(T))
            throw std::runtime_error("Payload too small for decodeStruct");
        T obj;
        std::memcpy(&obj, payload.data(), sizeof(T));
        return obj;
    }
};

} // namespace rcf
