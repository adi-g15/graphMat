#pragma once

#include "3d_graph_mat_decl.hpp"
#include "iterators.hpp"
#include <type_traits>	// for std::is_invocable
#include <thread>
#include <cassert>

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::resize(const dimen_t newX, const dimen_t newY, const dimen_t newZ, const Init_Func& initialiser, RESIZE_TYPE resize_type)
{
	this->set_initialiser(initialiser);
	this->resize(newX, newY, newZ, resize_type);
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::resize(const dimen_t newX, const dimen_t newY, const dimen_t newZ, RESIZE_TYPE resize_type)
{
	if ( newX == this->total_abs.mX && newY == this->total_abs.mY && newZ == this->total_abs.mZ )	return;

	const bool was_auto_expanding{ this->__expansion_state.expansion_flag };
	tmp_resize_data.curr_resize_type = resize_type;

	if ( resize_type == RESIZE_TYPE::MANUAL ) {
		if( was_auto_expanding ) this->pause_auto_expansion();	// if auto_expansion is on, pause it
	}

	/**
	* @note - Above code maybe a bit confusing... the "initialiser function" is decided by the add_* or push_*
	*		  functions themselves, BASED on `curr_resize_type`, that is why we stored it
	*/

	// the flag has been used to `alternately` pop or push layers, from the both parts of the axis
	bool flag = false;

	while (this->total_abs.mX > newX) {
		flag ?
			this->pop_xplus_layer() :	// planes intersected by the +ve x axis
			this->pop_xminus_layer();	// planes intersected by the -ve x axis

		flag = !flag;
	}

	while (this->total_abs.mY > newY) {
		flag ?
			this->pop_yplus_layer() :
			this->pop_yminus_layer();

		flag = !flag;
	}

	while (this->total_abs.mZ > newZ) {
		flag ?
			this->pop_zplus_layer() :
			this->pop_zminus_layer();

		flag = !flag;
	}

	auto org_t_abs{ this->total_abs.mX };	// since total_abs will be modified by the next operations, we need to store the ORIGINAL value of the total abs values
	if (this->total_abs.mX < newX) {
		if (tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL) {
			this->add_x_layer((newX - org_t_abs) / 2);	// adding layers to the +ve x axis side
			this->inject_x_layer((newX - org_t_abs) - (newX - org_t_abs) / 2);	// adding layers to the -ve x axis side
		}
		else {	// using different behaviour in Auto Expansion, so that layers/planes don't just get added only in direction (for eg. only in +ve direction, this happens when each time increment is only of 1 units)
			tmp_resize_data.add_or_inject_flag ? 
				this->add_x_layer(newX - org_t_abs) :
				this->inject_x_layer(newX - org_t_abs);
		}
	}

	org_t_abs = total_abs.mY;
	if (this->total_abs.mY < newY) {
		if (tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL) {
			this->add_y_layer((newY - org_t_abs) / 2);	// adding a layer to the +ve y axis side
			this->inject_y_layer((newY - org_t_abs) - (newY - org_t_abs) / 2);	// adding a layer to the -ve y axis side
		}
		else {	// using different behaviour in Auto Expansion, so that layers/planes don't just get added only in direction (for eg. only in +ve direction, this happens when each time increment is only of 1 units)
			tmp_resize_data.add_or_inject_flag ?
				this->add_y_layer(newY - org_t_abs) :
				this->inject_y_layer(newY - org_t_abs);
		}
	}

#ifdef DEBUG
	static bool resize_called_again = false;
	if (resize_called_again) std::cout << "Already resizing" << std::endl;

	resize_called_again = true;
#endif // DEBUG

	org_t_abs = total_abs.mZ;
	if (this->total_abs.mZ < newZ) {
		if (tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL) {
			this->add_z_layer((newZ - org_t_abs) / 2);	// adding a layer to the +ve z axis side
			this->inject_z_layer((newZ - org_t_abs) - (newZ - org_t_abs) / 2);	// adding a layer to the -ve z axis side
		}
		else {	// using different behaviour in Auto Expansion, so that layers/planes don't just get added only in direction (for eg. only in +ve direction, this happens when each time increment is only of 1 units)
			tmp_resize_data.add_or_inject_flag ? 
				this->add_z_layer(newZ - org_t_abs) :
				this->inject_z_layer(newZ - org_t_abs);
		}
	}

	if (resize_type == RESIZE_TYPE::MANUAL) {
		if (was_auto_expanding) {
			this->__expansion_state.expansion_flag.store(true);

			// we `start` it again on another thread (with previous expansion metadata still in __expansion_state object)
			std::thread(&Graph_Matrix_3D::auto_expansion, this).detach();
		}
	}
	else {
		tmp_resize_data.add_or_inject_flag = ! tmp_resize_data.add_or_inject_flag;	// toggle it, so next time, planes get added in alternative fashion (equally on both sides of each axis)
	}

#ifdef DEBUG
	resize_called_again = false;
#endif // DEBUG
}

template<typename node_dtype, typename dimen_t>
inline std::tuple<dimen_t, dimen_t, dimen_t>  Graph_Matrix_3D<node_dtype, dimen_t>::get_size() const noexcept {
	return { this->total_abs.mX, this->total_abs.mY, this->total_abs.mZ };
}

template<typename node_dtype, typename dimen_t>
inline std::array<std::pair<dimen_t, dimen_t>, 3> Graph_Matrix_3D<node_dtype, dimen_t>::_implementation_detail_get_dimension_bounds() const noexcept {
	return {
		std::make_pair(this->min_x, this->max_x),
		std::make_pair(this->min_y, this->max_y),
		std::make_pair(this->min_z, this->max_z)
	};
}

template<typename node_dtype, typename dimen_t>
template<typename Func>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::for_each(graph_box_type* source, Direction dir, Func func) noexcept
{
	Graph_Box_3D<node_dtype>* tmp{ source };

	while (tmp)
	{
		func(tmp->data);	// depends on func whether it takes by reference or not
		tmp = tmp->get_adj_box(dir);
	}
}

template<typename node_dtype, typename dimen_t>
template<typename Func>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::for_each(graph_box_type* begin, graph_box_type* end, Direction dir, Func func) noexcept
{
	static_assert(std::is_invocable_v<Func, const node_dtype&>,
		"The Callable expected by for_each must take only `node_dtype&` as input");

	Graph_Box_3D<node_dtype>* tmp{ begin };

	while (tmp != end)
	{
#ifndef GRAPH_MAT_NO_COORD
		// simple preventive measure to throw when begin and end not connected
		if (tmp->coordinate.mX < min_x || tmp->coordinate.mX > max_x || tmp->coordinate.mY < min_y || tmp->coordinate.mY > max_y || tmp->coordinate.mZ < min_z || tmp->coordinate.mZ > max_z) {
			throw std::logic_error("Begin and End passed to Graph_Matrix_3D::for_each() not connected !!");
		}
#endif
		func(tmp->data);	// depends on func whether it takes by reference or not
		tmp = tmp->get_adj_box(dir);
	}

}

template<typename node_dtype, typename dimen_t>
template<typename _Func, typename std::enable_if<std::is_invocable_r_v<node_dtype, _Func, dimen_t, dimen_t, dimen_t>>::type >
inline void Graph_Matrix_3D<node_dtype, dimen_t>::for_all(_Func func) {
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
template<typename _Func, typename std::enable_if<std::is_invocable_r_v<void, _Func, node_dtype&>>::type >
inline void Graph_Matrix_3D<node_dtype, dimen_t>::for_all(_Func func) {
	graph_box_type*
		x_temp{ this->top_left_front },
		* y_temp{ this->top_left_front },
		* z_temp{ this->top_left_front };

	while (z_temp)
	{
		while (y_temp)
		{
			while (x_temp)
			{
				func(x_temp->data);	// call the lambda with reference to the box

				x_temp = x_temp->RIGHT;
			}

			y_temp = y_temp->DOWN;
			x_temp = y_temp;
		}

		z_temp = z_temp->BACK_FACING;
		y_temp = z_temp;
	}
}

template<typename node_dtype, typename dimen_t>
Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::find(const node_dtype& value) {	// uses operator== for comparison
	// @future - Use the top positions tree to chose from the best positions

	// @note- For small matrices, there is a speed tradeoff using threads actually
	return this->swastic_find(&this->origin, value);
}

//template<typename node_dtype, typename dimen_t>
//template<typename UnaryPredicate>
//Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::find(UnaryPredicate& func) {
//	static_assert(std::is_invocable_r_v<bool, UnaryPredicate, const node_dtype&>,
//		"UnaryPredicate only considers functions that take in a `const node_dtype&` reference, and return a boolean value");;
//
//	// @future - Use the top positions tree to chose from the best positions
//
//	return this->swastic_find(this->origin, func);
//}

template<typename node_dtype, typename dimen_t>
Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::swastic_find(graph_box_type* center_node, const node_dtype& value) {	// uses operator== for comparison
	if (center_node->data == value)	return center_node;

	std::vector<std::thread> layer_threads;	// all threads that are searching different layers

	std::condition_variable cv;	// any thread will notify if it has found it
	std::atomic_bool found = false;
	graph_box_type* found_box{ nullptr };

	auto swastic = [&](graph_box_type* layer_center) mutable {
		std::array<std::thread, 4> t;

		graph_box_type 
			*up{ layer_center->UP }, 
			*down{ layer_center->DOWN },
			*left{ layer_center->LEFT },
			*right{ layer_center->RIGHT };

		t[0] = std::thread([&]() mutable {
			graph_box_type* tmp;
			while (!found && up)
			{
				tmp = up;
				while (tmp && tmp->getData() != value)
				{
					tmp = tmp->RIGHT;
				}
				if (tmp != nullptr) {
					found.store(true);
					found_box = tmp;
					cv.notify_one();
					return;
				}
				up = up->UP;
			}
			});
		t[1] = std::thread([&]() mutable {
			graph_box_type* tmp;
			while (!found && down)
			{
				tmp = down;
				while (tmp && tmp->getData() != value)
				{
					tmp = tmp->LEFT;
				}
				if (tmp != nullptr) {
					found.store(true);
					found_box = tmp;
					cv.notify_one();
					return;
				}
				down = down->DOWN;
			}
			});
		t[2] = (std::thread([&]() mutable {
			graph_box_type* tmp;
			while (!found && left)
			{
				tmp = left;
				while (tmp && tmp->getData() != value)
				{
					tmp = tmp->UP;
				}
				if (tmp != nullptr) {
					found.store(true);
					found_box = tmp;
					cv.notify_one();
					return;
				}
				left = left->LEFT;
			}
			}));
		t[3] = std::thread([&]() mutable {
			graph_box_type* tmp;
			while (!found && right)
			{
				tmp = right;
				while (tmp && tmp->getData() != value)
				{
					tmp = tmp->DOWN;
				}
				if (tmp != nullptr) {
					found.store(true);
					found_box = tmp;
					cv.notify_one();
					return;
				}
				right = right->RIGHT;
			}
			});

		for (auto& th : t)
			if (th.joinable())	th.join();

	};

	// call to the above is being made here, it maybe confusing sorry :-)
	layer_threads.reserve(this->total_abs.mZ);
	layer_threads.push_back(std::thread(swastic, center_node));

	graph_box_type* tmp1{ center_node->FRONT_FACING }, *tmp2{ center_node->BACK_FACING };

	while (tmp1 != nullptr)
	{
		layer_threads.push_back(std::thread(swastic, tmp1));

		tmp1 = tmp1->FRONT_FACING;
	}

	while (tmp2 != nullptr)
	{
		layer_threads.push_back(std::thread(swastic, tmp2));

		tmp2 = tmp2->BACK_FACING;
	}

	std::unique_lock<std::mutex> lock(m);
	while (!found)
	{
		// if no thread is joinable (ie. all have done executing) have done executing and we didn't find, then exit
		if (std::none_of(layer_threads.begin(), layer_threads.end(), [](const auto& th) {return th.joinable();}))	break;

		cv.wait_for(lock, std::chrono::milliseconds(200));
	}

	for (auto& th : layer_threads)
	{
		if (th.joinable())	th.join();
	}
	return found_box;
}

//template<typename node_dtype, typename dimen_t>
//template<typename UnaryPredicate>
//Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::swastic_find(graph_box_type* center_node, UnaryPredicate& func) {
//	static_assert(std::is_invocable_r_v<bool, UnaryPredicate, const node_dtype&>,
//		"UnaryPredicate only considers functions that take in a `const node_dtype&` reference, and return a boolean value");;
//
//	//if()
//	return nullptr;
//}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::expand_once()
{
	constexpr float decrease_rate = 0.90f;	// 90% of previous expansion speed

	if (this->__expansion_state.time_since_speed_updated % 10 == 0) {
		this->__expansion_state.curr_expansion_speed = this->__expansion_state.expansion_speed;
		this->__expansion_state.time_since_speed_updated = 0;
	}

	this->__expansion_state.increase_units += this->__expansion_state.curr_expansion_speed * decrease_rate;
	this->__expansion_state.curr_expansion_speed *= decrease_rate;

	// will be optimised away by compiler,since const local
	const int int_part{ (int)(this->__expansion_state.increase_units) };
	expand_n_unit(int_part);

	// get float value after the decimal (logically equivalent to (float % 1) )
	this->__expansion_state.increase_units -= int_part ;

	++this->__expansion_state.time_since_speed_updated;
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::expand_n_unit(const uint8_t n)
{
	if (n <= 0)	return;

	this->resize(
		this->total_abs.mX + n,
		this->total_abs.mY + n,
		this->total_abs.mZ + n,
		RESIZE_TYPE::AUTO_EXPANSION	// this method is ONLY meant to be used by auto expansion
	);
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::add_x_layer(int num)	// adds to end of xz planes (towards end of +x axis)
{
	// @note - Currently there is a LOT of duplicacy due to the add_*_layer being repeated for all three directions, i plan to use template to ease this, though it will require a few more if statements inside loops, but depending on compiler to pre inline on basis of a template parameter (of a type like DIRECTION etc), won't be doing that for now though, will do
	// @brief tmp_z goes towards -ve z axis, and tmp_y goes towards -ve y axis
	if (num < 1)	 return;

	const int num_box_required = num * ( total_abs.mY * total_abs.mZ );

	const bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);

	graph_box_type* tmp_y{ this->top_right_front },
		* tmp_z{ this->top_right_front },
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)

	graph_box_type* curr_new;

	dimen_t y{max_y}, z{max_z};

	for (auto i = 0; i < num; i++)
	{
		tmp_y = this->top_right_front;
		tmp_z = this->top_right_front;
		tmp_z_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;
		z = max_z;

		while (tmp_z)
		{
			y = max_y;

			do
			{
				tmp_y->RIGHT = (new_boxes + index++);

				curr_new = tmp_y->RIGHT;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(max_x + 1, y, z);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , max_x + 1, y, z);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(max_x + 1, y, z);
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

				--y;
			} while (tmp_y);

			tmp_y_curr_prev = nullptr;

			tmp_z_curr_prev = tmp_z->RIGHT;
			tmp_z = tmp_z->BACK_FACING;
			tmp_y = tmp_z;

			--z;
		}

		this->top_right_back = this->top_right_back->RIGHT;
		this->top_right_front = this->top_right_front->RIGHT;
		this->bottom_right_back = this->bottom_right_back->RIGHT;
		this->bottom_right_front = this->bottom_right_front->RIGHT;

		++(this->max_x);
		++(this->total_abs.mX);
	}

	assert(num_box_required == index);
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_x_layer(int num)
{
	// @brief tmp_z goes towards -ve z axis, and tmp_y goes towards -ve y axis
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mY * total_abs.mZ);

	bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);

	graph_box_type* tmp_y{ this->top_left_front },
		* tmp_z{ this->top_left_front },
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)

	graph_box_type* curr_new;

	dimen_t y{ max_y }, z{ max_z };

	for (auto i = 0; i < num; i++)
	{
		tmp_y = this->top_left_front;
		tmp_z = this->top_left_front;
		tmp_z_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;
		z = max_z;

		while (tmp_z)
		{
			y = max_y;

			do
			{
				if (index > num_box_required)	exit(1);

				tmp_y->LEFT = (new_boxes + index++);

				curr_new = tmp_y->LEFT;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(max_x + 1, y, z);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , max_x + 1, y, z);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(min_x - 1, y, z);
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

				--y;
			} while (tmp_y);

			tmp_y_curr_prev = nullptr;

			tmp_z_curr_prev = tmp_z->LEFT;
			tmp_z = tmp_z->BACK_FACING;
			tmp_y = tmp_z;

			--z;
		}

		this->top_left_back = this->top_left_back->LEFT;
		this->top_left_front = this->top_left_front->LEFT;
		this->bottom_left_back = this->bottom_left_back->LEFT;
		this->bottom_left_front = this->bottom_left_front->LEFT;

		--(this->min_x);
		++(this->total_abs.mX);
	}

	assert(num_box_required == index);
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
	// @brief tmp_z goes towards -ve z axis, and tmp_x goes towards +ve x axis, both starting at top_left_front
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mX * total_abs.mZ);

	bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);

	graph_box_type* tmp_x{ this->top_left_front },
		* tmp_z{ this->top_left_front },
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* x_prev_new_box{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)

	graph_box_type* curr_new;

	dimen_t x{ min_x }, z{ max_z };

	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->top_left_front;
		tmp_z = this->top_left_front;
		tmp_z_curr_prev = nullptr;
		x_prev_new_box = nullptr;
		z = max_z;

		while (tmp_z)
		{
			x = min_x;

			do
			{
				tmp_x->UP = (new_boxes + index++);	// ie. &new_box[index++]

				curr_new = tmp_x->UP;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(x, max_y + 1, z);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , x, max_y + 1, z);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(x, max_y + 1, z);
#endif // !GRAPH_MAT_NO_COORD

				curr_new->DOWN = tmp_x;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->LEFT = x_prev_new_box;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				x_prev_new_box = curr_new;	// @important @note -> temp_x_prev and temp_z_prev WILL be on the new layer itself, don't equate with tmp_y or tmp_<.> since it's on the previous layer
				tmp_x = tmp_x->RIGHT;	// move towards +x axis

				if (tmp_z_curr_prev) {
					tmp_z_curr_prev = tmp_z_curr_prev->RIGHT;
				}

				++x;
			} while (tmp_x);

			x_prev_new_box = nullptr;	// for the first new block at min_x, there won't be a prev element

			tmp_z_curr_prev = tmp_z->UP;	// now the tmp_z_curr_prev will be on the new layer formed
			tmp_z = tmp_z->BACK_FACING;
			tmp_x = tmp_z;

			--z;
		}

		this->top_right_back = this->top_right_back->UP;
		this->top_right_front = this->top_right_front->UP;
		this->top_left_back = this->top_left_back->UP;
		this->top_left_front = this->top_left_front->UP;

		++(this->max_y);
		++(this->total_abs.mY);
	}


	assert(num_box_required == index);
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_y_layer(int number_of_layers)
{
	// @brief tmp_z goes towards -ve z axis, and tmp_x goes towards +ve x axis
	if (number_of_layers < 1)	 return;
	const int num_box_required = number_of_layers * (total_abs.mX * total_abs.mZ);

	bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}

	graph_box_type* new_boxes = allocator.Alloc(num_box_required);

	graph_box_type* tmp_x{ this->bottom_left_front },
		* tmp_z{ this->bottom_left_front },
		* tmp_z_curr_prev{ nullptr },	// just before the current new box
		* x_prev_new_box{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)

	graph_box_type* curr_new;

	dimen_t x{ min_x }, z{ max_z };
	for (auto i = 0; i < number_of_layers; i++)
	{
		tmp_x = this->bottom_left_front;
		tmp_z = this->bottom_left_front;
		tmp_z_curr_prev = nullptr;
		x_prev_new_box = nullptr;
		z = max_z;

		while (tmp_z)
		{
			x = min_x;
			do
			{
				tmp_x->DOWN = (new_boxes + index++);

				curr_new = tmp_x->DOWN;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(x, min_y - 1, z);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , x, min_y - 1, z);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(x, min_y - 1, z);
#endif // !GRAPH_MAT_NO_COORD

				curr_new->UP = tmp_x;	// all other pointers are automatically nullptr
				curr_new->FRONT_FACING = tmp_z_curr_prev;
				curr_new->LEFT = x_prev_new_box;

				if (curr_new->FRONT_FACING)
					curr_new->FRONT_FACING->BACK_FACING = curr_new;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				if (tmp_z_curr_prev) {
					tmp_z_curr_prev = tmp_z_curr_prev->RIGHT;
				}

				++x;
				x_prev_new_box = curr_new;
				tmp_x = tmp_x->RIGHT;	// move towards +x axis
			} while (tmp_x);
#ifdef DEBUG
			assert(x == this->max_x + 1);
#endif // DEBUG

			x_prev_new_box = nullptr;

			tmp_z_curr_prev = tmp_z->DOWN;
			tmp_z = tmp_z->BACK_FACING;
			tmp_x = tmp_z;

			--z;
		}
