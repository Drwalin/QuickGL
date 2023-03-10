
#include "Test.hpp"

std::vector<TestInfo> testsInfos;

namespace TestsAllocator {
	void RunAll();
}

namespace TestsIdsManager {
	void RunAll();
}

int main() {
	TestsAllocator::RunAll();
	TestsIdsManager::RunAll();
	
	int correct = 0;
	for(int i=0; i<testsInfos.size(); ++i) {
// 		testsInfos[i].Print(i ? &(testsInfos[i]) : NULL);
		if(testsInfos[i].success)
			++correct;
	}
	
	printf("\n\n Summary:  %i / %i correct\n", correct, (int)testsInfos.size());
	
	fflush(stdout);
	return 0;
}

