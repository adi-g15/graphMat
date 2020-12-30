#pragma once

#include "graph_mat.hpp"
#include "graph_square_mat.hpp"

#include "3d_graph_mat.hpp"
#include "cube_mat.hpp"

template<typename node_dtype, int dimen_t = int32_t>
using Matrix_2D = Graph_Matrix<node_dtype, dimen_t>;

template<typename node_dtype, int dimen_t = int32_t>
using Square_Matrix = Square_Matrix<node_dtype, dimen_t>;

template<typename node_dtype, int dimen_t = int32_t>
using Matrix_3D = Graph_Matrix_3D<node_dtype, dimen_t>;

template<typename node_dtype, int dimen_t = int32_t>
using Cubic_Matrix = Cube_Matrix<node_dtype, dimen_t>;
