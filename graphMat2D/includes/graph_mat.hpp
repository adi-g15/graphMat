#pragma once

#include "graph_mat_decl.hpp"

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::pushCol(){ // add a column at end
	this->tr_box->RIGHT = new graph_box_type;
	this->tr_box->RIGHT->LEFT = this->tr_box;

	graph_box_type* prev_new_box = this->tr_box->RIGHT;

	// @future- Can use util::range_iterator later, for the same iteration
	graph_box_type* last_col_iter = this->tr_box->DOWN != nullptr ? this->tr_box->DOWN : this->br_box;   //iterator in last column
		// in the else case of ?: so that the next loop doesn't execute in case the matrix contained only 1 row

	dimen_t count{1};
	while( last_col_iter != this->br_box ){ //ideally shouldn't be infinite, since last_col_iter->DOWN should reach the bottom surely
		// we also have to give the previous box, left, right, down pointer to the new Box
		last_col_iter->RIGHT = new graph_box_type;
		last_col_iter->RIGHT->LEFT = last_col_iter;
		last_col_iter->RIGHT->UP = prev_new_box;

		prev_new_box->DOWN = last_col_iter->RIGHT;
		prev_new_box = last_col_iter->RIGHT;
		last_col_iter = last_col_iter->DOWN;

		++count;
	}

	if( this->br_box != this->tr_box ){
		this->br_box->RIGHT = new graph_box_type;
		prev_new_box->DOWN = this->br_box->RIGHT;
		prev_new_box->DOWN->UP = prev_new_box;
		this->br_box->RIGHT->LEFT = this->br_box;
		this->br_box->RIGHT->UP = prev_new_box;
	}

	this->tr_box = this->tr_box->RIGHT;
	this->br_box = this->br_box->RIGHT;    // new bottom right

	++(this->x_max);
	++(this->_total_x_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::pushRow(){ // add a row at downward end
	this->bl_box->DOWN = new graph_box_type;
	this->bl_box->DOWN->UP = this->bl_box;

	graph_box_type* prev_new_box = this->bl_box->DOWN;

	// @future- Can use util::range_iterator later, for the same iteration
	graph_box_type* last_row_iter = this->bl_box->RIGHT != nullptr ? this->bl_box->RIGHT : this->br_box;   //iterator in last column
		// in the else case of ?: so that the next loop doesn't execute in case the matrix contained only 1 row

	dimen_t count{1};
	// the below condition can be replaced with equivalent for loop using _N_Columns
	while( last_row_iter != this->br_box ){ //ideally shouldn't be infinite, since last_row_iter->RIGHT should reach the bottomRight surely
		// we also have to give the previous box, left, right, down pointer to the new Box
		last_row_iter->DOWN = new graph_box_type;
		last_row_iter->DOWN->UP = last_row_iter;
		last_row_iter->DOWN->LEFT = prev_new_box;

		prev_new_box->RIGHT = last_row_iter->DOWN;
		prev_new_box = last_row_iter->DOWN;
		last_row_iter = last_row_iter->RIGHT;

		++count;
	}

	if( this->br_box != this->bl_box ){
		this->br_box->DOWN = new graph_box_type;
		prev_new_box->RIGHT = this->br_box->DOWN;
		prev_new_box->RIGHT->LEFT = prev_new_box;
		this->br_box->DOWN->UP = this->br_box;
		this->br_box->DOWN->LEFT = prev_new_box;
	}

	this->bl_box = this->bl_box->DOWN;
	this->br_box = this->br_box->DOWN;    // new bottom right

	--(this->y_min);	// ie. increase in negative
	++(this->_total_y_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::injectCol(){ // add a column at leftmost
	this->tl_box->LEFT = new graph_box_type;
	this->tl_box->LEFT->RIGHT = this->tl_box;

	graph_box_type* prev_new_box = this->tl_box->LEFT;	//the latest new box

	// @future- Can use util::range_iterator later, for the same iteration
	graph_box_type* last_col_iter = this->tl_box->DOWN != nullptr ? this->tl_box->DOWN : this->bl_box;   //iterator in last column
		// in the else case of ?: so that the next loop doesn't execute in case the matrix contained only 1 row

	dimen_t count{1};
	// @CAUTION - Seg Fault here, Cause-> tl_box and bl_box aren't connected in a up to down fashion
	while( last_col_iter != this->bl_box ){ //ideally shouldn't be infinite, since last_col_iter->DOWN should reach the bottom surely
		// we also have to give the previous box, left, right, down pointer to the new Box
		last_col_iter->LEFT = new graph_box_type;
		last_col_iter->LEFT->RIGHT = last_col_iter;
		last_col_iter->LEFT->UP = prev_new_box;

		prev_new_box->DOWN = last_col_iter->LEFT;
		prev_new_box = last_col_iter->LEFT;
		last_col_iter = last_col_iter->DOWN;

		++count;
	}

	#if DEBUG_MODE
		test::logger::__IsConncted(this->tl_box, this->bl_box, DOWN);
	#endif
	if( this->bl_box != this->tl_box ){
		this->bl_box->LEFT = new graph_box_type;
		prev_new_box->DOWN = this->bl_box->LEFT;
		prev_new_box->DOWN->UP = prev_new_box;
		this->bl_box->LEFT->RIGHT = this->bl_box;
		this->bl_box->LEFT->UP = prev_new_box;
	}

	this->tl_box = this->tl_box->LEFT;
	this->bl_box = this->bl_box->LEFT;    // new bottom right

	--(this->x_min);	//increment in negative side
	++(this->_total_x_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::injectRow(){ // add a row at begin
	this->tl_box->UP = new graph_box_type;
	this->tl_box->UP->DOWN = this->tl_box;

	graph_box_type* prev_new_box = this->tl_box->UP;	//the latest new box

	graph_box_type* last_row_iter = this->tl_box->RIGHT != nullptr ? this->tl_box->RIGHT : this->tr_box;   //iterator in last column
		// in the else case of ?: so that the next loop doesn't execute in case the matrix contained only 1 row

	dimen_t count{1};
	// the below condition can be replaced with equivalent for loop using _N_Columns
	while( last_row_iter != this->tr_box ){ //ideally shouldn't be infinite, since last_row_iter->RIGHT should reach the bottomRight surely
		// we also have to give the previous box, left, right, down pointer to the new Box
		last_row_iter->UP = new graph_box_type;
		last_row_iter->UP->DOWN = last_row_iter;
		last_row_iter->UP->LEFT = prev_new_box;

		prev_new_box->RIGHT = last_row_iter->UP;
		prev_new_box = last_row_iter->UP;
		last_row_iter = last_row_iter->RIGHT;

		++count;
	}

	if( this->tr_box != this->tl_box ){	// ie. row was not having only one element, since in that case we already did a new graph_box_type
		this->tr_box->UP = new graph_box_type;
		prev_new_box->RIGHT = this->tr_box->UP;
		prev_new_box->RIGHT->LEFT = prev_new_box;
		this->tr_box->UP->DOWN = this->tr_box;
		this->tr_box->LEFT->UP = prev_new_box;
	}

	this->tl_box = this->tl_box->UP;
	this->tr_box = this->tr_box->UP;    // new top right

	++(this->y_max);
	++(this->_total_y_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::popRow_upper(){   //remove the uppermost row ( = y_max)
	if( this->tl_box == this->bl_box )   return; //don't pop out the origin row (y=0)

	graph_box_type* temp = this->tl_box;
	this->tl_box = this->tl_box->DOWN;    //since this bl_box will be removed

	// we have to clear the right pointers of this->bl_box here
	while( temp != this->tr_box ){
		temp->DOWN->UP = nullptr;    // logically temp->LEFT != nullptr, since atleast it will point to an element in the origin 1st column
		temp = temp->RIGHT;

		delete temp->LEFT;
	}

	this->tr_box = this->tr_box->DOWN;
	delete this->tr_box->UP;
	this->tr_box->UP = nullptr;

	--(this->y_max);
	--(this->_total_y_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::popCol_left(){   //remove the leftmost column; INTERNAL but can be used
	if( this->tl_box == this->tr_box )   return; //don't pop out the origin column (x=0)

	graph_box_type* temp = this->tl_box;
	this->tl_box = this->tl_box->RIGHT;

	// we have to clear the down pointers of this->tr_box here
	while( temp != this->bl_box ){
		temp->RIGHT->LEFT = nullptr;
		temp = temp->DOWN;

		delete temp->UP;
	};

	this->bl_box = this->bl_box->RIGHT;
	delete this->bl_box->LEFT;
	this->bl_box->LEFT = nullptr;

	++(this->x_min);	//decrement in negative
	--(this->_total_x_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::popCol(){   //by default, removes the rightmost column ( = x_max ), if not available calls popCol_front() to try to remove the leftmost column, so it's safe to just use this function only
	if( this->tr_box == this->tl_box )   return; //don't pop out the origin column (x=0)

	if( this->x_max == 0 ) return this->popCol_left();

	graph_box_type* temp = this->tr_box;
	this->tr_box = this->tr_box->LEFT;

	// we have to clear the down pointers of this->tr_box here
	while( temp != this->br_box ){
		temp->LEFT->RIGHT = nullptr;    // logically temp->left != nullptr, since atleast it will point to an element in the origin 1st column
		temp = temp->DOWN;

		delete temp->UP;
	};

	this->br_box = this->br_box->LEFT;
	delete this->br_box->RIGHT;
	this->br_box->RIGHT = nullptr;

	--(this->x_max);
	--(this->_total_x_abs);
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::popRow(){   //remove the last row ( = n_row-1)
	if( this->tl_box == this->bl_box )   return; //don't pop out the origin row (y=0)

	if( this->y_min == 0 )	return this->popRow_upper();

	graph_box_type* temp = this->bl_box;
	this->bl_box = this->bl_box->UP;    //since this bl_box will be removed

	// we have to clear the right pointers of this->bl_box here
	while( temp != this->br_box ){
		// @CAUTION - SEG FAULT
		temp->UP->DOWN = nullptr;    // logically temp->left != nullptr, since atleast it will point to an element in the origin 1st column
		temp = temp->RIGHT;

		delete temp->LEFT;
	}

	this->br_box = this->br_box->UP;
	delete this->br_box->DOWN;
	this->br_box->DOWN = nullptr;

	++(this->y_min);	//decrement in negative side
	--(this->_total_y_abs);
}

template< typename node_dtype, typename dimen_t>
std::pair< std::make_unsigned_t<dimen_t>, std::make_unsigned_t<dimen_t> > Graph_Matrix<node_dtype, dimen_t>::getDimensions() const{
	return { this->_total_y_abs, this->_total_x_abs };    //return in form of rows, cols
}

template< typename node_dtype, typename dimen_t>
auto Graph_Matrix<node_dtype, dimen_t>::getNumRows() const{
	return this->_total_y_abs;
}

template< typename node_dtype, typename dimen_t>
auto Graph_Matrix<node_dtype, dimen_t>::getNumCols() const{
	return this->_total_x_abs;
}

template<typename node_dtype, typename dimen_t>
template<typename _Func>
inline void Graph_Matrix<node_dtype, dimen_t>::for_each(graph_box_type* source, Direction dir, _Func func)
{
	Graph_Box<node_dtype>* tmp{ source };

	while (tmp)
	{
		func(tmp->data);	// depends on func whether it takes by reference or not
		tmp = tmp->get_adj_box(dir);
	}
}

template<typename node_dtype, typename dimen_t>
template<typename _Func>
inline void Graph_Matrix<node_dtype, dimen_t>::for_each(graph_box_type* begin, graph_box_type* end, Direction dir, _Func func)
{
	Graph_Box<node_dtype>* tmp{ begin };

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

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::displayMat(std::ostream& os) const{
	// @todo - Use util::range_iterator here to iterate through all boxes

	const graph_box_type* box = this->tl_box;
	const graph_box_type* row_start = this->tl_box;

	while( row_start != nullptr ){
		// @todo - Have a check here, to see if `operator<<(ostream, node_dtype)` is available or not, if not then replace `os << box->data` to `os << box`
		box = row_start;
		while( box != nullptr ){
			os << box->data << " | ";
			box = box->RIGHT;
		}
		os << '\n';
		row_start = row_start->DOWN;
	}
}

template< typename node_dtype, typename dimen_t>
void Graph_Matrix<node_dtype, dimen_t>::resize(dimen_t _num_rows, dimen_t _num_cols){
	// @future - Use this->__capacity to, use the nodes, previously allocated only, instead of allocating again
	if( _num_rows < 0 || _num_cols < 0 ){
		throw std::length_error("Cannot have a graph with negative dimensions");
	}

	if( _num_cols == 0 || _num_rows == 0 )	return;	//as for now, don't allow 0 cols or rows, though even later it is less probable to have such behaviour

	--_num_cols;	--_num_rows;	//to adapt to the usual coordinate system, so that if user asks to resize to 1x1, program understands as 0x0 which is the origin

	// @future - Instead of simply popping, prefer to change size only, utilizing __capacity
	while( _num_rows < this->_total_y_abs ){
		this->popRow();
	}

#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif

	while( _num_cols < this->_total_x_abs ){
		this->popCol();
	}

#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif

	_num_cols -= this->_total_x_abs;	//_initial_num_cols and _initial_num_rows will now how how many `more` to add
	_num_rows -= this->_total_y_abs;

	dimen_t left_inc, right_inc, down_inc, up_inc;
	up_inc = _num_rows / 2;
	down_inc = _num_rows - _num_rows / 2;
	left_inc = _num_cols / 2;
	right_inc = _num_cols - _num_cols / 2;

#if DEBUG_MODE
	this->displayMat();
	std::cout << std::endl;
#endif
	while( down_inc-- > 0 ){  //add _initial_num_cols/2 columns, to bottom
		this->pushRow();
	}
#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif

#if DEBUG_MODE
	this->displayMat();
	std::cout << std::endl;
#endif
	while( up_inc-- > 0 ){  //add _initial_num_cols-_initial_num_cols/2 cols, above
		this->injectRow();
	}
#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif

#if DEBUG_MODE
	this->displayMat();
	std::cout << std::endl;
#endif
	while( right_inc-- > 0 ){  //add _initial_num_rows/2 cols, to right
		this->pushCol();
	}

#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif
#if DEBUG_MODE
	this->displayMat();
	std::cout << std::endl;
#endif
	while( left_inc-- > 0 ){  //add _initial_num_rows-_initial_num_rows/2 cols, to left
		this->injectCol();
	}
#if DEBUG_MODE
	this->displayMat();
	std::cout << std::endl;
#endif

#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif
}


// ACCESSOR FUNCTIONS //
template< typename node_dtype, typename dimen_t>
Graph_Box<node_dtype>* Graph_Matrix<node_dtype, dimen_t>::operator[](const coord_type& pos){

	return const_cast<
		const Graph_Matrix<node_dtype, dimen_t>*
	>(this)->operator[](pos);
}

template< typename node_dtype, typename dimen_t>
const Graph_Box<node_dtype>* Graph_Matrix<node_dtype, dimen_t>::operator[](const coord_type& pos) const{

	// we start from origin, ie. {0,0,0}
	graph_position g_path;
	if (pos.mX < 0) {
		g_path.push_back({ Direction::PASHCHIM, -pos.mX });
	}
	else {
		g_path.push_back({ Direction::PURVA, pos.mX });
	}

	if (pos.mY < 0) {
		g_path.push_back({ Direction::DAKSHIN, -pos.mY });
	}
	else {
		g_path.push_back({ Direction::UTTAR, pos.mY });
	}

	return this->operator[]( std::move(g_path) );
}

template< typename node_dtype, typename dimen_t>
const Graph_Box<node_dtype>* Graph_Matrix<node_dtype, dimen_t>::operator[](const graph_position& pos) const{
	// @note to viewer -> You can express your view on whether we should prefer simple [x][y] for position or the graph_position typedefed in graph_box.hpp

	const Graph_Box<node_dtype>* tmp{ &origin };
	for (auto& i : pos)
	{
		for (auto j = 0; j < i.second && tmp; j++)
		{
			tmp = tmp->get_adj_box(i.first);
		}
	}

	return tmp;
}

template< typename node_dtype, typename dimen_t>
Graph_Box<node_dtype>* Graph_Matrix<node_dtype, dimen_t>::operator[](const graph_position& pos) {

	/*Suggested in Effective C++, instead of writing the same code in both the const and non-const duplicate member functions*/
	return const_cast<Graph_Box<node_dtype>*> (
		const_cast<const Graph_Matrix<node_dtype, dimen_t>*>(this)
			->operator[](pos)
	);
}
// ACCESSOR FUNCTIONS //

// CONSTRUCTORS AND DESTRUCTORS	//

template< typename node_dtype, typename dimen_t>
Graph_Matrix<node_dtype, dimen_t>::Graph_Matrix() : Graph_Matrix(1, 1) {}

/*
	The origin -> as per the current initialisation of x_min, x_max and y_min y_max,
					the origin box is the square with the corners -> (0,0), (0,1), (1,1), (1,0)

					_total_x_abs(1), _total_y_abs(1), x_min(0), x_max(1), y_min(0), y_max(1), origin(0,0)
*/
template< typename node_dtype, typename dimen_t>
Graph_Matrix<node_dtype, dimen_t>::Graph_Matrix(dimen_t _initial_num_rows, dimen_t _initial_num_cols) : Matrix_Base(2), _total_x_abs(0), _total_y_abs(0), x_min(0), x_max(0), y_min(0), y_max(0){
	if( _initial_num_rows < 0 || _initial_num_cols < 0 ){
		throw std::length_error("Cannot have a graph with negative dimensions");
	}

	this->origin.data = decltype(this->origin.data){};	// @caution -> due to this , and maybe code at other places, the `node_dtype` has to copy-constructible

	this->tl_box = &origin;
	this->bl_box = &origin;
	this->tr_box = &origin;
	this->br_box = &origin;

	while( --(_initial_num_rows) ){  //add _initial_num_rows-1 columns (already initialises by one default)
		this->pushRow();
	}
	while( --(_initial_num_cols) ){  //add _initial_num_cols-1 columns (already initialises by one default)
		this->pushCol();
	}

#if DEBUG_MODE
	test::logger::__IsConncted(tl_box, bl_box, DOWN);
	test::logger::__IsConncted(tl_box, tr_box, RIGHT);
	test::logger::__IsConncted(tr_box, br_box, DOWN);
	test::logger::__IsConncted(bl_box, br_box, RIGHT);
#endif

}

template< typename node_dtype, typename dimen_t>
Graph_Matrix<node_dtype, dimen_t>::~Graph_Matrix(){
	while( this->_total_y_abs != 0 ){  //excluding the origin node, which will automatically get destructed, since it's not a pointer
		popRow();   //remove the last row ( = n_row-1)     
	}

	while( this->_total_x_abs != 0 ){  //excluding the origin node, which will automatically get destructed, since it's not a pointer
		popCol();
	}

#if DEBUG_MODE
	using std::cout; using std::endl;
	clog << "Removed all"<< endl;
#endif
}

// CONSTRUCTORS AND DESTRUCTORS	//
