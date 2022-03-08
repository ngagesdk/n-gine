cmake_minimum_required(VERSION 3.00)

if(DEFINED ENV{NGAGESDK})
    SET(NGAGESDK $ENV{NGAGESDK})
    set(CMAKE_TOOLCHAIN_FILE ${NGAGESDK}/cmake/ngage-toolchain.cmake)
else()
    message(FATAL_ERROR "The environment variable NGAGESDK needs to be defined.")
endif()

project(nrpg C CXX)

include(SDL)
include(cwalk)
#include(dbgprint)

# Use CMake or Visual Studio to enable these settings.
option(INSTALL_EKA2L1 "Install app for EKA2L1" OFF)

set(UID1 0x1000007a) # KExecutableImageUidValue, e32uid.h
set(UID2 0x100039ce) # KAppUidValue16, apadef.h
set(UID3 0x10005bbb) # nrpg UID

set(GCC_COMN_DEFS -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS -DNDEBUG -D_UNICODE)
set(GCC_DEFS      ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(nrpg_libs
    ${CMAKE_CURRENT_BINARY_DIR}/libSDL.a
#    ${CMAKE_CURRENT_BINARY_DIR}/libdbgprint.a
    ${CMAKE_CURRENT_BINARY_DIR}/libcwalk.a
    ${EPOC_PLATFORM}/gcc/lib/gcc-lib/arm-epoc-pe/2.9-psion-98r2/libgcc.a
    ${EPOC_LIB}/egcc.lib
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/estlib.lib
    ${EPOC_LIB}/ws32.lib
    ${EPOC_LIB}/hal.lib
    ${EPOC_LIB}/efsrv.lib
    ${EPOC_LIB}/scdv.lib
    ${EPOC_LIB}/gdi.lib)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(nrpg_sources
    "${SRC_DIR}/main.c"
    "${SRC_DIR}/core.c"
    "${SRC_DIR}/tiled.c"
    "${SRC_DIR}/pfs_reader.c")

add_library(nrpg STATIC ${nrpg_sources})
build_exe(nrpg exe ${UID1} ${UID2} ${UID3} "${nrpg_libs}")
if(INSTALL_EKA2L1)
    copy_file(nrpg.exe ${CMAKE_CURRENT_BINARY_DIR}     ${EKA2L1_E_DRIVE} nrpg.exe)
    copy_file(nrpg.exe ${CMAKE_CURRENT_SOURCE_DIR}/res ${EKA2L1_E_DRIVE} data.pfs)
endif()

add_dependencies(
    nrpg.exe
    nrpg)

add_dependencies(
    nrpg
    SDL)

target_compile_definitions(
    nrpg
    PUBLIC
    __EXE__
    ${GCC_DEFS}
    UID1=${UID1}
    UID2=${UID2}
    UID3=${UID3})

target_compile_options(
    nrpg
    PUBLIC
    -O3)

target_include_directories(
    nrpg
    PUBLIC
    ${SRC_DIR}
    ${SDL_INC_DIR}
    ${CWALK_INC_DIR})
