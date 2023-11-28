#!/usr/bin/python
# -*- coding: UTF-8 -*-
import sys
# import json
import os
import argparse
# import configparser

from sys import version_info

if version_info.major == 3:
    import importlib

    importlib.reload(sys)
elif version_info.major == 2:
    reload(sys)
    sys.setdefaultencoding("utf-8")

try:
    import simplejson as json
except:
    import json


class TEMPLATE_CONSTANTS:
    VERSION = "version"
    TYPE = "type"
    NAME = "name"
    ID = "id"
    MIN = "min"
    MAX = "max"
    START = "start"
    STEP = "step"
    DEFINE = "define"
    PROPERTIES = "properties"
    EVENTS = "events"
    ACTIONS = "actions"
    MAPPING = "mapping"
    UNIT = "unit"
    UNITDESC = "unitDesc"
    REQUIRED = "required"
    MODE = "mode"
    SPECS = "specs"
    BOOL = "bool"
    INT = "int"
    STRING = "string"
    FLOAT = "float"
    ENUM = "enum"
    TIMESTAMP = "timestamp"
    STRUCT = "struct"
    DATATYPE = "dataType"
    ARRAY = "array"
    ARRAYINFO = "arrayInfo"
    PARAMS = "params"
    INPUT = "input"
    OUTPUT = "output"
    COPYRIGHT = "/*\n\
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.\n\
 * Licensed under the MIT License (the \"License\"); you may not use this file except in\n\
 * compliance with the License. You may obtain a copy of the License at\n\
 * http://opensource.org/licenses/MIT\n\
 * Unless required by applicable law or agreed to in writing, software distributed under the License is\n\
 * distributed on an \"AS IS\" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,\n\
 * either express or implied. See the License for the specific language governing permissions and\n\
 * limitations under the License.\n\
 *\n\
 */"


action_max_input_id = 0
action_max_output_id = 0


class iot_object:
    def __init__(self):
        pass

    def get_elem_num(self, id):
        return 1

    def get_type(self, type):
        if type == TEMPLATE_CONSTANTS.BOOL:
            return "BLE_QIOT_DATA_TYPE_BOOL"
        elif type == TEMPLATE_CONSTANTS.INT:
            return "BLE_QIOT_DATA_TYPE_INT"
        elif type == TEMPLATE_CONSTANTS.STRING:
            return "BLE_QIOT_DATA_TYPE_STRING"
        elif type == TEMPLATE_CONSTANTS.FLOAT:
            return "BLE_QIOT_DATA_TYPE_FLOAT"
        elif type == TEMPLATE_CONSTANTS.ENUM:
            return "BLE_QIOT_DATA_TYPE_ENUM"
        elif type == TEMPLATE_CONSTANTS.TIMESTAMP:
            return "BLE_QIOT_DATA_TYPE_TIME"
        elif type == TEMPLATE_CONSTANTS.STRUCT:
            return "BLE_QIOT_DATA_TYPE_STRUCT"
        elif type == TEMPLATE_CONSTANTS.ARRAY:
            return "BLE_QIOT_DATA_TYPE_ARRAY"

    def get_array_type(self):
        return ""

    def get_header_data(self, ctx_format, id="", sub_id=""):
        return ""

    def get_function_name(self, ctx_format, id, suffix):
        return ctx_format + "{}_{}".format(id, suffix)

    def get_function_param(self, param_num, const=""):
        if param_num == 2:
            return "({} char *data, uint16_t len)".format(const)
        if param_num == 3:
            return "({} char *data, uint16_t len, uint16_t index)".format(const)
        return ""

    def get_value_function(self, ctx_format, id, type, param_num=2):
        ctx = "\n"
        ctx += self.get_function_name(ctx_format, id, "get") + self.get_function_param(param_num)
        if type == TEMPLATE_CONSTANTS.BOOL:
            ctx += "\n{{\n\tuint8_t tmp_bool = 1;" \
                   "\n\tdata[0] = tmp_bool;" \
                   "\n\tble_qiot_log_d(\"get id {} bool value %02x\", data[0]);" \
                   "\n\treturn sizeof(uint8_t);\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.INT:
            ctx += "\n{{\n\tint tmp_int = 1;" \
                   "\n\ttmp_int = HTONL(tmp_int);" \
                   "\n\tmemcpy(data, &tmp_int, sizeof(int));" \
                   "\n\tble_qiot_log_d(\"get id {} int value %d\", 12345678);" \
                   "\n\treturn sizeof(int);\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.STRING:
            ctx += "\n{{\n\tchar tmp_str[2] = \"a\";" \
                   "\n\tmemcpy(data, tmp_str, strlen(tmp_str));" \
                   "\n\tble_qiot_log_d(\"get id {} string value %s\", data);" \
                   "\n\treturn strlen(tmp_str);\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.FLOAT:
            ctx += "\n{{\n\tfloat tmp_float = 1.23456;" \
                   "\n\tmemcpy(data, &tmp_float, sizeof(float));" \
                   "\n\tble_qiot_log_d(\"get id {} float value %f\", tmp_float);" \
                   "\n\treturn sizeof(float);\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.ENUM:
            ctx += "\n{{\n\tuint16_t tmp_enum = 0;" \
                   "\n\ttmp_enum = HTONS(tmp_enum);" \
                   "\n\tmemcpy(data, &tmp_enum, sizeof(uint16_t));" \
                   "\n\tble_qiot_log_d(\"get id {} int value %d\", 1234);" \
                   "\n\treturn sizeof(uint16_t);\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.TIMESTAMP:
            ctx += "\n{{\n\tuint32_t tmp_time = 12345678;" \
                   "\n\ttmp_time = HTONL(tmp_time);" \
                   "\n\tmemcpy(data, &tmp_time, sizeof(uint32_t));" \
                   "\n\tble_qiot_log_d(\"get id {} time value %d\", 12345678);" \
                   "\n\treturn sizeof(uint32_t);\n}}\n".format(id)
        return ctx

    def set_value_function(self, ctx_format, id, type, param_num=2):
        ctx = "\n"
        ctx += self.get_function_name(ctx_format, id, "set") + self.get_function_param(param_num, "const")
        if type == TEMPLATE_CONSTANTS.BOOL:
            ctx += "\n{{\n\tuint8_t tmp_bool = 0;" \
                   "\n\ttmp_bool = data[0];" \
                   "\n\tble_qiot_log_d(\"set id {} bool value %02x\", data[0]);" \
                   "\n\treturn 0;\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.INT:
            ctx += "\n{{\n\tint tmp_int = 0;" \
                   "\n\tmemcpy(&tmp_int, data, sizeof(int));" \
                   "\n\ttmp_int = NTOHL(tmp_int);" \
                   "\n\tble_qiot_log_d(\"set id {} int value %d\", tmp_int);" \
                   "\n\treturn 0;\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.STRING:
            ctx += "\n{{\n\tchar tmp_str[128] = \"\";" \
                   "//copy the actual length of the text" \
                   "\n\tmemcpy(tmp_str, data, 1);" \
                   "\n\tble_qiot_log_d(\"set id {} string value %s\", data);" \
                   "\n\treturn 0;\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.FLOAT:
            ctx += "\n{{\n\tfloat tmp_float = 0;" \
                   "\n\tmemcpy(&tmp_float, data, sizeof(float));" \
                   "\n\tble_qiot_log_d(\"set id {} float value %f\", tmp_float);" \
                   "\n\treturn 0;\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.ENUM:
            ctx += "\n{{\n\tuint16_t tmp_enum = 0;" \
                   "\n\tmemcpy(&tmp_enum, data, sizeof(uint16_t));" \
                   "\n\ttmp_enum = NTOHS(tmp_enum);" \
                   "\n\tble_qiot_log_d(\"set id {} int value %d\", tmp_enum);" \
                   "\n\treturn 0;\n}}\n".format(id)
        elif type == TEMPLATE_CONSTANTS.TIMESTAMP:
            ctx += "\n{{\n\tuint32_t tmp_time = 0;" \
                   "\n\tmemcpy(&tmp_time, data, sizeof(uint32_t));" \
                   "\n\ttmp_time = NTOHL(tmp_time);" \
                   "\n\tble_qiot_log_d(\"set id {} time value %d\", tmp_time);" \
                   "\n\treturn 0;\n}}\n".format(id)
        return ctx


