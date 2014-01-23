cmake_minimum_required(VERSION 2.8)

project(rme)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_FLAGS                "-Wall -Wextra -Wno-unused-parameter -fpermissive -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D__DEBUG__")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

find_package(OpenGL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost 1.34.0 COMPONENTS thread system REQUIRED)

find_package(wxWidgets COMPONENTS html aui gl adv core net base REQUIRED)
include(${wxWidgets_USE_FILE})

include(source/CMakeLists.txt)
add_executable(rme ${rme_SRC})

include_directories(${Boost_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(rme ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${LibArchive_LIBRARIES} ${OPENGL_LIBRARIES})
