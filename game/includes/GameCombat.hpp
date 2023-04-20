#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>

namespace spark {
using namespace common;
namespace game {

class GameObject;
class Party;

class GameState {
public:
    virtual ~GameState() {}

public:
    virtual void enterState() = 0;
    virtual void leaveState() = 0;
    virtual void logicUpdate(u8) = 0;
};

// NOTE 1:
// Player-based combat sends logic updates to all game objects
// whenever the player does any meaningful action (move, cast, etc),
// meaning that the world moves at the rythm of the player.
//
// This combat mode allows for meaningful reactive game mechanics
// involving projectiles, and for this to work effectively all animations
// should be measured in logicCycles.
//
// IMPLEMENTATION:
// This can be simply implemented from the UI itself based on
// the turn-based combat by calling the combat.nextParty for each other
// party whenever a player action is requested.
//
// eg. Cadence of Hyrule
//
// NOTE 2:
// Turn-based combat sends logic updates to a party whenever
// their turn begins. Animations are resolved instantly or may
// take X turns to play out.
//
// IMPLEMENTATION:
// Implemented using a "next turn" button.
//
// eg. Divinity Original Sin, any Final fantasy below 15, etc
class Combat final : public GameState {
public:
    Combat();
    ~Combat();

    void addParty(Party *);
    void removeParty(Party *);
    u32 partyCount() const;
    Party *currentParty() const;

    void enterState() override;
    void leaveState() override;
    void nextParty(u8 logicCycles = 1u);
    u32 currentTurn() const { return turn_; }

private:
    void logicUpdate(u8) override;
    astl::vector<Party *> parties_;
    u32 activeParty_;
    u32 turn_;
};

class Party final {
public:
    Party(const char *name) : name_(name) {}
    const char *name() const { return name_.c_str(); }
    bool addMember(GameObject *);
    void removeMember(GameObject *);
    bool isMember(const GameObject &) const;
    u32 memberCount() const;

    bool enterCombat(Combat *);
    void leaveCombat();
    void logicUpdate(u8);

    void beginTurn();
    void endTurn();
    bool playingTurn() const { return playingTurn_; }

    Combat *currentCombat() const { return currentCombat_; }
protected:
    Combat *currentCombat_ = nullptr;
    astl::string name_;
    astl::vector<GameObject *> members_;
    bool playingTurn_ = false;
};

} // namespace game
} // namespace spark
