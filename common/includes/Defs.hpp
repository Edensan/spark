#pragma once
#include <niLang/Types.h>
#include <niLang/StringDef.h>
#include <stdlib.h>
#include <signal.h>

#ifdef _DEBUG
#define skDebug
#endif
#define skUnused(X) (void)X
#define skExport

#define VA_ARGS(...) , ##__VA_ARGS__
#define skLogD(FMT, ...) niDebugFmt(("(%s:%d) [DEBUG]: " FMT "\n", __FILE__, __LINE__ VA_ARGS(__VA_ARGS__)))
#define skLogI(FMT, ...) niDebugFmt(("(%s:%d) [INFO]: " FMT "\n", __FILE__, __LINE__ VA_ARGS(__VA_ARGS__)))
#define skLogW(FMT, ...) niDebugFmt(("(%s:%d) [WARNING]: " FMT "\n", __FILE__, __LINE__ VA_ARGS(__VA_ARGS__)))
#define skLogE(FMT, ...) niDebugFmt(("(%s:%d) [ERROR]: " FMT "\n", __FILE__, __LINE__ VA_ARGS(__VA_ARGS__)))
#ifdef skDebug
#define skUnreachable(MSG, ...) skLogE(MSG, __VA_ARGS__); niAssertUnreachable("Unreachable!")
#else
#define skUnreachable(MSG, ...) skLogE(MSG, __VA_ARGS__); skLogE("Unreachable!")
#endif


#define skLoop_(I,V,N) for (auto I = static_cast<int32_t>(V), N_ = static_cast<int32_t>(N); I < N_; ++I)
#define skLoop(I,N) skLoop_(I,0,N)
#define skLoopr(RI,N) for (auto RI = static_cast<int32_t>(N)-1; RI >= 0; --RI)
#define skLoopIt(IT,DS) for (auto IT = DS.begin(); IT != DS.end(); ++IT)

namespace spark {
namespace common {

template <typename T>
inline bool skFindErase(astl::vector<T *> &vec, T *obj) {
    auto it = astl::find(vec.begin(), vec.end(), obj);
    if (it != vec.end()) {
        vec.erase(it);
        return true;
    }
    return false;
}

template <typename T>
inline bool skFindErase(astl::vector<T> &vec, T &obj) {
    auto it = astl::find(vec.begin(), vec.end(), obj);
    if (it != vec.end()) {
        vec.erase(it);
        return true;
    }
    return false;
}

// Order does not matter,
// swap listener to the back and pop for efficiency.
template <typename T>
inline bool skFindEraseUnordered(astl::vector<T *> &vec, T *obj) {
    auto it = astl::find(vec.begin(), vec.end(), obj);
    if (it != vec.end()) {
        astl::swap(*it, vec.back());
        vec.pop_back();
        return true;
    }
    return false;
}

// Order does not matter,
// swap listener to the back and pop for efficiency.
template <typename T>
inline bool skFindEraseUnordered(astl::vector<T> &vec, T &obj) {
    auto it = astl::find(vec.begin(), vec.end(), obj);
    if (it != vec.end()) {
        astl::swap(*it, vec.back());
        vec.pop_back();
        return true;
    }
    return false;
}

} // namespace common
} // namespace spark
