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

import json
import os
import sys
import argparse
from ln_tools import check_python_version
from before_build_base import BeforeBuildBase


class BeforeBuildKeil(BeforeBuildBase):

    def __init__(self) -> None:
        super().__init__()

    def prepare(self, *args):
        if len(args) >= 1:
            temp = os.path.abspath(args[0])
            if not os.path.exists(temp):
                print("Error: not exist: {}".format(temp))
                return False
            self.partcfg_filepath = temp

        if len(args) >= 2:
            self.headfile_filepath = os.path.abspath(args[1])

        if (self.headfile_filepath is None) or (len(self.headfile_filepath) == 0):
            self.headfile_filepath = os.path.join(os.path.dirname(self.partcfg_filepath),
                                                BeforeBuildBase.DEFAULT_FLASH_TABLE_NAME)
        return True

    def build_stage_first(self) -> bool:
        return super().build_stage_first()

    def build_stage_second(self) -> bool:
        return True

    def build_stage_third(self) -> bool:
        return True

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
    desc = "Keil Before Build Action"
    parser = argparse.ArgumentParser(prog=prog, description=desc)
    parser.add_argument("-p", "--partcfg",  help="filepath of flash partition config", type=str)
    parser.add_argument("-o", "--output",   help="filepath of flash_partition_table.h", type=str)
    args = parser.parse_args()

    if not args.partcfg:
        print("Error: flash partition config file (flash_partition_cfg.json) has not been set!!!")
        exit(-1)

    if not args.output:
        print("Error: flash partition table filepaht has not been set!!!")
        exit(-2)

    buildObj = BeforeBuildKeil()
    if not buildObj.prepare(args.partcfg, args.output):
        exit(-11)

    if not buildObj.doAllWork():
        exit(-12)

    exit(0)
