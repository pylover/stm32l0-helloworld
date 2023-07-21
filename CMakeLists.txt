cmake_minimum_required(VERSION 3.20)

project(foo)
enable_language(ASM)

add_executable(${CMAKE_PROJECT_NAME}
    stm32l071cztx.s
    main.c
    syscalls.c
    sysmem.c
)
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} 
    POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=Cortex-M0plus")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -mthumb-interwork")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --specs=nano.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

set(LINKER_SCRIPT            "../stm32l071cztx_flash.ld")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${LINKER_SCRIPT}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=test.map")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc -lm")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--end-group")

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")


add_custom_target(flash
    COMMAND openocd -f ../openocd.cfg 
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset exit\"
)


add_custom_target(openocd
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMAND openocd -f ../openocd.cfg 
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset\"
)


add_custom_target(gdb
    COMMAND arm-none-eabi-gdb --init-command=../.gdbinit ${CMAKE_PROJECT_NAME}.elf
)
