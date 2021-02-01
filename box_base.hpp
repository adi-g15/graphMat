#pragma once

#include "direction.hpp"

class Box_Base {
    // Edit: 26th Jan, modifying these from virtual to non-virtual methods
    //Box_Base* get_adj_box(Direction dir) const;

protected:
    Box_Base() {}
    ~Box_Base() {}
};
