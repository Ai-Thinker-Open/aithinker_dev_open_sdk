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
import os
import subprocess
import shutil

path_var_sep    = ";"
exe_suffix      = ".exe"


def check_platform():
    global path_var_sep
    global exe_suffix

    if sys.platform != "win32":
        path_var_sep = ":"
        exe_suffix = ""


class BuildCheck:
    def __init__(self):
        # C:\Keil_v5\UV4\UV4.exe
        self.uv4_filepath = None

        # C:\Program Files\CMake\bin\cmake.exe
        self.cmake_filepath = None

    def prepare(self):
        """
        Check UV4.exe && cmake executable filepath.
        return True on success, False on failure.
        """

        os_path_list = os.environ["PATH"].split(path_var_sep)
        for p in os_path_list:
            if self.uv4_filepath is None:
                found, uv4 = self.__find_uv4(p)
                if found:
                    self.uv4_filepath = uv4

            if self.cmake_filepath is None:
                found, cmake_filepath = self.__find_cmake(p)
                if found:
                    self.cmake_filepath = cmake_filepath


        if self.cmake_filepath is None:
            print("===========================================================")
            print("==================  Error: cmake not found!!!  ============")
            print("===========================================================")
            return False

        if sys.platform == "win32":
            if self.uv4_filepath is None:
                print("===========================================================")
                print("==================  Error: UV4 not found!!!  ==============")
                print("===========================================================")
                return False

        return True

    def check_gcc_proj(self, proj_path_list):
        """
        NOTE: Stop building when error occurs.
        return True on success, False on failure.
        """
        if proj_path_list is None:
            print("Error: no GCC project is provided!!!")
            return False

        for proj in proj_path_list:
            build_path = os.path.abspath(os.path.join(os.path.curdir, "build-check-{}".format(proj)))
            print(build_path)

            ######  clean  ######
            if os.path.exists(build_path):
                shutil.rmtree(build_path, ignore_errors=True)

            ######  config  ######
            cmd = "\"{_c}\"  -DUSER_PROJECT={_up}  -S .  -B {_b}  -G Ninja".format(_c=self.cmake_filepath, _up=proj, _b=build_path)
            print(cmd)
            result = os.system(cmd)
            if 0 == result:
                print("PASS: cmake config")
            else:
                print("Failed: cmake config")
                return False
        
            ######  build  ######
            cmd = "\"{_c}\"  --build  {_b}  --target all".format(_c=self.cmake_filepath, _b=build_path)
            result = os.system(cmd)
            if 0 == result:
                print("PASS: cmake build")
            else:
                print("Failed: cmake build")
                return False            

        return True

    def check_keil_proj(self, proj_path_list):
        """
        NOTE: Stop building when error occurs.
        return True on success, False on failure.
        """
        if proj_path_list is None:
            print("Error: no keil project is provided!!!")
            return False

        for proj_path in proj_path_list:
            user_proj_dir = os.path.join(os.path.abspath(os.path.curdir), proj_path)
            keil_proj_filepath = self.__find_keil_proj_name(user_proj_dir)
            keil_proj_name = keil_proj_filepath.split("\\")[-3]
            if keil_proj_filepath:
                print(keil_proj_filepath)
                if not self.__do_keil_proj_build(keil_proj_filepath, keil_proj_name):
                    return False
        return True

    def __do_keil_proj_build(self, keil_proj_filepath, keil_proj_name):
        """
        UV4.exe  -b  /path/to/keil_xxx.uvprojx  -o build_keil_xxx_output.txt
        return True on success, False on failure.
        """
        buildout_filepath = "buildout_keil_{}.txt".format(keil_proj_name)
        try:
            result = subprocess.check_call("{_uv}  -b  {_kp}  -o {_b}".format(_uv=self.uv4_filepath, _kp=keil_proj_filepath, _b=buildout_filepath), shell=True)
            if 0 != result:
                print("==========" * 10)
                print("Error: keil project {} build failed!!!".format(keil_proj_name))
                print("       please check {}".format(buildout_filepath))
                print("==========" * 10)
                return False
        except subprocess.CalledProcessError as err:
            # print("returncode = {}".format(err.returncode))
            if 1 == err.returncode:
                return True
            else:
                print("==========" * 10)
                print("Error: keil project {} build failed!!!".format(keil_proj_name))
                print("       please check {}".format(buildout_filepath))
                print("returncode: {}".format(err.returncode))
                print("cmd: {}".format(err.cmd))
                print("output: {}".format(err.output))
                print("==========" * 10)
                return False

        return True

    def __find_keil_proj_name(self, keil_dir):
        """
        Find "*.uvprojx" filepath.
        return the absolute path on success, None on failure.
        """
        keil_proj_name = None
        if os.path.isdir(keil_dir):
            for item in os.listdir(keil_dir):
                if item.endswith(".uvprojx"):
                    keil_proj_name = os.path.abspath(os.path.join(keil_dir, item))
                    break

        return keil_proj_name

    def __find_uv4(self, path):
        retcode = False
        uv4_filepath = None

        if path.find("ARMCC") >= 0:
            temp = os.path.dirname(os.path.dirname(os.path.dirname(path)))
            result_path = os.path.join(temp, "UV4", "UV4.exe")
            if os.path.exists(result_path):
                retcode = True
                uv4_filepath = '"' + result_path + '"'

        if retcode:
            print("----------" * 10)
            print("UV4.exe found at: {}".format(uv4_filepath))
            print("----------" * 10)

        return retcode, uv4_filepath

    def __find_cmake(self, path):
        retcode = False
        cmake_filepath = None

        if sys.platform == "win32":
            cmd = "where"
        else:
            cmd = "which"

        try:
            msg = subprocess.check_output([cmd, "cmake"])
            if len(msg) > 0:
                cmake_filepath = msg.strip().decode(encoding="utf-8")
                retcode = True
        except subprocess.CalledProcessError as err:
            print("returncode: {}".format(err.returncode))
            print("cmd: {}".format(err.cmd))
            print("output: {}".format(err.output))
            
        if retcode:
            print("----------" * 10)
            print("cmake found at: {}".format(cmake_filepath))
            print("----------" * 10)

        return retcode, cmake_filepath


