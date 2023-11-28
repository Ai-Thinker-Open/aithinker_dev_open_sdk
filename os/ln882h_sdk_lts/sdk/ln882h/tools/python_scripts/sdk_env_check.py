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
import sys


def check_python3():
    python3_path = None

    sep = None
    suffix = ""
    if "win32" == sys.platform:
        sep = ";"
        suffix = ".exe"
    else:
        sep = ":"

    path_list = os.environ["PATH"].split(sep)
    for item in path_list:
        python3_path = os.path.join(item, "python3" + suffix)
        if os.path.exists(python3_path):
            break
        python3_path = None

    if python3_path:
        print("python3.exe found at: {}".format(python3_path))
        return True
    else:
        print("                                                                                      ")
        print("######################################################################################")
        print("##                       !!! Error: python3.exe not found !!!                       ##")
        print("## 1) install python3.6(+) first.                                                   ##")
        print("##                                                                                  ##")
        print("## 2) add python3 to system environment variable $PATH$.                            ##")
        print("##                                                                                  ##")
        print("## 3) copy /path/to/python3/python.exe --> /path/to/python3/python3.exe             ##")
        print("##                                                                                  ##")
        print("##   If you still can't solve this problem, please read the documentation           ##")
        print("##   [./sdk/doc/LN882H KeilARMCC Development Environment Setup guide.docx]          ##")
        print("##                                                                                  ##")
        print("##   You can also refer to the Chinese guidance document, document address:         ##")
        print("##   URL https://gitee.com/lightningsemi/ln882h/tree/master/doc                     ##")
        print("######################################################################################")
        print("                                                                                      ")
        return False


def main():
    if not check_python3():
        return (1)

    return(0)


if __name__ == "__main__":
    sys.exit(main())
