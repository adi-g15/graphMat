#pragma once

#include "3d_graph_mat_decl.hpp"
#include <cassert>
#include <stack>
#include <bitset>

// .begin() will be this default constructed using a box pointer, and .end() will be an iterator with center_box = nullptr, and directions_to_check.empty() == true
namespace graphMat{
	// @flaw - There's a flaw in NeighbourIterator, ie. it will just keep going backwards (z axis) to find neighbours (since it starts search from box->BACK_FACING)
	template<typename node_dtype, bool IS_CONST = false>
	struct NeighbourIterator {
		typedef std::forward_iterator_tag iterator_category;
		typedef Graph_Box_3D<node_dtype> value_type;
		typedef void difference_type;

		using pointer = std::conditional_t<IS_CONST, const value_type*, value_type*>;
		using reference = std::conditional_t<IS_CONST, const value_type&, value_type&>;

		pointer center_box, curr_box;
		pointer _original_center_box;
		std::bitset<2> free_planes;
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

		// warning this returns increment coords wrt. to the current center, without considering any plane changes, you have to handle it yourselves, for eg. by just checking if your center == this center, if not then only 2 checks if UP or DOWN
		util::_coord3D<typename value_type::dimen_t> _getIncrementCoords() const noexcept;	// can be used to calculate the new coordinates through the last direction turned from center
		std::optional<Direction> _getLastTurnedDirection() const noexcept;

		NeighbourIterator(pointer begin_node) noexcept;
	};

	template<typename node_dtype, bool IS_CONST = false>
	class LinearIterator {
		using iterator_type = std::forward_iterator_tag;	// it can be made random access, but that has an additional cost of if conditions and swicthes to find opposite direction
		using value_type = Graph_Box_3D<node_dtype>;
		using difference_type = void;
		using pointer = std::conditional_t<IS_CONST, const value_type*, value_type*>;
		using reference = std::conditional_t<IS_CONST, const value_type&, value_type&>;

	public:
		pointer curr_box;
		const Direction forward_direction;

		operator bool() const noexcept;
		LinearIterator& operator++() noexcept;
		reference operator*() noexcept;
		pointer operator->() noexcept;

		LinearIterator(pointer start_box, Direction forward_direction) noexcept;
	};

	// @note - Just compile time version of the graphMat::LinearIterator
	template<typename node_dtype, Direction forward_direction, bool IS_CONST = false>
	class DirectionalIterator {
		using iterator_type = std::forward_iterator_tag;	// it can be made random access, but that has an additional cost of if conditions and swicthes to find opposite direction
		using value_type = Graph_Box_3D<node_dtype>;
		using difference_type = void;
		using pointer = std::conditional_t<IS_CONST, const value_type*, value_type*>;
		using reference = std::conditional_t<IS_CONST, const value_type&, value_type&>;

	public:
		pointer curr_box;

		operator bool() const noexcept;
		DirectionalIterator& operator++() noexcept;
		reference operator*() noexcept;
		pointer operator->() noexcept;

