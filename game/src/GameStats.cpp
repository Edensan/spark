#include <GameStats.hpp>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

void Stats::resetAll() {
     for (Value &s : stats) {
        s.reset();
    }
    markDirty();
}

void Stats::computeMaxHitPoints() {
    // Give out enough HP to sustain at least 10 basic hits
    set(Type::MaxHitPoints,
        (computed(Type::Strength)
         * 10));
}

void Stats::computeAttackPower() {
    // Give out enough HP to sustain at least 10 basic hits
    set(Type::AttackPower,
        (computed(Type::Strength)));
}

void Stats::computeSpellPower() {
    // Give out enough HP to sustain at least 10 basic hits
    set(Type::SpellPower,
        (computed(Type::Intelligence)));
}

bool Stats::computeStats() {
    if (consumeDirty()) {
        computeMaxHitPoints();
        computeAttackPower();
        computeSpellPower();
        return true;
    }
    return false;
}

} }; // namespace spark::game
