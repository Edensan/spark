#include <GameGrid.hpp>
#include <Types.hpp>
#include <MathTypes.hpp>
#include <GameObject.hpp>
#include <niLang/STL/vector.h>

namespace spark {
using namespace common;
using namespace common::math;
namespace game {

GameGrid::GameGrid(SizeU gridDimensions, astl::shared_ptr<MoveValidator> validator, initTypeFunc initFunc)
    : validator_(validator)
    , size_(gridDimensions) {
    const u32 w = gridDimensions.w();
    const u32 h = gridDimensions.h();
    grid_.resize(h);
    PositionI p;
    skLoop(y, h) {
        astl::vector<Cell> &row = grid_[y];
        row.resize(w);
        p.y() = y;
        skLoop(x, w) {
            p.x() = x;

            // Define all cell types at init
            row[x].type = initFunc(p);
        }
    }
}

GameGrid::~GameGrid() {
}

u32 GameGrid::area() const {
    return size_.w() * size_.h();
}

const GameGrid::Cell *GameGrid::cellAt(const PositionI &p) const {
    const u32 x = p.x();
    const u32 y = p.y();
    if (y < grid_.size()) {
        const astl::vector<Cell> &row = grid_[y];
        if (x < row.size()) {
          const Cell &cell = row[x];
            return &cell;
        }
    }

    return nullptr;
}

bool GameGrid::leave(GameGrid::Listener *ggl) {
    if (ggl->currentGrid() == this) {
        const PositionI p = ggl->position();
        Cell *c = const_cast<Cell *>(cellAt(p));
        if (c == nullptr) {
            skUnreachable("Couldn't find cell at position(%d,%d)", p.x(), p.y());
            return false;
        }
        skFindEraseUnordered(listeners_, ggl);
        ggl->onGridLeft(this);
        ggl->setPosition(PositionI::undefined());
        ggl->grid_ = nullptr;
        c->data = nullptr;
        return true;
    }
    return false;
}

u32 GameGrid::move(GameGrid::Listener *ggl, const PositionI &p) {
    const u32 ret = validator_->validateMove(this, ggl, p);
    if (!validator_->isError(ret)) {
        if (!ggl->currentGrid()) {
            ggl->grid_ = this;
            ggl->onGridEntered(this);
            listeners_.push_back(ggl);
        }
        Cell *prevCell = const_cast<Cell *>(cellAt(ggl->position()));
        if (prevCell) {
            prevCell->data = nullptr;
        }
        const_cast<Cell *>(cellAt(p))->data = ggl;
        ggl->setPosition(p);
        ggl->onGridMoved(p, ret);
    }
    else {
        ggl->onGridMoveRejected(ret);
    }
    return ret;
}

}; }; // namespace spark::game
