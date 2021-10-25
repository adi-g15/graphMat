#include "graph_mat.hpp"

template <typename node_dtype, typename dimen_t = int32_t>
class Cube_Matrix : public Graph_Matrix_3D<node_dtype, dimen_t> {
  public:
    inline void resizeOrder(int n) { this->resize({n, n, n}); }

    inline auto getOrder() const noexcept {
        return std::get<0>(this->get_size());
    }

    Cube_Matrix() : Graph_Matrix_3D<node_dtype, dimen_t>() {}

    Cube_Matrix(dimen_t order)
        : Graph_Matrix_3D<node_dtype, dimen_t>({order, order, order}) {}

    template <typename Func>
    Cube_Matrix(dimen_t order, Func &&func)
        : Graph_Matrix_3D<node_dtype, dimen_t>({order, order, order}, func) {}

    ~Cube_Matrix() {}
};
