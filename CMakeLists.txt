cmake_minimum_required(VERSION 2.8.4)
project(camrem)

option(COVERAGE "COVERAGE" OFF)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

FIND_PACKAGE( Threads REQUIRED )
FIND_PACKAGE( Microhttpd REQUIRED )
FIND_PACKAGE( Gphoto2 REQUIRED )
FIND_PACKAGE( Boost COMPONENTS system filesystem thread unit_test_framework REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} {$MICROHTTPD_INCLUDE_DIR} ${GPHOTO2_INCLUDE_DIR})

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_NO_CXX11_SCOPED_ENUMS=1)

ADD_DEFINITIONS(-DELPP_THREAD_SAFE)
ADD_DEFINITIONS(-DELPP_STACKTRACE_ON_CRASH)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-local-typedef")

if(COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif(COVERAGE)

set(SOURCE_FILES_CAMREM src/Helpers.cpp src/ConfigHandler.cpp src/Http/Http.cpp src/Http/Daemon.cpp src/Http/Request.cpp src/Http/Response.cpp src/Http/Response.h src/Http/Bindings.cpp src/Http/Bindings.h src/Cam/CamHandler.cpp src/Cam/CamHandler.h src/Cam/GPWrapper.h src/Cam/Action.cpp src/Cam/Action.h src/Cam/Result.cpp src/Cam/Result.h src/Cam/CamFacade.cpp src/Cam/CamFacade.h src/Cam/GPWrapper/GPhotoCameraList.cpp src/Cam/GPWrapper/GPhotoCameraList.h src/Cam/GPWrapper/GPhotoAbilitiesList.cpp src/Cam/GPWrapper/GPhotoAbilitiesList.h src/Cam/GPWrapper/GPhotoPortInfoList.cpp src/Cam/GPWrapper/GPhotoPortInfoList.h src/Cam/GPWrapper/GPhotoCamera.cpp src/Cam/GPWrapper/GPhotoCamera.h src/Cam/GPWrapper/GPhotoContext.cpp src/Cam/GPWrapper/GPhotoContext.h src/Cam/GPWrapper/GPhotoCameraFile.cpp src/Cam/GPWrapper/GPhotoCameraFile.h)


add_library(camremobjects STATIC ${SOURCE_FILES_CAMREM})
add_executable(camremd src/camrem.cpp)
add_executable(camrem_unit_tests tests/unit_tests.cpp tests/unit_tests.h)

TARGET_LINK_LIBRARIES(camremd camremobjects ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MICROHTTPD_LIBRARY} ${GPHOTO2_LIBRARY} ${GPHOTO2_PORT_LIBRARY})
TARGET_LINK_LIBRARIES(camrem_unit_tests camremobjects ${Boost_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MICROHTTPD_LIBRARY} ${GPHOTO2_LIBRARY} ${GPHOTO2_PORT_LIBRARY})
