#pragma once

	// this is a customization, worldLine Simulator won't be needing the coordinates, so disabling use of them
	// this is a Sparse matrix, that works by constantly `activating` and `deactivating` regions (logically), and since this may have multiple allocations and deallocations repeatedly, so a memory pool is a MUST for this
	// also there is a `watcher thread` that keeps running in the background throughout the existence of this matrix, which basically works like a `garbage collector` (difference being it's not on basis of usecount, rather last time used), that implements the `deactivating` part
#define GRAPH_MAT_NO_COORD true

#include "../../3d_graph_mat.hpp"

template<typename node_dtype, typename dimen_t>
class SparseSimulationMat: public Graph_Matrix_3D<std::remove_pointer_t<node_dtype>*, dimen_t> {
	using dtuple = std::tuple<dimen_t, dimen_t, dimen_t>;
public:
	SparseSimulationMat() : Graph_Matrix_3D({ 1,1,1 }, []() {return nullptr;}) {

	}
private:
	void ActivateRegions(const dtuple&, const dtuple&);
	void DeAllocateRegions(const dtuple&, const dtuple&);
};
