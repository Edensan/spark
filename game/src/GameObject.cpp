#include <GameObject.hpp>
#include <GameCombat.hpp>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

GameObject::GameObject(u32 uid, const char *name)
    : baseBundle_({ uid, name }) {
}

GameObject::~GameObject() {
}

u8 GameObject::type() const {
    return static_cast<u8>(GameObject::Type::Base);
}

const char *GameObject::typeName() const {
    return "Base";
}

u32 GameObject::uid() const {
    return baseBundle_.uid;
}

void GameObject::setName(const char *name) {
    baseBundle_.name = name;
}

const char *GameObject::name() const {
    return baseBundle_.name.c_str();
}

void GameObject::setSize(SizeU s) {
    transformBundle_.size = s;
}

SizeU GameObject::size() const {
    return transformBundle_.size;
}

void GameObject::setPosition(PositionI s) {
    transformBundle_.position = s;
}

PositionI GameObject::position() const {
    return transformBundle_.position;
}

void GameObject::setDirection(Array2I dir) {
    transformBundle_.direction = dir;
}

Array2I GameObject::direction() const {
    return transformBundle_.direction;
}

Skill::CastError GameObject::canCastSkill(u32 index, PositionI target, const astl::shared_ptr<Skill::Params> params) const {
    Skill *skill = nullptr;
    return canCastSkillImpl(&skill, index, target, params);
}

Skill::CastError GameObject::canCastSkillImpl(Skill **skillPPtr, u32 index, PositionI target, const astl::shared_ptr<Skill::Params> params) const {
    if (!active()) {
        return Skill::CastError::SourceNotActive;
    }
    const u32 skillId = skillBundle_.equippedSkill(index);
    if (skillId == skUndefinedU) {
        return Skill::CastError::SourceNotEquipped;
    }
    Skill *skill = skillBundle_.knownSkill(skillId);
    if (skill == nullptr) {
        return Skill::CastError::SourceNotLearned;
    }
    const Skill::CastError err = skill->canCast(params);
    if (err != Skill::CastError::OK) {
      return err;
    }
    const u32 distance = skDistance(position(), target);
    if (distance > skill->range()) {
        return Skill::CastError::OutOfRange;
    }

    *skillPPtr = skill;
    return Skill::CastError::OK;
}

Skill::CastError GameObject::castSkill(u32, PositionI, const astl::shared_ptr<Skill::Params>) {
    // DON'T IMPLEMENT
    return Skill::CastError::OK;
}

Skill::Effect GameObject::computeSkillEffect(const Skill &) {
    // DON'T IMPLEMENT
    static Skill::Effect kEmptyEffect;
    return kEmptyEffect;
}

bool SkillBundle::learnSkill(astl::shared_ptr<Skill> skill) {
    auto it = knownSkills.find(skill->id());
    if (it != knownSkills.end()) {
        return false;
    }
    knownSkills[skill->id()] = astl::move(skill);
    return true;
}

bool SkillBundle::forgetSkill(u32 skillId) {
    auto it = knownSkills.find(skillId);
    if (it == knownSkills.end()) {
        return false;
    }
    knownSkills.erase(it);
    return true;
}

Skill *SkillBundle::knownSkill(u32 skillId) const {
    auto it = knownSkills.find(skillId);
    if (it == knownSkills.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool SkillBundle::isKnown(u32 skillId) const {
    auto it = knownSkills.find(skillId);
    return it != knownSkills.end();
}

void SkillBundle::resizeEquipment(u32 size) {
    const size_t s = equippedSkills.size();
    equippedSkills.resize(size);
    if (size > s) {
        for (auto i = s; i < size; ++i) {
            equippedSkills[i] = skUndefinedU;
        }
    }
}

u32 SkillBundle::equipmentSize() const {
    return equippedSkills.size();
}

bool SkillBundle::equipSkill(u32 index, u32 skillId) {
    if (index >= equippedSkills.size()) {
        // skLogW("SkillBundle::equipSkill: invalid skill index=%d (size=%d)", skillId, index, equippedSkills.size());
        return false;
    }
    Skill *skill = knownSkill(skillId);
    if (skill == nullptr) {
        // skLogW("SkillBundle::equipSkill: could not find skill=%d", skillId);
        return false;
    }
    const u32 oldSkillId = equippedSkills[index];
    if (oldSkillId != skUndefinedU) {
        skLogI("SkillBundle::equipSkill: replacing previous skill=%d", oldSkillId);
    }
    equippedSkills[index] = skillId;
    return true;
}

bool SkillBundle::unequipSkill(u32 index) {
    if (index >= equippedSkills.size()) {
        return false;
    }
    const u32 oldSkillId = equippedSkills[index];
    if (oldSkillId == skUndefinedU) {
        return false;
    }
    equippedSkills[index] = skUndefinedU;
    return false;
}

u32 SkillBundle::equippedSkill(u32 index) const {
    if (index >= equippedSkills.size()) {
        return skUndefinedU;
    }
    return equippedSkills[index];
}


bool SkillBundle::isEquipped(u32 skillId) const {
    for (auto skillId_ : equippedSkills) {
        if (skillId_ == skillId) {
            return true;
        }
    }
    return false;
}

}; }; // namespace spark::game
