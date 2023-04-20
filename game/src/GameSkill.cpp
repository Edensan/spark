#include <GameSkill.hpp>
#include <GameObject.hpp>

namespace spark {
namespace game {

Skill::Skill(Bundle bundle)
    : dataBundle_(bundle) {
}

void Skill::triggerCooldown() {
    if (cooldown_ == 0)
        cooldown_ = dataBundle_.baseCooldown;
}

u8 Skill::cooldown() const {
    return cooldown_;
}

u8 Skill::cost() const {
    return dataBundle_.cost;
}

u32 Skill::range() const {
    return dataBundle_.range;
}

u32 Skill::id() const {
    return dataBundle_.id;
}

void Skill::logicUpdate(u8 delta) {
    if (cooldown_ < delta) {
        cooldown_ = 0;
    }
    else {
        cooldown_ -= delta;
    }
    switch (state_) {
    case State::Casting: {
        if (timeUntilCast_ <= delta) {
            timeUntilCast_ = 0;
            cast();
        }
        else {
            timeUntilCast_ -= delta;
        }
        break;
    }
    case State::Resolving: {
        if (timeUntilResolveCast_ <= delta) {
            timeUntilResolveCast_ = 0;
            resolveCast();
        }
        else {
            timeUntilResolveCast_ -= delta;
        }
        break;
    }
    case State::Idle: {
        break;
    }
    case State::Done: {
        timeUntilCast_ = 0;
        timeUntilResolveCast_ = 0;
        state_ = State::Idle;
        break;
    }
    }
}

Skill::CastError Skill::queryCast(GameObject *src
                                  , const Skill::Effect &eff
                                  , PositionI pos
                                  , const astl::function<void()> &&callbackDone
                                  , const astl::shared_ptr<Skill::Params> params) {
    if (state_ == State::Casting) {
        return CastError::AlreadyCasting;
    }

    if (cooldown_ > 0) {
        return CastError::OnCooldown;
    }

    callbackDone_ = callbackDone;
    const ResolutionInfo info { src, eff, pos };
    beginCast(info, params);
    return CastError::OK;
}

Skill::CastError Skill::canCast(const astl::shared_ptr<Skill::Params>) const {
    if (state_ == State::Casting) {
        return CastError::AlreadyCasting;
    }
    else if (cooldown_ > 0) {
        return CastError::OnCooldown;
    }
    else {
        return CastError::OK;
    }
}

void Skill::beginCast(const ResolutionInfo &resolutionInfo, const astl::shared_ptr<Skill::Params> params) {
    state_ = State::Casting;
    timeUntilCast_ = onBeginCast(resolutionInfo, params);
    if (timeUntilCast_ == 0) {
        cast();
    }
}

void Skill::cast() {
    state_ = State::Resolving;
    timeUntilResolveCast_ = onCast();
    if (timeUntilResolveCast_ == 0) {
        resolveCast();
    }
}

void Skill::resolveCast() {
    state_ = State::Done;
    onResolveCast();
    callbackDone_();
}

void Skill::delay(u8 delay) {
    if (state_ == State::Casting) {
        timeUntilCast_ += delay;
        onDelayedCast(delay);
    }
}

void Skill::interrupt() {
    if (state_ > State::Done) {
        state_ = State::Done;
        onInterruptedCast();
        callbackDone_();
    }
}

} };
