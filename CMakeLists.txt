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
set(SKIP_INSTALL_ALL ON)
set(LIB_ZLIB ${CMAKE_SOURCE_DIR}/src/Dependency/zlib)
set(LIB_ZCONF ${CMAKE_BINARY_DIR}/zlib_build)
include_directories(${LIB_ZLIB})
include_directories(${LIB_ZCONF})
add_subdirectory(${LIB_ZLIB} zlib_build)
target_link_libraries(${PROJECT_NAME} PUBLIC zlibstatic)
# set zlib info to use on libpng
set(PNG_BUILD_ZLIB ON)
set(ZLIB_LIBRARIES zlibstatic)
set(ZLIB_INCLUDE_DIRS ${LIB_ZLIB} ${LIB_ZCONF})

# set(LIB_ZCONF ${CMAKE_SOURCE_DIR}/build/src/Dependency/zlib)
# set(ZLIB_INCLUDE_DIRS ${LIB_ZLIB} ${LIB_ZCONF})
# add_subdirectory(${LIB_ZLIB} ${LIB_ZCONF})
# add_dependencies(${PROJECT_NAME} zlibstatic)

# libpng dependencies to handle png files
set(PNG_LIB ${CMAKE_SOURCE_DIR}/src/Dependency/libpng)
set(PNG_SHARED OFF)
set(PNG_STATIC ON)
set(PNG_FRAMEWORK OFF)
set(PNG_EXECUTABLES OFF)
set(PNG_TESTS OFF)
set(PNG_DEBUG OFF)
include_directories(${PNG_LIB})
add_subdirectory(${PNG_LIB})
configure_file(${PNG_LIB}/scripts/pnglibconf.h.prebuilt
    ${PNG_LIB}/pnglibconf.h)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    pico_stdlib
    FatFs_SPI
    GUI
    Config
    # zlibstatic
    png_static)

pico_enable_stdio_usb(${PROJECT_NAME} 1) # enable usb output
pico_enable_stdio_uart(${PROJECT_NAME} 0) # disable uart output
pico_add_extra_outputs(${PROJECT_NAME}) # create map/bin/hex/uf2 files in addition to elf