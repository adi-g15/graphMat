# Matrix as a Graph
----

GraphMat

A matrix header-only library, uses graphs internally, helpful when your matrix is part of a simulation where it needs to grow many times.

This was originally a subproject for the worldline project (a catchy name, but an interesting small simulator for me)
So this is still in development, to make it more general, though as of now, it should work for most templated versions, following the needs (for example you need to overload the << operator for std::ostream for display function to work)

## Purpose -

I needed a matrix like data structure, that can have
1. Expansion and deletetion capabilities like that of a list (ie. no need to copy again and again for expansion as in std::vector (and i am not talking about the capacity vs size here, that would not work for my purpose))

2. access as a vector 2x2 matrix, or atleast fast access

Then, i had read somewhere, _A matrix is inherently a graph, each node connected to 4 adjacent nodes_, though it maybe very simple, but quite intutive it seems.

So, now the point of it, So i created a matrix data structure, based on a graph, that completes the requirements for `point 1 above`, though it doesn't have constant time lookups like std::vector, but for my case, i chose to create it like this.

<hr />

# Graph Matrix 3D (Documentation)
These are brief documentation of the important parts of `Graph_Matrix_3D`

## Creating a 3D GraphMatrix

* Currently, three constructors are there for a `Graph_Matrix_3D` object -
  * Graph_Matrix_3D() - The default constructor creates a new matrix for you with the dimensions {1,1,1}, and no initialiser
  * Graph_Matrix_3D(initial_size) - Constructs a new matrix with the size you pass in. This is equivalent to using the default constructor, then resize(initial_size)
  * Graph_Matrix_3D(initial_size, custom_initialiser) - This is the most descriptive of all three, and provides most control, since apart from the initial size, you can also pass in a lambda here, that will be executed for each new graph node that will be created, and used to assign/modify the data. For the signature of the initialiser function, it is discussed later on.
  
  > Example code:
  > ```cpp
  > Graph_Matrix_3D<int> matrix;
  > Graph_Matrix_3D<std::string> smat;
  > Graph_Matrix_3D<DataClass> dmat({5, 5, 5});
  > Graph_Matrix_3D<DataClass> dmat({5, 5, 5}, [n=1](DataClass& data, int x, int y, int z) mutable {
  >      data.my_data_member = 100*x + 10*y + z;
  >      std::cout << "Adding box: " << n++ << '\n';
  >  });
  > 
  > Graph_Matrix_3D<WillUsePointer*> ptrmat;
  > Graph_Matrix_3D<WillUsePointer*> ptrmat({200,100,300}, [](){return nullptr;});
  >
  > ```

  > Note - As of now, if using a non-pointer data type to instantiate the matrix, it MUST be default_constructible, though still you have about the same level of control, since after the data is constructed, you can recieve a modifiable reference passed as parameter to your initialiser
  > **Special Notes for Pointers** - If using pointers to point to data held by the graph boxes, IMO using a initialiser function that returns a pointer can be very helpful

## Resizing the matrices
The signature of resize functions is the same as the constructors, it has 2 overloads - 
  * `resize(final_size)` - Resizes to the final size you pass
  * `resize(final_size, initialiser)` - Again, while constructing the new nodes, this callback/initialiser will be used, or you can simple accept a `node_dtype&` parameter in your callback/lambda function passed as the initialiser

  > Example Code:
  > ```cpp
  > matrix.resize({80,100,90}, [](int x, int y, int z) {
  >    return x*y*z;    // this will be assigned to the data stored
  > })
  > 
  > dmat.resize({1,1,1});   // if you specify a dimension lesser than the current size, the rest portion is dropped, and {1,1,1} will be the final size
  >
  > ```

### Signature of the initialiser function
These are the supported signatures of the data initialiser functions

  * std::function<node_dtype && ()>
  * std::function<node_dtype && (int, int, int)>
  * std::function<void (node_dtype&, int, int, int)>
  * std::function<void (Graph_Box_3D<node_dtype>&)>

## Auto Expanding matrices

* Now this is the actual reason, this started as a subproject of the worldLineSimulator.
* This data structure can *AUTO EXPAND* and can be totally independent from other actions, as it runs on a different thread of its own (though for things like explicit resize or pause_auto_expansion, the threads do have communication using convars)

