cmake_minimum_required(VERSION 3.20)

project(foo)
enable_language(ASM)

set(FAMILY "stm32l0xx")
set(CHIP "stm32l071cztx")
set(ROLLUP "${PROJECT_SOURCE_DIR}/rollup")

set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

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

set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/${FAMILY}/${CHIP}_flash.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${LINKER_SCRIPT}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrdimon --specs=rdimon.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=test.map")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc -lm")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--end-group")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

include_directories(
    ${PROJECT_SOURCE_DIR}/cmsis
    ${PROJECT_SOURCE_DIR}/${FAMILY}
)


# Output binary
add_executable(${CMAKE_PROJECT_NAME}
    ${FAMILY}/${FAMILY}.s
    ${FAMILY}/system_${FAMILY}.c
    main.c
)
target_link_options(${CMAKE_PROJECT_NAME}
    PRIVATE 
    "LINKER:--no-warn-rwx-segments"
)


# Report output binary size after each successful build.
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} 
    POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)


# Lint
set(PRETTYC_FLAGS
    --recursive
    --verbose=0
    --repository=.
    --extensions=c,h,in
    --linelength=80
    --headers=h,in
    --includeorder=standardcfirst
    --root=.
    --exclude=../build
    --exclude=../cmsis
    --exclude=../stm32l0xx
    #--quiet
    ${PROJECT_SOURCE_DIR}
)
add_custom_target(lint
    COMMAND prettyc
    ${PRETTYC_FLAGS}
)


# OpenOCD
set(OPENOCD_FLAGS 
    --search ${ROLLUP}
    --file ${ROLLUP}/openocd.cfg
    --debug=1
)


add_custom_target(flash
    COMMAND openocd ${OPENOCD_FLAGS}
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset exit\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)


# Debug
add_custom_target(openocd
    COMMAND openocd ${OPENOCD_FLAGS}
    -c \"program ${CMAKE_PROJECT_NAME}.elf verify reset\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)


add_custom_target(gdb
    COMMAND ${TOOLCHAIN_PREFIX}gdb -q
    --init-command=${ROLLUP}/.gdbinit
    --command=${ROLLUP}/gdbinit
    ${CMAKE_PROJECT_NAME}.elf
)


add_custom_target(elfread
    COMMAND readelf 
    -l ${CMAKE_PROJECT_NAME}.elf
)
add_custom_target(elfsize
    COMMAND ${CMAKE_SIZE} 
    -tA ${CMAKE_PROJECT_NAME}.elf
)
add_custom_target(analyze 
    DEPENDS elfsize elfread)