class iot_bool(iot_object):
    def __init__(self, ctx):
        self.false = 0
        self.true = 1

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.BOOL)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.BOOL, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.BOOL, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.BOOL)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.BOOL)
        return ctx


class iot_int(iot_object):
    def __init__(self, ctx):
        self.min = ctx[TEMPLATE_CONSTANTS.MIN]
        self.max = ctx[TEMPLATE_CONSTANTS.MAX]
        self.start = ctx[TEMPLATE_CONSTANTS.START]
        self.step = ctx[TEMPLATE_CONSTANTS.STEP]
        # self.unit = ctx[TEMPLATE_CONSTANTS.UNIT]

    def get_header_data(self, ctx_format, id="", sub_id=""):
        ctx = ""
        min_format = "#define " + ctx_format + "_{}_MIN\t({})"
        max_format = "#define " + ctx_format + "_{}_MAX\t({})"
        start_format = "#define " + ctx_format + "_{}_START\t({})"
        step_format = "#define " + ctx_format + "_{}_STEP\t({})"
        if id and sub_id:
            ctx += "\n// define {} {} attributes".format(id, sub_id)
            min_format = "#define " + ctx_format + "_{}_MIN\t({})"
            ctx += "\n" + min_format.format(id.upper(), sub_id.upper(), self.min)
            ctx += "\n" + max_format.format(id.upper(), sub_id.upper(), self.max)
            ctx += "\n" + start_format.format(id.upper(), sub_id.upper(), self.start)
            ctx += "\n" + step_format.format(id.upper(), sub_id.upper(), self.step)
        else:
            ctx += "\n// define {} attributes".format(id)
            ctx += "\n" + min_format.format(id.upper(), self.min)
            ctx += "\n" + max_format.format(id.upper(), self.max)
            ctx += "\n" + start_format.format(id.upper(), self.start)
            ctx += "\n" + step_format.format(id.upper(), self.step)
        return ctx

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.INT)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.INT, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.INT, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.INT)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.INT)
        return ctx


class iot_string(iot_object):
    def __init__(self, ctx):
        self.minLen = ctx[TEMPLATE_CONSTANTS.MIN]
        self.maxLen = ctx[TEMPLATE_CONSTANTS.MAX]

    def get_string_max_len(self):
        return self.maxLen

    def get_header_data(self, ctx_format, id="", sub_id=""):
        ctx = ""
        min_format = "#define " + ctx_format + "_{}_LEN_MIN\t({})"
        max_format = "#define " + ctx_format + "_{}_LEN_MAX\t({})"
        if id and sub_id:
            ctx += "\n// define {}{} length limit".format(id, sub_id)
            ctx += "\n" + min_format.format(id.upper(), sub_id.upper(), self.minLen)
            ctx += "\n" + max_format.format(id.upper(), sub_id.upper(), self.maxLen)
        else:
            ctx += "\n// define {} length limit".format(id)
            ctx += "\n" + min_format.format(id.upper(), self.minLen)
            ctx += "\n" + max_format.format(id.upper(), self.maxLen)
        return ctx

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.STRING)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.STRING, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.STRING, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.STRING)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.STRING)
        return ctx


class iot_float(iot_object):
    def __init__(self, ctx):
        self.obj = iot_int(ctx)

    def get_header_data(self, ctx_format, id="", sub_id=""):
        return self.obj.get_header_data(ctx_format, id, sub_id)

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.FLOAT)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.FLOAT, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.FLOAT, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.FLOAT)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.FLOAT)
        return ctx


class iot_enum(iot_object):
    def __init__(self, ctx):
        self.enum = ctx

    def get_header_data(self, ctx_format, id="", sub_id=""):
        ctx = ""
        enum_format = ctx_format + "_{}_{} = {},"
        if id and sub_id:
            ctx += "\n// define {} {} enum".format(id, sub_id)
            ctx += "\nenum {"
            for k in sorted(self.enum.keys()):
                ctx += "\n\t" + enum_format.format(id.upper(), sub_id.upper(), self.enum[k].upper(), int(k))
            enum_format = ctx_format + "_{}_BUTT,"
            ctx += "\n\t" + enum_format.format(id.upper(), sub_id.upper())
            ctx += "\n};\n"
        else:
            ctx += "\n// define {} enum".format(id)
            ctx += "\nenum {"
            for k in sorted(self.enum.keys()):
                ctx += "\n\t" + enum_format.format(id.upper(), self.enum[k].upper(), int(k))
            enum_format = ctx_format + "_{}_BUTT,"
            ctx += "\n\t" + enum_format.format(id.upper())
            ctx += "\n};\n"
        return ctx

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.ENUM)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.ENUM, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.ENUM, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.ENUM)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.ENUM)
        return ctx


