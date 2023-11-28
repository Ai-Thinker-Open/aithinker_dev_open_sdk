// cli
#include "cli.h"
#include <utils_hex.h>
#include <utils_hexdump.h>
#include <utils_tlv_bl.h>
#include <utils_getopt.h>
#include <utils_string.h>
#include <bl_os_system.h>

#define SHELL_CMD_EXPORT_ALIAS(func, name, desc)                                                                \
    static void func##bf_adapter (char *buf, int len, int argc, char **argv)                                    \
    {                                                                                                           \
        func(argc, argv);                                                                                       \
                                                                                                                \
    }                                                                                                           \
    const static struct cli_command name##cli[] STATIC_CLI_CMD_ATTRIBUTE = {                                    \
                                                                            {#name, #desc , func##bf_adapter}   \
    }


