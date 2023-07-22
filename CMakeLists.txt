cmake_minimum_required(VERSION 3.20)

project(foo)
enable_language(ASM)

set(FAMILY "stm32l0")
set(CHIP "stm32l071cztx")
set(ROLLUP "${PROJECT_SOURCE_DIR}/rollup")

add_executable(${CMAKE_PROJECT_NAME}
    ${ROLLUP}/stm32l071cztx.s
    ${ROLLUP}/sysmem.c
    main.c
)
include_directories(
    ${PROJECT_SOURCE_DIR}/cmsis/core/include
    ${PROJECT_SOURCE_DIR}/cmsis/${FAMILY}xx/include
)
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} 
    POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=Cortex-M0plus")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -g3 -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSTM32 -DSTM32L0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSTM32L071xx")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSTM32L071CZTx")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -mthumb-interwork")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-usage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

set(LINKER_SCRIPT "${ROLLUP}/${CHIP}_flash.ld")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${LINKER_SCRIPT}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrdimon --specs=rdimon.specs")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=test.map")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc -lm")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--end-group")

set (CMAKE_EXECUTABLE_SUFFIX ".elf")
set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")


add_custom_target(flash
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMAND openocd -f ${ROLLUP}/openocd.cfg 
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset exit\"
)


add_custom_target(openocd
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMAND openocd -f ${ROLLUP}/openocd.cfg 
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset\"
)


add_custom_target(gdb
    COMMAND ${TOOLCHAIN_PREFIX}gdb -q
    --init-command=${ROLLUP}/.gdbinit
    --command=${ROLLUP}/gdbinit
    ${CMAKE_PROJECT_NAME}.elf
)


add_custom_target(analyze
    COMMAND ${CMAKE_SIZE} -tA ${CMAKE_PROJECT_NAME}.elf
)
