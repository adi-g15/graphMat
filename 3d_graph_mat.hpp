#pragma once

#include "3d_graph_mat_decl.hpp"
#include <type_traits>	// for std::is_invocable
	
template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::resize(const coord_type& xyz)
{
	if (xyz == this->total_abs)	return;

		// the flag has been used to `alternately` pop or push layers, from the both parts of the axis
	bool flag = false;

	while (this->total_abs.mX > xyz.mX) {
		flag ?
			this->pop_xplus_layer() :	// planes intersected by the +ve x axis
			this->pop_xminus_layer();	// planes intersected by the -ve x axis

		flag = !flag;
	}

	while (this->total_abs.mY > xyz.mY) {
		flag ?
			this->pop_yplus_layer() :
			this->pop_yminus_layer();

		flag = !flag;
	}

	while (this->total_abs.mZ > xyz.mZ) {
		flag ?
			this->pop_zplus_layer() :
			this->pop_zminus_layer();

		flag = !flag;
	}

	auto org_t_abs{ this->total_abs.mX };	// since total_abs will be modified by the next operations, we need to store the ORIGINAL value of the total abs values
	if (this->total_abs.mX < xyz.mX) {
		this->add_x_layer((xyz.mX - org_t_abs) / 2);	// adding layers to the +ve x axis side
		this->inject_x_layer((xyz.mX - org_t_abs) - (xyz.mX - org_t_abs)/2);	// adding layers to the -ve x axis side
	}

	org_t_abs = total_abs.mY;
	if (this->total_abs.mY < xyz.mY) {
		this->add_y_layer((xyz.mY - org_t_abs) / 2);	// adding a layer to the +ve y axis side
		this->inject_y_layer((xyz.mY - org_t_abs) - (xyz.mY - org_t_abs) / 2);	// adding a layer to the -ve y axis side
	}

	org_t_abs = total_abs.mZ;
	if (this->total_abs.mZ < xyz.mZ) {
		this->add_z_layer((xyz.mZ - org_t_abs) / 2);	// adding a layer to the +ve z axis side
		this->inject_z_layer((xyz.mZ - org_t_abs) - (xyz.mZ - org_t_abs) / 2);	// adding a layer to the -ve z axis side
	}

}

