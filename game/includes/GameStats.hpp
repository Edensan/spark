#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>
#include <Impls.hpp>

#include <niLang/STL/hash_map.h>
#include <niLang/STL/vector.h>
#include <ostream>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

// Considering a battle starting at 6 max action points,
// if the player is expected to spend 2 action points
// to move towards the ennemy and still having a budget
// of 6 points, being able to cast a couple basic actions
// and another more powerful action seems acceptable.
//
// Then every new turn would award one action point for
// movement and 2 for attacks, or just 3 for attacks
// in a best case scenario.
static constexpr u8 kBaseMaxAp = 8;
static constexpr u8 kBaseApRecovery = 3;
static constexpr u8 kUnitApCost = 1; // Unit of basic movement / action

class Value final {
public:
    Value() {}
    Value(i32 v) : baseValue(v) {}
    i32 operator=(i32 v) { baseValue = v;  return baseValue; }
    i32 operator+(i32 v) { baseValue += v; return baseValue; }
    i32 operator-(i32 v) { baseValue -= v; return baseValue; }
    void reset() {
        multiplier = 1.0f;
        additive = 0;
    }
    inline i32 computedValue() const {
        return (baseValue * multiplier) + additive;
    }
    f32 multiplier = 1.0f;
    i32 additive = 0;
    i32 baseValue = 0;
};

class Stats {
public:
    enum class Type : u8 {
        BeginAttributes = 0,

        // Attributes
        Strength = BeginAttributes + 0,
        Agility = BeginAttributes + 1,
        Intelligence = BeginAttributes + 2,

        EndAttributes = Intelligence,
        AttributesCount = EndAttributes - BeginAttributes + 1,
        BeginStats = EndAttributes + 1,

        // Combat stats
        MaxHitPoints = BeginStats + 0,
        MaxActionPoints = BeginStats + 1,
        ActionPointsRecovery = BeginStats + 2,
        AttackPower = BeginStats + 3,
        SpellPower = BeginStats + 4,

        EndStats = SpellPower,
        Last = EndStats,
        StatsCount = EndStats - BeginStats + 1,

        Count = AttributesCount + StatsCount,
        Invalid = astl::numeric_limits<u8>::max(),
    };
    static constexpr bool isAttribute(const Type t) {
        return static_cast<u8>(t) >= static_cast<u8>(Type::BeginAttributes)
            && static_cast<u8>(t) < static_cast<u8>(Type::EndAttributes);
    }
    static constexpr bool isStat(const Type t) {
        return static_cast<u8>(t) >= static_cast<u8>(Type::BeginStats)
            && static_cast<u8>(t) < static_cast<u8>(Type::EndStats);
    }

private:
    void computeMaxHitPoints();
    void computeAttackPower();
    void computeSpellPower();
    static f32 attributeCountF() { return static_cast<f32>(Type::AttributesCount); }
    static f32 maxValue() { return 40.0f; }
    static f32 maxValueF() { return maxValue(); }

public:

    Stats()
        : stats({ 0 }) {
    }
    Stats(i32 str, i32 agi, i32 intel, i32 baseMaxAp = kBaseMaxAp, i32 baseApRecovery = kBaseApRecovery)
        : stats({ 0 }) {
        stats[static_cast<u8>(Type::Strength)] = str;
        stats[static_cast<u8>(Type::Agility)] = agi;
        stats[static_cast<u8>(Type::Intelligence)] = intel;
        stats[static_cast<u8>(Type::MaxActionPoints)] = baseMaxAp;
        stats[static_cast<u8>(Type::ActionPointsRecovery)] = baseApRecovery;
        markDirty();
    }
    Stats(const Stats &other)
        : stats({ 0 }) {
        *this = other;
    }
    skPodImplOpsEqEx(Stats, sizeof(stats))
    Stats &operator=(const Stats &other) {
        // We copy the attributes only,
        // then compute the stats on the next update.
        memcpy(&stats, &other.stats, sizeof(Value) * static_cast<i32>(Type::AttributesCount));
        stats[static_cast<u8>(Type::MaxActionPoints)] = other.stats[static_cast<u8>(Type::MaxActionPoints)];
        stats[static_cast<u8>(Type::ActionPointsRecovery)] = other.stats[static_cast<u8>(Type::ActionPointsRecovery)];
        markDirty();
        return *this;
    }
    i32 operator[](Type t) const {
        assert(static_cast<u8>(t) < static_cast<u8>(Type::Last));
        return stats[static_cast<u8>(t)].baseValue;
    }

    void set(Type t, i32 v) {
        stats[static_cast<u8>(t)].baseValue = v;
        if (isAttribute(t)) {
            markDirty();
        }
    }
    inline i32 get(Type t) const {
        return stats[static_cast<u8>(t)].baseValue;
    }
    inline i32 computed(Type t) const {
        return stats[static_cast<u8>(t)].computedValue();
    }

    inline void applyStatMultiplier(Type t, f32 m) {
        stats[static_cast<u8>(t)].multiplier += m;
        if (isAttribute(t)) {
            markDirty();
        }
    }
    inline void expireStatMultiplier(Type t, f32 m) {
        stats[static_cast<u8>(t)].multiplier -= m;
        if (isAttribute(t)) {
            markDirty();
        }
    }
    inline void applyStatAdditive(Type t, i32 a) {
        stats[static_cast<u8>(t)].additive += a;
        if (isAttribute(t)) {
            markDirty();
        }
    }
    inline void expireStatAdditive(Type t, i32 a) {
        stats[static_cast<u8>(t)].additive -= a;
        if (isAttribute(t)) {
            markDirty();
        }
    }
    void resetAll();

    bool computeStats();
    inline void markDirty() { dirty_ = true; }

    astl::string toString() const {
        astl::string ret;
        ret += "{ ";
        for (u32 i = 0;;) {
            switch (static_cast<Type>(i)) {
            case Type::Strength: ret += "Str:"; break;
            case Type::Agility: ret += "Agi:"; break;
            case Type::Intelligence: ret += "Int:"; break;
            case Type::MaxHitPoints: ret += "MaxHp:"; break;
            case Type::MaxActionPoints: ret += "MaxAp:"; break;
            case Type::ActionPointsRecovery: ret += "ApRecovery:"; break;
            case Type::AttackPower: ret += "AtkP:"; break;
            case Type::SpellPower: ret += "SpellP:"; break;
            default: ret += "Unknown:"; break;
            }
            ret += astl::to_string(get(static_cast<Stats::Type>(i)));
            ++i;
            if (i < static_cast<u32>(Stats::Type::Count)) {
                ret += ", ";
            }
            else {
                break;
            }
        }
        ret += " }";
        return ret;
    }

    // For GoogleTest
    friend void PrintTo(const Stats& stats, std::ostream* os) {
        *os << stats.toString().c_str();
    }

private:
    inline bool dirty() const { return dirty_; }
    inline bool consumeDirty() {
        const bool ret = dirty_;
        dirty_ = false;
        return ret;
    }
    // This data structure can be used to compare
    // Attributes instances for equality.
    astl::array<Value, static_cast<u8>(Type::Count)> stats;

    bool dirty_ = false;
};

}; }; // namespace spark::game
