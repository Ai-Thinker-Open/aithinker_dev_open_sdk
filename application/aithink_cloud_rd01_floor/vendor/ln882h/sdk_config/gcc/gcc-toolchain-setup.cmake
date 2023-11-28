# the name of the target operating system (cross-compling)
set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)
set(CMAKE_SYSTEM_PROCESSOR  Arm)

set(TOOL_SUFFIX  "")
if (WIN32)
    if($ENV{CROSS_TOOLCHAIN_ROOT}  STREQUAL "")
        message(FATAL_ERROR  "CROSS_TOOLCHAIN_ROOT must be set!!!")
    endif($ENV{CROSS_TOOLCHAIN_ROOT}  STREQUAL "")

    set(COMPILER_HOME  $ENV{CROSS_TOOLCHAIN_ROOT}  CACHE  PATH  "cross-compiler home"  FORCE)
    set(TOOL_SUFFIX     ".exe")
elseif (APPLE)
    MESSAGE(FATAL_ERROR  "NOT SUPPORT")
elseif (UNIX)
    if($ENV{CROSS_TOOLCHAIN_ROOT}  STREQUAL "")
        message(FATAL_ERROR  "CROSS_TOOLCHAIN_ROOT must be set!!!")
    endif($ENV{CROSS_TOOLCHAIN_ROOT}  STREQUAL "")

    set(COMPILER_HOME  $ENV{CROSS_TOOLCHAIN_ROOT}  CACHE  PATH  "cross-compiler home"  FORCE)
    set(TOOL_SUFFIX     "")
endif ()

# set compiler prefix.
set(CROSS_COMPILE_PREFIX  ${COMPILER_HOME}/bin/arm-none-eabi-)

# set compiler tool path.
set(CMAKE_C_COMPILER    ${CROSS_COMPILE_PREFIX}gcc${TOOL_SUFFIX}                CACHE  FILEPATH  "C compiler"    FORCE)
set(CMAKE_CXX_COMPILER  ${CROSS_COMPILE_PREFIX}g++${TOOL_SUFFIX}                CACHE  FILEPATH  "CXX compiler"  FORCE)
set(CMAKE_ASM_COMPILER  ${CROSS_COMPILE_PREFIX}gcc${TOOL_SUFFIX}                CACHE  FILEPATH  "ASM compiler"  FORCE)
set(CMAKE_OBJCOPY       ${CROSS_COMPILE_PREFIX}objcopy${TOOL_SUFFIX}            CACHE  FILEPATH  "objcopy tool"  FORCE)
set(CMAKE_OBJDUMP       ${CROSS_COMPILE_PREFIX}objdump${TOOL_SUFFIX}            CACHE  FILEPATH  "objdump tool"  FORCE)
set(CMAKE_SIZE          ${CROSS_COMPILE_PREFIX}size${TOOL_SUFFIX}               CACHE  FILEPATH  "size tool"     FORCE)
set(LN_MKIMAGE          ${LN_SDK_ROOT}/tools/python_scripts/after_build_gcc.py    CACHE  FILEPATH  "mkimage tool"  FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE       STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM   NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY   ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE   ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE   ONLY)
