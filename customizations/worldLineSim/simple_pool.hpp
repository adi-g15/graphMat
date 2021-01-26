// Only slightly different from the other memory_pool in the sense that the user doesn't have to keep record of the bucket_handles received, and this itself stores very little additional info, so this one is NOT recommended for use if big array allocations are to be used used

#include <stack>
#include <cstdint>
#include <cstdlib>	// for std::malloc
#include <cassert>
#include <exception>
#include <memory>
#include <vector>
#include <bitset>

/*
 * @author - Aditya Gupta
 */

template<typename T, uint8_t BlockCount = 32>
class Bucket {	// each bucket in itself is NOT thread-safe
	using T_Ptr = T*;
	typedef uint8_t size_type;

	T_Ptr memory;
	// when a box in use, the ledger stores 1 at that pos
	std::bitset<BlockCount> use_ledger;

	[[nodiscard]] T_Ptr allocate_single() {
		size_type pos = 0;
		for(; pos < BlockCount; ++)
		{
			if(!use_ledger[pos]){
				break;
			}
		}
		if(pos < BlockCount) {
			this->set_mem_in_use(pos);
			return memory + pos;
		} else return nullptr;
	}

	// 0-based indexing
	void set_mem_in_use(size_type pos) {
#ifdef DEBUG
		assert(pos <= BlockCount)
#endif

		use_ledger.set(pos);
	}

	void set_mem_free(size_type pos) {
#ifdef DEBUG
		assert(pos <= BlockCount)
#endif

		use_ledger.reset();
	}

	T_Ptr get_handle_with_size(size_type size) {
		int tmp = size;
		int k = 0;
		for (;k < this->length; ++k)
		{
			--tmp;
			if (memory[k])	tmp = size;

			if (tmp == 0)
			{
				return memory + k;
			}
		}

		return nullptr;
	}
public:
	Bucket() {
		this->memory = std::malloc(BlockCount * sizeof(T));	// @warning - Initialising the data ;?
#ifdef DEBUG
		assert(memory != nullptr);
#endif // DEBUG
	}

	~Bucket() {
#ifdef DEBUG
		assert(use_ledger.none());	// The bucket must ONLY be destructed when no memory held by it is in use
#endif // DEBUG

		std::free(this->memory);
	}

};

template<typename T>
class MemoryPool {
	using T_Ptr = T*;
	std::vector<Bucket<T, 32>> buckets;

public:
	MemoryPool() {
		this->buckets.reserve(64);
		this->buckets.emplace_back();
	}

	[[nodiscard]] void* allocate() {	// allocate one
		auto iter = std::find_if(buckets.begin(), buckets.end(), [](const auto& bucket) {
			return bucket.use_ledger.any();
		});

		if(iter != buckets.end()) {
			return iter->allocate_single();
		} else {
			this->buckets.emplace_back();
			return this->buckets.back().allocate_single();
		}
	}

	void deallocate(T_Ptr ptr) noexcept {
		auto iter = std::find_if(buckets.begin(), buckets.end(), [](const auto& bucket) {
			return bucket.contains(ptr);
		});

#ifdef DEBUG
		assert(iter != buckets.end());
#endif

		if(iter != buckets.end()) {
			iter->deallocate(ptr);
		}
	}
};