		DirectionalIterator(pointer start_box) noexcept;
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
		if (directions_to_check.empty() && center_box && free_planes.any() ) {	// directions_to_check will only be resetted when center_box is still valid
			for (auto dir : NeighbourIterator::all_directions)
				directions_to_check.push(dir);

			if (free_planes[0]) {
				this->center_box = this->_original_center_box->BACK_FACING;
				free_planes.reset(0);
			}
			else
			{
				this->center_box = this->_original_center_box->FRONT_FACING;
				free_planes.reset(1);
			}
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

			if (curr_box == nullptr) {	// directions_to_check has become empty(), this call recursive call causes for the center_box to be changed to another plane, and then we start afresh trying it's neighbours
				if (free_planes.any())	this->operator++();
				else this->center_box = nullptr;	// we don't have any more planes to check, and i don't want it to enter the if condition, even then center_box will become null and stack will have content
			}
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
	inline util::_coord3D<typename Graph_Box_3D<node_dtype>::dimen_t> NeighbourIterator<node_dtype, IS_CONST>::_getIncrementCoords() const noexcept
	{
		if (directions_to_check.size() == all_directions.size())	return { 0,0,0 };
		else
		{
			const Direction last_turned_direction = all_directions[directions_to_check.size()];

			return util::direction3D::getIncrementCoords(last_turned_direction);
		}
	}

	template<typename node_dtype, bool IS_CONST>
	inline std::optional<Direction> NeighbourIterator<node_dtype, IS_CONST>::_getLastTurnedDirection() const noexcept
	{
		if (directions_to_check.size() == all_directions.size()) {
			if (this->center_box != this->_original_center_box) {
				return (this->center_box == this->_original_center_box->FRONT_FACING) ?
						Direction::URDHWA :
						Direction::ADHARASTHA;
			}
			else
			{
				return std::nullopt;	// ie. NO turn has been made
			}
		}
		else
		{
			return all_directions[directions_to_check.size()];
		}
	}

	template<typename node_dtype, bool IS_CONST>
	inline NeighbourIterator<node_dtype, IS_CONST>::NeighbourIterator(pointer begin_node) noexcept : center_box(begin_node), curr_box(begin_node), _original_center_box(center_box)
	{
		if (center_box->BACK_FACING != nullptr)	free_planes.set(0);

		if (center_box->FRONT_FACING != nullptr) free_planes.set(1);

		for (auto dir : NeighbourIterator::all_directions)
			directions_to_check.push(dir);

		/**
		 * This `directions_to_check is a stack whose bottom is all_directions[0]`
		 * 
		 * |        |
		 * |VAIYAVYA|
		 * |PASHCHIM|
		 * ----------
		 */
	}




	//	LINEAR ITERATOR	//
	template<typename node_dtype, bool IS_CONST>
	inline LinearIterator<node_dtype, IS_CONST>::operator bool() const noexcept {
		return this->curr_box != nullptr;	// true when curr_box still valid
	}

	template<typename node_dtype, bool IS_CONST>
	inline LinearIterator<node_dtype, IS_CONST>& LinearIterator<node_dtype, IS_CONST>::operator++() noexcept {
		this->curr_box = this->curr_box->get_adj_box(forward_direction);
		return *(this);
	}

	template<typename node_dtype, bool IS_CONST>
	inline typename LinearIterator<node_dtype, IS_CONST>::reference LinearIterator<node_dtype, IS_CONST>::operator*() noexcept {
		return *(this->curr_box);
	}

	template<typename node_dtype, bool IS_CONST>
	inline typename LinearIterator<node_dtype, IS_CONST>::pointer LinearIterator<node_dtype, IS_CONST>::operator->() noexcept {
		return this->curr_box;
	}

	template<typename node_dtype, bool IS_CONST>
	LinearIterator<node_dtype, IS_CONST>::LinearIterator(pointer start_box, Direction forward_direction) noexcept :
		curr_box(start_box),
		forward_direction(forward_direction)
	{
#ifdef DEBUG
		assert(start_box != nullptr);
#endif // DEBUG
	}

	//	DIRECTIONAL ITERATOR	//
	template<typename node_dtype, Direction forward_direction, bool IS_CONST>
	inline DirectionalIterator<node_dtype, forward_direction, IS_CONST>::operator bool() const noexcept {
		return this->curr_box != nullptr;	// true when curr_box still valid
	}

	template<typename node_dtype, Direction forward_direction, bool IS_CONST>
	inline DirectionalIterator<node_dtype, forward_direction, IS_CONST>& DirectionalIterator<node_dtype, forward_direction, IS_CONST>::operator++() noexcept {
		this->curr_box = this->curr_box->get_adj_box(forward_direction);
		return *(this);
	}

	template<typename node_dtype, Direction forward_direction, bool IS_CONST>
	inline typename DirectionalIterator<node_dtype, forward_direction, IS_CONST>::reference DirectionalIterator<node_dtype, forward_direction, IS_CONST>::operator*() noexcept {
		return *(this->curr_box);
	}

	template<typename node_dtype, Direction forward_direction, bool IS_CONST>
	inline typename DirectionalIterator<node_dtype, forward_direction, IS_CONST>::pointer DirectionalIterator<node_dtype, forward_direction, IS_CONST>::operator->() noexcept {
		return this->curr_box;
	}

	template<typename node_dtype, Direction forward_direction, bool IS_CONST>
	DirectionalIterator<node_dtype, forward_direction, IS_CONST>::DirectionalIterator(pointer start_box) noexcept:
		curr_box(start_box)
	{
#ifdef DEBUG
		assert(start_box != nullptr);
#endif // DEBUG
	}

}
