#include "TestMain.hpp"
#include <GameGrid.hpp>
#include <GameObject.hpp>

namespace spark {
using namespace common::math;
using namespace game;
namespace tests {

#define gw 32
#define gh 32

enum class CellType : u32 {
    Ground = 0,
    GroundDirt = Ground + 1,
    GroundMud = Ground + 2,
    GroundGrass = Ground + 3,
    GroundSand = Ground + 4,
    GroundRock = Ground + 5,
    GroundIce = Ground + 6,
    LastGround = Ground + 32,

    Water = LastGround + 1,
    WaterPond = Water + 1,
    WaterLake = Water + 2,
    WaterRiver = Water + 3,
    WaterSea = Water + 4,
    LastWater = Water + 32,

    Block = LastWater + 1,
    BlockTree = Block + 1,
    BlockStone = Block + 2,
    BlockMisc = Block + 3,
    LastBlock = Block + 32,

    Danger = LastBlock + 1,
    DangerFire = Danger + 1,
    DangerLava = Danger + 2,
    DangerAcid = Danger + 3,
    LastDanger = Danger + 32,

    Invalid = astl::numeric_limits<u16>::max()
};

static u32 initTypeFuncGround(const PositionI &) {
    return static_cast<u32>(CellType::Ground);
}

inline bool isGround(const GameGrid::Cell &c) {
    return c.type >= static_cast<u32>(CellType::Ground) && c.type < static_cast<u32>(CellType::LastGround);
}
inline bool isWater(const GameGrid::Cell &c) {
    return c.type >= static_cast<u32>(CellType::Water) && c.type < static_cast<u32>(CellType::LastWater);
}
inline bool isDangerous(const GameGrid::Cell &c) {
    return c.type >= static_cast<u32>(CellType::Danger) && c.type < static_cast<u32>(CellType::LastDanger);
}
inline bool isBlocked(const GameGrid::Cell &c) {
    return c.type >= static_cast<u32>(CellType::Block) && c.type < static_cast<u32>(CellType::LastBlock);
}
inline bool isInvalid(const GameGrid::Cell &c) {
    return c.type == static_cast<u32>(CellType::Invalid);
}

static constexpr u32 kErrorCodeRange = 256u;
enum class ErrorCode : u32 {
    OK = 0,

    BeginWarning = OK + 1,
    WarningUnchanged = BeginWarning + 1,
    WarningNotGround = BeginWarning + 2,
    EndWarning = BeginWarning + kErrorCodeRange,

    BeginError = EndWarning + 1,
    ErrorOutOfRange = BeginError + 1,
    ErrorWrongGrid = BeginError + 2,
    ErrorInvalid = BeginError + 3,
    ErrorBlocked = BeginError + 4,
    ErrorOccupied = BeginError + 5,
    EndError = BeginError + kErrorCodeRange,

};

class MoveValidatorImpl : public GameGrid::MoveValidator {
public:
    MoveValidatorImpl()
        : GameGrid::MoveValidator() {
    }

    virtual u32 validateMove(GameGrid *gg, GameGrid::Listener *ggl, const PositionI &p) override {
        if (p.x() >= (i32)gg->size().w() || p.y() >= (i32)gg->size().h()) {
            // Out of range
            return static_cast<u32>(ErrorCode::ErrorOutOfRange);
        }
        if (ggl->position() == p) {
            // Didn't change
            return static_cast<u32>(ErrorCode::WarningUnchanged);
        }
        const GameGrid::Cell *c = gg->cellAt(p);
        return validateMove(gg, ggl, *c);
    }

    u32 validateMove(GameGrid *gg, GameGrid::Listener *ggl, const GameGrid::Cell &c) {
        GameGrid *cgg = ggl->currentGrid();
        if (cgg && cgg != gg) {
            return static_cast<u32>(ErrorCode::ErrorWrongGrid);
        }
        if (isInvalid(c)) {
            return static_cast<u32>(ErrorCode::ErrorInvalid);
        }
        if (isBlocked(c)) {
            return static_cast<u32>(ErrorCode::ErrorBlocked);
        }
        if (c.data) {
            if (c.data == ggl) {
                return static_cast<u32>(ErrorCode::WarningUnchanged);
            }
            return static_cast<u32>(ErrorCode::ErrorOccupied);
        }
        if (!isGround(c)) {
            return static_cast<u32>(ErrorCode::WarningNotGround);
        }
        return static_cast<u32>(ErrorCode::OK);
    }

