#pragma once

#include <Types.hpp>
#include <MathTypes.hpp>
#include <stdint.h>
#include <niLang/STL/memory.h>
#include <niLang/STL/map.h>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

class Aura;
class GameGrid;
class GameObject;
class Character;

class Skill {
public:
    struct Params {
        Params(u32 t) : type(t) {}
        u32 type;
    };
    struct Effect {
        u32 attackDamage = 0;
        i32 spellDamage = 0; // negative spell damage is healing!
        astl::vector<astl::shared_ptr<Aura>> auras;
    };
    struct ResolutionInfo {
        GameObject *source;
        Effect effect;
        PositionI destination;
    };
    struct Bundle {
        u32 id;
        u32 range;
        u8 cost; // action points cost
        u8 baseCooldown; // base cooldown, in turns
    };
    enum class State : u8 {
        Idle,
        Done,
        Casting,
        Resolving,
    };
    enum class CastError : u8 {
        OK,
        SourceNotLearned,
        SourceNotEquipped,
        SourceNotActive,
        OutOfRange,
        OutOfActionPoints,
        OnCooldown,
        AlreadyCasting
    };

    Skill(Bundle);
    virtual ~Skill() {}

    virtual void logicUpdate(u8);

    // Validate parameters
    // @return Parameters are indeed valid
    virtual bool onValidateParams(const astl::shared_ptr<Params>) { return true; }

    // Begin cast event
    // @return Time to cast the skill, in logic cycles
    virtual u8 onBeginCast(const ResolutionInfo &, const astl::shared_ptr<Params>) = 0;

    // Cast trigger event, might send a projectile or resolve the damage instantly
    // @return Time to resolve the skill (projectile animation time etc), in logic cycles
    virtual u8 onCast() = 0;

    // Resolve damage
    // @note Might have to gather targets at cast destination
    virtual void onResolveCast() = 0;

    // Cast has been delayed
    // @param[in] Delayed cycles
    virtual void onDelayedCast(u8) {}

    // Cast has been interrupted
    virtual void onInterruptedCast() {}

    // Computed effects to apply on target
    virtual f32 attackDamageMultiplier() const { return 0.0f; }
    virtual f32 spellDamageMultiplier() const { return 0.0f; }
    virtual const astl::vector<astl::shared_ptr<Aura>> &auras() const {
        static const astl::vector<astl::shared_ptr<Aura>> kEmptyAurasDiff;
        return kEmptyAurasDiff;
    }

    // Validates the cast request against internals & parameters
    // @param[in] Parameters
    // @return Cast error
    virtual CastError canCast(const astl::shared_ptr<Params>) const;

    bool requiresTargeting() const { return false; }
    void triggerCooldown();
    u8 cooldown() const;
    u8 cost() const;
    u32 range() const;
    u32 id() const;
    State state() const { return state_; }
    u8 timeUntilCast() const { return timeUntilCast_; }
    u8 timeUntilResolveCast() const { return timeUntilResolveCast_; }

private:
    void delay(u8);
    void interrupt();

    // Query the skill to begin casting
    // @param[in] Source game object
    // @param[in] Computed skill damage & effects
    // @param[in] Destination
    // @param[in] Callback when done
    // @return Cast error
    CastError queryCast(GameObject *, const Skill::Effect &, PositionI, const astl::function<void()> &&, const astl::shared_ptr<Params>);
    void beginCast(const ResolutionInfo &, const astl::shared_ptr<Params>);
    void cast();
    void resolveCast();

    Bundle dataBundle_;
    astl::function<void()> callbackDone_;
    u8 cooldown_ = 0; // current cooldown, in turns
    u8 timeUntilCast_ = 0;
    u8 timeUntilResolveCast_ = 0;
    State state_ = State::Idle;

    friend class Character;
};

class AttackDamageSkill : public Skill {
public:
    AttackDamageSkill(Bundle bundle, f32 mul)
        : Skill(bundle)
        , attackDamageMultiplier_(mul) {
    }
    virtual ~AttackDamageSkill() {}
    f32 attackDamageMultiplier() const override {
        return attackDamageMultiplier_;
    }

private:
    f32 attackDamageMultiplier_;
};

class SpellDamageSkill : public Skill {
public:
    SpellDamageSkill(Bundle bundle, f32 mul)
        : Skill(bundle)
        , spellDamageMultiplier_(mul) {
    }
    virtual ~SpellDamageSkill() {}
    f32 spellDamageMultiplier() const override {
        return spellDamageMultiplier_;
    }

private:
    const f32 spellDamageMultiplier_;
};

class AuraSkill : public Skill {
public:
    AuraSkill(Bundle bundle, astl::shared_ptr<Aura> aura)
        : Skill(bundle)
        , auras_({ aura }) {
    }
    virtual ~AuraSkill() {}
    const astl::vector<astl::shared_ptr<Aura>> &auras() const override {
        return auras_;
    }

private:
    astl::vector<astl::shared_ptr<Aura>> auras_;
};

} };