template<typename node_dtype, typename dimen_t>
template<typename _Func>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::init(_Func func)
{
	static_assert(std::is_invocable_r<node_dtype, _Func, dimen_t, dimen_t, dimen_t>::value, 
		"Please ensure these conditions for function passed to Graph_Matrix_3D::init() -> \nMUST be invocable \nTakes 3 inputs of type convertible to dimen_t (integral type)\nReturns a result of type node_dtype (the type of data used to instantiate the matrix) :D ");

	graph_box_type*
		x_temp{ this->top_left_front },
		* y_temp{ this->top_left_front },
		* z_temp{ this->top_left_front };

	/*at this->top_left_front, only x is at minimum, while y and z coords are maximum there*/
	dimen_t x_coord{ min_x },
		y_coord{ max_y },
		z_coord{ max_z };

	while (z_temp)
	{
		while (y_temp)
		{
			while (x_temp)
			{
				x_temp->data = func(x_coord, y_coord, z_coord);

				++x_coord;
				x_temp = x_temp->RIGHT;
			}

			--y_coord;
			y_temp = y_temp->DOWN;
			x_temp = y_temp;
		}

		--z_coord;
		z_temp = z_temp->BACK_FACING;
		y_temp = z_temp;
	}

}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::Graph_Matrix_3D() : Graph_Matrix_3D({ 1,1,1 }) {}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::Graph_Matrix_3D(const coord_type& dimensions) : 
	Matrix_Base(3), 
	total_abs({1,1,1}),	/*absolute_ values aren't max-min, since at beginning min=max=0, but absolute is 1*/
	top_left_front(&origin),
	top_left_back(&origin),
	bottom_left_front(&origin),
	bottom_left_back(&origin),
	top_right_front(&origin),
	top_right_back(&origin),
	bottom_right_front(&origin),
	bottom_right_back(&origin),
	min_x(0),
	min_y(0),
	min_z(0),
	max_x(0),
	max_y(0),
	max_z(0)
{
	this->resize(dimensions);
}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::~Graph_Matrix_3D()
{
	//if ( total_abs.mX > 1 )
	//{
	//	while (max_x > 0)
	//		this->pop_xplus_layer();

	//	while (min_x < 0)
	//		this->pop_xminus_layer();
	//}

	//if (total_abs.mY > 1)
	//{
	//	while (max_y > 0)
	//		this->pop_yplus_layer();

	//	while (min_y < 0)
	//		this->pop_yminus_layer();
	//}

	//if (total_abs.mZ > 1)
	//{
	//	while (max_z > 0)
	//		this->pop_zplus_layer();

	//	while (min_z < 0)
	//		this->pop_zminus_layer();
	//}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::add_x_layer(int num)	// adds to end of xz planes (towards end of +x axis)
{
	if (num < 1)	 return;

	int num_box_required = num * ( total_abs.mY * total_abs.mZ );

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	// @brief tmp_z goes towards -ve z axis, and tmp_y goes towards -ve y axis
	graph_box_type* tmp_y{ this->top_right_front },
		* tmp_z{ this->top_right_front },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

#ifndef GRAPH_MAT_NO_COORD
	dimen_t y{max_y}, z{max_z};
#endif // !GRAPH_MAT_NO_COORD

	for (auto i = 0; i < num; i++)
	{
		tmp_y = this->top_right_front;
		tmp_z = this->top_right_front;
		tmp_z_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;
#ifndef GRAPH_MAT_NO_COORD
		z = max_z;
#endif // !GRAPH_MAT_NO_COORD

		while (tmp_z)
		{
#ifndef GRAPH_MAT_NO_COORD
			y = max_y;
#endif // !GRAPH_MAT_NO_COORD

			do
			{
				tmp_y->RIGHT = &(new_boxes[index++]);

				curr_new = tmp_y->RIGHT;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = max_x + 1;
				curr_new->coordinate.mY = y;
				curr_new->coordinate.mZ = z;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->LEFT = tmp_y;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_y_curr_prev = curr_new;
				tmp_y = tmp_y->DOWN;	// parallel to z, towards -z

				if (tmp_z_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_z_curr_prev = tmp_z_curr_prev->DOWN;
				}

#ifndef GRAPH_MAT_NO_COORD
				--y;
#endif // !GRAPH_MAT_NO_COORD
			} while (tmp_y);

			tmp_y_curr_prev = nullptr;

			tmp_z_curr_prev = tmp_z->RIGHT;
			tmp_z = tmp_z->BACK_FACING;
			tmp_y = tmp_z;

#ifndef GRAPH_MAT_NO_COORD
			--z;
#endif // !GRAPH_MAT_NO_COORD

		}

		this->top_right_back = this->top_right_back->RIGHT;
		this->top_right_front = this->top_right_front->RIGHT;
		this->bottom_right_back = this->bottom_right_back->RIGHT;
		this->bottom_right_front = this->bottom_right_front->RIGHT;

		++(this->max_x);
		++(this->total_abs.mX);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_x_layer(int num)
{
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mY * total_abs.mZ);

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	// @brief tmp_z goes towards -ve z axis, and tmp_y goes towards -ve y axis
	graph_box_type* tmp_y{ this->top_left_front },
		* tmp_z{ this->top_left_front },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

#ifndef GRAPH_MAT_NO_COORD
	dimen_t y{ max_y }, z{ max_z };
#endif // !GRAPH_MAT_NO_COORD
	for (auto i = 0; i < num; i++)
	{
		tmp_y = this->top_left_front;
		tmp_z = this->top_left_front;
		tmp_z_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;

#ifndef GRAPH_MAT_NO_COORD
		z = max_z;
#endif // !GRAPH_MAT_NO_COORD

		while (tmp_z)
		{
#ifndef GRAPH_MAT_NO_COORD
			y = max_y;
#endif // !GRAPH_MAT_NO_COORD

			do
			{
				tmp_y->LEFT = &(new_boxes[index++]);

				curr_new = tmp_y->LEFT;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = min_x - 1;
				curr_new->coordinate.mY = y;
				curr_new->coordinate.mZ = z;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->RIGHT = tmp_y;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_y_curr_prev = curr_new;
				tmp_y = tmp_y->DOWN;	// parallel to z, towards -z

				if (tmp_z_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_z_curr_prev = tmp_z_curr_prev->DOWN;
				}

#ifndef GRAPH_MAT_NO_COORD
				--y;
#endif // !GRAPH_MAT_NO_COORD
			} while (tmp_y);

			tmp_y_curr_prev = nullptr;

			tmp_z_curr_prev = tmp_z->LEFT;
			tmp_z = tmp_z->BACK_FACING;
			tmp_y = tmp_z;

#ifndef GRAPH_MAT_NO_COORD
			--z;
#endif // !GRAPH_MAT_NO_COORD
		}

		this->top_left_back = this->top_left_back->LEFT;
		this->top_left_front = this->top_left_front->LEFT;
		this->bottom_left_back = this->bottom_left_back->LEFT;
		this->bottom_left_front = this->bottom_left_front->LEFT;

		--(this->min_x);
		++(this->total_abs.mX);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_xplus_layer()
{
	if (total_abs.mX == 0 || max_x == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* y_tmp{this->top_right_front}, * z_tmp{ this->top_right_front };

	// z_tmp will move towards -ve z-axis
	// and for each such move, y_tmp will move towards -ve y-axis

	this->top_right_front = this->top_right_front->LEFT;
	this->top_right_back = this->top_right_back->LEFT;
	this->bottom_right_front = this->bottom_right_front->LEFT;
	this->bottom_right_back = this->bottom_right_back->LEFT;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* z_tmp_next{ z_tmp->BACK_FACING };
	do{
		z_tmp_next = z_tmp->BACK_FACING;

		do
		{
			to_delete = y_tmp;

			y_tmp->LEFT->RIGHT = nullptr;	// clearing pointers of previous level to use
			y_tmp = y_tmp->DOWN;

			//delete to_delete;
		} while (y_tmp);

		//if (z_tmp != to_delete)	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	z_tmp = z_tmp->BACK_FACING;
		//else z_tmp = nullptr;
		z_tmp = z_tmp_next;

		y_tmp = z_tmp;
	} while (z_tmp);

	--max_x;
	--total_abs.mX;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_xminus_layer()
{
	// @brief -> We go from the top left front to bottom left back
	// strategy same as pop_xplus_layer, just so we use RIGHT instead of LEFT, AND TOP_LEFT instead of TOP_RIGHT
	if (total_abs.mX == 0 || min_x == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* y_tmp{ this->top_left_front }, * z_tmp{ this->top_left_front };
	// z_tmp will move towards -ve z-axis
	// and for each such move, y_tmp will move towards -ve y-axis

	this->top_left_front = this->top_left_front->RIGHT;
	this->top_left_back = this->top_left_back->RIGHT;
	this->bottom_left_front = this->bottom_left_front->RIGHT;
	this->bottom_left_back = this->bottom_left_back->RIGHT;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* z_tmp_next{ z_tmp->BACK_FACING };
	while (z_tmp) {
		z_tmp_next = z_tmp->BACK_FACING;
		
		while (y_tmp)
		{
			y_tmp->RIGHT->LEFT = nullptr;	// clearing pointers of previous level to us

			to_delete = y_tmp;
			y_tmp = y_tmp->DOWN;

			//delete to_delete;	// DON'T DELETE IT NOW, THIS IS A TRADEOFF ! Memory overhead will be less due to less allocations, but at the same time old memory won't be used again, and will remain as such and removed in destructor only, so this approach is likely only helpful for EXPANDING MATRICES
		}

		//if ( z_tmp != to_delete )	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	z_tmp = z_tmp->BACK_FACING;
		//else z_tmp = nullptr;
		z_tmp = z_tmp_next;

		y_tmp = z_tmp;
	}

	++min_x;
	--total_abs.mX;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::add_y_layer(int num)
{
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mX * total_abs.mZ);

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	// @brief tmp_z goes towards -ve z axis, and tmp_x goes towards -ve y axis
	graph_box_type* tmp_x{ this->top_left_front },
		* tmp_z{ this->top_left_front },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_x_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

#ifndef GRAPH_MAT_NO_COORD
	dimen_t x{ min_x }, z{ max_z };
#endif // !GRAPH_MAT_NO_COORD

	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->top_left_front;
		tmp_z = this->top_left_front;
		tmp_z_curr_prev = nullptr;
		tmp_x_curr_prev = nullptr;
#ifndef GRAPH_MAT_NO_COORD
		z = max_z;
#endif // !GRAPH_MAT_NO_COORD

		while (tmp_z)
		{
#ifndef GRAPH_MAT_NO_COORD
			x = min_x;
#endif // !GRAPH_MAT_NO_COORD

			do
			{
				tmp_x->UP = &(new_boxes[index++]);

				curr_new = tmp_x->UP;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = x;
				curr_new->coordinate.mY = max_y + 1;
				curr_new->coordinate.mZ = z;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->DOWN = tmp_x;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->LEFT = tmp_x_curr_prev;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				tmp_x_curr_prev = curr_new;	// @important @note -> temp_x_prev and temp_z_prev WILL be on the new layer itself, don't equate with tmp_y or tmp_<.> since it's on the previous layer
				tmp_x = tmp_x->RIGHT;	// move towards +x axis

				if (tmp_z_curr_prev) {
					tmp_z_curr_prev = tmp_z_curr_prev->RIGHT;
				}

#ifndef GRAPH_MAT_NO_COORD
				++x;
#endif // !GRAPH_MAT_NO_COORD
			} while (tmp_x);

			tmp_x_curr_prev = nullptr;	// for the first new block at min_x, there won't be a prev element

			tmp_z_curr_prev = tmp_z->UP;	// now the tmp_z_curr_prev will be on the new layer formed
			tmp_z = tmp_z->BACK_FACING;
			tmp_x = tmp_z;

#ifndef GRAPH_MAT_NO_COORD
			--z;
#endif // !GRAPH_MAT_NO_COORD
		}

		this->top_right_back = this->top_right_back->UP;
		this->top_right_front = this->top_right_front->UP;
		this->top_left_back = this->top_left_back->UP;
		this->top_left_front = this->top_left_front->UP;

		++(this->max_y);
		++(this->total_abs.mY);
	}

}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_y_layer(int num)
{
	if (num < 1)	 return;
	int num_box_required = num * (total_abs.mX * total_abs.mZ);

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	// @brief tmp_z goes towards -ve z axis, and tmp_x goes towards -ve y axis
	graph_box_type* tmp_x{ this->bottom_left_front },
		* tmp_z{ this->bottom_left_front },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_x_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

#ifndef GRAPH_MAT_NO_COORD
	dimen_t x{ min_x }, z{ max_z };
#endif // !GRAPH_MAT_NO_COORD

	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->bottom_left_front;
		tmp_z = this->bottom_left_front;
		tmp_z_curr_prev = nullptr;
		tmp_x_curr_prev = nullptr;
#ifndef GRAPH_MAT_NO_COORD
		z = max_z;
#endif // !GRAPH_MAT_NO_COORD

		while (tmp_z)
		{
#ifndef GRAPH_MAT_NO_COORD
			x = min_x;
#endif // !GRAPH_MAT_NO_COORD
			do
			{
				tmp_x->DOWN = &(new_boxes[index++]);

				curr_new = tmp_x->DOWN;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = x;
				curr_new->coordinate.mY = min_y - 1;
				curr_new->coordinate.mZ = z;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->UP = tmp_x;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->LEFT = tmp_x_curr_prev;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				tmp_x_curr_prev = curr_new;
				tmp_x = tmp_x->RIGHT;	// move towards +x axis

				if (tmp_z_curr_prev) {
					tmp_z_curr_prev = tmp_z_curr_prev->RIGHT;
				}

#ifndef GRAPH_MAT_NO_COORD
				++x;
#endif // !GRAPH_MAT_NO_COORD

			} while (tmp_x);

			tmp_x_curr_prev = nullptr;

			tmp_z_curr_prev = tmp_z->DOWN;
			tmp_z = tmp_z->BACK_FACING;
			tmp_x = tmp_z;

#ifndef GRAPH_MAT_NO_COORD
			--z;
#endif // !GRAPH_MAT_NO_COORD

		}

		this->bottom_right_back = this->bottom_right_back->DOWN;
		this->bottom_right_front = this->bottom_right_front->DOWN;
		this->bottom_left_back = this->bottom_left_back->DOWN;
		this->bottom_left_front = this->bottom_left_front->DOWN;

		--(this->min_y);
		++(this->total_abs.mY);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_yplus_layer()
{
	// @brief -> Move from the top left FRONT to top LEFT BACK
	if (total_abs.mY == 0 || max_y == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* x_tmp{ this->top_left_front }, * z_tmp{ this->top_left_front };
	// z_tmp will move towards -ve z-axis
	// and for each such move, x_tmp will move towards +ve x-axis

	this->top_left_front = this->top_left_front->DOWN;
	this->top_left_back = this->top_left_back->DOWN;
	this->top_right_front = this->top_right_front->DOWN;
	this->top_right_back = this->top_right_back->DOWN;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* z_tmp_next{ z_tmp->BACK_FACING };
	do {
		z_tmp_next = z_tmp->BACK_FACING;
		
		do
		{
			x_tmp->DOWN->UP = nullptr;	// clearing pointers of previous level to us

			to_delete = x_tmp;
			x_tmp = x_tmp->RIGHT;

			//delete to_delete;
		} while (x_tmp);

		//if (z_tmp != to_delete)	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	z_tmp = z_tmp->BACK_FACING;
		//else z_tmp = nullptr;
		z_tmp = z_tmp_next;
		x_tmp = z_tmp;
	} while (z_tmp);

	--max_y;
	--total_abs.mY;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_yminus_layer()
{
	// @brief -> Move from the bottom left FRONT to BOTTOM LEFT BACK
	if (total_abs.mY == 0 || min_y == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* x_tmp{ this->bottom_left_front }, * z_tmp{ this->bottom_left_front };
	// z_tmp will move towards -ve z-axis
	// and for each such move, x_tmp will move towards +ve x-axis

	this->bottom_left_front = this->bottom_left_front->UP;
	this->bottom_left_back = this->bottom_left_back->UP;
	this->bottom_right_front = this->bottom_right_front->UP;
	this->bottom_right_back = this->bottom_right_back->UP;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* z_tmp_next{ z_tmp->BACK_FACING };
	do {
		z_tmp_next = z_tmp->BACK_FACING;

		do
		{
			x_tmp->UP->DOWN = nullptr;	// clearing pointers of previous level to us

			to_delete = x_tmp;
			x_tmp = x_tmp->RIGHT;

			//delete to_delete;
		} while (x_tmp);

		//if (z_tmp != to_delete)	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	z_tmp = z_tmp->BACK_FACING;
		//else z_tmp = nullptr;
		z_tmp = z_tmp_next;	// in case z_tmp had been equal to x_tmp, ie. only 1 layer would have been present, then that node is now freed, and we have garbage value, which we shouldn't dereference to z_tmp->BACK_FACING;

		x_tmp = z_tmp;
	} while (z_tmp);

	++min_y;
	--total_abs.mY;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::add_z_layer(int num)
{
	// @brief -> first tmp_y goes from MAX_y to MIN_y (towards -ve y), and for each such tmp_y, tmp_x moves from min_x to max_x (towards +x axis)
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mX * total_abs.mY);

	graph_box_type* new_boxes = allocator.Alloc( num_box_required );
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	graph_box_type* tmp_x{ this->top_left_front },
		* tmp_y{ this->top_left_front },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_x_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->top_left_front;
		tmp_y = this->top_left_front;
		tmp_x_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;

#ifndef GRAPH_MAT_NO_COORD
		dimen_t x{ min_x }, y{ max_y };
#endif // !GRAPH_MAT_NO_COORD
		while (tmp_y)
		{
#ifndef GRAPH_MAT_NO_COORD
			x = min_x;
#endif // !GRAPH_MAT_NO_COORD
			do
			{
				tmp_x->FRONT_FACING = &(new_boxes[index++]);

				curr_new = tmp_x->FRONT_FACING;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = x;
				curr_new->coordinate.mY = y;
				curr_new->coordinate.mZ = max_z + 1;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->BACK_FACING = tmp_x;	// all other pointers are automatically nullptr
				curr_new->LEFT = tmp_x_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_x_curr_prev = curr_new;

				//if (tmp_x->UP) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
				//	tmp_y_curr_prev = tmp_x->UP->FRONT_FACING;
				//}
				// @note - The above is also a good way to update tmp_y_curr_prev, and will work

				if (tmp_y_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_y_curr_prev = tmp_y_curr_prev->RIGHT;
				}

				tmp_x = tmp_x->RIGHT;

#ifndef GRAPH_MAT_NO_COORD
				++x;
#endif // !GRAPH_MAT_NO_COORD

			} while (tmp_x);

			tmp_x_curr_prev = nullptr;	// there isn't anything preeceding the 1st element (in loop when it enters)
			tmp_y_curr_prev = tmp_y->FRONT_FACING;

			tmp_y = tmp_y->DOWN;
			tmp_x = tmp_y;

#ifndef GRAPH_MAT_NO_COORD
			--y;
#endif // !GRAPH_MAT_NO_COORD

		}

		this->top_right_front = this->top_right_front->FRONT_FACING;
		this->bottom_right_front = this->bottom_right_front->FRONT_FACING;
		this->top_left_front = this->top_left_front->FRONT_FACING;
		this->bottom_left_front = this->bottom_left_front->FRONT_FACING;

		++(this->max_z);
		++(this->total_abs.mZ);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_z_layer(int num)
{
	// @brief -> first tmp_y goes from MAX_y to MIN_y (towards -ve y), and for each such tmp_y, tmp_x moves from min_x to max_x (towards +x axis)
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mX * total_abs.mY);

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);
	//graph_box_type* new_boxes = new graph_box_type[num_box_required];	// deletion not this function's task; destructor does that

	graph_box_type* tmp_x{ this->top_left_back },
		* tmp_y{ this->top_left_back },	// horizontal(parallel to x) and vertical(parallel to y)
		* tmp_x_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)
	//	@ assert that total number of times these loops run doesn't exceed `num_box_required`

	graph_box_type* curr_new;

#ifndef GRAPH_MAT_NO_COORD
	dimen_t x{ min_x }, y{ max_y };
#endif // !GRAPH_MAT_NO_COORD
	
	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->top_left_back;
		tmp_y = this->top_left_back;
		tmp_x_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;

#ifndef GRAPH_MAT_NO_COORD
		y = max_y;
#endif // !GRAPH_MAT_NO_COORD

		while (tmp_y)
		{
#ifndef GRAPH_MAT_NO_COORD
			x = min_x;
#endif // !GRAPH_MAT_NO_COORD
			do
			{
				tmp_x->BACK_FACING = &(new_boxes[index++]);

				curr_new = tmp_x->BACK_FACING;

#ifndef GRAPH_MAT_NO_COORD
				curr_new->coordinate.mX = x;
				curr_new->coordinate.mY = y;
				curr_new->coordinate.mZ = min_z - 1;
#endif // !GRAPH_MAT_NO_COORD

				curr_new->FRONT_FACING = tmp_x;	// all other pointers are automatically nullptr
				curr_new->LEFT = tmp_x_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_x_curr_prev = curr_new;

				//if (tmp_x->UP) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
				//	tmp_y_curr_prev = tmp_x->UP->FRONT_FACING;
				//}
				// @note - The above is also a good way to update tmp_y_curr_prev, and will work

				if (tmp_y_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_y_curr_prev = tmp_y_curr_prev->RIGHT;
				}

				tmp_x = tmp_x->RIGHT;

#ifndef GRAPH_MAT_NO_COORD
				++x;
#endif // !GRAPH_MAT_NO_COORD

			} while (tmp_x);

			tmp_x_curr_prev = nullptr;	// there isn't anything preeceding the 1st element (in loop when it enters)
			tmp_y_curr_prev = tmp_y->BACK_FACING;

			tmp_y = tmp_y->DOWN;
			tmp_x = tmp_y;

#ifndef GRAPH_MAT_NO_COORD
			--y;
#endif // !GRAPH_MAT_NO_COORD

		}

		this->top_right_back = this->top_right_back->BACK_FACING;
		this->bottom_right_back = this->bottom_right_back->BACK_FACING;
		this->top_left_back = this->top_left_back->BACK_FACING;
		this->bottom_left_back = this->bottom_left_back->BACK_FACING;

		--(this->min_z);
		++(this->total_abs.mZ);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_zplus_layer()
{
	// @brief -> We move from the top left front to the bottom right front
	if (total_abs.mZ == 0 || max_z == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* y_tmp{ this->top_left_front }, * x_tmp{ this->top_left_front };
	// x_tmp will move towards +ve x-axis
	// and for each such move, y_tmp will move towards -ve y-axis

	this->top_left_front = this->top_left_front->BACK_FACING;
	this->bottom_left_front = this->bottom_left_front->BACK_FACING;
	this->top_right_front = this->top_right_front->BACK_FACING;
	this->bottom_right_front = this->bottom_right_front->BACK_FACING;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* x_tmp_next { x_tmp->RIGHT };
	do{
		x_tmp_next = x_tmp->RIGHT;

		do
		{
			y_tmp->BACK_FACING->FRONT_FACING = nullptr;	// clearing pointers of previous level to us

			to_delete = y_tmp;
			y_tmp = y_tmp->DOWN;

			//delete to_delete;
		} while (y_tmp);

		//if (x_tmp != to_delete)	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	x_tmp = x_tmp->RIGHT;
		//else x_tmp = nullptr;
		x_tmp = x_tmp_next;
		y_tmp = x_tmp;
	} while (x_tmp);

	--max_z;
	--total_abs.mZ;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pop_zminus_layer()
{
	// @brief -> We move from the top left BACK to the bottom right BACK
	if (total_abs.mZ == 0 || min_z == 0)	return;	// don't pop the layer with origin in it

	graph_box_type* y_tmp{ this->top_left_back }, * x_tmp{ this->top_left_back };
	// x_tmp will move towards +ve x-axis
	// and for each such move, y_tmp will move towards -ve y-axis

	this->top_left_back = this->top_left_back->BACK_FACING;
	this->bottom_left_back = this->bottom_left_back->BACK_FACING;
	this->top_right_back = this->top_right_back->BACK_FACING;
	this->bottom_right_back = this->bottom_right_back->BACK_FACING;

	graph_box_type* to_delete;	// temp var to store the one to delete
	graph_box_type* x_tmp_next { x_tmp->RIGHT };
	do{
		x_tmp_next = x_tmp->RIGHT;
		
		do
		{
			y_tmp->FRONT_FACING->BACK_FACING = nullptr;	// clearing pointers of previous level to us

			to_delete = y_tmp;
			y_tmp = y_tmp->DOWN;

			//delete to_delete;
		} while (y_tmp);

		//if (x_tmp != to_delete)	// in case when there is only single plane xy, and no other parallel, then z_tmp and y_tmp will be equal, so when y_tmp deleted, z_tmp still points to it, and considers the garbage value, and continue to prevent that
		//	x_tmp = x_tmp->RIGHT;
		//else x_tmp = nullptr;
		x_tmp = x_tmp_next;

		y_tmp = x_tmp;
	} while (x_tmp);

	++min_z;
	--total_abs.mZ;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::disp_xy_layer(MatrixLayer ltype)
{
	switch (ltype)
	{
	case MatrixLayer::TOP:
		return disp_xy_layer(max_z);
	case MatrixLayer::DEEPEST:
		return disp_xy_layer(min_z);
	case MatrixLayer::ORIGIN:
		return disp_xy_layer(0);
	}
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::disp_xy_layer(int z_lnum, std::ostream& os)
{
	if (z_lnum < min_z || z_lnum > max_z)	return disp_xy_layer(MatrixLayer::TOP);

	graph_box_type* tmp_y, * temp_loc{ this->top_left_front };

	int num_jumps = std::abs( max_z - z_lnum );
	while (num_jumps --> 0)
	{
		temp_loc = temp_loc->BACK_FACING;
	}

	tmp_y = temp_loc;
	while ( tmp_y )
	{
		while (temp_loc)
		{
			os << '(' << temp_loc->coordinate << ')' << ' ';

			temp_loc = temp_loc->RIGHT;
		}
		os << '\n';

		tmp_y = tmp_y->DOWN;
		temp_loc = tmp_y;
	}
}
