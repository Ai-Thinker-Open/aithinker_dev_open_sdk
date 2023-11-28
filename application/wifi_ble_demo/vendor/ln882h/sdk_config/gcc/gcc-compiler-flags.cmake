string(TOUPPER ${CHIP_SERIAL}  DEF_CHIP_SERIAL)
add_compile_definitions(${DEF_CHIP_SERIAL})
add_compile_definitions(ARM_MATH_CM4)
add_compile_definitions("MBEDTLS_CONFIG_FILE=<mbedtls_config.h>")

set(CPU         "-mcpu=cortex-m4")
set(FPU         "-mfpu=fpv4-sp-d16")
set(FLOAT_ABI   "-mfloat-abi=hard")
set(MCU         "-mthumb  -mabi=aapcs  ${CPU}  ${FPU}  ${FLOAT_ABI}")

set(C_WARNFLAGS   "-Wall  -Wextra  -Wundef  -Wshadow  -Wredundant-decls        \
    -Wstrict-prototypes  -Wimplicit-function-declaration  -Wmissing-prototypes \
    -Wdouble-promotion  -Wfloat-conversion  -pedantic"
)

set(CXX_WARNFLAGS   "-Wall  -Wextra  -Wundef  -Wshadow  -Wredundant-decls      \
    -Wdouble-promotion  -Wfloat-conversion  -pedantic"
)

set(PREPFLAGS   "-MD -MP")
set(SPECSFLAGS  "")
set(ASFLAGS     ${MCU})
set(ARCHFLAGS   ${MCU})
set(OPTFLAGS    ${OPTFLAGS})
set(LINK_FLAGS  "${ARCHFLAGS} -Wl,--gc-sections -u _printf_float -u _scanf_float")


##################################  C Flags  ###################################
set(CMAKE_C_STANDARD        99)
set(CMAKE_C_FLAGS   "${ARCHFLAGS}  ${OPTFLAGS}  ${C_WARNFLAGS}    ${PREPFLAGS} \
    -ffunction-sections -fdata-sections -fno-strict-aliasing ${SPECSFLAGS}"
    CACHE INTERNAL "C compiler flags"
)

set(CMAKE_C_FLAGS_DEBUG   "-ggdb"
  CACHE INTERNAL  "Flags used by the C compiler during DEBUG builds."
)

set(CMAKE_C_FLAGS_RELEASE "-O1 -DNDEBUG"
  CACHE INTERNAL  "Flags used by the C compiler during RELEASE builds."
)


#################################  CXX Flags  ##################################
set(CMAKE_CXX_STANDARD      11)
set(CMAKE_CXX_FLAGS "${ARCHFLAGS}  ${OPTFLAGS}  ${CXX_WARNFLAGS}  ${PREPFLAGS} \
    -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-rtti -fno-exceptions  ${SPECSFLAGS}"
    CACHE INTERNAL "Cxx compiler flags"
)

set(CMAKE_CXX_FLAGS_DEBUG   "-ggdb"
  CACHE INTERNAL  "Flags used by the CXX compiler during DEBUG builds."
)

set(CMAKE_CXX_FLAGS_RELEASE "-O1 -DNDEBUG"
  CACHE INTERNAL  "Flags used by the CXX compiler during RELEASE builds."
)


#################################  ASM Flags  ##################################
set(CMAKE_ASM_FLAGS  "${ASFLAGS}  -x  assembler-with-cpp"
    CACHE INTERNAL "ASM compiler flags"
)

set(LINKER_SCRIPT "${LN_SDK_ROOT}/project/${USER_PROJECT}/gcc/${CHIP_SERIAL}.ld")

set(EXTRA_LINK_FLAGS  "-Wl,-Map=${USER_PROJECT}.map,--cref,--no-warn-mismatch \
    -Wl,--print-memory-usage  --specs=nano.specs  --specs=nosys.specs"
)

set(CMAKE_EXE_LINKER_FLAGS  "${LINK_FLAGS}"
    CACHE INTERNAL "Exe linker flags"
)

set(CMAKE_C_OUTPUT_EXTENSION_REPLACE  1)
