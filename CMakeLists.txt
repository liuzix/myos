cmake_minimum_required(VERSION 3.6)
project(myos CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SYSTEM_NAME  Myos)
MESSAGE(STATUS "Setting toochain...")
set(CMAKE_C_COMPILER /Users/zixiong/x-tools/x86_64-unknown-elf/bin/x86_64-unknown-elf-gcc)
set(CMAKE_CXX_COMPILER /Users/zixiong/x-tools/x86_64-unknown-elf/bin/x86_64-unknown-elf-g++)

#include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

MESSAGE(STATUS "Adding arch-dependent files")

ENABLE_LANGUAGE(ASM_NASM)
SET(CMAKE_ASM_NASM_COMPILE_OBJECT
        "<CMAKE_ASM_NASM_COMPILER>  -felf64 -o <OBJECT> <SOURCE>")

file(GLOB ARCH_SOURCE_FILES src/boot/*.asm)

#set(CMAKE_CXX_LINK_EXECUTABLE "/usr/local/binutils/bin/x86_64-unknown-linux-gnu-ld")
set(CMAKE_CXX_FLAGS "-g -O0 -nostdlib -fno-exceptions  -mno-red-zone -lgcc")
#src/crti.o src/crtbegin.o src/crtend.o src/crtn.o
add_executable(bootloader ${ARCH_SOURCE_FILES} src/crti.o src/crtbegin.o
        src/devices/pci.cc src/kmain.cc src/vga.cc src/vga.h src/utils.h
        src/lib/printf.cc src/lib/printf.h src/meminit.cc src/lib/assert.h src/frame.cc
        src/frame.h src/lib/bitmap.h src/interrupts.cc src/interrupts.h src/heap_allocator.cc
        src/heap_allocator.h src/runtime.cc src/paging.cc src/paging.h src/timer.cc src/timer.h
        src/thread.cc src/thread.h src/sync.cc src/sync.h src/devices/pci.h src/devices/ahci.cc src/devices/ahci.h src/tests/sync_malloc.cc
        src/crtend.o src/crtn.o
        src/lib/list.cc src/lib/tree.cc)
set_target_properties(bootloader PROPERTIES LINK_FLAGS  "-ggdb -n -T ${CMAKE_CURRENT_SOURCE_DIR}/src/boot/linker.ld" LINKER_LANGUAGE CXX)


if ( APPLE )
    string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} )
    string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} )
endif ()

find_package(Boost 1.36.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()


#add_executable(myos ${SOURCE_FILES})