// This memory pool is based on using the Bucket_Handle, but there will likely need to be management using it, it's likely beneficial when many array allocations (and in special cases, of almost same sizes) are to be done

#include <stack>
#include <cstdint>
#include <cstdlib>	// for std::malloc
#include <cassert>
#include <exception>
#include <memory>
#include <vector>

/*
* @author - Aditya Gupta
* 
 * @CAUTION - This is highly experimental, especially as in current state, as for what i think, there is still some memory wastage to be accounted for, though still if in terms of allocation speed it is a bit faster
 * 
 */

template<typename Graph_Box_Type>
class Bucket_Handle {
	Bucket<Graph_Box_Type>* parent_bucket;
	int start_index;	// start index in parent's memory
	int size_allowed;

	void deallocate_handle() {
		// it doesn't 'own' the memory
		parent_bucket->set_mem_free(start_index, size_allowed);
	}

public:
	Bucket_Handle(parent_bucket, start_index, size) : parent_bucket(parent_bucket), start_index(start_index), size_allowed(size) {
		parent_bucket->set_mem_in_use(start_index, size);
	}

protected:	// shouldn't be used for polymorphic access to delete a derived class (not suggested though)
	~Bucket_Handle() { deallocate_handle(); }
};

template<typename Graph_Box_Type>
class Bucket {	// each bucket in itself is NOT thread-safe
	using Box_Ptr = Graph_Box_Type*;
	typedef uint16_t size_type;

	Box_Ptr mem_arr;
	size_type length;
	// when a box in use, the ledger stores 1 at that pos
	std::vector<bool> use_ledger;	// 2^15-1 boxes max
	void set_mem_in_use(int16_t start, int16_t length) {
		length += start;
		while ( start != length )	// length is acting now as the 'end'
		{
			use_ledger[start] = true;
		}
	}

	void set_mem_free(int16_t start, int16_t length) {
		length += start;
		while (start != length)	// length is acting now as the 'end'
		{
			use_ledger[start] = false;
		}
	}

	std::unique_ptr<Bucket_Handle*> get_handle_with_size(int16_t size) {
		int tmp = size;
		int k = 0;
		for (;k < this->length; ++k)
		{
			--tmp;
			if (mem_arr[k])	tmp = size;

			if (tmp == 0)
			{
				return new Bucket_Handle<Graph_Box_Type>(this, k+1 -size, size);
			}
		}

		return nullptr;
	}
public:
	Bucket(int16_t size): use_ledger(size, false) {
		mem_arr = std::malloc(size);	// @warning - Initialising the data ;?
#ifdef DEBUG
		assert(mem_arr != nullptr);
#endif // DEBUG

		this->length = size;
	}

	~Bucket() {
#ifdef DEBUG
			// @warning - Because std::vector<bool>::iterator is implementation-defined, it may not satisfy the LegacyForwardIterator requirement. Use of algorithms such as std::search that require LegacyForwardIterators may result in either compile-time or run-time errors.
		assert(std::none_of(use_ledger.begin(), use_ledger.end(), [](bool b) {return b;}));	// The bucket must ONLY be destructed when no memory held by it is in use
#endif // DEBUG
	}

	friend Bucket_Handle<Graph_Box_Type>::Bucket_Handle(), void Bucket_Handle<Graph_Box_Type>::deallocate_handle();
};

template<typename Graph_Box_Type>
final class MemoryPool {
	std::stack< Graph_Box_Type* > free_mem_locations;
	std::stack< int16_t > free_mem_locations_sizes;

public:
	void* allocate() {	// allocate one
		return this->allocate_many(1);
	}

	void* allocate_many(int16_t num_boxes) {
		return new Graph_Box_Type[num_boxes];	// throws std::bad_alloc when invalid
	}

	void deallocate() {

	}
};