class iot_timestamp(iot_object):
    def __init__(self):
        pass

    def get_source_get_function(self, ctx_format, id):
        return self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.TIMESTAMP)

    def get_source_array_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.TIMESTAMP, 3)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.TIMESTAMP, 3)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        ctx += self.set_value_function(ctx_format, id, TEMPLATE_CONSTANTS.TIMESTAMP)
        ctx += self.get_value_function(ctx_format, id, TEMPLATE_CONSTANTS.TIMESTAMP)
        return ctx


class iot_struct(iot_object):
    def __init__(self, ctx):
        self.struct_fields = []
        self.struct_fields_num = 0

        class struct_member:
            def __init__(self, member_ctx):
                self.id = member_ctx[TEMPLATE_CONSTANTS.ID]
                self.name = member_ctx[TEMPLATE_CONSTANTS.NAME]
                self.type = member_ctx[TEMPLATE_CONSTANTS.DATATYPE][TEMPLATE_CONSTANTS.TYPE]
                if self.type == TEMPLATE_CONSTANTS.BOOL:
                    self.value = iot_bool(member_ctx[TEMPLATE_CONSTANTS.DATATYPE][TEMPLATE_CONSTANTS.MAPPING])
                elif self.type == TEMPLATE_CONSTANTS.INT:
                    self.value = iot_int(member_ctx[TEMPLATE_CONSTANTS.DATATYPE])
                elif self.type == TEMPLATE_CONSTANTS.STRING:
                    self.value = iot_string(member_ctx[TEMPLATE_CONSTANTS.DATATYPE])
                elif self.type == TEMPLATE_CONSTANTS.FLOAT:
                    self.value = iot_float(member_ctx[TEMPLATE_CONSTANTS.DATATYPE])
                elif self.type == TEMPLATE_CONSTANTS.ENUM:
                    self.value = iot_enum(member_ctx[TEMPLATE_CONSTANTS.DATATYPE][TEMPLATE_CONSTANTS.MAPPING])
                elif self.type == TEMPLATE_CONSTANTS.TIMESTAMP:
                    self.value = iot_timestamp()
                else:
                    print(u"错误：文件内容非法，请检查文件结构体内容是否正确。")
                    return

            def get_struct_member_id(self):
                return self.id

            def get_struct_member_type(self):
                if self.type == TEMPLATE_CONSTANTS.BOOL:
                    return "BLE_QIOT_DATA_TYPE_BOOL"
                elif self.type == TEMPLATE_CONSTANTS.INT:
                    return "BLE_QIOT_DATA_TYPE_INT"
                elif self.type == TEMPLATE_CONSTANTS.STRING:
                    return "BLE_QIOT_DATA_TYPE_STRING"
                elif self.type == TEMPLATE_CONSTANTS.FLOAT:
                    return "BLE_QIOT_DATA_TYPE_FLOAT"
                elif self.type == TEMPLATE_CONSTANTS.ENUM:
                    return "BLE_QIOT_DATA_TYPE_ENUM"
                elif self.type == TEMPLATE_CONSTANTS.TIMESTAMP:
                    return "BLE_QIOT_DATA_TYPE_TIME"

        for member in ctx:
            member_obj = struct_member(member)
            self.struct_fields.append(member_obj)
            self.struct_fields_num += 1

    def get_struct_define(self, property_id):
        ctx = ""
        ctx += "\ntypedef struct{"
        for member in self.struct_fields:
            if member.type == TEMPLATE_CONSTANTS.BOOL:
                ctx += "\n\tbool m_{};".format(member.id)
            elif member.type == TEMPLATE_CONSTANTS.INT:
                ctx += "\n\tint32_t m_{};".format(member.id)
            elif member.type == TEMPLATE_CONSTANTS.STRING:
                ctx += "\n\tchar m_{}[{}];".format(member.id, member.value.get_string_max_len())
            elif member.type == TEMPLATE_CONSTANTS.FLOAT:
                ctx += "\n\tfloat m_{};".format(member.id)
            elif member.type == TEMPLATE_CONSTANTS.ENUM:
                ctx += "\n\tuint16_t m_{};".format(member.id)
            elif member.type == TEMPLATE_CONSTANTS.TIMESTAMP:
                ctx += "\n\tuint32_t m_{};".format(member.id)
            else:
                print(u"错误：结构体内容非法，请检查文件结构体内容是否正确。")
                return
        ctx += "\n}struct_property_" + property_id + ";\n"
        return ctx

    def get_struct_id_enum(self, property_id):
        ctx = "\n// define {} property id".format(property_id)
        ctx += "\nenum {\n"
        for member in self.struct_fields:
            ctx += "\tBLE_QIOT_STRUCT_{}_PROPERTY_ID_{},\n".format(property_id.upper(),
                                                                   member.get_struct_member_id().upper())
        ctx += "\tBLE_QIOT_STRUCT_{}_PROPERTY_ID_BUTT,\n".format(property_id.upper())
        ctx += "};\n"
        return ctx

    def get_header_data(self, ctx_format, id="", sub_id=""):
        ctx = ""
        ctx_format = "BLE_QIOT_STRUCT_{}_PROPERTY"
        for member in self.struct_fields:
            ctx += member.value.get_header_data(ctx_format, id=id, sub_id=member.get_struct_member_id())
        ctx += self.get_struct_id_enum(id)
        ctx += self.get_struct_define(id)

        return ctx

    def get_struct_array_elem(self, id, member):
        ctx = ""
        ctx += "\n\t{"
        ctx += "(property_set_cb)ble_property_{0}_{1}_set, (property_get_cb)ble_property_{0}_{1}_get, 0, {2}, 1". \
            format(id, member.get_struct_member_id(), member.get_struct_member_type())
        ctx += "},"
        return ctx

    def get_source_data_for_array(self, ctx_format, id, arr_size):
        ctx = ""

        ctx_format = "static int ble_property_{}_".format(id)
        for member in self.struct_fields:
            ctx += member.value.get_source_array_data(ctx_format, member.get_struct_member_id())

        ctx += "\nstatic ble_property_t sg_ble_{}_property_array[{}] = {{".format(id, self.struct_fields_num)
        for member in self.struct_fields:
            ctx += self.get_struct_array_elem(id, member)
        ctx += "\n};\n"

        ctx += "\nstatic int ble_property_{}_set(const char *data, uint16_t len)".format(id)
        ctx += "\n{"
        ctx += "\n\treturn ble_user_property_struct_array_set(BLE_QIOT_PROPERTY_ID_{}, data, len, sg_ble_{}_property_array, {});". \
            format(id.upper(), id, self.struct_fields_num)
        ctx += "\n}\n"

        ctx += "\nstatic int ble_property_{}_get(char *data, uint16_t len)".format(id)
        ctx += "\n{"
        ctx += "\n\treturn ble_user_property_struct_array_get(BLE_QIOT_PROPERTY_ID_{}, data, len, sg_ble_{}_property_array, {});". \
            format(id.upper(), id, self.struct_fields_num)
        ctx += "\n}\n"
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""

        ctx_format = "static int ble_property_{}_".format(id)
        for member in self.struct_fields:
            ctx += member.value.get_source_data(ctx_format, member.get_struct_member_id())

        ctx += "\nstatic ble_property_t sg_ble_{}_property_array[{}] = {{".format(id, self.struct_fields_num)
        for member in self.struct_fields:
            ctx += self.get_struct_array_elem(id, member)
        ctx += "\n};\n"

        ctx += "\nstatic int ble_property_{}_set(const char *data, uint16_t len)".format(id)
        ctx += "\n{"
        ctx += "\n\treturn ble_user_property_struct_handle(data, len, sg_ble_{}_property_array, {});".format(id, self.struct_fields_num)
        ctx += "\n}\n"

        ctx += "\nstatic int ble_property_{}_get(char *data, uint16_t len)".format(id)
        ctx += "\n{"
        ctx += "\n\treturn ble_user_property_struct_get_data(data, len, sg_ble_{}_property_array, {});".format(id, self.struct_fields_num)
        ctx += "\n}\n"
        return ctx


