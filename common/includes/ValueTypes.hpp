#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>

namespace spark {
namespace common {

class Value {
public:
    enum class Type : uint8_t {
        // Primitive types
        Primitive = 0,
        U8 = Primitive + 0,
        U16 = Primitive + 1,
        I16 = Primitive + 2,
        U32 = Primitive + 3,
        I32 = Primitive + 4,
        U64 = Primitive + 5,
        I64 = Primitive + 6,
        F32 = Primitive + 7,
        F64 = Primitive + 8,
        Array2F = Primitive + 9, // 2*f32, still fits
        LastPrimitive = Primitive + 24,

        // Pointer types
        Pointer = LastPrimitive + 1,
        String = Pointer + 0,
        LastPointer = Pointer + 8,

        // Allocated Pointer types
        AllocatedPointer = LastPointer + 1,
        AllocatedString = AllocatedPointer + 0,
        LastAllocatedPointer = AllocatedPointer + 8
    };

    Value(u8 u) {
        type_ = Type::U8;
        u8_ = u;
    }
    Value(u16 u) {
        type_ = Type::U16;
        u16_ = u;
    }
    Value(u32 u) {
        type_ = Type::U32;
        u32_ = u;
    }
    Value(u64 u) {
        type_ = Type::U64;
        u64_ = u;
    }
    Value(i16 i) {
        type_ = Type::I16;
        i16_ = i;
    }
    Value(i32 i) {
        type_ = Type::I32;
        i32_ = i;
    }
    Value(i64 i) {
        type_ = Type::I64;
        i64_ = i;
    }
    Value(f32 f) {
        type_ = Type::F32;
        f32_ = f;
    }
    Value(f64 f) {
        type_ = Type::F64;
        f64_ = f;
    }
    Value(const math::Array2F &arr) {
        type_ = Type::Array2F;
        arr2F_ = arr;
    }
    Value(astl::string &s, bool alloc) {
        if (alloc) {
            astl::string *allocedS = new astl::string(s);
            ptr_ = allocedS;
            type_ = Type::AllocatedString;
        }
        else {
            ptr_ = &s;
            type_ = Type::String;
        }
    }
    ~Value() {
        // NOTE: We could just receive and run a deleter
        // but decided against it to save memory.
        if (type_ >= Type::AllocatedPointer) {
            switch (type_) {
            case Type::AllocatedString: {
                delete static_cast<astl::string *>(ptr_);
                break;
            }
            default: {
                skUnreachable("Unsupported Pointer Value type '%d'!", static_cast<uint8_t>(type_));
                break;
            }
            }
        }
    }

    Type type() const { return type_; }

    template <typename T>
    inline T &refAs() {
        return *reinterpret_cast<T *>(&u8_);
    }

    template <typename T>
    inline const T &constRefAs() const {
        return *reinterpret_cast<const T *>(&u8_);
    }

    template <typename T>
    inline T valueAs() const {
        return constRefAs<T>();
    }

private:
    Value() = delete;

    // 8 bytes
    union {
        // Number types;
        u8 u8_;
        u16 u16_;
        u32 u32_;
        u64 u64_;
        i16 i16_;
        i32 i32_;
        i64 i64_;
        f32 f32_;
        f64 f64_;
        math::Array2F arr2F_;
        // Any structure that is > 8 bytes;
        void *ptr_;
    };

    // 1 byte,
    // although alignment will pad this to 4 bytes...
    Type type_;

    friend class PackedValue;
};
// static_assert(sizeof(Value) == 12, "Value should be 12 bytes!");

// NOTE: Packing the value trades performance for memory,
// usually meaningful when doing networking
#if niPragmaPack
#pragma niPackPush(1)
#endif
class PackedValue {
public:
    static PackedValue pack(const Value &v) {
        // TODO: Pack any integer to its smallest space based on actual value.
        return PackedValue(v);
    }
private:
    PackedValue(const Value &v) {
        data_[0] = static_cast<u8>(v.type_);
        memcpy(&data_[1], &v.u8_, 8);
    }
    PackedValue() = delete;
    u8 data_[9];
} niPacked(1);
#if niPragmaPack
#pragma niPackPop()
#endif

static_assert(sizeof(PackedValue) == 9, "PackedValue should be 9 bytes!");

class DataWriter {
    void push(const char *);
    void write(const char *, Value);
    void writeInner(Value);
    bool pop();
    void flush();
};

class DataReader {
    bool push(const char *);
    Value read(const char *);
    Value readInner();
    bool pop();
};

class Serializable {
public:
    virtual void serialize(DataWriter *) = 0;
    virtual void deserialize(DataReader *) = 0;
};

} };
