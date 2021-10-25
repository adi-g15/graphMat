#pragma once

/**
 * This header is not meant to be included directly
 */

#include "direction.hpp"

class Box_Base {
    // Edit: 26th Jan, modifying these from virtual to non-virtual methods
    // Box_Base* get_adj_box(Direction dir) const;

  protected:
    Box_Base() {}
    ~Box_Base() {}
};
