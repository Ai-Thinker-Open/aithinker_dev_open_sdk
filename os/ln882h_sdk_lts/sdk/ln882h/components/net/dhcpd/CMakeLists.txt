file(GLOB_RECURSE  DHCPD_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
set(dhcpd_static_target "dhcpd")
add_library(${dhcpd_static_target} STATIC ${DHCPD_SRC})
add_library(ln::dhcpd ALIAS ${dhcpd_static_target})

target_link_libraries(${dhcpd_static_target}  lwip)

target_include_directories(${dhcpd_static_target}
    PUBLIC
    ${DHCPD_DIR}
)