#ifdef DEBUG
		assert(z == this->min_z - 1);
#endif // DEBUG

		this->bottom_right_back = this->bottom_right_back->DOWN;
		this->bottom_right_front = this->bottom_right_front->DOWN;
		this->bottom_left_back = this->bottom_left_back->DOWN;
		this->bottom_left_front = this->bottom_left_front->DOWN;

		--(this->min_y);
		++(this->total_abs.mY);
	}

#ifdef DEBUG
	assert(num_box_required == index);
#endif // DEBUG
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

	bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}

	graph_box_type* new_boxes = allocator.Alloc( num_box_required );

	graph_box_type* tmp_x{ this->top_left_front },
		* tmp_y{ this->top_left_front },
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

		dimen_t x{ min_x }, y{ max_y };

		while (tmp_y)
		{
			x = min_x;

			do
			{
				tmp_x->FRONT_FACING = (new_boxes + index++);

				curr_new = tmp_x->FRONT_FACING;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(x, y, max_z + 1);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , x, y, max_z + 1);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(x, y, max_z + 1);
#endif // !GRAPH_MAT_NO_COORD

				curr_new->BACK_FACING = tmp_x;	// all other pointers are automatically nullptr
				curr_new->LEFT = tmp_x_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_x_curr_prev = curr_new;
				tmp_x = tmp_x->RIGHT;

				if (tmp_y_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_y_curr_prev = tmp_y_curr_prev->RIGHT;
				}

				++x;
			} while (tmp_x);
