
#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <map>

#include "../include/quickgl/AllocatorVBO.hpp"

#include "Test.hpp"

	
#define ALLOCATOR(name) qgl::AllocatorVBO name("")

namespace TestsAllocatorVBO {
	
// 	struct Pair {
// 		uint32_t p;
// 		uint32_t count;
// 		uint64_t vStart;
// 	};
// 	
// 	class TestAllocator {
// 	public:
// 		qgl::AllocatorVBO allocator;
// 		std::map<uint32_t, Pair> allocated;
// 		std::vector<uint64_t> data;
// 		
// 		uint64_t gCounter;
// 		
// 		TestAllocator() : allocator(8, false) {}
// 		
// 		uint64_t& at(uint32_t id) {
// 			if(data.size() <= id) {
// 				data.resize(id+1);
// 			}
// 			return data[id];
// 		}
// 
// 		uint32_t AllocateInit(uint32_t count) {
// 			uint32_t p = allocator.Allocate(count);
// 			allocated[p] = {p, count, gCounter};
// 			for(int i=0; i<count; ++i, ++gCounter) {
// 				at(i) = gCounter;
// 			}
// 			return p;
// 		}
// 		
// 		uint64_t FreeCheck(Pair pair) {
// 			uint64_t counter = pair.vStart;
// 			uint64_t invalid = 0;
// 			for(int i=0; i<pair.count; ++i, ++counter) {
// 				if(data[pair.p+i] != counter) {
// 					++invalid;
// 				}
// 			}
// 			allocator.Free(pair.p, pair.count);
// 			allocated.erase(pair.p);
// 			return invalid;
// 		}
// 		
// 		uint64_t FreeRandom() {
// 			auto it = allocated.begin();
// 			std::advance(it, rand() % allocated.size());
// 			return FreeCheck(it->second);
// 		}
// 	};
// 	
// 	void TestAllocation() {
// 		TestAllocator alloc;
// 
// 		for(int j=0; j<64; ++j) {
// 			for(int32_t i=0; i<64; ++i) {
// 				alloc.AllocateInit(i);
// 			}
// 		}
// 		
// 		for(int i=0; i<32; ++i) {
// 			
// 		}
// 
// 
// 
// 	}
	
	
	
	void allocate_one_two() {
		ALLOCATOR(allocator);
		
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		
		ASSERT_EQUAL(first, 0, "First allocation has to be equal 0");
		ASSERT_EQUAL(second, 32, "");
	}
	
	void allocate_two_free_allocate_two() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		
		allocator.Free(first, 32);
		uint32_t third = allocator.Allocate(48);
		uint32_t fourth = allocator.Allocate(29);
		uint32_t fifth = allocator.Allocate(3);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		ASSERT_EQUAL(third, 96, "");
		ASSERT_EQUAL(fourth, 0, "");
		ASSERT_EQUAL(fifth, 29, "");
	}
	
	void free_neighboring_front() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		uint32_t third = allocator.Allocate(128);
		uint32_t fourth = allocator.Allocate(256);
		
		allocator.Free(first, 32);
		allocator.Free(second, 64);
		uint32_t fifth = allocator.Allocate(95);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		ASSERT_EQUAL(fifth, first, "");
	}
	
	void free_neighboring_middle() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		uint32_t third = allocator.Allocate(128);
		uint32_t fourth = allocator.Allocate(256);
		
		allocator.Free(second, 64);
		allocator.Free(third, 128);
		uint32_t fifth = allocator.Allocate(191);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		ASSERT_EQUAL(fifth, second, "");
	}
	
	void free_neighboring_back() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		uint32_t third = allocator.Allocate(128);
		uint32_t fourth = allocator.Allocate(256);
		
		allocator.Free(third, 128);
		allocator.Free(fourth, 256);
		uint32_t fifth = allocator.Allocate(383);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		ASSERT_EQUAL(fifth, third, "");
	}
	
	void free_single_end_allocate_more() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(32);
		uint32_t second = allocator.Allocate(64);
		uint32_t third = allocator.Allocate(128);
		uint32_t fourth = allocator.Allocate(256);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		allocator.Free(third, 128);
		allocator.Free(fourth, 256);
		uint32_t fifth = allocator.Allocate(512);
		
		ASSERT_EQUAL(fifth, third, "");
	}
	
	void RunAll() {
		allocate_one_two();
		allocate_two_free_allocate_two();
		free_neighboring_front();
		free_neighboring_middle();
		free_neighboring_back();
		free_single_end_allocate_more();
	}
}

