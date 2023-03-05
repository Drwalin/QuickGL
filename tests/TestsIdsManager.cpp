
#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <map>

#include "../include/quickgl/IdsManager.hpp"

#include "Test.hpp"

namespace TestsIdsManager {
	void free_two_allocate_three() {
		qgl::IdsManager mg;
		uint32_t first = mg.GetNewId();
		uint32_t second = mg.GetNewId();
		uint32_t third = mg.GetNewId();
		uint32_t fourth = mg.GetNewId();
		
		mg.FreeId(third);
		mg.FreeId(second);
		uint32_t fifth = mg.GetNewId();
		uint32_t sixth = mg.GetNewId();
		uint32_t seventh = mg.GetNewId();
		
		ASSERT_EQUAL(fifth, second, "");
		ASSERT_EQUAL(sixth, third, "");
		ASSERT_EQUAL(seventh, fourth+1, "");
		ASSERT_EQUAL(first, second-1, "");
	}
	
	void RunAll() {
		free_two_allocate_three();
	}
}


