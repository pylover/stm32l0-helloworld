cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "crosstool.cmake")
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
set(LINKER_SCRIPT            "../stm32l071cztx_flash.ld")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=Cortex-M0plus")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --specs=nano.specs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")

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


# # usage: Edit "VARIABLES"-section to suit project requirements.
# # For debug build:
# #   cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Debug -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
# #   make -C Debug VERBOSE=1
# # For release build:
# #   cmake -DCMAKE_TOOLCHAIN_FILE=cubeide-gcc.cmake  -S ./ -B Release -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
# #   make -C Release VERBOSE=1
# 
# set (PROJECT_TYPE_EXECUTABLE          "exe")
# set (PROJECT_TYPE_STATIC_LIBRARY      "static-lib")
# set (MCPU_CORTEX_M0				      "-mcpu=cortex-m0")
# set (MCPU_CORTEX_M0PLUS				  "-mcpu=cortex-m0plus")
# set (MCPU_CORTEX_M3				      "-mcpu=cortex-m3")
# set (MCPU_CORTEX_M4				      "-mcpu=cortex-m4")
# set (MCPU_CORTEX_M7				      "-mcpu=cortex-m7")
# set (MCPU_CORTEX_M33				  "-mcpu=cortex-m33")
# set (MFPU_FPV4_SP_D16                 "-mfpu=fpv4-sp-d16")
# set (MFPU_FPV5_D16                    "-mfpu=fpv5-d16")
# set (RUNTIME_LIBRARY_REDUCED_C        "--specs=nano.specs")
# set (RUNTIME_LIBRARY_STD_C            "")
# set (RUNTIME_LIBRARY_SYSCALLS_MINIMAL "--specs=nosys.specs")
# set (RUNTIME_LIBRARY_SYSCALLS_NONE    "")
# set (MFLOAT_ABI_SOFTWARE              "-mfloat-abi=soft")
# set (MFLOAT_ABI_HARDWARE              "-mfloat-abi=hard")
# set (MFLOAT_ABI_MIX                   "-mfloat-abi=softfp")
# 
# 
# set (PROJECT_SOURCES
# 	# LIST SOURCE FILES HERE
# 	Startup/startup_stm32l071cztx.s
# 	Sources/main.c
# 	Sources/syscalls.c
# 	Sources/sysmem.c
# 
# 
# 	)
# 
# set (PROJECT_DEFINES
# 	# LIST COMPILER DEFINITIONS HERE
# 
#     )
# 
# set (PROJECT_INCLUDES
# 	# LIST INCLUDE DIRECTORIES HERE
# 
#     )
# 
# project(${PROJECT_NAME})
# enable_language(ASM)
# 
# if (${PROJECT_TYPE} MATCHES ${PROJECT_TYPE_EXECUTABLE})
#   add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
#   add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
# elseif (${PROJECT_TYPE} MATCHES ${PROJECT_TYPE_STATIC_LIBRARY})
#   add_library(${PROJECT_NAME} ${PROJECT_SOURCES})
# endif()
# 
# add_compile_definitions (${PROJECT_DEFINES})
# include_directories (${PROJECT_INCLUDES})
# 
# set (CMAKE_EXECUTABLE_SUFFIX ".elf")
# set (CMAKE_STATIC_LIBRARY_SUFFIX ".a")
# set (CMAKE_C_FLAGS "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror")
# set (CMAKE_EXE_LINKER_FLAGS "-T${LINKER_SCRIPT} ${RUNTIME_LIBRARY_SYSCALLS} -Wl,-Map=test.map -Wl,--gc-sections -static -Wl,--start-group -lc -lm -Wl,--end-group")
# set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")