def main():
    """
    return 0 on success, negative number on failure.
    """

    keil_proj_list = [
        # bootload
        "project/bootcode/bootload/keil_ln882h",
        "project/bootcode/ramcode_dl/keil_ln882h",

        # ble_mcu_data_trans
        "project/ble_mcu_data_trans/keil_ln882h_rel",
        # ble_mcu_prf_battery
        "project/ble_mcu_prf_battery/keil_ln882h_rel",
        # ble_mcu_prf_hid_mouse
        "project/ble_mcu_prf_hid_mouse/keil_ln882h_rel",

        # wifi_mcu_basic_example
        "project/wifi_mcu_basic_example/keil_ln882h_rel",
        # combo_mcu_basic_example
        "project/combo_mcu_basic_example/keil_ln882h_rel",

        # http_ota_example
        "project/http_ota_example/keil_ln882h_rel",

        # mcu_peripheral_driver_test
        "project/mcu_peripheral_driver_test/keil_ln882h_dev",
        # mcu_peripheral_driver_demo
        "project/mcu_peripheral_driver_demo/ADC/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/ADC_DMA/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/AES/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/DMA/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/EXT/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/GPIO/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/I2C_AT24C04/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/I2C_MPU9250/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/I2C_OLED/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/I2C_SHT30/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/PWM/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/RTC/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/SPI_DMA_FLASH/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/SPI_FLASH/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/SPI_LCD/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/TIMER/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/TRNG/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/UART/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/UART_DMA/keil_ln882h_dev",
        "project/mcu_peripheral_driver_demo/WDT/keil_ln882h_dev",
    ]

    gcc_proj_list = [
        "ble_mcu_data_trans",
        "ble_mcu_prf_battery",
        "ble_mcu_prf_hid_mouse",

        "wifi_mcu_basic_example",
        "combo_mcu_basic_example",
        "mcu_peripheral_driver_test",
        "http_ota_example",
    ]

    check_platform()

    buildChkObj = BuildCheck()
    if not buildChkObj.prepare():
        return(-1)

    if sys.platform == "win32":
        if not buildChkObj.check_keil_proj(keil_proj_list):
            return(-2)

    if not buildChkObj.check_gcc_proj(gcc_proj_list):
        return(-3)

    print("===================================================================")
    print("============================  All Done  ===========================")
    print("===================================================================")
    return 0


if __name__ == "__main__":
    """
    If Keil project build failed, please check the log file buildout_keil_xxxx.txt in keil project directory.
    If GCC project build failed, please check the building output.
    """

    if sys.version_info.major == 2:
        print("==========" * 10)
        print("## Usage:")
        print("         python3  ln_sdk_build_check.py")
        print("==========" * 10)
        exit(-100)

    sys.exit(main())