class iot_array(iot_object):
    def __init__(self, ctx):
        self.type = ctx[TEMPLATE_CONSTANTS.TYPE]
        if self.type == TEMPLATE_CONSTANTS.INT:
            self.value = iot_int(ctx)
        elif self.type == TEMPLATE_CONSTANTS.FLOAT:
            self.value = iot_float(ctx)
        elif self.type == TEMPLATE_CONSTANTS.STRING:
            self.value = iot_string(ctx)
        elif self.type == TEMPLATE_CONSTANTS.STRUCT:
            self.value = iot_struct(ctx[TEMPLATE_CONSTANTS.SPECS])
        else:
            print(u"错误：文件内容非法，请检查文件结构体内容是否正确。")
            return

    def get_array_type(self):
        if self.type == TEMPLATE_CONSTANTS.INT:
            return "BLE_QIOT_ARRAY_INT_BIT_MASK"
        elif self.type == TEMPLATE_CONSTANTS.FLOAT:
            return "BLE_QIOT_ARRAY_FLOAT_BIT_MASK"
        elif self.type == TEMPLATE_CONSTANTS.STRING:
            return "BLE_QIOT_ARRAY_STRING_BIT_MASK"
        elif self.type == TEMPLATE_CONSTANTS.STRUCT:
            return "BLE_QIOT_ARRAY_STRUCT_BIT_MASK"

    def get_elem_num(self, id):
        return "BLE_QIOT_PROPERTY_{}_SIZE_MAX".format(id.upper())

    def get_array_obj_define(self, id, sub_id):
        ctx = ""
        arr_size_macro = "BLE_QIOT_PROPERTY_{}_SIZE_MAX".format(id.upper())
        ctx += "\n//define the actual size of array"
        ctx += "\n#define " + arr_size_macro + "\t(0)"
        ctx += "\n#if " + arr_size_macro + " == 0"
        ctx += "\n\t#error \"please define {} array size first\"".format(id)
        ctx += "\n#endif"
        ctx += "\ntypedef struct{"
        if self.type == TEMPLATE_CONSTANTS.INT:
            ctx += "\n\tint32_t m_int_arr[{}];".format(arr_size_macro)
            ctx += "\n\tuint16_t m_arr_size;"
        elif self.type == TEMPLATE_CONSTANTS.FLOAT:
            ctx += "\n\tfloat m_float_arr[{}];".format(arr_size_macro)
            ctx += "\n\tuint16_t m_arr_size;"
        elif self.type == TEMPLATE_CONSTANTS.STRING:
            ctx += "\n\tchar m_str_arr[{}][{}];".format(arr_size_macro, self.value.get_string_max_len())
            ctx += "\n\tuint16_t m_arr_size;"
        elif self.type == TEMPLATE_CONSTANTS.STRUCT:
            ctx += "\n\tstruct_property_{} m_struct_arr[{}];".format(id, arr_size_macro)
            ctx += "\n\tuint16_t m_arr_size;"
        ctx += "\n}}array_struct_{};\n".format(id)
        return ctx

    def get_header_data(self, ctx_format, id="", sub_id=""):
        ctx = ""
        ctx += self.value.get_header_data(ctx_format, id, sub_id)
        ctx += self.get_array_obj_define(id, sub_id)
        return ctx

    def get_source_data(self, ctx_format, id):
        ctx = ""
        if self.type != TEMPLATE_CONSTANTS.STRUCT:
            ctx += self.value.get_source_array_data(ctx_format, id)
        else:
            arr_size_macro = "BLE_QIOT_PROPERTY_{}_SIZE_MAX".format(id.upper())
            ctx += self.value.get_source_data_for_array(ctx_format, id, arr_size_macro)
        return ctx


