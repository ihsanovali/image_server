cmake_minimum_required(VERSION 3.4)

project(image_server C)
include(ExternalProject)

set(CMAKE_C_STANDARD 99)

option(USE_SYSTEM_LIBUV "use system libuv" OFF)
option(STATIC_LINK_LIBUV "static link libuv" ON)

if(NOT USE_SYSTEM_LIBUV AND NOT STATIC_LINK_LIBUV)
    message(FATAL_ERROR "Not support dynamic linking libuv without using system libuv!")
endif()

if(NOT USE_SYSTEM_LIBUV)
    set(LibUV_INCLUDE_DIR lib/libuv/include)
    add_subdirectory(lib/libuv)
    if(STATIC_LINK_LIBUV)
        add_library(uv::uv-static ALIAS uv_a)
        set_target_properties(uv PROPERTIES EXCLUDE_FROM_ALL TRUE)
        set_target_properties(uv_a PROPERTIES POSITION_INDEPENDENT_CODE 1)
        set(LibUV_LIBRARIES uv::uv-static)
    else()
        add_library(uv::uv-shared ALIAS uv)
        set_target_properties(uv PROPERTIES POSITION_INDEPENDENT_CODE 1)
        set(LibUV_LIBRARIES uv::uv-shared)
    endif()
endif()

set(ImageMagick_INCLUDE_DIRS /usr/local/include/ImageMagick-7)
set(ImageMagick_LIBRARIES /usr/local/lib/libMagickCore-7.Q16HDRI.so
        /usr/local/lib/libMagickWand-7.Q16HDRI.so)

# ImageMagick =================================================================
 # set(imagemagick_configure_cmd
 #   bash -c "C_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include CPLUS_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include LIBRARY_PATH=${PROJECT_SOURCE_DIR}/deps/local/lib LDFLAGS='-L${PROJECT_SOURCE_DIR}/deps/local/lib' ./configure  --prefix=${PROJECT_SOURCE_DIR}/deps/local --without-threads --disable-openmp --disable-opencl")
 # ExternalProject_Add(imagemagick
 #   PREFIX ${PROJECT_SOURCE_DIR}/deps/imagemagick
 #   URL http://www.imagemagick.org/download/ImageMagick.tar.gz
 #   # URL_MD5 010b63a2542c4ec4918c8cb431c00356
 #   BUILD_IN_SOURCE 1
 #   CONFIGURE_COMMAND ${imagemagick_configure_cmd}
 #   BUILD_COMMAND
 #   C_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
 #   CPLUS_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
 #   LIBRARY_PATH=${PROJECT_SOURCE_DIR}/deps/local/lib
 # #  LDFLAGS=-L${PROJECT_SOURCE_DIR}/deps/local/lib
 #   make
 #   INSTALL_DIR ${PROJECT_SOURCE_DIR}/deps/local)

#ExternalProject_Add(imagemagick
#  PREFIX ${PROJECT_SOURCE_DIR}/deps/imagemagick
#  URL http://www.imagemagick.org/download/ImageMagick.tar.gz
#  CONFIGURE_COMMAND
#  C_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
#  CPLUS_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
#  LIBRARY_PATH=${PROJECT_SOURCE_DIR}/deps/local/lib
#  LDFLAGS=-L${PROJECT_SOURCE_DIR}/deps/local/lib
#  <SOURCE_DIR>/configure
#  --prefix=<INSTALL_DIR>
#  --without-threads
#  --disable-openmp
#  --disable-opencl
#  BUILD_COMMAND
#  C_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
#  CPLUS_INCLUDE_PATH=${PROJECT_SOURCE_DIR}/deps/local/include
#  LIBRARY_PATH=${PROJECT_SOURCE_DIR}/deps/local/lib
#  LDFLAGS=-L${PROJECT_SOURCE_DIR}/deps/local/lib
#  make
#  INSTALL_DIR ${PROJECT_SOURCE_DIR}/deps/local)
#include_directories(${PROJECT_SOURCE_DIR}/deps/local/include/ImageMagick)

include_directories(${LibUV_INCLUDE_DIR}
        ${ImageMagick_INCLUDE_DIRS})

add_executable(image_server
        src/main.c src/server.c src/server.h)

target_compile_options(image_server PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>)

target_compile_options(image_server PRIVATE -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)

target_link_libraries(image_server
        ${LibUV_LIBRARIES}
        ${ImageMagick_LIBRARIES})