    virtual bool isOK(u32 err) override {
        return err == static_cast<u32>(ErrorCode::OK);
    }
    virtual bool isWarning(u32 err) override {
        return
            err >= static_cast<u32>(ErrorCode::BeginWarning)
            && err <= static_cast<u32>(ErrorCode::EndWarning);
    }
    virtual bool isError(u32 err) override {
        return
            err >= static_cast<u32>(ErrorCode::BeginError)
            && err <= static_cast<u32>(ErrorCode::EndError);
    }
};

static u32 initTypeFuncMisc(const PositionI &p) {
    // Blocked off grid
    if (p.x() == 0
        || p.y() == 0
        || p.x() == (gw-1)
        || p.y() == (gh-1)) {
        return static_cast<u32>(CellType::BlockTree);
    }
    // Additional Fire border
    else if (p.x() == 1
             || p.y() == 1
             || p.x() == (gw-2)
             || p.y() == (gh-2)) {
        return static_cast<u32>(CellType::DangerFire);
    }
    // Additional Water border
    else if (p.x() == 2
             || p.y() == 2
             || p.x() == (gw-3)
             || p.y() == (gh-3)) {
        return static_cast<u32>(CellType::WaterRiver);
    }
    // Everyting else is healthy Grass
    else {
        return static_cast<u32>(CellType::GroundGrass);
    }
}

TEST_F(UnitTests, Game_GameGrid_Init) {
    {
        GameGrid gg { { gw, gh }, astl::make_shared<MoveValidatorImpl>(), initTypeFuncGround};
        EXPECT_EQ(gg.area(), (gw * gh));

        PositionI p;
        skLoop(x, gw) {
            p.x() = x;
            skLoop(y, gh) {
                p.y() = y;
                const GameGrid::Cell *c = gg.cellAt(p);
                EXPECT_FALSE(isInvalid(*c));
                EXPECT_TRUE(isGround(*c));
            }
        }
    }
    {
        astl::shared_ptr<MoveValidatorImpl> validator = astl::make_shared<MoveValidatorImpl>();
        DummyGameObject go { 0, "Test" };
        EXPECT_EQ(go.position(), PositionI::undefined());

        GameGrid gg { { gw, gh }, validator, initTypeFuncMisc};
        EXPECT_EQ(gg.area(), (gw * gh));

        PositionI p;
        skLoop(x, gw) {
            p.x() = x;
            skLoop(y, gh) {
                p.y() = y;
                const GameGrid::Cell *c = gg.cellAt(p);
                EXPECT_EQ(static_cast<u32>(c->type), initTypeFuncMisc(p));
            }
        }

        // Checking for errors.
        const u32 err = gg.move(&go, { 0, 0 });
        EXPECT_EQ(err, static_cast<u32>(ErrorCode::ErrorBlocked));
        EXPECT_TRUE(validator->isError(err));
        EXPECT_FALSE(validator->isWarning(err));
        EXPECT_FALSE(validator->isOK(err));
        EXPECT_FALSE(go.currentGrid());
        EXPECT_EQ(go.position(), PositionI::undefined());
        EXPECT_EQ(gg.move(&go, { gw-1, gh-1 }), static_cast<u32>(ErrorCode::ErrorBlocked));
        EXPECT_FALSE(go.currentGrid());
        EXPECT_EQ(go.position(), PositionI::undefined());
        EXPECT_EQ(gg.move(&go, { gw-1, 0 }), static_cast<u32>(ErrorCode::ErrorBlocked));
        EXPECT_FALSE(go.currentGrid());
        EXPECT_EQ(go.position(), PositionI::undefined());
        EXPECT_EQ(gg.move(&go, { 0, gh-1 }), static_cast<u32>(ErrorCode::ErrorBlocked));
        EXPECT_FALSE(go.currentGrid());
        EXPECT_EQ(go.position(), PositionI::undefined());
        EXPECT_EQ(gg.move(&go, { gw, gh }), static_cast<u32>(ErrorCode::ErrorOutOfRange));
        EXPECT_FALSE(go.currentGrid());
        EXPECT_EQ(go.position(), PositionI::undefined());

        // Checking for warnings.
        // GameGrid::validateMove checks the status of a potential move
        // without actually moving the gameObject.
        const u32 preWarn = validator->validateMove(&gg, &go, { 1, 1 });
        EXPECT_EQ(go.currentGrid(), nullptr);
        EXPECT_EQ(go.position(), PositionI::undefined());

        // GameGrid::move should have same result as the previous
        // GameGrid::validateMove while actually moving the gameObject.
        u32 warn = gg.move(&go, { 1, 1 });
        EXPECT_EQ(warn, preWarn);
        EXPECT_EQ(warn, static_cast<u32>(ErrorCode::WarningNotGround));
        EXPECT_TRUE(validator->isWarning(warn));
        EXPECT_FALSE(validator->isError(warn));
        EXPECT_FALSE(validator->isOK(warn));
        EXPECT_EQ(go.currentGrid(), &gg);
        EXPECT_EQ(go.position(), PositionI(1, 1));
        warn = gg.move(&go, { 2, 2 });
        EXPECT_EQ(warn, static_cast<u32>(ErrorCode::WarningNotGround));
        EXPECT_FALSE(validator->isOK(warn));
        EXPECT_FALSE(validator->isError(warn));
        EXPECT_FALSE(validator->isOK(warn));
        EXPECT_EQ(go.currentGrid(), &gg);
        EXPECT_EQ(go.position(), PositionI(2, 2));
        warn = gg.move(&go, { 2, 2 });
        EXPECT_EQ(warn, static_cast<u32>(ErrorCode::WarningUnchanged));
        EXPECT_FALSE(validator->isOK(warn));
        EXPECT_FALSE(validator->isError(warn));
        EXPECT_FALSE(validator->isOK(warn));
        EXPECT_EQ(go.currentGrid(), &gg);
        EXPECT_EQ(go.position(), PositionI(2, 2));

        // Checking for an "OK" move.
        const u32 ok = gg.move(&go, { 3, 3 });
        EXPECT_TRUE(validator->isOK(ok));
        EXPECT_FALSE(validator->isError(ok));
        EXPECT_FALSE(validator->isWarning(ok));
        EXPECT_EQ(go.currentGrid(), &gg);
        EXPECT_EQ(go.position(), PositionI(3, 3));

        // Leaving the grid.
        gg.leave(&go);
        EXPECT_EQ(go.currentGrid(), nullptr);
        EXPECT_EQ(go.position(), PositionI::undefined());
    }
}

};
};
