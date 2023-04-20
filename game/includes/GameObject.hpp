#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>
#include <GameGrid.hpp>
#include <GameStats.hpp>
#include <GameSkill.hpp>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

enum class Direction : u8 {
    Left = skBit(0),
    Up = skBit(1),
    Right = skBit(2),
    Down = skBit(3),
    UpLeft = Up | Left,
    UpRight = Up | Right,
    DownLeft = Down | Left,
    DownRight = Down | Right,
};

// NOTE: Structuring this way gives us freedom to manipulate
// various data categories more efficiently when batching processes.
// NOTE: Might also be convenient for serialization.
struct BaseBundle {
    const u32 uid;
    astl::string name;
};

struct TransformBundle {
    PositionI position = PositionI::undefined();
    Array2I direction = {{ 0, 1 }};
    SizeU size;
};

class SkillBundle {
public:
    // Learns a given skill
    // @param[in] Skill
    // @return Whether the skill was learned successfully
    bool learnSkill(astl::shared_ptr<Skill> skill);

    // Forgets a skill
    // @param[in] Skill id
    // @return Whether the skill was found and removed
    bool forgetSkill(u32);

    // Gets known skill from its id
    // @param[in] Skill id
    // @return Skill (read-only)
    Skill *knownSkill(u32) const;

    // Gets known skill from its id
    // @param[in] Skill id
    // @return Skill (read-only)
    bool isKnown(u32) const;

    // Gets the known skill count
    // @return Known skill count
    u32 knownSkillsCount() const { return knownSkills.size(); }

    // Sets the equipment size
    // @param[in] Size
    void resizeEquipment(u32);

    // Gets the equipment size
    // @return Size
    u32 equipmentSize() const;

    // Equips the specified skill
    // @param[in] Index
    // @param[in] Skill id
    // @return Successfully equipped
    bool equipSkill(u32, u32);

    // Unequips the specified skill
    // @param[in] Index
    // @return Successfully unequipped
    bool unequipSkill(u32);

    // Gets the equipped skill at the given index
    // @param[in] Slot index
    // @return Skill id
    u32 equippedSkill(u32) const;

    // Whether the specified skill is equipped
    // @param[in] Skill id
    // @return Equipped
    bool isEquipped(u32) const;

    astl::map<u32, astl::shared_ptr<Skill>> knownSkills;
    astl::vector<u32> equippedSkills;
};

class Party;
class GameObject : public GameGrid::Listener {
public:
    enum class Type : u8 {
        Base = 0,
        Character = 1,
    };

    GameObject(u32, const char *);
    virtual ~GameObject();

    virtual u8 type() const;
    virtual const char *typeName() const;

    u32 uid() const;
    void setName(const char *);
    const char *name() const;
    SkillBundle *skillBundle() { return &skillBundle_; }

    void setSize(SizeU) override;
    SizeU size() const override;
    void setPosition(PositionI) override;
    PositionI position() const override;
    void setDirection(Array2I);
    Array2I direction() const;
    virtual void onPartyEntered(const Party &) {}
    virtual void onPartyLeft(const Party &) {}
    virtual void onGridEntered(GameGrid *) override {}
    virtual void onGridLeft(GameGrid *) override {}
    virtual void onGridMoveRejected(u32) override {}
    virtual void onGridMoved(PositionI, u32) override {}

    void joinParty(Party *);
    void leaveParty();
    Party *currentParty() const { return currentParty_; }

    // Most actions (move, cast) can only be performed when active
    void activate() { active_ = true; }
    void deactivate() { active_ = false; }
    bool active() const { return active_; }

    // IMPLEMENT THESE
    virtual void logicUpdate(u8) = 0;
    virtual Skill::CastError canCastSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) const = 0;
    virtual Skill::CastError castSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) = 0;
    virtual Skill::Effect computeSkillEffect(const Skill &) = 0;
    virtual void applyResolvedSkillEffect(const Skill::ResolutionInfo &) = 0;
    virtual void applySpellDamage(const GameObject &, i32) = 0;
    virtual void applyAttackDamage(const GameObject &, i32) = 0;
    virtual void applyAura(const GameObject &, astl::shared_ptr<Aura>) = 0;
    virtual void expireAura(Aura *) = 0;

protected:
    virtual Skill::CastError canCastSkillImpl(Skill **, u32, PositionI, const astl::shared_ptr<Skill::Params>) const;

private:
    Party *currentParty_ = nullptr;
    BaseBundle baseBundle_;
    TransformBundle transformBundle_;
    SkillBundle skillBundle_;
    bool active_ = false;
    friend class GameGrid;
    friend class Party;
};

class DummyGameObject : public GameObject {
public:
    DummyGameObject(u32 uid, const char *name) : GameObject(uid,name) {}
    void logicUpdate(u8) override {}
    Skill::CastError canCastSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) const override { return Skill::CastError::OK; }
    Skill::CastError castSkill(u32, PositionI, const astl::shared_ptr<Skill::Params> = nullptr) override { return Skill::CastError::OK; }
    Skill::Effect computeSkillEffect(const Skill &) override { return {}; }
    void applyResolvedSkillEffect(const Skill::ResolutionInfo &) override {}
    void applySpellDamage(const GameObject &, i32) override {}
    void applyAttackDamage(const GameObject &, i32) override {}
    void applyAura(const GameObject &, astl::shared_ptr<Aura>) override {}
    void expireAura(Aura *) override {}
};

}; }; // namespace spark::game
