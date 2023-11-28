#!/bin/bash

cd "$(dirname "$0")"

app_tmp_dir=$1
sdk_prj_dir=$2/sdk/bl_iot_sdk/customer_app/dev_project

rm -rf $app_tmp_dir && rm -rf $sdk_prj_dir
