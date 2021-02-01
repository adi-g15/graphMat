#pragma once

#include "3d_graph_mat_decl.hpp"
#include <cassert>
#include <stack>

// .begin() will be this default constructed using a box pointer, and .end() will be an iterator with center_box = nullptr, and directions_to_check.empty() == true
namespace graphMat{
	template<typename node_dtype, bool IS_CONST = false>
	struct NeighbourIterator {
		typedef std::forward_iterator_tag iterator_category;
		typedef Graph_Box_3D<node_dtype> value_type;
		typedef void difference_type;

		using pointer = std::conditional_t<IS_CONST, const value_type*, value_type*>;
		using reference = std::conditional_t<IS_CONST, const value_type&, value_type&>;

		pointer center_box;
		pointer curr_box;
		int plane_count;	// either 1,2, or 3 signifying number of planes to try
		static constexpr std::array<Direction, 8> all_directions{
			Direction::PASHCHIM,
			Direction::VAYAVYA,
			Direction::UTTAR,
			Direction::ISHANYA,
			Direction::PURVA,
			Direction::AGNEYA,
			Direction::DAKSHIN,
			Direction::NAIRUTYA
		};
		std::stack<Direction> directions_to_check;

		operator bool() const noexcept;
		NeighbourIterator& operator++() noexcept;
		reference operator*() noexcept;
		pointer operator->() noexcept;
		NeighbourIterator(pointer begin_node) noexcept;
	};
}

namespace graphMat {
	template<typename node_dtype, bool IS_CONST>
	inline NeighbourIterator<node_dtype, IS_CONST>::operator bool() const noexcept
	{
		return this->center_box != nullptr;
	}

	template<typename node_dtype, bool IS_CONST>
	inline NeighbourIterator<node_dtype, IS_CONST>& NeighbourIterator<node_dtype, IS_CONST>::operator++() noexcept
	{
		// if center_box == nullptr :	ie. no DIRECT neighbour remaining, three planes tried

		// at max we will try the plane in FRONT, MIDDLE, AND BACK, plane_count keeps track ot this, else we may just keep doing this->center = this->center->BACK_FACING;, counting even non-neighbours
		if (center_box && plane_count != 0 && directions_to_check.empty()) {	// directions_to_check will only be resetted when center_box is still valid
			for (auto dir : NeighbourIterator::all_directions)
				directions_to_check.push(dir);

			--plane_count;

			this->center_box = plane_count != 0 ? this->center_box->BACK_FACING: nullptr;
			this->curr_box = this->center_box;
		}
		else {
			this->curr_box = nullptr;
			while (curr_box == nullptr && !directions_to_check.empty())	// likely this will run only once, only at border cases, it may run multiple times, to find a non-null neighbour
			{
				const auto& dir = directions_to_check.top();
				curr_box = center_box->get_adj_box(dir);
				directions_to_check.pop();
			}

			if (curr_box == nullptr && plane_count > 0)	this->operator++();
		}

		return *this;
	}

	template<typename node_dtype, bool IS_CONST>
	inline typename NeighbourIterator<node_dtype, IS_CONST>::reference NeighbourIterator<node_dtype, IS_CONST>::operator*() noexcept
	{
#ifdef DEBUG
		assert(this->curr_box != nullptr);
#endif // DEBUG

		return *(this->curr_box);
	}

	template<typename node_dtype, bool IS_CONST>
	inline typename NeighbourIterator<node_dtype, IS_CONST>::pointer NeighbourIterator<node_dtype, IS_CONST>::operator->() noexcept
	{
		return this->curr_box;
	}

	template<typename node_dtype, bool IS_CONST>
	inline NeighbourIterator<node_dtype, IS_CONST>::NeighbourIterator(pointer begin_node) noexcept : center_box(begin_node), curr_box(begin_node), plane_count(1)
	{
		if (center_box->BACK_FACING != nullptr)	++plane_count;
	
		if (center_box->FRONT_FACING != nullptr) {
			center_box = center_box->FRONT_FACING;
			++plane_count;
		}

		for (auto dir : NeighbourIterator::all_directions)
			directions_to_check.push(dir);
	}
}
