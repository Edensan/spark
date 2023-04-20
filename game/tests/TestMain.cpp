#include "TestMain.hpp"

#include <GameStats.hpp>

namespace spark {
namespace game {
} // namespace game
} // namespace spark

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
