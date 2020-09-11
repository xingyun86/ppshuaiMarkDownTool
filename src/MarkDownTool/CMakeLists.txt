# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(MarkDownTool)

set(ADMIN_LEVEL "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")

# Include sub-projects.
#add_subdirectory("maddy")
add_subdirectory("discount")
add_subdirectory("MarkDownTool")