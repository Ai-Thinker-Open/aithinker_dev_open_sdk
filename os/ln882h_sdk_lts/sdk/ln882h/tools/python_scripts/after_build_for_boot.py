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

import argparse
import zlib
from ln_tools import *
import subprocess
import xml.etree.ElementTree as ET
from boot_header import BootHeader


class AfterBuildForBoot:
    def __init__(self) -> None:
        self.keilproj = None    # filepath of *.uvprojx
        self.output   = None    # output filename, default is "boot_ram.bin"
        self.crp      = 0       # SWD CRP.

        # parsed from *.uvprojx
        self.buildout_dir   = None # Keil build output directory, default is "Objects"
        self.buildout_name  = None # Keil build output name without suffix.
        self.chip_type      = None # LN88XX
        self.app_filepath   = None # custom build output filename

    def prepare(self, *args) -> bool:
        self.keilproj = realpath(args[0])

        self.output = "boot_ram.bin"

        if len(args) >= 2:
            self.output = args[1]

        if len(args) >= 3:
            if isinstance(args[2], int):
                if args[2] > 0:
                    self.crp = 1
                else:
                    self.crp = 0

        if not os.path.exists(self.keilproj):
            print("Error: not exist: {}".format(self.keilproj))
            return False

        keil_proj_dir    = os.path.dirname(os.path.abspath(self.keilproj))
        self.app_filepath = os.path.join(keil_proj_dir, self.output)
        tree = ET.ElementTree(file=self.keilproj)
        root_node = tree.getroot()
        for ele in root_node.iter():
            if "OutputDirectory" == ele.tag:
                self.buildout_dir = os.path.abspath(os.path.join(keil_proj_dir, ele.text))
            if "OutputName" == ele.tag:
                self.buildout_name = ele.text
            if "VariousControls" == ele.tag:
                define_ele = ele.find("Define")
                if define_ele.text:
                    strlist = define_ele.text.strip(" ").split(",")
                    for temp in strlist:
                        if temp.startswith("LN"):
                            self.chip_type = temp.strip("").lower()
                            break
        if not self.chip_type:
            print("Error: chip type macro (such as LN882H) has not been found!!!")
            return False

        print("keil buildout dir: {}".format(self.buildout_dir))
        print("keil buildout name: {}".format(self.buildout_name))
        print("chip type = {}".format(self.chip_type))

        return True

    def build_stage_first(self) -> bool:
        axf_filepath = os.path.join(self.buildout_dir, self.buildout_name + ".axf")

        # app.axf --> app.asm
        asm_filepath = os.path.join(os.path.dirname(self.keilproj), self.buildout_name + ".asm")
        try:
            retval = subprocess.check_call(["fromelf", "-c", "-v", "-a", "--output={}".format(asm_filepath), axf_filepath], stdout=subprocess.DEVNULL, shell=True)
        except subprocess.CalledProcessError as err:
            retval = err.returncode
            print("returncode: {}".format(err.returncode))
            print("cmd: {}".format(err.cmd))
            print("output: {}".format(err.output))

        if retval != 0:
            print("Error: produce {} failed!!!".format(os.path.basename(asm_filepath)))
            return False

        # app.axf --> app.bin
        app_filepath = self.app_filepath
        try:
            retval = subprocess.check_call(["fromelf", "--bin", "--output={}".format(app_filepath), axf_filepath], stdout=subprocess.DEVNULL, shell=True)
        except subprocess.CalledProcessError as err:
            retval = err.returncode
            print("returncode: {}".format(err.returncode))
            print("cmd: {}".format(err.cmd))
            print("output: {}".format(err.output))

        if retval != 0:
            print("Error: produce {} failed!!!".format(os.path.basename(app_filepath)))
            return False
        return True

    def build_stage_second(self) -> bool:
        # app.bin --- boot header crc ---> boot_ram.bin
        file_info = os.stat(self.app_filepath)
        if file_info.st_size > BootHeader.BOOT_SIZE_LIMIT:
            print("Fatal Error: boot file size ({_bs} bytes) > boot partition size ({_ps} bytes)"
                .format(_bs=file_info.st_size, _ps=BootHeader.BOOT_SIZE_LIMIT))
            return False

        try:
            with open(self.app_filepath, "rb+") as fObj:
                header_buf = fObj.read(BootHeader.BOOT_HEADER_SIZE)
                temp_boot_header = BootHeader(header_buf)
                temp_boot_header.bootram_bin_length = file_info.st_size

                if self.crp == 0:
                    temp_boot_header.crp_flag = 0
                else:
                    temp_boot_header.crp_flag = BootHeader.CRP_VALID_FLAG

                fObj.seek(temp_boot_header.bootram_crc_offset, os.SEEK_SET)
                body_buf = fObj.read()
                body_crc32 = zlib.crc32(body_buf)
                temp_boot_header.bootram_crc_value = body_crc32

                fObj.seek(0, os.SEEK_SET)
                fObj.write(temp_boot_header.toByteArray())
        except OSError as err:
            print("Error: read boot: {}".format(str(err)))
            return False
        return True

    def doAllWork(self) -> bool:
        if not self.build_stage_first():
            return False
        if not self.build_stage_second():
            return False

        return True


if __name__ == "__main__":

    check_python_version()

    prog = os.path.basename(__file__)
    desc = "after build for bootram"
    parser = argparse.ArgumentParser(prog=prog, description=desc)
    parser.add_argument("-k", "--keilproj", help="*.uvprojx filepath", type=str)
    parser.add_argument("-o", "--output",   help="output filename, default is 'boot_ram.bin'", type=str)
    parser.add_argument("-c", "--crp",      help="swd crp flag (0--disable CRP, 1--enable, default is 0)")
    args = parser.parse_args()

    if not args.keilproj:
        print("Error: no filepath of *.uvprojx is provided!!!")
        exit(-1)

    buildObj = AfterBuildForBoot()

    if not buildObj.prepare(args.keilproj, args.output, args.crp):
        exit(-2)

    if not buildObj.doAllWork():
        exit(-3)

    exit(0)
