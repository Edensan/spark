#include <GameAura.hpp>
#include <GameStats.hpp>
#include <objects/Character.hpp>

namespace spark {
namespace game {

void Aura::applyTo(Character *target) {
    if (target_) {
        skUnreachable("Aura::applyTo: Aura already has a target.");
        return;
    }
    target_ = target;
}

void Aura::expireFrom(Character *target) {
    if (target_ != target) {
        skUnreachable("Aura::expireFrom: Aura does not have a target.");
        return;
    }
    target_ = nullptr;
}

void Aura::logicUpdate(u8 logicCycle) {
    if (target_) {
        if (duration_ <= logicCycle) {
            duration_ = 0;
            expireFrom(target_);
        }
        else {
            duration_ -= logicCycle;
        }
    }
}

} // namespace game
} // namespace spark
