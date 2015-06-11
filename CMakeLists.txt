cmake_minimum_required(VERSION 2.8)
project(svnf_simu)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")


find_package(NS3 3.23 REQUIRED core csma network internet point-to-point applications)
find_package(Boost REQUIRED regex )

MESSAGE("${NS3_LIBRARIES}")



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNS3_LOG_ENABLE")


include_directories(${NS3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set(SOURCE_FILES
./src/main.cpp src/GwApplication.cc
src/CachingControllerApplication.cpp
src/MD5.cpp
src/VideoDataSource.cpp
)
add_executable(svnf_simu ${SOURCE_FILES})
target_link_libraries(svnf_simu ${NS3_LIBRARIES}  ${Boost_LIBRARIES} pthread thread_db)
