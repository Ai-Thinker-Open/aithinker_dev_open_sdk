#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Copyright 2021 Shanghai Lightning Semiconductor Technology Co., LTD

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
from os import path
import json

def show_usage():
    print("------------------------------  Usage  ----------------------------")
    print("--arg1: /path/to/flash_partition_cfg.json")
    print("--arg2: /path/to/ln882x.ld")
    print("-------------------------------------------------------------------")
    exit(-1)


def do_rewrite_linkerscript(cfg_filepath, ld_filepath):
    if not path.exists(cfg_filepath):
        print("Error: flash partition cfg file <{_f}> not exist.".format(_f=cfg_filepath))
        exit(-1)

    if not path.exists(ld_filepath):
        print("Error: wrong GCC linker script file path <{_f}>".format(_f=ld_filepath))
        exit(-1)

    app_start_addr = 0
    app_size_kb = 0
    with open(cfg_filepath, "rt", encoding='utf-8') as fObj:
        root = json.load(fObj)
        user_define = root["user_define"]
        for define in user_define:
            if define["partition_type"] == "APP":
                app_start_addr = int(define["start_addr"], 16)
                app_size_kb = define["size_KB"] * 1024 - 256
                break

    app_start_addr += 0x10000000 + 0x100

    ld_file_contents = []
    with open(ld_filepath, "rt") as fObj:
        ld_file_contents = fObj.readlines()

    new_file_contents = []
    for lineRaw in ld_file_contents:
        if (lineRaw.find("FLASH") > 0) and (lineRaw.find("ORIGIN") > 0) and (lineRaw.find("LENGTH") > 0) :
            newline = "  FLASH     (rx)  : ORIGIN = 0x{_size:08X}, LENGTH = {_len}\n".format(_size=app_start_addr, _len=app_size_kb)
            new_file_contents.append(newline)
        else:
            new_file_contents.append(lineRaw)

    with open(ld_filepath, "wt") as fObj:
        fObj.writelines(new_file_contents)


def main():
    if 3 != len(sys.argv):
        show_usage()

    cfg_filepath = sys.argv[1]
    ld_filepath = sys.argv[2]

    do_rewrite_linkerscript(cfg_filepath, ld_filepath)


if __name__ == "__main__":
    main()
