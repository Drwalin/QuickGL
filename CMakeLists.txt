cmake_minimum_required(VERSION 3.8)
project(QuickGL)
cmake_policy(SET CMP0115 OLD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QUICKGL_BUILD_EXAMPLES "Build QuickGL examples" ON)
option(QUICKGL_BUILD_TEST "Build QuickGL tests" ON)

add_subdirectory(OpenGLWrapper)

include_directories(./include/)
include_directories(./OpenGLWrapper/include)

aux_source_directory(./include/quickgl/ header_files)
aux_source_directory(./include/quickgl/util/ header_files_util)
aux_source_directory(./include/quickgl/materials/ header_files_materials)
aux_source_directory(./include/quickgl/pipelines/ header_files_pipelines)
aux_source_directory(./include/quickgl/postprocess/ header_files_postprocess)
aux_source_directory(./include/quickgl/cameras/ header_files_cameras)
aux_source_directory(./src/ source_files)
aux_source_directory(./src/util/ source_files_util)
aux_source_directory(./src/materials/ source_files_materials)
aux_source_directory(./src/pipelines/ source_files_pipelines)
aux_source_directory(./src/postprocess/ source_files_postprocess)
aux_source_directory(./src/cameras/ source_files_cameras)

# imgui
include_directories(./thirdparty/imgui/)
include_directories(./thirdparty/imgui/backends)
aux_source_directory(./thirdparty/imgui/ imgui)

add_library(QuickGL
	${header_files}
	${source_files}
	${header_files_util}
	${source_files_util}
	${header_files_materials}
	${source_files_materials}
	${header_files_pipelines}
	${source_files_pipelines}
	${header_files_postprocess}
	${source_files_postprocess}
	${header_files_cameras}
	${source_files_cameras}
	
	${imgui}
	./thirdparty/imgui/backends/imgui_impl_opengl3.h
	./thirdparty/imgui/backends/imgui_impl_opengl3.cpp
	./thirdparty/imgui/backends/imgui_impl_opengl3_loader.h
	./thirdparty/imgui/backends/imgui_impl_glfw.h
	./thirdparty/imgui/backends/imgui_impl_glfw.cpp
)
target_link_libraries(QuickGL OpenGLWrapper)

if(QUICKGL_BUILD_TESTS)
	add_executable(tests
		tests/TestsMain
		tests/TestsAllocator
		tests/TestsIdsManager
	)
	target_link_libraries(tests QuickGL)
endif()

if(QUICKGL_BUILD_EXAMPLES)
	add_executable(sample_conceptual_quickgl
		samples/conceptual/DrwalinLibraries/Noise.cpp
		samples/conceptual/DrwalinLibraries/Perlin.cpp
		samples/conceptual/main.cpp)
	target_link_libraries(sample_conceptual_quickgl QuickGL)

	add_custom_target(
		run ALL
		COMMAND sample_conceptual_quickgl
		DEPENDS sample_conceptual_quickgl
		COMMENT "Running sample"
	)
	set_target_properties(run PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

