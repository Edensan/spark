#include "TestMain.hpp"
#include <GameCombat.hpp>
#include <GameGrid.hpp>
#include <GameSkill.hpp>
#include <objects/Character.hpp>
#include <niLang/STL/limits.h>

namespace spark {
using namespace common::math;
using namespace game;
namespace tests {

// Every Cell is valid
static u32 initTypeFuncWorld(const PositionI &) { return 0; }
class MoveValidatorImpl : public GameGrid::MoveValidator {
public:
    MoveValidatorImpl()
        : GameGrid::MoveValidator() {
    }
    virtual u32 validateMove(GameGrid *, GameGrid::Listener *, const PositionI &) override { return 0; }
    virtual bool isOK(u32) override { return true; } // Always OK!
    virtual bool isWarning(u32) override { return false; }
    virtual bool isError(u32) override { return false; }
};

class EventListenerImpl : public Character::EventListener {
public:
    i32 damagedCount = 0;
    i32 healedCount = 0;
    i32 diedCount = 0;
    i32 partyEnteredCount = 0;
    i32 partyLeftCount = 0;
    i32 auraAppliedCount = 0;
    i32 auraExpiredCount = 0;

protected:
    void onDamaged(const GameObject &, u32) override { ++damagedCount; }
    void onHealed(const GameObject &, u32) override { ++healedCount; }
    void onDied(const GameObject &) override { ++diedCount; }
    void onPartyEntered(const Party &) override { ++partyEnteredCount; }
    void onPartyLeft(const Party &) override { ++partyLeftCount; }
    void onAuraApplied(const GameObject &, const Aura &) override { ++auraAppliedCount; }
    void onAuraExpired(const Aura &) override { ++auraExpiredCount; }
    void onGridMoveRejected(u32) override {}
    void onGridMoved(PositionI, u32) override {}
    void onGridEntered(GameGrid *) override {}
    void onGridLeft(GameGrid *) override {}
};

// Simulate a move skill.
class MoveSkill : public Skill {
public:
    MoveSkill(Skill::Bundle bundle)
        : Skill(bundle) {
    }

    u8 onBeginCast(const ResolutionInfo &info, const astl::shared_ptr<Skill::Params>) override {
        target_ = info.source;
        position_ = info.destination;
        return 0;
    }

    u8 onCast() override {
        return 0;
    }

    void onResolveCast() override {
        target_->setPosition(position_);
    }

private:
    GameObject *target_;
    PositionI position_;
};

// Simulate a single target cast without delays.
//
// Since we have no grid in this test case and we've provided
// the position based on the target character itself,
// pos is garanteed to be == target->position().
class AttackDamageSkillImpl : public AttackDamageSkill {
public:
    AttackDamageSkillImpl(Skill::Bundle bundle, f32 mul, u8 castingTime = 0, u8 resolutionTime = 0)
        : AttackDamageSkill(bundle, mul)
        , castingTime_(castingTime)
        , resolutionTime_(resolutionTime) {
    }

    u8 onBeginCast(const ResolutionInfo &info, const astl::shared_ptr<Skill::Params>) override {
        // NOTE: Here can potentially play a "casting" animation.
        resolutionInfo_ = info;
        return castingTime_;
    }

    u8 onCast() override {
        // NOTE: Here we can
        // a) play a "cast" animation
        // b) send out projectiles, animation time (in logic cycles) should be returned here
        return resolutionTime_;
    }

    void onResolveCast() override {
        // Resolve targets and apply the resolved skill effects
        auto targets = resolveTargets();
        for (auto target : targets) {
            target->applyResolvedSkillEffect(resolutionInfo_);
        }
    }

    void onDelayedCast(u8) override {
    }

    void onInterruptedCast() override {
    }

