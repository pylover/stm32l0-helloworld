cmake_minimum_required(VERSION 3.20)


# Project 
project(foo)
set(STM_FAMILY "stm32")
set(STM_TYPE "stm32l0xx")
set(STM_CHIP "stm32l071cztx")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/${STM_FAMILY}/${STM_CHIP}_flash.ld")
set(ELF "${CMAKE_PROJECT_NAME}.elf")


# Verbosity
set(CMAKE_RULE_MESSAGES OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)


# Production
# Uncomment it for prodution
#set(PROD yes)


# Compiler options
if(DEFINED PROD)
set(CFLAGS 
    -DPROD 
)
else()
set(CFLAGS
    -g3 
)
endif()
set(CFLAGS
    ${CFLAGS}
    -mcpu=Cortex-M0plus
    -O2
    -DSTM32 
    -DSTM32L0
    -DSTM32L071xx
    -DSTM32L071CZTx
    -std=gnu11
    -mthumb 
    -mthumb-interwork
    -fstack-usage
    -Wall 
    -Werror
)
add_compile_options(${CFLAGS})
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/cmsis
    ${PROJECT_SOURCE_DIR}/${STM_FAMILY}
)


# Assembler options
enable_language(ASM)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")


# Linker options
add_link_options(
    ${CFLAGS}
    -T${LINKER_SCRIPT}
    -lrdimon 
    --specs=rdimon.specs
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    -Wl,--gc-sections
    -static
    -Wl,--start-group
    -lc 
    -lm
    -Wl,--end-group
)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")


# Output binary
add_library(clog OBJECT clog.c)
add_executable(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:clog>
    ${STM_FAMILY}/${STM_TYPE}.s
    ${STM_FAMILY}/${STM_TYPE}_system.c
    device.c
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
    --exclude=../stm32
    ${PROJECT_SOURCE_DIR}
)
add_custom_target(lint
    COMMAND prettyc
    ${PRETTYC_FLAGS}
)


# OpenOCD options
set(OPENOCD_FLAGS 
    --file ${PROJECT_SOURCE_DIR}/.openocd.cfg
    --file ${PROJECT_SOURCE_DIR}/${STM_FAMILY}/${STM_TYPE}_openocd.cfg
    --debug=1
)


# Flash the chip
add_custom_target(flash
    COMMAND openocd ${OPENOCD_FLAGS}
    -c \"program ${ELF} verify reset exit\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)


# Debug
add_custom_target(openocd
    COMMAND openocd ${OPENOCD_FLAGS} 
    `printenv OPENOCD_ARGS`
    -c \"program ${ELF} reset\"
    DEPENDS ${CMAKE_PROJECT_NAME}
)
add_custom_target(gdb
    COMMAND ${TOOLCHAIN_PREFIX}gdb 
    -q
    --init-eval-command='target extended-remote :3333'
    --init-eval-command='set remotetimeout 120'
    --init-eval-command='monitor arm semihosting enable'
    --command=${PROJECT_SOURCE_DIR}/gdbinit
    ${ELF}
)
add_custom_target(debug
    COMMAND ../scripts/debug.sh
)


# Analyze ELF file
add_custom_target(elfread COMMAND readelf -l ${ELF})
add_custom_target(elfsize COMMAND ${CMAKE_SIZE} -tA ${ELF})
add_custom_target(analyze DEPENDS elfsize elfread)
