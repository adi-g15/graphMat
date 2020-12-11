#pragma once

#include "direction.hpp"

class Box_Base {
    // @note @future -> Have this coordinate in DEBUG builds, as well as the constructor taking the dimensions in debug builds
    virtual Box_Base* get_box() { return this; }
    virtual const Box_Base* get_box() const { return this; }
    virtual Box_Base* get_adj_box(Direction dir) const = 0;
};
