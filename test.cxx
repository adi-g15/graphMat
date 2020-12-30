#include <iostream>

#include "graph_square_mat.hpp"
#include "3d_graph_mat.hpp"

using namespace std;

void func(Graph_Matrix_3D<int>& mat){
	static int count = 1;
	std::cout << "Called " << count++ << "th time: \n";
	//for (int i = mat.min_z; i <= mat.max_z; i++)
	//{
	//	mat.disp_xy_layer(i);	// debug
	//	std::cout << "\n\n";	// debug
	//}
}

int main(int argc, char const *argv[])
{
#if 0
	int init_rows{4}, init_cols{8}, final_rows{3}, final_cols{15};
	Graph_Matrix<int> mat(init_rows,init_cols);
	mat.displayMat();
	mat.resize(final_rows, final_cols);
	mat.displayMat();
#endif

#if 1
	Graph_Matrix_3D<int> mat;

	mat.resize({ 3,3,3 });
	mat.init([](int x, int y, int z) {	return x + y + z;	});
	func(mat);
	//mat.pop_xminus_layer();
	//func(mat);
	//mat.pop_xplus_layer();

	//func(mat);
	//mat.pop_yminus_layer();	// @problem
	//func(mat);
	//mat.pop_yplus_layer();

	//func(mat);
	//mat.pop_zplus_layer();
	//func(mat);
	//mat.resize({ 4,6,5 });
	//mat.init([](int x, int y, int z) {	return x + y + z;	});
	//func(mat);
	//mat.pop_zminus_layer();
	func(mat);

#endif

	return 0;
}
