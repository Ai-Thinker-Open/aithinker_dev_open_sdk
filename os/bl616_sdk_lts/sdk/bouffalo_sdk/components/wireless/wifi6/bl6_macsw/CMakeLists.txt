#sdk_generate_library()

## These include paths would be exported to project level
sdk_add_include_directories(.)
sdk_add_include_directories(export)
sdk_add_include_directories(export/mac)
sdk_add_include_directories(export/dbg)
sdk_add_include_directories(export/adapter)
sdk_add_include_directories(export/common)
sdk_add_include_directories(export/ke)

if(NOT CONFIG_BL_WIFI_LP_FW)
  sdk_add_include_directories(export/ipc_fhost)
else()
  sdk_add_include_directories(export/ipc)
endif()

sdk_add_static_library(lib/libbl6_macsw.a)

if("${CHIP}" STREQUAL "bl616")
sdk_add_compile_definitions(-DCHIP_CODE_BL616)
endif()
if("${CHIP}" STREQUAL "wb03")
sdk_add_compile_definitions(-DCHIP_CODE_WB03)
endif()


if("${CHIP}" STREQUAL "bl616")
sdk_add_compile_definitions(-DCHIP=\"bl616.h\")
sdk_add_compile_definitions(-DCHIP_UART=\"bl616_uart.h\")
sdk_add_compile_definitions(-DCHIP_GLB=\"bl616_glb.h\")
sdk_add_compile_definitions(-DCHIP_HBN=\"bl616_hbn.h\")
endif()
if("${CHIP}" STREQUAL "wb03")
sdk_add_compile_definitions(-DCHIP=\"wb03.h\")
sdk_add_compile_definitions(-DCHIP_UART=\"wb03_uart.h\")
sdk_add_compile_definitions(-DCHIP_GLB=\"wb03_glb.h\")
sdk_add_compile_definitions(-DCHIP_HBN=\"wb03_hbn.h\")
endif()
