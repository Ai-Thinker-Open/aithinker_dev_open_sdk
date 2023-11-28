#!/bin/bash

rm -rf airkiss_demo/vendor
rm -rf at_demo/vendor
rm -rf hello_world/vendor
rm -rf i2s_demo/vendor
rm -rf pwmcap_demo/vendor
rm -rf spi_demo/vendor
rm -rf gpio_demo/vendor
rm -rf i2c_demo/vendor
rm -rf nfcconfig/vendor
rm -rf pwm_demo/vendor
rm -rf uart_demo/vendor
rm -rf wifi_demo/vendor

cp -a  demo/vendor airkiss_demo/vendor
cp -a  demo/vendor at_demo/vendor
cp -a  demo/vendor hello_world/vendor
cp -a  demo/vendor i2s_demo/vendor
cp -a  demo/vendor pwmcap_demo/vendor
cp -a  demo/vendor spi_demo/vendor
cp -a  demo/vendor gpio_demo/vendor
cp -a  demo/vendor i2c_demo/vendor
cp -a  demo/vendor nfcconfig/vendor
cp -a  demo/vendor pwm_demo/vendor
cp -a  demo/vendor uart_demo/vendor
cp -a  demo/vendor wifi_demo/vendor
