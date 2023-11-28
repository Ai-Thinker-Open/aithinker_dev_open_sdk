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

import os
import argparse
import subprocess
from ln_tools import check_python_version
import xml.etree.ElementTree as ET
from after_build_base import AfterBuildBase


class AfterBuildKeil(AfterBuildBase):
    def __init__(self) -> None:
        super().__init__()

    def prepare(self, *args) -> bool:
        if len(args) == 0:
            print("keil_proj_filepath MUST be provided!!!")
            return False

        sdk_root = os.path.abspath(args[0])
        if os.path.exists(sdk_root) and os.path.isdir(sdk_root):
            self.sdkroot_dir = sdk_root
        else:
            print("Error: `sdkroot` is invalid for keil after build cmd!!!")
            return False

        keil_proj_filepath = args[1]
        output_filename = None
        if len(args) > 1:
            output_filename = args[2]

        if not os.path.exists(keil_proj_filepath):
            print("\nError: not exist: {}".format(keil_proj_filepath))
            return False

        keil_proj_dir    = os.path.dirname(os.path.abspath(keil_proj_filepath))
        chip_type = None
        tree = ET.ElementTree(file=keil_proj_filepath)
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
                            chip_type = temp.strip("").lower()
                            break
        if not chip_type:
            print("Error: chip type macro (such as LN882H) has not been found!!!")
            return False

        self.boot_filepath = os.path.abspath(os.path.join(keil_proj_dir, "{_r}/lib/boot_{_c}.bin".format(_r=self.sdkroot_dir, _c=chip_type)))
        self.app_filepath = os.path.join(keil_proj_dir, self.buildout_name + ".bin")

        if (output_filename is None) or len(output_filename) == 0:
            output_filename = "flashimage.bin"
        self.output_filepath = os.path.join(keil_proj_dir, output_filename)

        self.partcfg_filepath = os.path.abspath(os.path.join(keil_proj_dir, "../cfg/flash_partition_cfg.json"))

        projcfg_filepath = os.path.abspath(os.path.join(keil_proj_dir, "../cfg/proj_config.h"))
        major = None
        minor = None
        crp   = None
        try:
            with open(projcfg_filepath, "rt") as fObj:
                for line in fObj:
                    if not line.strip(" ").startswith("#define"):
                        continue
                    if line.find(AfterBuildKeil.FLASH_IMAGE_VER_MAJOR_STR) > 0:
                        strlist = line.strip(" ").split(" ")
                        str = strlist[-1]
                        index = 0
                        for i in str:
                            if (ord(i) < 0x30) or (ord(i) > 0x39):
                                str = str[0:index]
                            else:
                                index = index + 1
                        major = int(str)
                    if line.find(AfterBuildKeil.FLASH_IMAGE_VER_MINOR_STR) > 0:
                        strlist = line.strip(" ").split(" ")
                        str = strlist[-1]
                        index = 0
                        for i in str:
                            if (ord(i) < 0x30) or (ord(i) > 0x39):
                                str = str[0:index]
                            else:
                                index = index + 1
                        minor = int(str)
                    if line.find(AfterBuildKeil.SOC_CRP_FLAG_STR) > 0:
                        strlist = line.strip(" ").split(" ")
                        str = strlist[-1]
                        index = 0
                        for i in str:
                            if (ord(i) < 0x30) or (ord(i) > 0x39):
                                str = str[0:index]
                            else:
                                index = index + 1
                        crp = int(str)
        except OSError as err:
            print("Error: read LN SDK version from file: {}".format(str(err)))
            return False

        if (major is None) or (minor is None):
            print("no LN SDK version is found!!!")
            return False

        self.ver_str = '{}.{}'.format(major, minor)

        if isinstance(crp, int):
            self.crp_flag = crp

        if not (self.buildout_dir and self.buildout_name and self.boot_filepath and self.app_filepath \
                and self.output_filepath and self.partcfg_filepath and self.ver_str):
            return False

        return True

    def build_stage_first(self) -> bool:
        """
        app.axf -> app.bin
        """
        axf_filepath = os.path.join(self.buildout_dir, self.buildout_name + ".axf")
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
        """
        flashimage.bin
        """
        return super().build_stage_second()

    def build_stage_third(self) -> bool:
        """
        OTA image
        """
        return super().build_stage_third()

    def doAllWork(self) -> bool:
        if not self.build_stage_first():
            return False
        if not self.build_stage_second():
            return False
        if not self.build_stage_third():
            return False
        return True


if __name__ == "__main__":

    check_python_version()

    prog = os.path.basename(__file__)
    desc = "Keil After Build Action"
    parser = argparse.ArgumentParser(prog=prog, description=desc)
    parser.add_argument("-k", "--keilproj", help="*.uvprojx filepath", type=str)
    parser.add_argument("-o", "--output",   help="final image name, default is 'flashimage.bin'", type=str)
    parser.add_argument("-s", "--sdkroot",  help="LN SDK root directory", type=str, default="../../../")
    args = parser.parse_args()

    if not args.keilproj:
        print("Error: no filepath of *.uvprojx is provided!!!")
        exit(-1)

    buildObj = AfterBuildKeil()

    if not buildObj.prepare(args.sdkroot,
                            args.keilproj,
                            args.output):
        exit(-2)

    if not buildObj.doAllWork():
        exit(-3)

    exit(0)