#ifdef DEBUG
			assert(x == this->max_x + 1);
#endif // DEBUG

			tmp_x_curr_prev = nullptr;	// there isn't anything preeceding the 1st element (in loop when it enters)
			tmp_y_curr_prev = tmp_y->FRONT_FACING;

			tmp_y = tmp_y->DOWN;
			tmp_x = tmp_y;

			--y;
		}
#ifdef DEBUG
		assert(y == this->min_y - 1);
#endif // DEBUG

		this->top_right_front = this->top_right_front->FRONT_FACING;
		this->bottom_right_front = this->bottom_right_front->FRONT_FACING;
		this->top_left_front = this->top_left_front->FRONT_FACING;
		this->bottom_left_front = this->bottom_left_front->FRONT_FACING;

		++(this->max_z);
		++(this->total_abs.mZ);
	}

#ifdef DEBUG
	assert(num_box_required == index);
#endif // DEBUG

}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::inject_z_layer(int num)
{
	// @brief -> first tmp_y goes from MAX_y to MIN_y (towards -ve y), and for each such tmp_y, tmp_x moves from min_x to max_x (towards +x axis)
	if (num < 1)	 return;

	int num_box_required = num * (total_abs.mX * total_abs.mY);

	bool use_initialiser = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ? this->data_initialiser.has_value() : this->__expansion_state.initializer_function.has_value();
	Init_Func* init_function;
	if (use_initialiser) {
		init_function = this->tmp_resize_data.curr_resize_type == RESIZE_TYPE::MANUAL ?
			&this->data_initialiser.value() :
			&this->__expansion_state.initializer_function.value();
	}
	else {
		init_function = nullptr;
	}
	
	graph_box_type* new_boxes = allocator.Alloc(num_box_required);

	graph_box_type* tmp_x{ this->top_left_back },
		* tmp_y{ this->top_left_back },
		* tmp_x_curr_prev{ nullptr },	// just before the current new box
		* tmp_y_curr_prev{ nullptr };	// just above the current new box

	int index = 0;	// index in the memory above (num_box_required)

	graph_box_type* curr_new;

	dimen_t x{ min_x }, y{ max_y };
	
	for (auto i = 0; i < num; i++)
	{
		tmp_x = this->top_left_back;
		tmp_y = this->top_left_back;
		tmp_x_curr_prev = nullptr;
		tmp_y_curr_prev = nullptr;
		y = max_y;

		while (tmp_y)
		{
			x = min_x;
			do
			{
				tmp_x->BACK_FACING = (new_boxes + index++);

				curr_new = tmp_x->BACK_FACING;

				if ( use_initialiser )
				{
					switch ( init_function->index() )	// @caution - When Init_Function typedef is modified, this needs to be modified too
					{
					case 0:
						curr_new->data = std::get<0>( *init_function )();
						break;
					case 1:
						curr_new->data = std::get<1>(*init_function)(x, y, min_z - 1);
						break;
					case 2:
						std::get<2>(*init_function)(curr_new->data , x, y, min_z - 1);
						break;
					case 3:
						std::get<3>(*init_function)(*curr_new);
						break;
					default:
						// @warning - This path NOT implemented, if it's ever reached
						break;
					}
				}

#ifndef GRAPH_MAT_NO_COORD
				curr_new->set_coord(x, y, min_z - 1);
#endif // !GRAPH_MAT_NO_COORD

				curr_new->FRONT_FACING = tmp_x;	// all other pointers are automatically nullptr
				curr_new->LEFT = tmp_x_curr_prev;
				curr_new->UP = tmp_y_curr_prev;

				if (curr_new->LEFT)
					curr_new->LEFT->RIGHT = curr_new;

				if (curr_new->UP)
					curr_new->UP->DOWN = curr_new;

				tmp_x_curr_prev = curr_new;

				if (tmp_y_curr_prev) {	// will only fail for the first vertial line (parallel to y-axis is added the the new layer formed)
					tmp_y_curr_prev = tmp_y_curr_prev->RIGHT;
				}

				tmp_x = tmp_x->RIGHT;

				++x;
			} while (tmp_x);
#ifdef DEBUG
			assert(x == this->max_x + 1);
#endif // DEBUG

			tmp_x_curr_prev = nullptr;	// there isn't anything preeceding the 1st element (in loop when it enters)
			tmp_y_curr_prev = tmp_y->BACK_FACING;

			tmp_y = tmp_y->DOWN;
			tmp_x = tmp_y;

			--y;
		}
#ifdef DEBUG
		assert(y == this->min_y - 1);
#endif // DEBUG

		this->top_right_back = this->top_right_back->BACK_FACING;
		this->bottom_right_back = this->bottom_right_back->BACK_FACING;
		this->top_left_back = this->top_left_back->BACK_FACING;
		this->bottom_left_back = this->bottom_left_back->BACK_FACING;

		--(this->min_z);
		++(this->total_abs.mZ);
	}

#ifdef DEBUG
	assert(num_box_required == index);
#endif // DEBUG
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
inline void Graph_Matrix_3D<node_dtype, dimen_t>::disp_xy_layer(MatrixLayer ltype) const  noexcept
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
inline void Graph_Matrix_3D<node_dtype, dimen_t>::disp_xy_layer(int z_lnum, std::ostream& os) const noexcept
{
	if (z_lnum < min_z || z_lnum > max_z)	return disp_xy_layer(MatrixLayer::TOP);

	auto* start_pos = this->top_left_front;
	int num_jumps = std::abs(max_z - z_lnum);
	while (num_jumps-- > 0)
		start_pos = start_pos->BACK_FACING;

	graphMat::DirectionalIterator<node_dtype, Direction::PURVA, true> y_iter(start_pos);
	graphMat::DirectionalIterator<node_dtype, Direction::DAKSHIN, true> x_iter(start_pos);

	while (y_iter)
	{
		x_iter.curr_box = y_iter.curr_box;
		while (x_iter)
		{
#ifndef GRAPH_MAT_NO_COORD
			os << '(' << x_iter->coordinate << ')' << ' ';
#else
			os << '(' << x_iter->data << ')' << ' ';
#endif
			++x_iter;
		}
		os << '\n';
		++y_iter;
	}
}

/**
* @note - Call this function on a different thread, this function itself, isn't responsible for creating any new threads
* 
* @note2 - This implementation calls expand_once to decide the expansion EACH 500 milliseconds (ie. 0.5 seconds)
*/
template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::auto_expansion()
{
	this->__expansion_state.expansion_flag = true;
	this->__expansion_state.is_actively_expanding = true;

	while (this->__expansion_state.expansion_flag)
	{
		this->expand_once();

		// sleep for 1 unit time
		std::this_thread::sleep_for(std::chrono::milliseconds(this->__expansion_state.milliseconds_in_unit_time));
	}

	this->__expansion_state.is_actively_expanding = false;
	//this->auto_expansion_convar.notify_one();
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::pause_auto_expansion()
{
	// return if not expanding, or "shouldn't" be auto expanding now
	if (!this->__expansion_state.expansion_flag)	return;

	this->__expansion_state.expansion_flag.store(false);
	while (this->__expansion_state.is_actively_expanding) {}	// block till auto_expansion doesn't signal that it's done

	//while (this->__expansion_state.expansion_flag)	// to prevent infinite blocking
	//	this->auto_expansion_convar.wait_for(lock, std::chrono::milliseconds(30000));
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::resume_auto_expansion()
{
	if (this->__expansion_state.expansion_flag) return;    // if already expanding, then return

	this->__expansion_state.reset_initializer();

	std::thread( &Graph_Matrix_3D::auto_expansion, this ).detach();
}

template<typename node_dtype, typename dimen_t>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::set_expansion_rate(float rate) noexcept
{
	this->__expansion_state.expansion_speed = rate;
	this->__expansion_state.curr_expansion_speed = rate;
}

template<typename node_dtype, typename dimen_t>
template<typename Callable>
inline void Graph_Matrix_3D<node_dtype, dimen_t>::resume_auto_expansion(Callable&& inititaliser_func) {	// callable receives the graph_box reference
	static_assert(
		std::is_assignable_v<Init_Func, Callable>,
		"The callable MUST have EITHER of these signatures - void(graph_box_type&) or void(node_dtype, dimen_t, dimen_t, dimen_t)   or   node_dtype(dimen_t, dimen_t, dimen_t)  [with the inside parenthesis being the parameters taken, and outside being the return type]"
	);

	if (this->__expansion_state.expansion_flag.load()) return;    // if already expanding, then return

	this->__expansion_state.set_initializer(inititaliser_func);

	// we `start` it again on another thread (with previous expansion metadata still in __expansion_state object)
	std::thread(&Graph_Matrix_3D::auto_expansion, this).detach();
}

template<typename node_dtype, typename dimen_t>
inline graph_box_type* _reach_from_to(const coord_type& from, const coord_type& to) {
	graph_position g_path;
	if(to.mX < from.mX) {
		g_path[0] = { Direction::PASCHIM, from.mX - to.mX };
	} else {
		g_path[0] = { Direction::PURVA, to.mX - from.mX };
	}

	if(to.mY < from.mY) {
		g_path[1] = { Direction::DAKSHIN, from.mY - to.mY };
	} else {
		g_path[1] = { Direction::UTTAR, to.mY - from.mY };
	}

	if(to.mZ < from.mZ) {
		g_path[2] = { Direction::ADHARASTHA, from.mZ - to.mZ };
	} else {
		g_path[2] = { Direction::URDHWA, to.mZ - from.mZ };
	}

	return this->operator[](std::move(g_path));
}

template<typename node_dtype, typename dimen_t>
inline const graph_box_type* _reach_from_to(const coord_type& from, const coord_type& to) const {

	return  const_cast<Graph_Matrix_3D<node_dtype, dimen_t>*>(this)
			->_reach_from_to(from, to);		// access non-const `this` methods
}

template<typename node_dtype, typename dimen_t>
inline Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::operator[](const coord_type& pos)
{
	// we start from origin, ie. {0,0,0}
	graph_position g_path;
	if (pos.mX < 0) {
		g_path[0] = { Direction::PASHCHIM, -pos.mX };
	}
	else {
		g_path[0] = { Direction::PURVA, pos.mX };
	}

	if (pos.mY < 0) {
		g_path[1] = { Direction::DAKSHIN, -pos.mY };
	}
	else {
		g_path[1] = { Direction::UTTAR, pos.mY };
	}

	if (pos.mZ < 0) {
		g_path[2] = { Direction::ADHARASTHA, -pos.mZ };
	}
	else {
		g_path[2] = { Direction::URDHWA, pos.mZ };
	}

	return this->operator[](g_path);
}

template<typename node_dtype, typename dimen_t>
inline const Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::operator[](const coord_type& pos) const
{
	return const_cast<
		Graph_Matrix_3D<node_dtype, dimen_t>*
	>(this)->operator[](pos);
}

template<typename node_dtype, typename dimen_t>
template< typename GraphPosition >
inline Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::operator[](const GraphPosition& pos)
{
	// @note to viewer -> You can express your view on whether we should prefer simple [x][y] for position or the graph_position typedefed in graph_box.hpp

	static_assert(std::is_same<graph_position, std::remove_reference_t<GraphPosition> > ||
				  std::is_same<old_graphMat::graph_position, std::remove_reference_t<GraphPosition> >,
				  "Only one of graph_position or old_graphMat::graph_position is supported");


	Graph_Box_3D<node_dtype>* tmp{ &origin };
	for (auto& i : pos)
	{
		for (auto j = 0; j < i.second && tmp; j++)
		{
			tmp = tmp->get_adj_box(i.first);
		}
	}

	return tmp;
}

template<typename node_dtype, typename dimen_t>
template< typename GraphPosition >
inline const Graph_Box_3D<node_dtype>* Graph_Matrix_3D<node_dtype, dimen_t>::operator[](const GraphPosition& pos) const
{

	/*Suggested in Effective C++, instead of writing the same code in both the const and non-const duplicate member functions*/
	//return const_cast<const Graph_Box_3D<node_dtype>*> (
	//	const_cast<Graph_Matrix_3D<node_dtype, dimen_t>*>(this)
	//	->operator[](pos)
	//	);

	return const_cast<Graph_Matrix_3D<node_dtype, dimen_t>*>(this)
		->operator[](pos);
}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::Graph_Matrix_3D() : Graph_Matrix_3D({ 1,1,1 }) {}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::Graph_Matrix_3D(const coord_type& dimensions) :
	Matrix_Base(3),
	total_abs({ 1,1,1 }),	/*absolute_ values aren't max-min, since at beginning min=max=0, but absolute is 1*/
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
	this->resize(dimensions.mX, dimensions.mY, dimensions.mZ);
}

template<typename node_dtype, typename dimen_t>
template<typename Func>
inline Graph_Matrix_3D<node_dtype, dimen_t>::Graph_Matrix_3D(const coord_type& dimensions, Func&& initialiser) :	// with initialiser
	Matrix_Base(3),
	total_abs({ 1,1,1 }),	/*absolute_ values aren't max-min, since at beginning min=max=0, but absolute is 1*/
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
	static_assert(
		std::is_assignable_v<Init_Func, Func>,
		"The callable MUST have EITHER of these signatures - void(graph_box_type&) or void(node_dtype, dimen_t, dimen_t, dimen_t)   or   node_dtype(dimen_t, dimen_t, dimen_t)  [with the inside parenthesis being the parameters taken, and outside being the return type]"
	);

	this->resize(dimensions.mX, dimensions.mY, dimensions.mZ, initialiser, RESIZE_TYPE::MANUAL);
}

template<typename node_dtype, typename dimen_t>
inline Graph_Matrix_3D<node_dtype, dimen_t>::~Graph_Matrix_3D()
{
	// actually wanted to use promise, so that we wait till the promise is set by pause_auto_expansion, but then we can't make the destructor noexcept if that matters, if i am seeing this you will consider this possibility :D
	this->pause_auto_expansion();

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
