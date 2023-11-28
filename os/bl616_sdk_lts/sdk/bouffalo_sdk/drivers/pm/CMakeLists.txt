sdk_generate_library()

sdk_add_include_directories(.)

if("${CHIP}" STREQUAL "bl602")
sdk_library_add_sources(
bl602/bl602_lp.c
)
elseif("${CHIP}" STREQUAL "bl702")
sdk_library_add_sources(
bl702/bl702_lp.c
)
elseif("${CHIP}" STREQUAL "bl702l")
sdk_library_add_sources(
bl702l/bl702l_lp.c
)
elseif("${CHIP}" STREQUAL "bl616")
sdk_library_add_sources(
bl616/bl616_lp.c
)
elseif("${CHIP}" STREQUAL "bl808")
sdk_library_add_sources(
bl808/bl808_lp.c
)
elseif("${CHIP}" STREQUAL "bl628")
sdk_library_add_sources(
bl628/bl628_lp.c
)
endif()

sdk_add_compile_definitions(-DCONFIG_PM)
sdk_add_include_directories(${CHIP})

