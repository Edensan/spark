#include <objects/Character.hpp>
#include <GameAura.hpp>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

Character::Character(u32 uid, const char *name, const Stats &in)
    : GameObject(uid, name) {
    stats_ = in;
}

Character::~Character() {
}

u8 Character::type() const {
    return static_cast<u8>(GameObject::Type::Character);
}

const char *Character::typeName() const {
    return "Character";
}

void Character::onGridMoveRejected(u32 err) {
    for (auto l : listeners_)
        l->onGridMoveRejected(err);
}

void Character::onGridMoved(PositionI p, u32 err) {
    for (auto l : listeners_)
        l->onGridMoved(p, err);
}

void Character::onGridEntered(GameGrid *gg) {
    for (auto l : listeners_)
        l->onGridEntered(gg);
}

void Character::onGridLeft(GameGrid *gg) {
    for (auto l : listeners_)
        l->onGridLeft(gg);
}

bool Character::hasAura(u32 auraUid) const {
    skLoopIt (it, auras_) {
        if ((*it)->uid() == auraUid) {
            return true;
        }
    }
    return false;
}

void Character::applyAura(const GameObject &src, astl::shared_ptr<Aura> aura) {
    if (hasAura(aura->uid())) {
        return;
    }
    auras_.push_back(aura);
    for (auto l : listeners_)
        l->onAuraApplied(src, *aura.get());
    dirtyBuffs();
}

void Character::expireAura(Aura *aura) {
    const u32 auraUid = aura->uid();
    skLoopIt (it, auras_) {
        astl::shared_ptr<Aura> &a = *it;
        if (a->uid() == auraUid) {
            auras_.erase(it);
            for (auto l : listeners_)
                l->onAuraExpired(*a.get());
            dirtyBuffs();
            break;
        }
    }
}

void Character::processDirty() {
    // Process all auras for attributes.
    if (hasDirtyBuffs_) {
        // Reset all multipliers and additives.
        stats_.resetAll();
        for (auto &aura : auras_) {
            if (Aura::typeAttr(*aura.get())) {
                aura->applyTo(this);
            }
        }
    }

    // Stats update based on attributes changes.
    const bool dirtyAttrs = stats_.computeStats();

    // Process all other auras.
    if (dirtyAttrs || hasDirtyBuffs_) {
        for (auto &aura : auras_) {
            if (!Aura::typeAttr(*aura.get())) {
                aura->applyTo(this);
            }
        }
        clampHitPoints();
        clampActionPoints();
    }

    hasDirtyBuffs_ = false;
}

void Character::clampHitPoints() {
    const i32 maxHp = stats_.computed(Stats::Type::MaxHitPoints);
    currentHitPoints_ = skClamp(currentHitPoints_, 0, maxHp);
}

void Character::clampActionPoints() {
    const i32 maxAp = stats_.computed(Stats::Type::MaxActionPoints);
    currentActionPoints_ = skClamp(currentActionPoints_, 0, maxAp);
}

void Character::logicUpdate(u8 logicCycle) {
    // Update available action points
    currentActionPoints_ += logicCycle * stats_.computed(Stats::Type::ActionPointsRecovery);
    clampActionPoints();

    AurasVec aurasCopy = auras_;
    skLoopIt (it, aurasCopy) {
        (*it)->logicUpdate(logicCycle);
    }
    ResolvingSkillsVec resolvingSkillsCopy = resolvingSkills_;
    skLoopIt (it, resolvingSkillsCopy) {
        (*it)->logicUpdate(logicCycle);
    }
}

Skill::Effect Character::computeSkillEffect(const Skill &skill) {
    Skill::Effect ret;
    ret.attackDamage = skMax(0, stats_.computed(Stats::Type::AttackPower) * skill.attackDamageMultiplier());
    ret.spellDamage = stats_.computed(Stats::Type::SpellPower) * skill.spellDamageMultiplier();
    ret.auras = skill.auras();
    return ret;
}

void Character::applySpellDamage(const GameObject &src, i32 dmg) {
    const i32 spDmg = spellDamageFirstPass(src, dmg);
    doDamage(src, spDmg);
}

i32 Character::spellDamageFirstPass(const GameObject &, i32 dmg) {
    // TODO: edmond
    // Take buffs/shields into account!
    return dmg;
}

void Character::applyAttackDamage(const GameObject &src, i32 dmg) {
    const i32 spDmg = attackDamageFirstPass(src, dmg);
    doDamage(src, spDmg);
}

i32 Character::attackDamageFirstPass(const GameObject &, i32 dmg) {
    // TODO: edmond
    // Take buffs/shields into account!
    return dmg;
}

