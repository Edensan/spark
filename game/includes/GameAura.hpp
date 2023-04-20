#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>

#include "GameStats.hpp"
#include "objects/Character.hpp"

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

class Character;
class Aura {
public:
    enum class Type : u8 {
        BeginAttrs = 0,
        AttrAdditive = BeginAttrs + 0,
        AttrMultiplicative = BeginAttrs + 1,
        EndAttrs = BeginAttrs + 2,

        BeginStats = EndAttrs + 1,
        StatsAdditive = BeginStats + 2,
        StatsMultiplicative = BeginStats + 3,
        EndStats = BeginStats + 4,

        BeginEffects = EndStats + 1,
        EndEffects = BeginEffects + 0,
    };
    static bool typeAttr(const Aura &a) {
        const u8 t_ = static_cast<u8>(a.type());
        return t_ >= static_cast<u8>(Type::BeginAttrs) && t_ < static_cast<u8>(Type::EndAttrs);
    }
    static bool typeStat(const Aura &a) {
        const u8 t_ = static_cast<u8>(a.type());
        return t_ >= static_cast<u8>(Type::BeginStats) && t_ < static_cast<u8>(Type::EndStats);
    }

    Aura(u32 uid, u16 duration)
        : uid_(uid)
        , duration_(duration) {
    }
    virtual Type type() const = 0;
    virtual const char *name() const = 0;
    virtual void applyTo(Character *target);
    virtual void expireFrom(Character *target);
    virtual void logicUpdate(u8 logicCycle);
    u32 uid() const { return uid_; }
    u16 duration() const { return duration_; }

private:
    Character *target_ = nullptr;
    u32 uid_;
    u16 duration_;
};

template <Stats::Type TYPE>
class AdditiveAura : public Aura {
public:
    AdditiveAura(u32 uid, i32 add, u16 duration = astl::numeric_limits<u16>::max())
        : Aura(uid, duration)
        , add_(add) {}

    Type type() const override {
        return Stats::isAttribute(TYPE) ? Type::AttrAdditive : Type::StatsAdditive;
    }

    i32 additive() const { return add_; }
    void applyTo(Character *c) override {
        c->rwStats().applyStatAdditive(TYPE, add_);
    }
    void expireFrom(Character *c) override {
        c->rwStats().expireStatAdditive(TYPE, add_);
    }

private:
    i32 add_;
};

template <Stats::Type TYPE>
class MultiplicativeAura : public Aura {
public:
    MultiplicativeAura(u32 uid, f32 mul, u16 duration = astl::numeric_limits<u16>::max())
        : Aura(uid, duration)
        , multiplier_(mul) {
    }

    Type type() const override {
        return Stats::isAttribute(TYPE) ? Type::AttrAdditive : Type::StatsAdditive;
    }

    f32 multiplier() const { return multiplier_; }
    void applyTo(Character *c) override {
        c->rwStats().applyStatMultiplier(TYPE, multiplier_);
    }
    void expireFrom(Character *c) override {
        c->rwStats().expireStatMultiplier(TYPE, multiplier_);
    }

private:
    f32 multiplier_;
};

} // namespace game
} // namespace spark