* To use it you just have to call matrix.`resume_auto_expansion()` and now the matrix will keep expanding with the default expansion rate (1 unit in each direction, per unit time, which by default is 1 second, which can be changed if you will by just changing the value of the miili_in_unit_time variable)

* `resume_auto_expansion(initialiser)` is another overload that accepts an initialiser too, just like the constructor

  > Tip: If you want you can set two different initialisers for manual resizes (or at construction), and the one used for auto_expansion, since internally auto_expansion data initialiser is treated as different from the one set using resize()

* To stop the auto expansion, you can either call `pause_auto_expansion()` or... wait for destruction ! (of the object, since it will neverthless be called in the destructor for you).

  It blocks the thread on which it is called till auto expansion has _safely_ stopped, since there can be cases when there is a resize already taking place inside some loop, so the pause_auto_expansion() function uses a `std::condition_variable` to communicate with the auto_expansion method (NOTE: `auto_expansion` shouldn't directly be called, hence is private), so pause_auto_expansion only returns after `auto_expansion()` notifies `pause_auto_expansion()` that it has finished executing, and will end

  > Example Code:
  > ```cpp
  > matrix.resume_auto_expansion(); // no initialiser is used for newer box's data
  > 
  > dmatrix.resume_auto_expansion([](Graph_Box<DataClass>& graph_node) -> void {
  >      std::cout << "Created a new box :D...";    // you will notice this message constantly popup on your console, as your matrix keeps expanding
  > });
  > 
  > intmat.set_expansion_rate(0.4);
  > intmat.resume_auto_expansion([n=1]() mutable -> int {
  >     std::cout << "Expanding at 0.4 units per unit time :D"; })
  >     return n++;
  > });
  > ```

  * **Controlling the expansion rate**

    For changing the expansion rate at any time, even during an auto_expansion (note that auto_expansion doesn't pay any special attention to the change in value, it just uses the current expansion rate), you can call `set_expansion_rate(float_value)` with a float value

    > Note: If interested, you can also think of this the other way, rather than changing the rate, just modify how long a unit of time last (this is thug life beware ;D), there is a variable named `milliseconds_in_unit_time`, so relative to our world, but it is constexpr currently, since you can get the intended change by changing the rate too

## Getting Dimensions of the matrix
  For current dimensions, `get_size()` member function can be called, it returns a `std::tuple` of the three dimensions.
  Internally, the data structure also holds the minimum and maximum extent of the x,y,z axis, ie, mix_x, max_x, and same for y and z axis, so they may also be retrieved by just accessing them (Shouldn't be needed in most cases)

  > Example Code:
  > ```cpp
  > 
  > auto dimen = matrix.get_size();
  > std::cout << "Current Size: " << std::get<0>(dimen) << ',' <<  std::get<1>(dimen) << ',' <<  std::get<2>(dimen);    // better overload std::ostream<< for std::tuple, if required many times
  > 
  > ```

## Finding an Element
  It is advised to have your own finding algorithm based on the data you are storing in the matrix, though a simple `find(value)` member function is also provided, that internally uses a `Swastic Finder` algorithm, i created for this only :D, it is not the best, and in fact can prove worse than a simple BFS, but i implemented this, as i said the programmer is provoked(i couldn't think of a better word), to write their own algorithm, but the default will also work.

  > Example Code:
  > ```cpp
  > auto* box = imat.find(235);
  > if(box != nullptr)  std::cout << "Found " << box->data << " in the matrix";
  >
  > auto* box2 = objectMatrix.find(some_object);    // the `==` operator is used to compare equality for the objects
  >
  > ```

  > Note: Another overload that allows passing a equality_check (BinaryPredicate) function should also be there

  > **Details about the Swastic Finder**
  >
  > In this algorithm, we actually iterate through the elements like a swastic, and it is a MULTI-THREADED algorithm (overkill for small matrices actually), each xz plane is searched on different threads, and for each plane, the center from where we start is the center of the imaginary Swastic sign, then for each branch as in the Swastic sign, the branches are searched concurrently (in the direction swastic symbol points out from each branch)

## Iterating on the boxes/elements
  You can chose to receive the box(ie. reference to a Graph_Box_3D<>) or the data(ie, node_dtype&) you passed for each box, by just specifying in the lambda/function you pass

  This data structure currently has 2 member functions to iterate -
  * `for_all` - Accepts a lambda or other function, that is executed for ALL the elements of the node (this can be a bit costly for a huge matrix)

    > Example Code:
    > ```cpp
    > 
    > dmatrix.for_all([n=1](const DataClass&) mutable {
    >      std::cout << "Executed";
    > });   // the number of times 'Executed' is printed, will be equal to total number of elements in the matrix
    > 
    > intmat.for_all([](int& data) mutable -> void {
    >     return data++;    // modifying value (not generally required like this)
    > });
    > ```

  * `for_each` - It has two overloads, 
    1. `for_each(source, direction, function)`: Starting from the source, it executes the function for EACH element in the passed direction
    2. `for_each(begin, end, direction, function)`: Takes begin and end pointers, pointing to two Graph_Box_3d* objects, and the direction in which to move (the direction is required else we will have to search for end starting from start), and executes the passed function object for EACH element between begin and end (both INCLUSIVE) in the direction specified

    > Note: `for_each(begin, end, dir, func)` expects that begin and end pointers are connected, and in the direction passed

    > Example Code:
    > ```cpp
    > 
    > auto* begin = dmatrix.find({45, 60}); // expecting {45, 60} is in the matrix, skipping null check here
    > auto* end = begin->RIGHT->RIGHT->RIGHT;
    > dmatrix.for_each(begin, end, Direction::RIGHT, [n=1](const DataClass&) mutable {
    >      std::cout << "Executed";
    > });   // the number of times 'Executed' is printed, will be equal to total number of elements in the matrix
    > 
    >
    > auto* begin2 = dmatrix.find(234);
    > auto* end2 = begin->DOWN->DOWN->DOWN; // should check for begin2 == nullptr
    > intmat.for_each(begin2, end2, Direction::DOWN, [](int& data) mutable -> void {
    >     return data += 45;    // modifying value (not generally required like this)
    > });
    > ```

    Though iterators can be implemented, and i did give it a try too, but either way the iterator needs to have internal state, specifying what direction to move, so instead of that, i preferred having these functions named similar to the counterparts in STL algorithms library

### Displaying the Matrix
Displaying the matrix is not of priority in this currently, and for_each and for_all functions also do it well for most use cases so it hasn't been implemented till now. For debugging purposes, there's a `disp_xy_layer(layer_number, output_stream)` is provided
Access to elements is through the other algorithms only for now, or can extend the class through inheritance to make more private members accessbile through public functions, if in case you would really want to do that.

### Multi-Thread nature
Many algorithms are multi-threaded by defualt, and so some use of locks and condition_variable is there, to for example, stop the search on other threads too, when one thread has found it, or for pause_auto_expansion etc.

### Errors
I have used static_asserts with possible reasons so that the programmer doesn;t actually get those lengthy errors that point out the errors in C++ STL code itself ;D, or the lambda, though still if you find something that can have better error messages using static_asserts or just some error to fix/ask please do mention in the issues.

### Tests
Currently i have another repo for tests for this, as i was still learning it, so No, it doesn't have any actual automated tests for now.

### Bonus (Customizations, MemoryPools and ThreadPools)
There is a customizations/ directory, that will contain some customization, say for example, an extended matrix that provides better efficiency for sparse matrices, but still can develop above this implementation, or implementation that utilise custom MemoryPool or ThreadPool implementations, these are under development only, but its development will significantly slow down, since i move to my WorldLine Simulator again, but neverthless i would keep making it more customizable.

<hr />

# Note -
You can likely just use a std::list<std::list>, though i preferred more specific use case, whihc can be the case with you too to try this

## Probable Uses -

It will likely be used as the `world plot`, since think, this world will be expanding (continuosly or on need basis, so I didn't wanted `vector<vector>`, that had been quite an overhead for expansion like 40\*40 to 100\*100)
Also, in case of `vector<vector>`, you will likely have it grow in ONLY TWO DIRECTIONS, and I WANTED IT TO GROW EQUALLY IN ALL FOUR DIRECTIONS, now this would, almost always mean, that even for a simple expansion, the `vector<vector>` has to completely allocate to a whole new place

© Aditya Gupta 2021
