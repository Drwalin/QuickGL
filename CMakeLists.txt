cmake_minimum_required(VERSION 3.8)
project(QuickGL)
cmake_policy(SET CMP0115 OLD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_DEBUG "Build debug" ON)
option(BUILD_RELEASE "Build release" OFF)
option(BUILD_TEST "Build tests" ON)

add_subdirectory(OpenGLWrapper)

include_directories(./include/)
include_directories(./OpenGLWrapper/include)

aux_source_directory(./include/quickgl/ header_files)
aux_source_directory(./src/ source_files)

add_library(QuickGL SHARED
	${header_files}
	${source_files}
)
target_link_libraries(QuickGL OpenGLWrapper)



# if(BUILD_TESTS)
	add_executable(tests
		tests/TestsMain
		tests/TestsAllocatorVBO
	)
	target_link_libraries(tests QuickGL)
# endif()



# if(BUILD_EXAMPLES)
# 	add_executable(samples
# 		samples/Sample
# 	)
# 	target_link_libraries(samples QuickGL)
# endif()



if(UNIX)
elseif(WIN32)
else()
	# print error message: not tested platform
endif()

add_compile_options(-ggdb3)
add_compile_options(-ggdb)
add_compile_options(-pg)
add_compile_options(-fprofile-arcs -ftest-coverage --coverage -lgcov)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -ggdb3 -ggdb -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -Ofast -s -pedantic")	
	
# if(BUILD_DEBUG)
# 	add_compile_options(-ggdb3)
# 	add_compile_options(-ggdb)
# 	add_compile_options(-pg)
# 	add_compile_options(--coverage)
# 	# gcov file.cpp
# 	# geninfo ./ -b ./ -o ./coverage.info
# 	# genhtml ./coverage.info -o /tmp/html
# else()
# endif()

# if(BUILD_RELEASE)
# 	add_compile_options(-Ofast)
# 	add_compile_options(-s)
# else()
# endif()




