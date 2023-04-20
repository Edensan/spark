#pragma once

#define skPodImplOpsEqEx(KLASS,SIZE)            \
    bool operator==(const KLASS &other) const { \
        return memcmp(this, &other, SIZE) == 0; \
    }                                           \
    bool operator!=(const KLASS &other) const { \
        return !(*this == other);               \
    }

#define skPodImplOpsEq(KLASS)                             \
    bool operator==(const KLASS &other) const {           \
        return memcmp(this, &other, sizeof(KLASS)) == 0;  \
    }                                                     \
    bool operator!=(const KLASS &other) const {           \
        return !(*this == other);                         \
    }
