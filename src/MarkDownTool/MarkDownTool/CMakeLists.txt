# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(project_name MarkDownTool)
project(${project_name})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include sub-projects.
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/discount
	${CMAKE_BINARY_DIR}/discount/include
	${CMAKE_SOURCE_DIR}/maddy/include
)
link_directories(
	${CMAKE_BINARY_DIR}/discount
)

IF(CMAKE_CL_64)
set(EXT_BIN 64)
ELSE(CMAKE_CL_64)
set(EXT_BIN 32)
ENDIF(CMAKE_CL_64)

FILE(GLOB SRCS "*.cpp" "*.c" "*.def" "*.rc")
add_executable(${project_name}${EXT_BIN} ${SRCS})

list(APPEND project_name_cflags /W4)
list(APPEND project_name_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=_WIN32_WINNT_VISTA)
MESSAGE(STATUS "CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE})

set(CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
	)
foreach(CompilerFlag ${CompilerFlags})
	string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
	
target_compile_definitions(${project_name}${EXT_BIN} PRIVATE ${project_name_defines})
target_compile_options(${project_name}${EXT_BIN} PRIVATE ${project_name_cflags})

set(MSVC_LIBS wininet)
	
if(MSVC)
SET_TARGET_PROPERTIES(${project_name}${EXT_BIN} PROPERTIES LINK_FLAGS ${ADMIN_LEVEL})
endif(MSVC)

target_link_libraries(${project_name}${EXT_BIN} 
PRIVATE libmarkdown
PRIVATE ${MSVC_LIBS}
)