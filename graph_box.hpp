#pragma once

#include <ostream>
#include <vector>
#include <utility>

#include "box_base.hpp"
#include "direction.hpp"

template< typename node_dtype >
struct Graph_Box    // @node - it should be non-copyable
    : Box_Base
{
    typedef int32_t dimen_t;

    // @note @future -> Have this coordinate in DEBUG builds, as well as the constructor taking the dimensions in debug builds
    util::_coord<dimen_t> coordinate;

    public:
        Graph_Box* get_box(){ return this; }
        const Graph_Box* get_box() const{ return this; }
        inline Graph_Box* get_adj_box( Direction dir ) const{
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
            default:
                return nullptr; // when direction is skywards or downwards
            }
        }

        Graph_Box() : Graph_Box(std::move(node_dtype{})) {}

        template<typename NODE_DTYPE = node_dtype>
        Graph_Box(node_dtype&& data, typename std::enable_if_t): data(data){
            this->RIGHT = nullptr;
            this->LEFT = nullptr;
            this->UP = nullptr;
            this->DOWN = nullptr;
        }

        Graph_Box(const Graph_Box<node_dtype>&) = delete;
        Graph_Box(const Graph_Box<node_dtype>&&) = delete;

        friend std::ostream& operator<<( std::ostream& os, const Graph_Box& box){
            os << box.getData();
            return os;
        }

        const node_dtype& getData() const{ return this->data; }

//        bool is_in_use{ false };    // the internal box item will only be created when this graph_box is in use
//        mutable int8_t counter{5};  // 5 = number of seconds for which inactive nodes will have these removed

    protected:

        node_dtype data;    /*This has been given as an extension, so that you can add more variables to the graph_box
                            though, note that, you will be able to access using this->data->your_var_name */

        Graph_Box* RIGHT;
        Graph_Box* LEFT;
        Graph_Box* UP;
        Graph_Box* DOWN;

        // LEARNT - friending a templated class (down below is the `syntax` found, to friend `all` templated versions of Graph_Matrix)
        template<typename, typename> friend class Graph_Matrix;    //will make all Graph_Matrix friend to this
        //friend class Graph_Matrix<int, int32_t>;    //will make all Graph_Matrix friend to this
};
