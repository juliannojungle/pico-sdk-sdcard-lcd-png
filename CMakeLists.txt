# version requirements
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

# initialization
include(${CMAKE_SOURCE_DIR}/src/Dependency/pico-sdk/pico_sdk_init.cmake)

# project definition
project(picopnglcd)

pico_sdk_init()

add_subdirectory(${CMAKE_SOURCE_DIR}/src/Dependency/no-OS-FatFS/src build)

add_executable(${PROJECT_NAME} src/main.cpp)

# target configuration
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)

# Waveshare Pico_code/c/lib/GUI for graphics
set(PICO_CODE_LIB ${CMAKE_SOURCE_DIR}/src/Dependency/Pico_code/c/lib)
add_subdirectory(${PICO_CODE_LIB}/Config)
add_subdirectory(${PICO_CODE_LIB}/GUI)
include_directories(${PICO_CODE_LIB}/Config)
include_directories(${PICO_CODE_LIB}/Fonts)
include_directories(${PICO_CODE_LIB}/LCD)
include_directories(${PICO_CODE_LIB}/GUI)

# zlib dependency to use libpng
set(LIB_ZLIB ${CMAKE_SOURCE_DIR}/src/Dependency/zlib)
# set a custom build script for zlibstatic, skipping the rest
file(REMOVE ${LIB_ZLIB}/CMakeLists.txt)
file(COPY ${CMAKE_SOURCE_DIR}/src/Dependency/zlibstatic.cmake DESTINATION ${LIB_ZLIB})
file(RENAME ${LIB_ZLIB}/zlibstatic.cmake ${LIB_ZLIB}/CMakeLists.txt)
# build zlibstatic
add_subdirectory(${LIB_ZLIB} zlib_build)
target_link_libraries(${PROJECT_NAME} PUBLIC zlibstatic)
# set libpng to use zlibstatic
set(LIB_ZCONF ${CMAKE_BINARY_DIR}/zlib_build)
include_directories(${LIB_ZLIB} ${LIB_ZCONF})
set(PNG_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON)
set(ZLIB_LIBRARIES zlibstatic)
set(ZLIB_INCLUDE_DIRS ${LIB_ZLIB} ${LIB_ZCONF})

# libpng dependencies to handle png files
set(PNG_LIB ${CMAKE_SOURCE_DIR}/src/Dependency/libpng)
# setup libpng to only build png_static, skipping the rest
set(PNG_SHARED OFF CACHE BOOL "")
set(PNG_STATIC ON CACHE BOOL "")
set(PNG_FRAMEWORK OFF CACHE BOOL "")
set(PNG_EXECUTABLES OFF CACHE BOOL "")
set(PNG_TESTS OFF CACHE BOOL "")
set(PNG_DEBUG OFF CACHE BOOL "")
include_directories(${PNG_LIB})
add_subdirectory(${PNG_LIB})
configure_file(${PNG_LIB}/scripts/pnglibconf.h.prebuilt ${PNG_LIB}/pnglibconf.h)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    pico_stdlib
    FatFs_SPI
    GUI
    Config
    png_static)

pico_enable_stdio_usb(${PROJECT_NAME} 1) # enable usb output
pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
pico_add_extra_outputs(${PROJECT_NAME}) # create map/bin/hex/uf2 files in addition to elf