#pragma once

#include <Types.hpp>
#include <assert.h>
#include <niLang/STL/array.h>

namespace spark {
namespace common {
namespace math {

#define skArrayOpsImpl(OP)                                      \
Array<T, SIZE> &operator OP##=(const Array<T, SIZE> &other) {   \
  skLoop (i, SIZE) {                                            \
    data_[i] OP##= other.data_[i];                              \
  }                                                             \
  return *this;                                                 \
}                                                               \
Array<T, SIZE> operator OP(const Array<T, SIZE> &other) const { \
  Array<T, SIZE> ret;                                           \
  skLoop (i, SIZE) {                                            \
    ret.data_[i] = data_[i] OP other.data_[i];                  \
  }                                                             \
  return ret;                                                   \
}                                                               \
Array<T, SIZE> &operator OP##=(T v) {                           \
  skLoop (i, SIZE) {                                            \
    data_[i] OP##= v;                                           \
  }                                                             \
  return *this;                                                 \
}                                                               \
Array<T, SIZE> operator OP(T v) const {                         \
  Array<T, SIZE> ret;                                           \
  skLoop (i, SIZE) {                                            \
    ret.data_[i] = data_[i] OP v;                               \
  }                                                             \
  return ret;                                                   \
}

template <typename T, size_t SIZE>
class Array {
public:
    Array() : data_({ 0 }) {}
    Array(const astl::array<T, SIZE> &a) : data_(a) {}
    Array(const astl::array<T, SIZE> &&a) : data_(astl::move(a)) {}
    virtual ~Array() = default;
    T& operator [](size_t idx) {
        assert(idx < SIZE);
        return data_[idx];
    }
    Array<T, SIZE> &operator =(const astl::array<T, SIZE> &other) {
        data_ = other.data_;
        return *this;
    }
    Array<T, SIZE> &operator =(const astl::array<T, SIZE> &&other) {
        data_ = astl::move(other.data_);
        return *this;
    }
    Array<T, SIZE> &operator =(const Array<T, SIZE> &other) {
        data_ = other.data_;
        return *this;
    }
    skArrayOpsImpl(-)
    skArrayOpsImpl(+)
    skArrayOpsImpl(*)
    skArrayOpsImpl(/)
    const T& operator [](size_t idx) const {
        assert(idx < SIZE);
        return data_[idx];
    }
    inline bool operator ==(const Array<T, SIZE> &other) const {
        return memcmp(&data_, &other.data_, sizeof(T) * SIZE) == 0;
    }
    inline bool operator !=(const Array<T, SIZE> &other) const {
        return !(*this == other);
    }
    static inline Array<T, SIZE> undefined() {
        Array<T, SIZE> tmp;
        memset(&tmp.data_[0], skUndefined<T>(), SIZE);
        return tmp;
    }
    static inline Array<T, SIZE> zero() {
        Array<T, SIZE> tmp;
        memset(&tmp.data_[0], static_cast<T>(0), SIZE);
        return tmp;
    }
    static inline Array<T, SIZE> one() {
        Array<T, SIZE> tmp;
        memset(&tmp.data_[0], static_cast<T>(1), SIZE);
        return tmp;
    }

    template <typename OT>
    inline Array<OT, SIZE> convert() const {
        Array<OT, SIZE> ret;
        skLoop (i, SIZE) {
            ret[i] = static_cast<OT>(data_[i]);
        }
        return ret;
    }

protected:
    astl::array<T, SIZE> data_;
};

#undef skArrayOpsImpl

template <typename T>
skExport class Position : public Array<T, 2> {
public:
    Position() : Array<T, 2>({ 0 }) {}
    Position(const Array<T, 2> &arr) : Array<T, 2>(arr) {}
    Position(const T &a, const T &b) : Array<T, 2>({ a, b }) {}
    const T &x () const { return this->data_[0]; }
    const T &y () const { return this->data_[1]; }
    T &x () { return this->data_[0]; }
    T &y () { return this->data_[1]; }
};

template <typename T>
skExport class Size : public Array<T, 2> {
public:
    Size() : Array<T, 2>({ 0 }) {}
    Size(const Array<T, 2> &arr) : Array<T, 2>(arr) {}
    Size(const T &a, const T &b) : Array<T, 2>({ a, b }) {}
    const T &w () const { return this->data_[0]; }
    const T &h () const { return this->data_[1]; }
    T &w () { return this->data_[0]; }
    T &h () { return this->data_[1]; }
};

typedef Array<f32, 2> Array2F;
typedef Array<u32, 2> Array2U;
typedef Array<i32, 2> Array2I;
typedef Position<f32> PositionF;
typedef Size<f32> SizeF;
typedef Position<u32> PositionU;
typedef Position<i32> PositionI;
typedef Size<u32> SizeU;

inline float skDistanceSq(const PositionF &a, const PositionF &b) {
    const float l = a.x() - b.x();
    const float r = a.y() - b.y();
    return l * l + r * r;
}

inline float skDistance(const PositionF &a, const PositionF &b) {
    return ni::Sqrt(skDistanceSq(a, b));
}

inline u32 skDistance(const PositionI &a, const PositionI &b) {
    const PositionF af = { static_cast<f32>(a.x()), static_cast<f32>(a.y()) };
    const PositionF bf = { static_cast<f32>(b.x()), static_cast<f32>(b.y()) };
    return ni::Sqrt(skDistanceSq(af, bf)) + 0.5f; // Ceil the value
}

template <typename T, size_t SIZE>
static void skNormalize(Array<T, SIZE> &out) {
    f32 tmp = 0;
    skLoop (i, SIZE) {
        const f32 d = out[i];
        tmp += d * d;
    }
    const f32 div = ni::Sqrt(tmp);
    if (div == 0) {
        out = Array<T, SIZE>::zero();
    }
    else {
        const f32 mul = 1.0f / div;
        skLoop (i, SIZE)
            out[i] *= mul;
    }
}

};
};
};
