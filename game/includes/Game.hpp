#pragma once
#include <Types.hpp>
#include <GameGrid.hpp>

namespace spark {
using namespace common;
namespace game {

class Game {
public:
    Game();
    ~Game();
    bool startup();
    bool shutdown();
    void update(u64);
};

}
}
