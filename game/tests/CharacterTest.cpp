#include "TestMain.hpp"
#include <GameAura.hpp>
#include <objects/Character.hpp>

namespace spark {
using namespace common::math;
using namespace game;
namespace tests {

constexpr i32 unitAttr = 1;
const Stats unitAttributes = {
    unitAttr
    , unitAttr
    , unitAttr
};

TEST_F(UnitTests, Game_Character_Init) {
    const Character character { 0, "Edmond", unitAttributes };
    EXPECT_EQ(character.uid(), 0u);
    EXPECT_STREQ(character.name(), "Edmond");
    EXPECT_EQ(character.stats().computed(Stats::Type::Strength), unitAttr);
    EXPECT_EQ(character.stats().computed(Stats::Type::Agility), unitAttr);
    EXPECT_EQ(character.stats().computed(Stats::Type::Intelligence), unitAttr);
}

class AdditiveStrengthAuraImpl : public AdditiveAura<Stats::Type::Strength> {
public:
    AdditiveStrengthAuraImpl(u32 uid, i32 add)
        : AdditiveAura<Stats::Type::Strength>(uid, add) {
    }
    const char *name() const override {
        return "AdditiveStrengthAura";
    }
};

class MultiplicativeStrengthAuraImpl : public MultiplicativeAura<Stats::Type::Strength> {
public:
    MultiplicativeStrengthAuraImpl(u32 uid, f32 mul)
        : MultiplicativeAura<Stats::Type::Strength>(uid, mul) {
    }
    const char *name() const override {
        return "MultiplicativeStrengthAura";
    }
};

class MultiplicativeAgilityAuraImpl : public MultiplicativeAura<Stats::Type::Agility> {
public:
    MultiplicativeAgilityAuraImpl(u32 uid, f32 mul)
        : MultiplicativeAura<Stats::Type::Agility>(uid, mul) {
    }
    const char *name() const override {
        return "MultiplicativeAgilityAura";
    }
};

TEST_F(UnitTests, Game_Character_Buffs) {
    const i32 baseStr = 1;
    const i32 baseAgi = 3;
    const i32 baseInt = 5;
    Stats baseStats = { baseStr, baseAgi, baseInt };
    baseStats.computeStats();

    Character character { 0, "Edmond", baseStats };
    const Stats &stats = character.stats();

    // Update the character to handle dirty states
    // and compute combat stats based on attributes.
    character.processDirty();

    // No buffs so character stats == baseStats
    EXPECT_EQ(stats, baseStats);

    // Create an additive strength buff
    astl::shared_ptr<AdditiveStrengthAuraImpl> addStrAura = astl::make_shared<AdditiveStrengthAuraImpl>(0, 1);
    EXPECT_STREQ(addStrAura->name(), "AdditiveStrengthAura");
    EXPECT_EQ(addStrAura->uid(), 0u);

    // Apply buff
    character.applyAura(character, addStrAura);

    // Changes not yet applied!
    EXPECT_EQ(stats.get(Stats::Type::Strength), baseStr);
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr);
    EXPECT_EQ(stats.computed(Stats::Type::MaxHitPoints), stats.computed(Stats::Type::Strength) * 10);

    // Apply changes with an update
    character.processDirty();

    // Check for changes
    EXPECT_EQ(stats.get(Stats::Type::Strength), baseStr);
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr + addStrAura->additive());
    EXPECT_EQ(stats.computed(Stats::Type::MaxHitPoints), stats.computed(Stats::Type::Strength) * 10);

    // Create a multiplicative strength buff
    astl::shared_ptr<MultiplicativeStrengthAuraImpl> mulStrAura = astl::make_shared<MultiplicativeStrengthAuraImpl>(4, 1.0f);
    EXPECT_STREQ(mulStrAura->name(), "MultiplicativeStrengthAura");
    EXPECT_EQ(mulStrAura->uid(), 4u);

    // Apply buff
    character.applyAura(character, mulStrAura);

    // Create a multiplicative agility buff
    astl::shared_ptr<MultiplicativeAgilityAuraImpl> mulAgiAura = astl::make_shared<MultiplicativeAgilityAuraImpl>(8, 1.0f);
    EXPECT_STREQ(mulAgiAura->name(), "MultiplicativeAgilityAura");
    EXPECT_EQ(mulAgiAura->uid(), 8u);

    // Apply buff
    character.applyAura(character, mulAgiAura);

    // Changes not yet applied!
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr + addStrAura->additive());
    EXPECT_EQ(stats.computed(Stats::Type::Agility), baseAgi);

    // Apply all changes with an update
    character.processDirty();

    // Check changes
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr * (1.0f + mulStrAura->multiplier()) + addStrAura->additive());
    EXPECT_EQ(stats.computed(Stats::Type::Agility), baseAgi * (1.0f + mulAgiAura->multiplier()));

    // Expire the additive strength aura
    character.expireAura(addStrAura.get());

    // Unchanged until update
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr * (1.0f + mulStrAura->multiplier()) + addStrAura->additive());

    // Apply change!
    character.processDirty();

    // Additive changes removed
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr * (1.0f + mulStrAura->multiplier()));

    // Expire multiplicative strength aura
    character.expireAura(mulStrAura.get());

    // Unchanged until update
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr * (1.0f + mulStrAura->multiplier()));
    EXPECT_EQ(stats.computed(Stats::Type::MaxHitPoints), stats.computed(Stats::Type::Strength) * 10);

    // Apply change
    character.processDirty();

    // All strength changes removed
    EXPECT_EQ(stats.computed(Stats::Type::Strength), baseStr);
    EXPECT_EQ(stats.computed(Stats::Type::MaxHitPoints), stats.computed(Stats::Type::Strength) * 10);

    // Expire multiplicative agility aura
    character.expireAura(mulAgiAura.get());

    // Unchanged until update
    EXPECT_EQ(stats.computed(Stats::Type::Agility), baseAgi * (1.0f + mulAgiAura->multiplier()));

    // Apply change
    character.processDirty();

    // All agility changes removed
    EXPECT_EQ(stats.computed(Stats::Type::Agility), baseAgi);
}

}; }; // namespace spark::tests
