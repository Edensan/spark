#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>
#include <GameObject.hpp>
#include <GameSkill.hpp>

#include <niLang/STL/memory.h>
#include <niLang/STL/vector.h>
#include <niLang/STL/map.h>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

class Aura;
typedef astl::vector<astl::shared_ptr<Aura>> AurasVec;
typedef astl::vector<Skill *> ResolvingSkillsVec;

class Character : public GameObject {
public:
    class EventListener {
    public:
        virtual ~EventListener() {}
        inline Character *character() { return target; }

    protected:
        virtual void onDamaged(const GameObject &, u32) = 0;
        virtual void onHealed(const GameObject &, u32) = 0;
        virtual void onDied(const GameObject &) = 0;
        virtual void onPartyEntered(const Party &) = 0;
        virtual void onPartyLeft(const Party &) = 0;
        virtual void onAuraApplied(const GameObject &, const Aura &) = 0;
        virtual void onAuraExpired(const Aura &) = 0;
        virtual void onGridMoveRejected(u32) = 0;
        virtual void onGridMoved(PositionI, u32) = 0;
        virtual void onGridEntered(GameGrid *) = 0;
        virtual void onGridLeft(GameGrid *) = 0;

    private:
        Character *target = nullptr;
        friend class Character;
    };
    typedef astl::vector<EventListener *> ListenersVec;

    Character(u32, const char *, const Stats &);
    virtual ~Character();

    virtual u8 type() const override;
    virtual const char *typeName() const override;

    // Broadcast grid events
    virtual void onGridMoveRejected(u32) override;
    virtual void onGridMoved(PositionI, u32) override;
    virtual void onGridEntered(GameGrid *) override;
    virtual void onGridLeft(GameGrid *) override;

    // Stats skills are combat skills, buffs & debuffs
    // Movement skills are skills that instantly displace a character
    virtual Skill::CastError canCastSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) const override;
    virtual Skill::CastError castSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) override;
    void delayCasting(u8);
    void interruptCasting();
    virtual Skill::Effect computeSkillEffect(const Skill &) override;
    void applyResolvedSkillEffect(const Skill::ResolutionInfo &) override;
    void applySpellDamage(const GameObject &from, i32) override;
    void applyAttackDamage(const GameObject &from, i32) override;
    void applyAura(const GameObject &from, astl::shared_ptr<Aura>) override;
    void expireAura(Aura *) override;
    virtual void logicUpdate(u8) override;
    void processDirty();

    void registerEventListener(EventListener *);
    void unregisterEventListener(EventListener *);

    void onPartyEntered(const Party &) override;
    void onPartyLeft(const Party &) override;

    const Stats &stats() const { return stats_; }
    Stats &rwStats() { return stats_; }
    i32 attackPower() const { return stats_.computed(Stats::Type::AttackPower); }
    i32 maxHitPoints() const { return stats_.computed(Stats::Type::MaxHitPoints); }
    i32 currentHitPoints() const { return currentHitPoints_; }
    i32 maxActionPoints() const { return stats_.computed(Stats::Type::MaxActionPoints); }
    i32 actionPointsRecovery() const { return stats_.computed(Stats::Type::ActionPointsRecovery); }
    i32 currentActionPoints() const { return currentActionPoints_; }

    // Increase/decrease action points
    // @param[in] Action points offset
    void offsetActionPoints(i32);

    // Whether this character has enough action points to spend
    // @param[in] Action point cost
    // @return True when sufficient
    bool hasEnoughActionPoints(i32) const;

private:
    Skill::CastError canCastSkillImpl(Skill **, u32, PositionI, const astl::shared_ptr<Skill::Params>) const override;
    void onCastingDone(u32);

    void clampHitPoints();
    void clampActionPoints();
    void doDamage(const GameObject &from, i32);
    i32 spellDamageFirstPass(const GameObject &from, i32);
    i32 attackDamageFirstPass(const GameObject &from, i32);
    inline void dirtyBuffs() { hasDirtyBuffs_ = true; }
    bool hasAura(u32) const;

    ResolvingSkillsVec resolvingSkills_;
    ListenersVec listeners_;
    AurasVec auras_;
    Stats stats_;
    i32 currentHitPoints_ = astl::numeric_limits<i32>::max();
    i32 currentActionPoints_ = astl::numeric_limits<i32>::max();
    bool hasDirtyBuffs_ = true;
};

}; }; // namespace spark::game
