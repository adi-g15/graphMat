#pragma once

#include <iostream>	// for std::cout set as default for std::ostream
#include <cstdint>

#include "matrix_base.hpp"
#include "3d_graph_box.hpp"

/*
@notes about directions ->

The screen is considered to be parallel to the xy plane, hence DOWN, UP, LEFT, RIGHT are lines parallel the x or y axis only

And, -z axis is considered as BACK_FACING (PATAL)
     +z axis is considered as FRONT_FACING (AAKASH)
*/

enum class MatrixLayer {
	TOP,
	DEEPEST,
	ORIGIN
};

template<typename node_dtype, typename dimen_t = int32_t>
class Graph_Matrix_3D : Matrix_Base{

	typedef Graph_Box_3D<node_dtype> graph_box_type;
	typedef util::_coord<dimen_t> coord_type;

	graph_box_type origin;

	graph_box_type* top_left_front;
	graph_box_type* top_left_back;
	graph_box_type* bottom_left_front;
	graph_box_type* bottom_left_back;
	graph_box_type* top_right_front;
	graph_box_type* top_right_back;
	graph_box_type* bottom_right_front;
	graph_box_type* bottom_right_back;

	struct {
		graph_box_type* top_left_front;
		graph_box_type* top_left_back;
		graph_box_type* bottom_left_front;
		graph_box_type* bottom_left_back;
		graph_box_type* top_right_front;
		graph_box_type* top_right_back;
		graph_box_type* bottom_right_front;
		graph_box_type* bottom_right_back;

		coord_type total_abs;
	} __capacity;   //capacity data

	MemoryAlloc< Graph_Box_3D<node_dtype> >	allocator;

public:	// temporarily
	dimen_t min_x, min_y, min_z;	// ALWAYS NON-POSITIVES (since origin layer won't be removed so at max 0)
	dimen_t max_x, max_y, max_z;	// ALWAYS NON-NEGATIVES (  "      "      "    "    "   "      "  " min 0)

	coord_type total_abs;	// total span of a single dimension

	void add_x_layer(int num = 1);
	void inject_x_layer(int num = 1);
	void pop_xplus_layer();
	void pop_xminus_layer();

	void add_y_layer(int num = 1);
	void inject_y_layer(int num = 1);
	void pop_yplus_layer();
	void pop_yminus_layer();

	void add_z_layer(int num = 1);
	void inject_z_layer(int num = 1);
	void pop_zplus_layer();
	void pop_zminus_layer();

	void disp_xy_layer(MatrixLayer ltype = MatrixLayer::TOP);
	void disp_xy_layer(int lnum, std::ostream& os = std::cout);

public:
	void resize(const coord_type& dimensions);

	struct {

	}__temp;

	template<typename _Func>
	void init(_Func);

	Graph_Matrix_3D();
	Graph_Matrix_3D(const coord_type& dimensions);

	~Graph_Matrix_3D();
};
