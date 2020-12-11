#pragma once

#include "box_base.hpp"

template<typename dtype>
class Graph_Box_3D : Box_Base{
    typedef int32_t dimen_t;

#ifndef GRAPH_MAT_NO_COORD
    util::_coord<dimen_t> coordinate;
    void setCoord(util::_coord<dimen_t>&& new_coord);
#endif

public:
    Graph_Box_3D* get_box() override{ return this; }    // @self - no need to specify templates here,since compiler can deduce from return statement (if doing definitions directly inside class)
    const Graph_Box_3D* get_box() const override { return this; }

    inline Graph_Box_3D* get_adj_box(Direction dir) const override {
        switch (dir)
        {
        case Direction::UTTAR:
            return this->UP;
        case Direction::PURVA:
            return this->RIGHT;
        case Direction::PASHCHIM:
            return this->LEFT;
        case Direction::DAKSHIN:
            return this->DOWN;
        case Direction::ISHANYA:
            return (!this->UP) ? nullptr : this->UP->RIGHT;
        case Direction::AGNEYA:
            return (!this->DOWN) ? nullptr : this->DOWN->RIGHT;
        case Direction::NAIRUTYA:
            return (!this->DOWN) ? nullptr : this->DOWN->LEFT;
        case Direction::VAYAVYA:
            return (!this->UP) ? nullptr : this->UP->LEFT;
        case Direction::URDHWA:
            return this->FRONT_FACING;
        case Direction::ADHARASTHA:
            return this->BACK_FACING;
        }
    }

    Graph_Box_3D() : Graph_Box_3D(std::move(dtype{})) {}

#ifndef GRAPH_MAT_NO_COORD
    Graph_Box_3D(dimen_t x, dimen_t y, dimen_t z) : coordinate(x,y,z) {
        this->RIGHT = nullptr;
        this->LEFT = nullptr;
        this->UP = nullptr;
        this->DOWN = nullptr;
        this->FRONT_FACING = nullptr;
        this->BACK_FACING = nullptr;
    }
#endif // !GRAPH_MAT_NO_COORD

    Graph_Box_3D(dtype&& data) : data(data) {
        this->RIGHT = nullptr;
        this->LEFT = nullptr;
        this->UP = nullptr;
        this->DOWN = nullptr;
        this->FRONT_FACING = nullptr;
        this->BACK_FACING = nullptr;
    }

    Graph_Box_3D(const Graph_Box_3D&) = delete;
    Graph_Box_3D(const Graph_Box_3D&&) = delete;

    friend std::ostream& operator<<(std::ostream& os, const Graph_Box_3D& box) {
        os << box.getData();
        return os;
    }

    const dtype& getData() const { return this->data; }

protected:

    dtype data;    /*This has been given as an extension, so that you can add more variables to the graph_box
                        though, note that, you will be able to access using this->data->your_var_name */
                        //        coord_type coords;  // @NOTE - not actually needed, though this maybe used in my implementation of snake
                                // @todo - I am removing the functionality of having a coord in each graph node, in an attempt to make it as small as sufficient

    Graph_Box_3D* RIGHT{nullptr};
    Graph_Box_3D* LEFT{ nullptr };
    Graph_Box_3D* UP{ nullptr };
    Graph_Box_3D* DOWN{ nullptr };
    Graph_Box_3D* FRONT_FACING{ nullptr };
    Graph_Box_3D* BACK_FACING{ nullptr };

    // LEARNT - friending a templated class (down below is the `syntax` found, to friend `all` templated versions of Graph_Matrix)
    template<typename, typename> friend class Graph_Matrix_3D;    //will make all Graph_Matrix friend to this
    //friend class Graph_Matrix<int, int32_t>;    //will make all Graph_Matrix friend to this
    friend class WorldPlot; // This line maybe removed later, it is worldLine Simulator specific

};

#ifndef Graph_MAT_NO_COORD


template<typename dtype>
inline void Graph_Box_3D<dtype>::setCoord(util::_coord<dimen_t>&& new_coord)
{
    this->coordinate = std::move(new_coord);
}
#endif // !Graph_MAT_NO_COORD
