#pragma once
#include <Types.hpp>
#include <MathTypes.hpp>

#include <niLang/STL/vector.h>
#include <niLang/STL/memory.h>
#include <niLang/STL/utils.h>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

// NOTE: 2 possibilities when generating grids,
// a) Dynamically allocate the grid at run-time
// b) Use templating to generate grid at compile-time (using config?)
class GameGrid {
private:
    static constexpr u64 kFrameTimePerTurn = 1000000000ULL;

public:
    class Listener {
    public:
        inline GameGrid *currentGrid() const { return grid_; }
        virtual void setSize(SizeU) = 0;
        virtual SizeU size() const = 0;
        virtual void setPosition(PositionI) = 0;
        virtual PositionI position() const = 0;
        virtual void onGridEntered(GameGrid *) = 0;
        virtual void onGridLeft(GameGrid *) = 0;

        // ABSTRACT
        virtual void onGridMoveRejected(u32) = 0;
        virtual void onGridMoved(PositionI, u32) = 0;

    private:
        GameGrid *grid_ = nullptr;
        friend class GameGrid;
    };

    struct Cell {
        u32 type;
        Listener *data;
    };

    class MoveValidator {
    public:
        MoveValidator() = default;
        virtual ~MoveValidator() {}

        virtual u32 validateMove(GameGrid *, Listener *, const PositionI &) = 0;
        virtual bool isOK(u32 err) = 0;
        virtual bool isWarning(u32 err) = 0;
        virtual bool isError(u32 err) = 0;
    };

    typedef astl::function<u32(const PositionI &)> initTypeFunc;

    GameGrid(SizeU, astl::shared_ptr<MoveValidator>, initTypeFunc);
    virtual ~GameGrid();

    void logicUpdate(u8);
    const Cell *cellAt(const PositionI &) const;
    bool leave(Listener *);
    u32 move(Listener *, const PositionI &);
    u32 area() const;
    SizeU size() const { return size_; }

    MoveValidator *validator() const {
        return validator_.get();
    }

    const astl::vector<Listener *> &listeners() {
        return listeners_;
    }

private:
    astl::shared_ptr<MoveValidator> validator_;
    astl::vector<astl::vector<Cell>> grid_;
    astl::vector<Listener *> listeners_;
    u64 timeUntilNextTurn_ = kFrameTimePerTurn;
    SizeU size_;
};

}; }; // namespace spark::game