void Character::doDamage(const GameObject &src, i32 dmg) {
    const i32 maxHp = stats_.computed(Stats::Type::MaxHitPoints);
    const i32 computedHp = currentHitPoints_ - static_cast<i32>(dmg);
    currentHitPoints_ = skClamp(computedHp, 0, maxHp);
    if (dmg > 0) {
        if (currentHitPoints_ <= 0) {
            for (auto l : listeners_) {
                l->onDamaged(src, dmg);
                l->onDied(src);
            }
        }
        else {
            for (auto l : listeners_)
                l->onDamaged(src, dmg);
        }
    }
    else if (dmg < 0) {
        for (auto l : listeners_)
            l->onHealed(src, -dmg);
    }
    else {
        // No damage nor healing!
    }
}

void Character::offsetActionPoints(i32 offset) {
    // actionPoints regen
    currentActionPoints_ += offset;
    clampActionPoints();
}

bool Character::hasEnoughActionPoints(i32 cost) const {
    return currentActionPoints_ >= cost;
}

Skill::CastError Character::canCastSkill(u32 index, PositionI target, const astl::shared_ptr<Skill::Params> params) const {
    Skill *skill = nullptr;
    return canCastSkillImpl(&skill, index, target, params);
}

Skill::CastError Character::canCastSkillImpl(Skill **skillPPtr, u32 index, PositionI target, const astl::shared_ptr<Skill::Params> params) const {
    const Skill::CastError err = GameObject::canCastSkillImpl(skillPPtr, index, target, params);
    if (err != Skill::CastError::OK) {
        return err;
    }
    if (!hasEnoughActionPoints((*skillPPtr)->cost())) {
        return Skill::CastError::OutOfActionPoints;
    }
    return Skill::CastError::OK;
}

Skill::CastError Character::castSkill(u32 index, PositionI target, const astl::shared_ptr<Skill::Params> params) {
    Skill *skill = nullptr;
    Skill::CastError err = canCastSkillImpl(&skill, index, target, params);
    if (err != Skill::CastError::OK) {
        return err;
    }

    const u32 skillId = skill->id();
    err = skill->queryCast(
        this
        , computeSkillEffect(*skill)
        , target
        , [self = this, skillId]() -> void {
              self->onCastingDone(skillId);
          }
        , params);

    // Already checked with the skill->canCast.
    (void)err;

    offsetActionPoints(-skill->cost());
    resolvingSkills_.push_back(skill);
    return Skill::CastError::OK;
}

void Character::onCastingDone(u32 skillId) {
    skLoopIt (it, resolvingSkills_) {
        if ((*it)->id() == skillId) {
            resolvingSkills_.erase(it);
            break;
        }
    }
}

void Character::delayCasting(u8 delay) {
    skLoopIt (it, resolvingSkills_) {
        if ((*it)->state() == Skill::State::Casting) {
            (*it)->delay(delay);
        }
    }
}

void Character::interruptCasting() {
    // Avoid issues with onCastingDone invalidating iterators,
    // the resolvingSkills_ should always be negligible anyway.
    ResolvingSkillsVec resolvingCopy = resolvingSkills_;
    skLoopIt (it, resolvingCopy) {
        if ((*it)->state() == Skill::State::Casting) {
            (*it)->interrupt();
        }
    }
}

void Character::applyResolvedSkillEffect(const Skill::ResolutionInfo &resolutionInfo) {
    const GameObject &src = *resolutionInfo.source;
    const Skill::Effect &eff = resolutionInfo.effect;
    const i32 atDamage = attackDamageFirstPass(src, eff.attackDamage);
    const i32 spDamage = spellDamageFirstPass(src, eff.spellDamage);
    doDamage(src, spDamage + atDamage);
    skLoopIt (it, eff.auras) {
        applyAura(src, *it);
    }

    // TODO: edmond
    // Need to support skills that can expire auras
}

void Character::registerEventListener(EventListener *l) {
    if (l->target != nullptr) {
        skUnreachable("%s: Could not register EventListener: already registered!", __func__);
        return;
    }

    if (l->target != this) {
        l->target = this;
        listeners_.push_back(l);
    }
}

void Character::unregisterEventListener(EventListener *l) {
    if (l->target == this) {
        skLoopIt (it, listeners_) {
            EventListener *l_ = *it;
            if (l_ == l) {
                l_->target = nullptr;
                listeners_.erase(it);
                return;
            }
        }
    }
}

void Character::onPartyEntered(const Party &party) {
    for (auto l : listeners_) {
        l->onPartyEntered(party);
    }
}

void Character::onPartyLeft(const Party &party) {
    for (auto l : listeners_) {
        l->onPartyLeft(party);
    }
}

}; }; // namespace spark::game
