
#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <map>

#include "../include/quickgl/AllocatorVBO.hpp"

#include "Test.hpp"

#define ALLOCATOR(name) qgl::AllocatorVBO name("")

namespace TestsAllocatorVBO {
	
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
		uint32_t fifth = allocator.Allocate(192);
		
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
		
		allocator.Free(third, 128);
		allocator.Free(fourth, 256);
		uint32_t fifth = allocator.Allocate(512);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 32, "");
		ASSERT_EQUAL(fifth, third, "");
	}
	
	void free_single_end_allocate_more_2() {
		ALLOCATOR(allocator);
		uint32_t first = allocator.Allocate(1024);
		uint32_t second = allocator.Allocate(1024);
		uint32_t third = allocator.Allocate(1024);
		uint32_t fourth = allocator.Allocate(1024);
		
		allocator.Free(third, 1024);
		allocator.Free(fourth, 1024);
		uint32_t fifth = allocator.Allocate(3072);
		
		ASSERT_EQUAL(first, 0, "");
		ASSERT_EQUAL(second, 1024, "");
		ASSERT_EQUAL(fifth, third, "");
	}
	
	void RunAll() {
		allocate_one_two();
		allocate_two_free_allocate_two();
		free_neighboring_front();
		free_neighboring_middle();
		free_neighboring_back();
		free_single_end_allocate_more();
		free_single_end_allocate_more_2();
	}
}

