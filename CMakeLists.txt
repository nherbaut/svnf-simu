cmake_minimum_required(VERSION 2.8)
project(svnf_simu)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#SET(CMAKE_BUILD_TYPE "DEBUG")

find_package(NS3 3.23 REQUIRED core csma network internet point-to-point applications stats)
find_package(Boost REQUIRED regex )

MESSAGE("${NS3_LIBRARIES}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNS3_LOG_ENABLE" )






include_directories(${NS3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set(SOURCE_FILES
./src/main.cpp src/GwApplication.cc
src/CachingControllerApplication.cpp
src/MD5.cpp
src/VideoDataSource.cpp
)


add_executable(svnf_simu ${SOURCE_FILES} src/commons.h)

message("${CMAKE_BUILD_TYPE}------------")
IF(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
target_link_libraries(svnf_simu
     ${NS3_LIBRARIES_DEBUG}
  )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pg")
message("going debug")
ELSE()
target_link_libraries(svnf_simu
     ${NS3_LIBRARIES_RELEASE}
    )
message("going optimized")
ENDIF()

target_link_libraries(svnf_simu ${Boost_LIBRARIES}     pthread     thread_db)