#pragma once

/* @NOTE- 
 * This class is originally meant to be inherited to from, to provide implementations as needed per your needs
 * 
 * @NOTE - Regarding how the graph is stored (maybe slighlty confusing at beginning) ->
 *      Internally all nodes are stored and given coordinate values as per the usual x-y coordinate system
 * 
 *      But, for the user side, the main functions exposed are -> the constructor taking arguments in form of row*col
 *                                                                and, the resize function taking arguments in form of row*col
 *      The thing to understand here is, that the user is entering according to the usual matrix size (row*column form),
 *          but the coordinates won't be like that, for example, going down, the x_coord as in matrix won't increase, the y will increase, same for y coordinates 
*/

#include <iostream> // use at displaing matrix
#include <cstdint>
#include <utility>
#include <list>
// #include <concepts>

#include "../../matrix_base.hpp"
#include "graph_box.hpp"
#include <util/coord.hpp>

    // @limitation (wrt vector<vector>) - Currently you can't get back the nth row, say mat[4], isn't valid, though can be implemented by simply returning, mat[4][0] which will logically be a linked list

// dimen_t will be the unit of dimension
template< typename node_dtype, typename dimen_t=int32_t> // @concepts -> make sure that std::is_signed<dimen_t> == true
class Graph_Matrix: Matrix_Base{
    static_assert(std::is_signed_v<dimen_t> == true, "Dimension type must be a signed integral (for safety reasons, so that bugs don't appear due to unsigned subtractions)");
    static_assert(std::is_default_constructible<node_dtype>::value == true, "The data type of values in the matrix must be default constructible");
    static_assert(std::is_pointer<node_dtype>::value != true, "Currently it doesn't support using pointers to be the data held by each box, though it maybe implemented, but not now");

    typedef Graph_Box<node_dtype> graph_box_type;
    typedef std::make_unsigned_t<dimen_t> udimen_t;
    typedef util::_coord<int32_t> coord_type;
    // typedef int32_t dimen_t;  //dimension unit

protected:
    std::list<graph_box_type*> special_nodes;   // special_nodes.begin() will always be the techincal ROOT (ORIGIN, 0,0,0)
    graph_box_type origin; // [0,0]; will never change

        //pointers to the 3 nodes
    graph_box_type* tl_box;  // top_left
    graph_box_type* bl_box;  // bottom_left
    graph_box_type* tr_box;  // bottom_right
    graph_box_type* br_box;  // bottom_right

    struct {
        graph_box_type* origin;
        graph_box_type* tl_box;  // top_left
        graph_box_type* bl_box;  // bottom_left
        graph_box_type* br_box;  // bottom_right
        graph_box_type* tr_box;  // bottom_right
        dimen_t _total_y_abs;
        dimen_t _total_x_abs;
    } __capacity;   //capacity data

    dimen_t x_min, x_max;
    dimen_t y_min, y_max;

    // @warning - May later change it as well to `dimen_t`, since `if _total_y_abs and _N_cols are used in substraction, it may well cause `wrapping around of the unsigned``

    // these ..._abs are basically ..._max - ..._min
    udimen_t _total_y_abs;   // originally it was named `_N_Rows`, but the name has been changed to adopt to the usual x-y coordinate system, AND for each box to have coordinates, which required updation of all coordinates, like if a row is added, origin(0,0) shifted downwards, so all other coords should change as well
    udimen_t _total_x_abs;  // originally names `_N_Cols`

    void popCol_left(); // remove the leftmost column
    void popRow_upper(); // remove the upmost row

    void pushCol(); // add a column at end
    void pushRow(); // add a row at end
    void injectCol(); // add a column at begin
    void injectRow(); // add a row at begin

    void popCol(); // removes a column, preferably rightmost, if not calls popCol_left()
    void popRow(); // removes a row, preferably downmost, if not calls popRow_upper()

public:
        // @deprecated+incomplete -> use util::range_iterator for now
        // @note - util::range_iterator has ben postponed for now, will be implemented as needed
    //bool forRange(graph_box_type* iteratee); //returns false when iterated all of that of that row

    virtual std::pair< udimen_t, udimen_t > getDimensions() const;  // returns in form of 

    auto getNumRows() const;
    auto getNumCols() const;

    template < typename _Func >
    void for_each(graph_box_type* source, Direction, _Func);	// func will receive only one param, ie. the node_dtype data

    template<typename _Func>
    void for_each(graph_box_type* begin, graph_box_type* end, Direction dir, _Func func);

    //template < typename _Cond, typename _Func >	// though this is an idea, but doesn't seem of much help
    //void for_each(_Cond condition_function_takes_data_returns_direction_to_move, _Func);	// func will receive only one param, ie. the node_dtype data


        // @note to viewer -> You can express your view on whether we should prefer simple [x][y] for position or the graph_position typedefed in graph_box.hpp
    graph_box_type* operator[](const coord_type&);
    const graph_box_type* operator[](const coord_type&) const;
    graph_box_type* operator[](const graph_position& pos);
    const graph_box_type* operator[](const graph_position& pos) const;

    // @future - These two to be implemented later and should utilize this->__capacity
    // void reserve(int _x_dimen, int _y_dimen);
    // void shrink_to_fit();

    // virtual void resize(dimen_t x_dimen, dimen_t y_dimen, Direction a, ...);   // @future - resize with variadic arguments, that takes directions after the new dimensions
    void resize(dimen_t _num_rows, dimen_t _num_cols);

    void displayMat(std::ostream& output_stream = std::cout) const;

    Graph_Matrix();
    Graph_Matrix(dimen_t _num_rows, dimen_t _num_cols);
    ~Graph_Matrix();
};
