#include "3d_graph_mat.hpp"

template < typename node_dtype, typename dimen_t = int32_t >
class Cube_Matrix : protected Graph_Matrix_3D<node_dtype, dimen_t>
{
	dimen_t order{ 1 };
public:
	void resizeOrder(int n) {
		this->resize({ n,n,n });
		this->order = std::min ( n, this->total_abs.mX );	// can directly do `= n;` too
	}

	auto getOrder() const { return this->order; }

	Cube_Matrix()
	{
	}

	Cube_Matrix(dimen_t order)
	{
		resizeOrder(order);
	}

	~Cube_Matrix()
	{
	}

private:

};