    astl::vector<GameObject *> resolveTargets() const {
        return { target };
    }
    Character *target = nullptr;

private:
    ResolutionInfo resolutionInfo_;
    u8 castingTime_;
    u8 resolutionTime_;
};

TEST_F(UnitTests, Game_Skill_Cast) {
    constexpr i32 kPlayerMaxAp = 10;
    constexpr i32 kPlayerApRecovery = 4;
    Character player = { 0, "player", { 1, 1, 1, kPlayerMaxAp, kPlayerApRecovery } };
    player.setPosition({ 0, 0 });
    player.processDirty();

    // Learn a normal skill
    const u32 moveSkillId = 0u;
    const u32 atkSkillId = 1u;
    const u32 slowAtkSkillId = 2u;
    const u32 slowAtkSkillCastingTime = 2u;
    const u32 slowAtkSkillCastingResolutionTime = 2u;
    {
        // Learn a normal movement skill
        Skill::Bundle skillData;
        skillData.id = moveSkillId;
        skillData.cost = 1;
        skillData.range = 1;
        skillData.baseCooldown = 0;
        auto moveSkill = astl::make_shared<MoveSkill>(skillData);
        EXPECT_FALSE(player.skillBundle()->isKnown(moveSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(moveSkillId));
        player.skillBundle()->learnSkill(moveSkill);
        EXPECT_TRUE(player.skillBundle()->isKnown(moveSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(moveSkillId));
        EXPECT_FALSE(player.skillBundle()->equipSkill(0, moveSkillId)); // Skill slot unavailable
        EXPECT_FALSE(player.skillBundle()->isEquipped(moveSkillId));
        player.skillBundle()->resizeEquipment(4);
        EXPECT_EQ(player.skillBundle()->equipmentSize(), 4u);
        EXPECT_TRUE(player.skillBundle()->equipSkill(0, moveSkillId)); // Now OK!
        EXPECT_TRUE(player.skillBundle()->isKnown(moveSkillId));
        EXPECT_TRUE(player.skillBundle()->isEquipped(moveSkillId));
    }
    {
        // Learn a normal attack skill
        Skill::Bundle skillData;
        skillData.id = atkSkillId;
        skillData.cost = 1;
        skillData.range = 1;
        skillData.baseCooldown = 0;
        auto atkSkill = astl::make_shared<AttackDamageSkillImpl>(skillData, 1.0f);
        EXPECT_FALSE(player.skillBundle()->isKnown(atkSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(atkSkillId));
        player.skillBundle()->learnSkill(atkSkill);
        EXPECT_TRUE(player.skillBundle()->isKnown(atkSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(atkSkillId));
        EXPECT_TRUE(player.skillBundle()->equipSkill(1, atkSkillId));
        EXPECT_TRUE(player.skillBundle()->isKnown(atkSkillId));
        EXPECT_TRUE(player.skillBundle()->isEquipped(atkSkillId));
    }
    {
        // Learn a slower skill
        Skill::Bundle skillData;
        skillData.id = slowAtkSkillId;
        skillData.cost = 1;
        skillData.range = 1;
        skillData.baseCooldown = 1;
        auto slowAtkSkill = astl::make_shared<AttackDamageSkillImpl>(
            skillData
            , 1.0f
            , slowAtkSkillCastingTime
            , slowAtkSkillCastingResolutionTime);
        EXPECT_FALSE(player.skillBundle()->isKnown(slowAtkSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(slowAtkSkillId));
        player.skillBundle()->learnSkill(slowAtkSkill);
        EXPECT_TRUE(player.skillBundle()->isKnown(slowAtkSkillId));
        EXPECT_FALSE(player.skillBundle()->isEquipped(slowAtkSkillId));
        EXPECT_TRUE(player.skillBundle()->equipSkill(2, slowAtkSkillId));
        EXPECT_TRUE(player.skillBundle()->isKnown(slowAtkSkillId));
        EXPECT_TRUE(player.skillBundle()->isEquipped(slowAtkSkillId));
    }

    auto moveSkill = player.skillBundle()->knownSkill(moveSkillId);
    auto atkSkill = player.skillBundle()->knownSkill(atkSkillId);
    auto slowAtkSkill = player.skillBundle()->knownSkill(slowAtkSkillId);
    {
        // Create a training dummy with 10hp,
        // which means player should cast atkSkill 10 times to destroy it.
        EventListenerImpl dummyListener;
        Character trainingDummy = { 0, "firstTrainingDummy", { 1, 0, 0 } };
        trainingDummy.registerEventListener(&dummyListener);
        trainingDummy.setPosition({ 2, 2 });
        trainingDummy.processDirty();
        static_cast<AttackDamageSkillImpl *>(atkSkill)->target = &trainingDummy;

        // Checking if can cast, but player isn't active
        EXPECT_EQ(player.canCastSkill(atkSkillId, trainingDummy.position()), Skill::CastError::SourceNotActive);
        player.activate();

        // Checking if can cast, but player is out of range
        EXPECT_EQ(player.canCastSkill(atkSkillId, trainingDummy.position()), Skill::CastError::OutOfRange);

        // Move player in range
        EXPECT_EQ(player.castSkill(moveSkillId, { 1, 0 }), Skill::CastError::OK);
        EXPECT_EQ(player.castSkill(moveSkillId, { 2, 0 }), Skill::CastError::OK);
        EXPECT_EQ(player.castSkill(moveSkillId, { 2, 1 }), Skill::CastError::OK);

        // Checking if can cast, seems OK!
        EXPECT_EQ(player.canCastSkill(atkSkillId, trainingDummy.position()), Skill::CastError::OK);

        // Currently player has max action points
        EXPECT_EQ(player.currentActionPoints(), player.maxActionPoints() - (moveSkill->cost() * 3));

        // Hit the dummy until it is destroyed
        const i32 skillDmg = 1;
        i32 hitCount = 0;
        const i32 dummyMaxHp = trainingDummy.stats().get(Stats::Type::MaxHitPoints);
        const i32 expectedHitCount = trainingDummy.currentHitPoints() / skillDmg;
        constexpr u32 kExpectedApRecovery = 1u;
        u32 apRegenCount = 0u;
        while (++hitCount <= expectedHitCount) {
            // Recover ap if necessary
            if (player.canCastSkill(atkSkillId, trainingDummy.position()) == Skill::CastError::OutOfActionPoints) {
                const i32 ap = player.currentActionPoints();
                player.logicUpdate(1);
                EXPECT_EQ(player.currentActionPoints(), ap + kPlayerApRecovery);
                ++apRegenCount;
            }
            EXPECT_EQ(player.castSkill(atkSkillId, trainingDummy.position()), Skill::CastError::OK);
            EXPECT_EQ(trainingDummy.currentHitPoints(), dummyMaxHp - (hitCount * skillDmg));
            EXPECT_EQ(dummyListener.damagedCount, hitCount);
        }

        // We expect one ap recovery was necessary
        EXPECT_EQ(apRegenCount, kExpectedApRecovery);

        // Dummy dead.
        EXPECT_EQ(dummyListener.diedCount, 1);
    }

    // Recharge all action points
    while (player.currentActionPoints() < player.stats().get(Stats::Type::MaxActionPoints)) {
        player.logicUpdate(1);
    }

    {
        // Test hitting with a skill that has a casting & resolution time > 0
        EventListenerImpl dummyListener;
        Character trainingDummy = { 0, "secondTrainingDummy", { 1, 0, 0 } };
        trainingDummy.registerEventListener(&dummyListener);
        trainingDummy.setPosition({ 2, 2 });
        trainingDummy.processDirty();
        static_cast<AttackDamageSkillImpl *>(slowAtkSkill)->target = &trainingDummy;

        // Checking if can cast the slow skill, seems OK!
        EXPECT_EQ(player.canCastSkill(slowAtkSkillId, trainingDummy.position()), Skill::CastError::OK);
        EXPECT_EQ(player.castSkill(slowAtkSkillId, trainingDummy.position()), Skill::CastError::OK);

        // Casting time for the slow skill is 2
        EXPECT_EQ(dummyListener.damagedCount, 0);
        player.logicUpdate(1); // Still preparing...
        EXPECT_EQ(dummyListener.damagedCount, 0);
        player.logicUpdate(1); // Casting spell here!
        // Resolution time for the slow skill is also 2
        EXPECT_EQ(dummyListener.damagedCount, 0);
        player.logicUpdate(1); // Skill needs time to resolve...
        EXPECT_EQ(dummyListener.damagedCount, 0);
        player.logicUpdate(1); // Skill resolved!
        EXPECT_EQ(dummyListener.damagedCount, 1);
    }
}

TEST_F(UnitTests, Game_Combat_NoGrid) {
    EventListenerImpl eventListener;
    Party playerParty = { "playerParty" }
        , trainingDummies = { "trainingDummies" };
    Character player = { 0, "player", { 5, 2, 2 } };
    player.skillBundle()->resizeEquipment(4); // Can equip up to 2 skills
    player.setPosition({ 1, 0 });
    Character trainingDummy = { 0, "trainingDummy", { 10, 0, 0 } };
    trainingDummy.skillBundle()->resizeEquipment(1); // Can equip up to 1 skill
    trainingDummy.setPosition({ 0, 0 });
    trainingDummy.registerEventListener(&eventListener);
    playerParty.addMember(&player);
    trainingDummies.addMember(&trainingDummy);
    EXPECT_EQ(eventListener.partyEnteredCount, 1);

    // Define all the skills
    constexpr f32 kNoDamage = 0.0f;
    constexpr f32 kUnitDamage = 1.0f;
    constexpr u32 kUnitCooldown = 1u;
    constexpr u32 kUnitRange = 1u;
    constexpr u32 kIdAtkSkillDummy = 0u;
    constexpr u32 kIdAtkSkill = 1u;
    constexpr u32 kIdAtkSkillOneShot = 2u;
    constexpr u32 kIdAtkSkillExpensive = 3u;
    constexpr u32 kIndexAtkSkillDummy = 0u;
    constexpr u32 kIndexAtkSkill = 0u;
    constexpr u32 kIndexAtkSkillOneShot = 1u;
    constexpr u32 kIndexAtkSkillExpensive = 2u;
    {
        // No damage skill
        auto atkSkillDummy = astl::make_shared<AttackDamageSkillImpl>(
            Skill::Bundle { kIdAtkSkillDummy
              , kUnitRange
              , kUnitApCost
              , kUnitCooldown }
            , kNoDamage );
        atkSkillDummy->target = &player;
        trainingDummy.skillBundle()->learnSkill(atkSkillDummy);
        trainingDummy.skillBundle()->equipSkill(kIndexAtkSkillDummy, atkSkillDummy->id());

        // 1x damage, 1 range
        auto atkSkill = astl::make_shared<AttackDamageSkillImpl>(
            Skill::Bundle { kIdAtkSkill
              , kUnitRange
              , kUnitApCost
              , kUnitCooldown }
            , kUnitDamage );
        atkSkill->target = &trainingDummy;
        player.skillBundle()->learnSkill(atkSkill);
        player.skillBundle()->equipSkill(kIndexAtkSkill, atkSkill->id());

        // Garanteed one shot cheat skill
        auto atkSkillOneShot = astl::make_shared<AttackDamageSkillImpl>(
            Skill::Bundle { kIdAtkSkillOneShot
              , kUnitRange * 2
              , kUnitApCost * 3
              , kUnitCooldown }
            , kUnitDamage * 100.0f );
        atkSkillOneShot->target = &trainingDummy;
        player.skillBundle()->learnSkill(atkSkillOneShot);
        player.skillBundle()->equipSkill(kIndexAtkSkillOneShot, atkSkillOneShot->id());

        // Garanteed too expensive to cast
        auto atkSkillExpensive = astl::make_shared<AttackDamageSkillImpl>(
            Skill::Bundle { kIdAtkSkillExpensive
              , kUnitRange
              , astl::numeric_limits<u8>::max()
              , kUnitCooldown }
            , kUnitDamage );
        atkSkillExpensive->target = &trainingDummy;
        player.skillBundle()->learnSkill(atkSkillExpensive);
        player.skillBundle()->equipSkill(kIndexAtkSkillExpensive, atkSkillExpensive->id());
    }
    // auto atkSkillDummy = trainingDummy.skillBundle()->knownSkill(kIdAtkSkillDummy);
    // auto atkSkill = player.skillBundle()->knownSkill(kIdAtkSkill);
    // auto atkSkillExpensive = player.skillBundle()->knownSkill(kIdAtkSkillExpensive);
    // auto atkSkillOneShot = player.skillBundle()->knownSkill(kIdAtkSkillOneShot);

    EXPECT_FALSE(player.active());
    EXPECT_FALSE(trainingDummy.active());

    EXPECT_EQ(player.currentParty(), &playerParty);
    EXPECT_EQ(trainingDummy.currentParty(), &trainingDummies);

    player.processDirty();
    trainingDummy.processDirty();
    EXPECT_EQ(player.currentActionPoints(), player.maxActionPoints());
    EXPECT_EQ(trainingDummy.currentActionPoints(), trainingDummy.maxActionPoints());

    // Add the parties involved in the combat
    Combat combat;
    combat.addParty(&playerParty);
    combat.addParty(&trainingDummies);

    // At this point everyone should be inactive
    EXPECT_FALSE(playerParty.playingTurn());
    EXPECT_FALSE(trainingDummies.playingTurn());

    // Begin turn 1,
    // activates the playerParty
    combat.enterState();

    // playerParty is active
    EXPECT_TRUE(playerParty.playingTurn());
    EXPECT_FALSE(trainingDummies.playingTurn());

    // Cast skill on the dummy
    EXPECT_EQ(player.castSkill(kIndexAtkSkill, trainingDummy.position()), Skill::CastError::OK);

    // Cache player Ap before new turn
    const i32 prevAp = player.currentActionPoints();

    // Begin turn 2
    combat.nextParty(1);

    // Player Ap should be unchanged because it's not his turn yet!
    EXPECT_EQ(player.currentActionPoints(), prevAp);

    // trainingDummies are active
    EXPECT_FALSE(playerParty.playingTurn());
    EXPECT_TRUE(trainingDummies.playingTurn());

    // trainingDummies can cast
    const i32 prevHp = trainingDummy.currentHitPoints();
    EXPECT_EQ(trainingDummy.canCastSkill(kIndexAtkSkillDummy, player.position()), Skill::CastError::OK);
    EXPECT_EQ(player.canCastSkill(kIndexAtkSkill, trainingDummy.position()), Skill::CastError::SourceNotActive);
    EXPECT_EQ(player.castSkill(kIndexAtkSkill, trainingDummy.position()), Skill::CastError::SourceNotActive);
    EXPECT_EQ(eventListener.damagedCount, 1);

    // Player can't cast, so no damage done!
    EXPECT_EQ(trainingDummy.currentHitPoints(), prevHp);

    // Back to player turn
    combat.nextParty(1);

    // Player Ap should now be greater than old value due to Ap recovery
    EXPECT_GT(player.currentActionPoints(), prevAp);

    // Player moves a further aways from target (range == 2)
    player.setPosition({ 2, 0 });

    // Fails to cast the normal skill with range == 1
    EXPECT_EQ(player.canCastSkill(kIndexAtkSkill, trainingDummy.position()), Skill::CastError::OutOfRange);

    {
        // Casting the one shot skill with range == 2
        const auto castErrOneShot = player.castSkill(kIndexAtkSkillOneShot, trainingDummy.position());
        EXPECT_EQ(castErrOneShot, Skill::CastError::OK);
        EXPECT_EQ(trainingDummy.currentHitPoints(), 0);
        EXPECT_EQ(eventListener.damagedCount, 2);
        EXPECT_EQ(eventListener.diedCount, 1);
    }

    trainingDummies.leaveCombat();
    trainingDummies.removeMember(&trainingDummy);
    EXPECT_EQ(eventListener.healedCount, 0);
    EXPECT_EQ(eventListener.partyLeftCount, 1);
    EXPECT_EQ(eventListener.auraAppliedCount, 0);
    EXPECT_EQ(eventListener.auraExpiredCount, 0);
}

} // namespace tests
} // namespace spark