class iot_property(iot_object):
    def __init__(self, property):
        self.id = property[TEMPLATE_CONSTANTS.ID]
        self.name = property[TEMPLATE_CONSTANTS.NAME]
        self.mode = property[TEMPLATE_CONSTANTS.MODE]
        self.type = property[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.TYPE]

        if self.type == TEMPLATE_CONSTANTS.BOOL:
            self.value = iot_bool(property[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.MAPPING])
        elif self.type == TEMPLATE_CONSTANTS.INT:
            self.value = iot_int(property[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.STRING:
            self.value = iot_string(property[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.FLOAT:
            self.value = iot_float(property[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.ENUM:
            self.value = iot_enum(property[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.MAPPING])
        elif self.type == TEMPLATE_CONSTANTS.TIMESTAMP:
            self.value = iot_timestamp()
        elif self.type == TEMPLATE_CONSTANTS.STRUCT:
            self.value = iot_struct(property[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.SPECS])
        elif self.type == TEMPLATE_CONSTANTS.ARRAY:
            self.value = iot_array(property[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.ARRAYINFO])
        else:
            print(u"错误：文件内容非法，请检查文件结构体内容是否正确。")
            return

    def get_property_type(self):
        return self.get_type(self.type)

    def get_property_id(self):
        return self.id

    def get_property_header(self):
        ctx_format = "BLE_QIOT_PROPERTY"
        return self.value.get_header_data(ctx_format, id=self.id)

    def get_property_source(self):
        ctx_format = "static int ble_property_"
        return self.value.get_source_data(ctx_format, self.id)


class event_action_member(iot_object):
    def __init__(self, member_ctx):
        self.id = member_ctx[TEMPLATE_CONSTANTS.ID]
        self.name = member_ctx[TEMPLATE_CONSTANTS.NAME]
        self.type = member_ctx[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.TYPE]
        if self.type == TEMPLATE_CONSTANTS.BOOL:
            self.value = iot_bool(member_ctx[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.MAPPING])
        elif self.type == TEMPLATE_CONSTANTS.INT:
            self.value = iot_int(member_ctx[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.STRING:
            self.value = iot_string(member_ctx[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.FLOAT:
            self.value = iot_float(member_ctx[TEMPLATE_CONSTANTS.DEFINE])
        elif self.type == TEMPLATE_CONSTANTS.ENUM:
            self.value = iot_enum(member_ctx[TEMPLATE_CONSTANTS.DEFINE][TEMPLATE_CONSTANTS.MAPPING])
        elif self.type == TEMPLATE_CONSTANTS.TIMESTAMP:
            self.value = iot_timestamp()
        else:
            print(u"错误：文件内容非法，请检查文件结构体内容是否正确。")
            return

    def get_event_type(self):
        return self.get_type(self.type)

    def get_event_action_value(self):
        return self.value

    def get_event_param_id(self):
        return self.id

    def get_action_input_id(self):
        return self.id

    def get_action_output_id(self):
        return self.id


class iot_event:
    def __init__(self, event):
        self.id = event[TEMPLATE_CONSTANTS.ID]
        self.name = event[TEMPLATE_CONSTANTS.NAME]
        self.type = event[TEMPLATE_CONSTANTS.TYPE]
        self.params = []
        self.params_num = 0
        for param in event[TEMPLATE_CONSTANTS.PARAMS]:
            param_obj = event_action_member(param)
            self.params.append((param_obj))
            self.params_num += 1

    def get_event_param_id_define(self):
        ctx = ""
        ctx += "\n// define event {} param id".format(self.id)
        ctx += "\nenum {\n"
        for param in self.params:
            ctx += "\tBLE_QIOT_EVENT_{}_PARAM_ID_{},\n".format(self.id.upper(), param.get_event_param_id().upper())
        ctx += "\tBLE_QIOT_EVENT_{}_PARAM_ID_BUTT,\n".format(self.id.upper())
        ctx += "};\n"
        return ctx

    def get_event_id(self):
        return self.id

    def get_event_params_num(self):
        return self.params_num

    def get_event_param_elem(self, param):
        ctx = ""
        ctx += "\n\t{"
        ctx += "ble_event_get_{0}_{1}_get, {2}".format(self.id, param.get_event_param_id(), param.get_event_type())
        ctx += "},"
        return ctx

    def get_event_header(self):
        ctx = ""
        ctx += self.get_event_param_id_define()
        ctx_format = "BLE_QIOT_EVENT_{}"
        for param in self.params:
            ctx += param.get_event_action_value().get_header_data(ctx_format, id=self.id,
                                                                  sub_id=param.get_event_param_id())

        return ctx

    def get_event_source(self):
        ctx = ""

        ctx_format = "static int ble_event_get_{}_".format(self.id)
        for param in self.params:
            ctx += param.value.get_source_get_function(ctx_format, param.get_event_param_id())

        ctx += "\nstatic ble_event_param sg_ble_event_{}_array[{}] = {{".format(self.id, self.params_num)
        for param in self.params:
            ctx += self.get_event_param_elem(param)
        ctx += "\n};\n"
        return ctx


class iot_action:
    def __init__(self, action):
        self.id = action[TEMPLATE_CONSTANTS.ID]
        self.name = action[TEMPLATE_CONSTANTS.NAME]
        self.input_params = []
        self.input_params_num = 0
        self.output_params = []
        self.output_params_num = 0
        for param in action[TEMPLATE_CONSTANTS.INPUT]:
            param_obj = event_action_member(param)
            self.input_params.append(param_obj)
            self.input_params_num += 1
        global action_max_input_id
        action_max_input_id = max(action_max_input_id, self.input_params_num)
        for param in action[TEMPLATE_CONSTANTS.OUTPUT]:
            param_obj = event_action_member(param)
            self.output_params.append(param_obj)
            self.output_params_num += 1
        global action_max_output_id
        action_max_output_id = max(action_max_output_id, self.output_params_num)

    def get_action_id(self):
        return self.id

    def get_action_input_param_num(self):
        return self.input_params_num

    def get_action_output_param_num(self):
        return self.output_params_num

    def get_action_input_id_define(self):
        ctx = ""
        ctx += "\n// define action {} input id ".format(self.id)
        ctx += "\nenum {\n"
        for param in self.input_params:
            ctx += "\tBLE_QIOT_ACTION_{}_INPUT_ID_{},\n".format(self.id.upper(), param.get_action_input_id().upper())
        ctx += "\tBLE_QIOT_ACTION_{}_INPUT_ID_BUTT,\n".format(self.id.upper())
        ctx += "};\n"
        return ctx

    def get_action_output_id_define(self):
        ctx = ""
        ctx += "\n// define action {} output id ".format(self.id)
        ctx += "\nenum {\n"
        for param in self.output_params:
            ctx += "\tBLE_QIOT_ACTION_{}_OUTPUT_ID_{},\n".format(self.id.upper(), param.get_action_output_id().upper())
        ctx += "\tBLE_QIOT_ACTION_{}_OUTPUT_ID_BUTT,\n".format(self.id.upper())
        ctx += "};\n"
        return ctx

    def get_action_header(self):
        ctx = ""
        ctx += self.get_action_input_id_define()
        ctx_format = "BLE_QIOT_ACTION_INPUT_{}"
        for param in self.input_params:
            ctx += param.get_event_action_value().get_header_data(ctx_format, id=self.id,
                                                                  sub_id=param.get_action_input_id())

        ctx += self.get_action_output_id_define()
        ctx_format = "BLE_QIOT_ACTION_OUTPUT_{}"
        for param in self.output_params:
            ctx += param.get_event_action_value().get_header_data(ctx_format, id=self.id,
                                                                  sub_id=param.get_action_output_id())
        return ctx

    def get_param_elem(self, param):
        ctx = ""
        ctx += "\n\t"
        ctx += "{}".format(param.get_event_type())
        ctx += ","
        return ctx

    def get_action_source(self):
        ctx = ""
        ctx_format = "static int ble_action_handle_{}_{}_cb"
        ctx += "\n" + ctx_format.format(self.id, "input") + "(e_ble_tlv *input_param_array, uint8_t input_array_size,uint8_t *output_id_array)"
        ctx += "\n{\n\tint i = 0;\n\tfor(i = 0; i < input_array_size; i++){"
        ctx += "\n\t\t//handle the data of input_param_array[i], set output_id_array value if triggered, the value of the output id obtained follow"
        ctx += "\n\t}\n\treturn 0;\n}\n"
        ctx += "\n" + ctx_format.format(self.id, "output") + "(uint8_t output_id, char *buf, uint16_t buf_len)"
        ctx += "\n{\n\t//get value of the output id, return actual length\n\treturn 0;\n}\n"

        ctx += "\nstatic uint8_t sg_ble_action_{}_input_type_array[{}] = {{".format(self.id, self.input_params_num)
        for param in self.input_params:
            ctx += self.get_param_elem(param)
        ctx += "\n};\n"

        ctx += "\nstatic uint8_t sg_ble_action_{}_output_type_array[{}] = {{".format(self.id, self.output_params_num)
        for param in self.output_params:
            ctx += self.get_param_elem(param)
        ctx += "\n};\n"

        return ctx


class iot_parse_dt:
    def __init__(self, dt_json):
        self.version = dt_json[TEMPLATE_CONSTANTS.VERSION]
        self.properties = []
        self.properties_num = 0
        self.events = []
        self.events_num = 0
        self.actions = []
        self.actions_num = 0

        for property in dt_json[TEMPLATE_CONSTANTS.PROPERTIES]:
            property_obj = iot_property(property)
            self.properties.append(property_obj)
            self.properties_num += 1

        for event in dt_json[TEMPLATE_CONSTANTS.EVENTS]:
            event_obj = iot_event(event)
            self.events.append(event_obj)
            self.events_num += 1

        for action in dt_json[TEMPLATE_CONSTANTS.ACTIONS]:
            action_obj = iot_action(action)
            self.actions.append((action_obj))
            self.actions_num += 1
        print("data template parse success")

    def get_header_file_start(self):
        return "\n#ifndef BLE_QIOT_TEMPLATE_H_\n" \
               "#define BLE_QIOT_TEMPLATE_H_\n" \
               "#ifdef __cplusplus\n" \
               "extern \"C\"{\n" \
               "#endif\n" \
               "\n" \
               "#include <stdint.h>\n" \
               "#include <stdbool.h>\n"

    def get_header_file_public_ctx(self):
        return "\n// data type in template, corresponding to type in json file\n" \
               "enum {\n" \
               "\tBLE_QIOT_DATA_TYPE_BOOL = 0,\n" \
               "\tBLE_QIOT_DATA_TYPE_INT,\n" \
               "\tBLE_QIOT_DATA_TYPE_STRING,\n" \
               "\tBLE_QIOT_DATA_TYPE_FLOAT,\n" \
               "\tBLE_QIOT_DATA_TYPE_ENUM,\n" \
               "\tBLE_QIOT_DATA_TYPE_TIME,\n" \
               "\tBLE_QIOT_DATA_TYPE_STRUCT,\n" \
               "\tBLE_QIOT_DATA_TYPE_ARRAY,\n" \
               "\tBLE_QIOT_DATA_TYPE_BUTT,\n" \
               "};\n" \
               "\n#define BLE_QIOT_ARRAY_INT_BIT_MASK     (1 << 4)" \
               "\n#define BLE_QIOT_ARRAY_STRING_BIT_MASK  (1 << 5)" \
               "\n#define BLE_QIOT_ARRAY_FLOAT_BIT_MASK   (1 << 6)" \
               "\n#define BLE_QIOT_ARRAY_STRUCT_BIT_MASK  (1 << 7)" \
               "\n// message type, reference data template \n" \
               "enum {\n" \
               "\tBLE_QIOT_PROPERTY_AUTH_RW = 0,\n" \
               "\tBLE_QIOT_PROPERTY_AUTH_READ,\n" \
               "\tBLE_QIOT_PROPERTY_AUTH_BUTT,\n" \
               "};\n\n" \
               "// define message flow direction\n" \
               "enum {\n" \
               "\tBLE_QIOT_EFFECT_REQUEST = 0,\n" \
               "\tBLE_QIOT_EFFECT_REPLY,\n" \
               "\tBLE_QIOT_EFFECT_BUTT,\n" \
               "};\n\n" \
               "#define	BLE_QIOT_PACKAGE_MSG_HEAD(_TYPE, _REPLY, _ID)	(((_TYPE) << 6) | (((_REPLY) == BLE_QIOT_EFFECT_REPLY) << 5) | ((_ID) & 0X1F))\n" \
               "#define	BLE_QIOT_PACKAGE_TLV_HEAD(_TYPE, _ID)   	    (((_TYPE) << 5) | ((_ID) & 0X1F))\n" \
               "\n// define tlv struct\n" \
               "typedef struct{" \
               "\tuint8_t type;\n" \
               "\tuint8_t id;\n" \
               "\tuint16_t len;\n" \
               "\tchar *val;\n" \
               "}e_ble_tlv;\n"

    def get_properties_header_public_data(self):
        return "\n// define property set handle return 0 if success, other is error\n" \
               "// sdk call the function that inform the server data to the device\n" \
               "typedef int (*property_set_cb)(const char *data, uint16_t len);\n" \
               "// define property get handle. return the data length obtained, -1 is error, 0 is no data\n" \
               "// sdk call the function fetch user data and send to the server, the data should wrapped by user adn skd just transmit\n" \
               "typedef int (*property_get_cb)(char *buf, uint16_t buf_len);\n" \
               "// each property have a struct ble_property_t, make up a array named sg_ble_property_array\n" \
               "typedef struct{\n" \
               "\tproperty_set_cb set_cb;	//set callback\n" \
               "\tproperty_get_cb get_cb;	//get callback\n" \
               "\tuint8_t authority;	//property authority\n" \
               "\tuint8_t type;	//data type\n" \
               "\tuint16_t elem_num;\n" \
               "}ble_property_t;\n" \
               "typedef int (*property_array_set_cb)(const char *data, uint16_t len, uint16_t index);\n" \
               "typedef int (*property_array_get_cb)(char *buf, uint16_t buf_len, uint16_t index);\n"

    def get_events_header_public_data(self):
        return "\n// define event get handle. return the data length obtained, -1 is error, 0 is no data\n" \
               "// sdk call the function fetch user data and send to the server, the data should wrapped by user adn skd just transmit\n" \
               "typedef int (*event_get_cb)(char *buf, uint16_t buf_len);\n" \
               "// each param have a struct ble_event_param, make up a array for the event\n" \
               "typedef struct{\n" \
               "\tevent_get_cb get_cb;	//get param data callback\n" \
               "\tuint8_t type;	//param type\n" \
               "}ble_event_param;\n" \
               "// a array named sg_ble_event_array is composed by all the event array\n" \
               "typedef struct{\n" \
               "\tble_event_param *event_array;	//array of params data\n" \
               "\tuint8_t array_size;	//array size\n" \
               "}ble_event_t;\n"

    def get_actions_header_public_data(self):
        return "\n// define action input handle, return 0 is success, other is error.\n" \
               "// input_param_array carry the data from server, include input id, data length ,data val\n" \
               "// input_array_size means how many input id\n" \
               "// output_id_array filling with output id numbers that need obtained, sdk will traverse it and call the action_output_handle to obtained data\n" \
               "typedef int (*action_input_handle)(e_ble_tlv *input_param_array, uint8_t input_array_size, uint8_t *output_id_array);\n" \
               "// define action output handle, return length of the data, 0 is no data, -1 is error\n" \
               "// output_id means which id data should be obtained\n" \
               "typedef int (*action_output_handle)(uint8_t output_id, char *buf, uint16_t buf_len);\n" \
               "// each action have a struct ble_action_t, make up a array named sg_ble_action_array\n" \
               "typedef struct{\n" \
               "\taction_input_handle input_cb;	//handle input data\n" \
               "\taction_output_handle output_cb;	// get output data in the callback\n" \
               "\tuint8_t *input_type_array;	//type array for input id\n" \
               "\tuint8_t *output_type_array;	//type array for output id\n" \
               "\tuint8_t input_id_size;	//numbers of input id\n" \
               "\tuint8_t output_id_size;	//numbers of output id\n" \
               "}ble_action_t;\n"

    def get_header_file_public_ctx2(self):
        return "\n// property module\n" \
               "#ifdef BLE_QIOT_INCLUDE_PROPERTY\n" \
               "uint8_t ble_get_property_type_by_id(uint8_t id);\n" \
               "int ble_user_property_set_data(const e_ble_tlv *tlv);\n" \
               "int ble_user_property_get_data_by_id(uint8_t id, char *buf, uint16_t buf_len);\n" \
               "int ble_user_property_report_reply_handle(uint8_t result);\n" \
               "int ble_lldata_parse_tlv(const char *buf, int buf_len, e_ble_tlv *tlv);\n" \
               "int ble_user_property_struct_handle(const char *in_buf, uint16_t buf_len, ble_property_t *struct_arr, uint8_t arr_size);\n" \
               "int ble_user_property_struct_get_data(char *in_buf, uint16_t buf_len, ble_property_t *struct_arr, uint8_t arr_size);\n" \
               "int ble_user_property_struct_array_set(uint8_t id, const char *in_buf, uint16_t buf_len, ble_property_t struct_arr[], uint8_t arr_size);\n" \
               "int ble_user_property_struct_array_get(uint8_t id, char *in_buf, uint16_t buf_len, ble_property_t struct_arr[], uint8_t arr_size);\n" \
               "#endif\n" \
               "// event module\n" \
               "#ifdef BLE_QIOT_INCLUDE_EVENT\n" \
               "int     ble_event_get_id_array_size(uint8_t event_id);\n" \
               "uint8_t ble_event_get_param_id_type(uint8_t event_id, uint8_t param_id);\n" \
               "int     ble_event_get_data_by_id(uint8_t event_id, uint8_t param_id, char *out_buf, uint16_t buf_len);\n" \
               "int     ble_user_event_reply_handle(uint8_t event_id, uint8_t result);\n" \
               "#endif\n" \
               "// action module\n" \
               "#ifdef BLE_QIOT_INCLUDE_ACTION\n" \
               "uint8_t ble_action_get_intput_type_by_id(uint8_t action_id, uint8_t input_id);\n" \
               "uint8_t ble_action_get_output_type_by_id(uint8_t action_id, uint8_t output_id);\n" \
               "int     ble_action_get_input_id_size(uint8_t action_id);\n" \
               "int     ble_action_get_output_id_size(uint8_t action_id);\n" \
               "int     ble_action_user_handle_input_param(uint8_t action_id, e_ble_tlv *input_param_array, uint8_t input_array_size, uint8_t *output_id_array);\n" \
               "int     ble_action_user_handle_output_param(uint8_t action_id, uint8_t output_id, char *buf, uint16_t buf_len);\n" \
               "#endif\n"

    def get_header_file_end(self):
        return "\n#ifdef __cplusplus\n" \
               "}\n" \
               "#endif\n" \
               "#endif //BLE_QIOT_TEMPLATE_H_"

    def get_properties_id_enum(self):
        ctx = "\n// define property id\nenum {\n"
        for property in self.properties:
            ctx += "\tBLE_QIOT_PROPERTY_ID_{},\n".format(property.get_property_id().upper())
        ctx += "\tBLE_QIOT_PROPERTY_ID_BUTT,\n"
        ctx += "};\n"
        return ctx

    def get_properties_header_data(self):
        ctx = ""
        if self.properties_num != 0:
            ctx += "\n\n#define	BLE_QIOT_INCLUDE_PROPERTY \n"
            ctx += self.get_properties_id_enum()
            for property in self.properties:
                ctx += property.get_property_header()
        return ctx

    def get_events_id_enum(self):
        ctx = "\n// define event id\nenum {\n"
        for event in self.events:
            ctx += "\tBLE_QIOT_EVENT_ID_{},\n".format(event.get_event_id().upper())
        ctx += "\tBLE_QIOT_EVENT_ID_BUTT,\n"
        ctx += "};\n"
        return ctx

    def get_events_header_data(self):
        ctx = ""
        if self.events_num != 0:
            ctx += "\n\n#define	BLE_QIOT_INCLUDE_EVENT \n"
            ctx += self.get_events_id_enum()
            for event in self.events:
                ctx += event.get_event_header()
        return ctx

    def get_actions_id_enum(self):
        ctx = "\n// define action id\nenum {\n"
        for action in self.actions:
            ctx += "\tBLE_QIOT_ACTION_ID_{},\n".format(action.get_action_id().upper())
        ctx += "\tBLE_QIOT_ACTION_ID_BUTT,\n"
        ctx += "};\n"
        return ctx

    def get_actions_header_data(self):
        ctx = ""
        if self.actions_num != 0:
            ctx += "\n\n#define	BLE_QIOT_INCLUDE_ACTION \n"
            ctx += self.get_actions_id_enum()
            for action in self.actions:
                ctx += action.get_action_header()
        ctx += "\n#define BLE_QIOT_ACTION_INPUT_ID_BUTT\t{}".format(action_max_input_id)
        ctx += "\n#define BLE_QIOT_ACTION_OUTPUT_ID_BUTT\t{}".format(action_max_output_id)
        return ctx

    def gen_header_file(self, path):
        output_header_file_name = path + '/ble_qiot_template.h'
        output_file = open(output_header_file_name, "w")
        output_file.write("{}".format(TEMPLATE_CONSTANTS.COPYRIGHT))
        output_file.write("{}".format(self.get_header_file_start()))
        output_file.write("{}".format(self.get_header_file_public_ctx()))
        output_file.write("{}".format(self.get_properties_header_data()))
        output_file.write("{}".format(self.get_properties_header_public_data()))
        output_file.write("{}".format(self.get_events_header_data()))
        output_file.write("{}".format(self.get_events_header_public_data()))
        output_file.write("{}".format(self.get_actions_header_data()))
        output_file.write("{}".format(self.get_actions_header_public_data()))
        output_file.write("{}".format(self.get_header_file_public_ctx2()))
        output_file.write("{}".format(self.get_header_file_end()))
        output_file.close()
        print(u"文件 {} 生成成功".format(output_header_file_name))

    def get_source_file_start(self):
        return "\n#ifdef __cplusplus\n" \
               "extern \"C\" {\n" \
               "#endif\n" \
               "#include \"ble_qiot_template.h\"\n" \
               "#include <stdio.h>\n" \
               "#include <stdbool.h>\n" \
               "#include <string.h>\n" \
               "#include \"ble_qiot_export.h\"\n" \
               "#include \"ble_qiot_common.h\"\n" \
               "#include \"ble_qiot_param_check.h\""

    def get_source_file_end(self):
        return "\n#ifdef __cplusplus\n" \
               "}\n" \
               "#endif\n"

    def get_property_array_elem(self, property):
        id = property.get_property_id()
        ctx = ""
        ctx += "\n\t{"
        if property.value.get_array_type():
            ctx += "(property_set_cb)ble_property_{0}_set, (property_get_cb)ble_property_{0}_get, 0, {1}, {2}". \
                format(id, property.get_property_type() + "|" + property.value.get_array_type(), property.value.get_elem_num(id))
        else:
            ctx += "ble_property_{0}_set, ble_property_{0}_get, 0, {1}, {2}". \
                format(id, property.get_property_type(), property.value.get_elem_num(id))
        ctx += "},"
        return ctx

    def get_properties_source_data(self):
        ctx = ""
        if self.properties_num != 0:
            for property in self.properties:
                ctx += property.get_property_source()

        ctx += "\nble_property_t sg_ble_property_array[{}] = {{".format(self.properties_num)
        for property in self.properties:
            ctx += self.get_property_array_elem(property)
        ctx += "\n};\n\n"
        return ctx

    def get_event_array_elem(self, event):
        ctx = ""
        ctx += "\n\t{"
        ctx += "sg_ble_event_{0}_array, {1}".format(event.get_event_id(), event.get_event_params_num())
        ctx += "},"
        return ctx

    def get_events_source_data(self):
        if self.events_num == 0:
            return ""
        ctx = ""
        for event in self.events:
            ctx += event.get_event_source()

        ctx += "\nble_event_t sg_ble_event_array[{}] = {{".format(self.events_num)
        for event in self.events:
            ctx += self.get_event_array_elem(event)
        ctx += "\n};\n\n"

        return ctx

    def get_action_elem(self, action):
        ctx = ""
        ctx += "\n\t{"
        ctx += "ble_action_handle_{0}_input_cb, ble_action_handle_{0}_output_cb," \
               "sg_ble_action_{0}_input_type_array, sg_ble_action_{0}_output_type_array," \
               "{1},{2}".format(action.get_action_id(), action.get_action_input_param_num(), action.get_action_output_param_num())
        ctx += "},"
        return ctx

    def get_actions_source_data(self):
        if self.actions_num == 0:
            return ""
        ctx = ""
        for action in self.actions:
            ctx += action.get_action_source()

        ctx += "\nble_action_t sg_ble_action_array[{}] = {{".format(self.actions_num)
        for action in self.actions:
            ctx += self.get_action_elem(action)
        ctx += "\n};\n"
        return ctx

    def gen_source_file(self, path):
        output_source_file_name = path + '/ble_qiot_template.c'
        output_file = open(output_source_file_name, "w")
        output_file.write("{}".format(TEMPLATE_CONSTANTS.COPYRIGHT))
        output_file.write("{}".format(self.get_source_file_start()))
        output_file.write("{}".format(self.get_properties_source_data()))
        output_file.write("{}".format(self.get_events_source_data()))
        output_file.write("{}".format(self.get_actions_source_data()))
        output_file.write("{}".format(self.get_source_file_end()))
        output_file.close()
        print(u"文件 {} 生成成功".format(output_source_file_name))


def main():
    print("enter main")
    parser = argparse.ArgumentParser(description='LLSync datatemplate and events config code generator.',
                                     usage='use "./interpret_dt_ble.py -c xx/config.json" gen config code')
    parser.add_argument('-c', '--config', dest='config', metavar='xxx.json', required=True, default='xxx.json',
                        help='copy the generated file (ble_qiot_template.c and ble_qiot_template.h) to data_template dir '
                             'or your own code dir with data_template. '
                             '\nconfig file can be download from tencent iot-hub platfrom. https://console.cloud.tencent.com/iotcloud')
    parser.add_argument('-d', '--dest', dest='dest', required=False, default='.',
                        help='Dest directory for generated code files, no / at the end.')

    args = parser.parse_args()

    config_path = args.config
    if not os.path.exists(config_path):
        print(u"错误：配置文件{}不存在，请重新指定数据模板配置文件路径,请参考用法 ./interpret_dt_ble.py -c xx/data_template.json".format(config_path))
        return 1

    config_dir = os.path.dirname(config_path)
    if config_dir:
        config_dir += "/"

    f = open(config_path, "rb")
    try:
        thingmodel = json.load(f)
        if TEMPLATE_CONSTANTS.PROPERTIES not in thingmodel:
            print(u"错误：{} 文件中未发现 DataTemplate 属性字段，请检查文件格式是否合法。".format(config_path))
            return 1
        if TEMPLATE_CONSTANTS.EVENTS not in thingmodel:
            thingmodel[TEMPLATE_CONSTANTS.EVENTS] = []
        if TEMPLATE_CONSTANTS.ACTIONS not in thingmodel:
            thingmodel[TEMPLATE_CONSTANTS.ACTIONS] = []
        print(u"加载 {} 文件成功".format(config_path))

    except ValueError as e:
        print(u"错误：文件格式非法，请检查 {} 文件是否是 JSON 格式。".format(config_path))
        return 1

    try:
        snippet = iot_parse_dt(thingmodel)
        snippet.gen_header_file(args.dest)
        snippet.gen_source_file(args.dest)

    except ValueError as e:
        print(e)
        return 1


if __name__ == '__main__':
    sys.exit(main())
