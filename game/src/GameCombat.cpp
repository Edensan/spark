#include <GameCombat.hpp>
#include <GameObject.hpp>

namespace spark {
namespace game {

constexpr u32 kInvalidPartyIndex = astl::numeric_limits<u32>::max() - 1;

void Party::logicUpdate(u8 logicCycle) {
    for (auto it : members_) {
        it->logicUpdate(logicCycle);
    }
}

bool Party::addMember(GameObject *go) {
    Party *prevParty = go->currentParty();
    if (prevParty == this) {
        return false;
    }
    if (prevParty) {
        prevParty->removeMember(go);
    }

    // TODO: edmond
    // Potentially could require existing members to vote
    // for new applications, with a majority vote meaning
    // acceptance.
    members_.push_back(go);
    go->currentParty_ = this;
    go->onPartyEntered(*this);

    if (playingTurn_) {
      go->activate();
    }
    return true;
}

void Party::removeMember(GameObject *go) {
    if (go->currentParty_ != this) {
        return;
    }
    go->currentParty_ = nullptr;
    skFindErase(members_, go);
    go->onPartyLeft(*this);
}

bool Party::isMember(const GameObject &go) const {
    return go.currentParty() == this;
}

bool Party::enterCombat(Combat *combat) {
    if (currentCombat_) {
        skLogE("Party::enterCombat: Already in combat!");
        return false;
    }

    currentCombat_ = combat;
    return true;
}

void Party::leaveCombat() {
    if (!currentCombat_) {
        skLogE("Party::leaveCombat: Not in combat!");
        return;
    }

    currentCombat_ = nullptr;
}

void Party::beginTurn() {
    if (!playingTurn_) {
        playingTurn_ = true;
        skLoopIt(it, members_) {
            GameObject *member = *it;
            member->activate();
        }
    }
}
void Party::endTurn() {
    if (playingTurn_) {
        playingTurn_ = false;
        skLoopIt(it, members_) {
            GameObject *member = *it;
            member->deactivate();
        }
    }
}

Combat::Combat() :
    activeParty_(kInvalidPartyIndex) {
}

Combat::~Combat() {
}

void Combat::addParty(Party *party) {
    if (party->currentCombat()) {
        skLogE("Combat::addParty: Party already in combat!");
        return;
    }

    parties_.push_back(party);
    party->enterCombat(this);
}

void Combat::removeParty(Party *party) {
    if (!party->currentCombat()) {
        skLogE("Combat::removeParty: Party not in combat!");
        return;
    }
    if (party->currentCombat() != this) {
        skLogE("Combat::removeParty: Party in another combat!");
        return;
    }

    skLoopIt(it, parties_) {
        if ((*it) == party) {
            parties_.erase(it);
            party->leaveCombat();
            break;
        }
    }
}

u32 Combat::partyCount() const {
    return parties_.size();
}

void Combat::enterState() {
    if (activeParty_ != kInvalidPartyIndex) {
        skLogE("Combat::enterState: Already in combat!");
        return;
    }
    logicUpdate(1); // Will wrap to 0
}

void Combat::leaveState() {
    activeParty_ = kInvalidPartyIndex;
    parties_.clear();
}

void Combat::nextParty(u8 logicCycle) {
    logicUpdate(logicCycle);
}

void Combat::logicUpdate(u8 logicCycle) {
    if (activeParty_ < parties_.size()) {
        Party *prevParty = !parties_.empty() ? parties_[activeParty_] : nullptr;
        prevParty->endTurn();
    }

    if (++activeParty_ >= parties_.size()) {
        activeParty_ = 0u;
        ++turn_;
    }

    Party *nextParty = !parties_.empty() ? parties_[activeParty_] : nullptr;
    nextParty->beginTurn();
    nextParty->logicUpdate(logicCycle);
}

Party *Combat::currentParty() const {
    return activeParty_ < parties_.size() ? parties_[activeParty_] : nullptr;
}

} // namespace game
} // namespace spark
