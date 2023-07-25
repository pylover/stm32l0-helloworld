cmake_minimum_required(VERSION 3.20)


# Project 
project(foo)
set(FAMILY "stm32l0xx")
set(CHIP "stm32l071cztx")
set(ROLLUP "${PROJECT_SOURCE_DIR}/rollup")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/${FAMILY}/${CHIP}_flash.ld")
set(OUTFILE "${CMAKE_PROJECT_NAME}.elf")


# Verbosity
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)


# Compiler options
add_compile_options(
    -mcpu=Cortex-M0plus
    -DDEBUG 
    -g3 
    -O2
    -DSTM32 -DSTM32L0
    -DSTM32L071xx
    -DSTM32L071CZTx
    -std=gnu11
    -mthumb -mthumb-interwork
    -fstack-usage
    -Wall -Werror
)
include_directories(
    ${PROJECT_SOURCE_DIR}/cmsis
    ${PROJECT_SOURCE_DIR}/${FAMILY}
)


# Assembler options
enable_language(ASM)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")


# Linker options
add_link_options(
    -T${LINKER_SCRIPT}
    -lrdimon --specs=rdimon.specs
    -Wl,-Map=test.map
    -Wl,--gc-sections
    -static
    -Wl,--start-group
    -lc -lm
    -Wl,--end-group
)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")


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


# Linter options
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
    ${PROJECT_SOURCE_DIR}
)
add_custom_target(lint
    COMMAND prettyc
    ${PRETTYC_FLAGS}
)


# OpenOCD options
set(OPENOCD_FLAGS 
    --search ${ROLLUP}
    --file ${ROLLUP}/openocd.cfg
    --debug=1
)


# Flash the chip
add_custom_target(flash
    COMMAND openocd ${OPENOCD_FLAGS}
    -c \"program ${OUTFILE} verify reset exit\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)


# Debug
add_custom_target(openocd
    COMMAND openocd ${OPENOCD_FLAGS}
    -c \"program ${OUT_FILE} verify reset\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)
add_custom_target(gdb
    COMMAND ${TOOLCHAIN_PREFIX}gdb -q
    --init-command=${ROLLUP}/.gdbinit
    --command=${ROLLUP}/gdbinit
    ${OUT_FILE}
)


# Analyze ELF file
add_custom_target(elfread COMMAND readelf -l ${OUT_FILE})
add_custom_target(elfsize COMMAND ${CMAKE_SIZE} -tA ${OUT_FILE})
add_custom_target(analyze DEPENDS elfsize elfread)
