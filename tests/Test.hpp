
#ifndef QUICKGL_TEST_HPP
#define QUICKGL_TEST_HPP

#include <cstdio>
#include <iostream>

#include <vector>
#include <map>
#include <string>
#include <sstream>

struct TestInfo {
	char const* name;
	uint32_t lineNumber;
	char const* fileName;
	char const* functionName;
	bool success;
	std::string failMessage;
	
	inline void Print(TestInfo* prev) {
		int level = 2;
		if(prev == NULL) {
			level = 0;
		} else if(prev->fileName != this->fileName) {
			level = 0;
// 		} else if(prev->name == this->name) {
// 			level = 2;
		} else if(prev->functionName == this->functionName) {
			level = 2;
		} else {
			level = 1;
		}
		
		if(level < 1) {
			printf("\n\ntest suite '%s'\n", fileName);
		}
		if(level < 2) {
			printf("\n\n  test '%s' in function %s\n", name, functionName);
		}
		printf("    %i\t ... %s", lineNumber, success?"SUCCESS":"FAILED");
		if(!success) {
			printf(" -> %s", failMessage.c_str());
		}
		printf("\n");
		fflush(stdout);
	}
};

extern std::vector<TestInfo> testsInfos;

#define TO_STR(__X) #__X

inline static void PushASSERT(TestInfo t) {
	if(testsInfos.size())
		t.Print(&testsInfos.back());
	testsInfos.push_back(t);
// 	if(t.success == false) {
// 		getchar();
// 	}
}

#define ASSERT_BASE_(__A, __B, __OP, __NAME) \
	{ \
		PushASSERT(TestInfo{__NAME, __LINE__, __FILE__, __func__, \
		__A __OP __B, \
			(std::stringstream()<<__A<<" "<<TO_STR(__OP)<<" "<<__B).str() \
				}); \
	}
// 		if(!(__A __OP __B)) { \
// 			std::cout<<"    Failed: "<<__FILE__<<":"<<__LINE__<<" ... "; \
// 			std::cout<<"    "<<__A<<" "<<TO_STR(__OP)<<" "<<__B<<"\n"; \
// 			std::cout<<std::flush; \
// 		} \
// 		

#define ASSERT_EQUAL(__A, __B, __NAME) ASSERT_BASE_(__A, __B, ==, __NAME)
#define ASSERT_NOTEQUAL(__A, __B, __NAME) ASSERT_BASE_(__A, __B, !=, __NAME)
#define ASSERT_TRUE(__A, __NAME) ASSERT_BASE_(__A, false, !=, __NAME)
#define ASSERT_FALSE(__A, __NAME) ASSERT_BASE_(__A, false, ==, __NAME)

#endif

