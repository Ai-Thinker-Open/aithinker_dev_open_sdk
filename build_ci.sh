#!/bin/bash

./build.sh update

./build.sh bl602 aithing_cloud_soc_mcu defconfig && ./build.sh bl602 aithing_cloud_soc_mcu cn debug && ./build.sh bl618 aithing_cloud_soc_mcu defconfig && ./build.sh bl618 aithing_cloud_soc_mcu cn debug && ./build.sh bl616 aithing_cloud_soc_mcu defconfig && ./build.sh bl616 aithing_cloud_soc_mcu cn debug

./build.sh bl602 aithink_cloud_general defconfig && ./build.sh bl602 aithink_cloud_general cn debug

./build.sh bl602 demo defconfig && ./build.sh bl602 demo cn debug && ./build.sh ln882h demo defconfig && ./build.sh ln882h demo cn debug

./build.sh bl602 gpio_demo defconfig && ./build.sh bl602 gpio_demo cn debug && ./build.sh ln882h gpio_demo defconfig && ./build.sh ln882h gpio_demo cn debug && ./build.sh bl618 gpio_demo defconfig && ./build.sh bl618 gpio_demo cn debug

./build.sh bl618 uart_demo defconfig && ./build.sh bl618 uart_demo cn debug
./build.sh bl602 uart_demo defconfig && ./build.sh bl602 uart_demo cn debug && ./build.sh ln882h uart_demo defconfig && ./build.sh ln882h uart_demo cn debug

./build.sh bl602 http_demo defconfig && ./build.sh bl602 http_demo cn debug && ./build.sh ln882h http_demo defconfig && ./build.sh ln882h http_demo cn debug && ./build.sh bl618 http_demo defconfig && ./build.sh bl618 http_demo cn debug

./build.sh bl602 https_demo defconfig && ./build.sh bl602 https_demo cn debug && ./build.sh ln882h https_demo defconfig && ./build.sh ln882h https_demo cn debug && ./build.sh bl618 https_demo defconfig && ./build.sh bl618 https_demo cn debug

./build.sh bl616 doubulelight_demo defconfig && ./build.sh bl616 doubulelight_demo cn debug
./build.sh bl618 doubulelight_demo defconfig && ./build.sh bl618 doubulelight_demo cn debug
./build.sh bl602 doubulelight_demo defconfig && ./build.sh bl602 doubulelight_demo cn debug &&./build.sh ln882h doubulelight_demo defconfig && ./build.sh ln882h doubulelight_demo cn debug

./build.sh bl616 relay_demo defconfig && ./build.sh bl616 relay_demo cn debug
./build.sh bl618 relay_demo defconfig && ./build.sh bl618 relay_demo cn debug
./build.sh bl602 relay_demo defconfig && ./build.sh bl602 relay_demo cn debug &&./build.sh ln882h relay_demo defconfig && ./build.sh ln882h relay_demo cn debug

./build.sh bl602 ds1302_demo defconfig && ./build.sh bl602 ds1302_demo cn debug &&./build.sh ln882h ds1302_demo defconfig && ./build.sh ln882h relay_demo cn debug

./build.sh bl616 buzzer_demo defconfig && ./build.sh bl616 buzzer_demo cn debug
./build.sh bl618 buzzer_demo defconfig && ./build.sh bl618 buzzer_demo cn debug
./build.sh bl602 buzzer_demo defconfig && ./build.sh bl602 buzzer_demo cn debug &&./build.sh ln882h buzzer_demo defconfig && ./build.sh ln882h buzzer_demo cn debug

./build.sh bl616 photosensitive_demo defconfig && ./build.sh bl616 photosensitive_demo cn debug
./build.sh bl618 photosensitive_demo defconfig && ./build.sh bl618 photosensitive_demo cn debug
./build.sh bl602 photosensitive_demo defconfig && ./build.sh bl602 photosensitive_demo cn debug &&./build.sh ln882h photosensitive_demo defconfig && ./build.sh ln882h photosensitive_demo cn debug

./build.sh bl602 tricolourlight_demo defconfig && ./build.sh bl602 tricolourlight_demo cn debug &&./build.sh ln882h tricolourlight_demo defconfig && ./build.sh ln882h tricolourlight_demo cn debug

./build.sh bl616 ntc_demo defconfig && ./build.sh bl616 ntc_demo cn debug
./build.sh bl618 ntc_demo defconfig && ./build.sh bl618 ntc_demo cn debug
./build.sh bl602 ntc_demo defconfig && ./build.sh bl602 ntc_demo cn debug &&./build.sh ln882h ntc_demo defconfig && ./build.sh ln882h ntc_demo cn debug

./build.sh bl616 laseremit_demo defconfig && ./build.sh bl616 laseremit_demo cn debug
./build.sh bl618 laseremit_demo defconfig && ./build.sh bl618 laseremit_demo cn debug
./build.sh bl602 laseremit_demo defconfig && ./build.sh bl602 laseremit_demo cn debug &&./build.sh ln882h laseremit_demo defconfig && ./build.sh ln882h laseremit_demo cn debug

./build.sh bl616 ds18b20_demo defconfig && ./build.sh bl616 ds18b20_demo cn debug
./build.sh bl618 ds18b20_demo defconfig && ./build.sh bl618 ds18b20_demo cn debug
./build.sh bl602 ds18b20_demo defconfig && ./build.sh bl602 ds18b20_demo cn debug &&./build.sh ln882h ds18b20_demo defconfig && ./build.sh ln882h ds18b20_demo cn debug

./build.sh bl616 tiltswitch_demo defconfig && ./build.sh bl616 tiltswitch_demo cn debug
./build.sh bl618 tiltswitch_demo defconfig && ./build.sh bl618 tiltswitch_demo cn debug
./build.sh bl602 tiltswitch_demo defconfig && ./build.sh bl602 tiltswitch_demo cn debug &&./build.sh ln882h tiltswitch_demo defconfig && ./build.sh ln882h tiltswitch_demo cn debug

./build.sh bl616 shock_demo defconfig && ./build.sh bl616 shock_demo cn debug
./build.sh bl618 shock_demo defconfig && ./build.sh bl618 shock_demo cn debug
./build.sh bl602 shock_demo defconfig && ./build.sh bl602 shock_demo cn debug &&./build.sh ln882h shock_demo defconfig && ./build.sh ln882h shock_demo cn debug

./build.sh bl602 adc_demo defconfig && ./build.sh bl602 adc_demo cn debug
./build.sh ln882h adc_demo defconfig && ./build.sh ln882h adc_demo cn debug
./build.sh bl618 adc_demo defconfig && ./build.sh bl618 adc_demo cn debug
./build.sh bl616 adc_demo defconfig && ./build.sh bl616 adc_demo cn debug

./build.sh bl602 airkiss_demo defconfig && ./build.sh bl602 airkiss_demo cn debug &&./build.sh ln882h airkiss_demo defconfig && ./build.sh ln882h airkiss_demo cn debug

./build.sh bl602 i2c_demo defconfig && ./build.sh bl602 i2c_demo cn debug &&./build.sh ln882h i2c_demo defconfig && ./build.sh ln882h i2c_demo cn debug

./build.sh ln882h ir_demo defconfig && ./build.sh ln882h ir_demo cn debug

./build.sh bl618 pwm_demo defconfig && ./build.sh bl618 pwm_demo cn debug &&./build.sh bl602 pwm_demo defconfig && ./build.sh bl602 pwm_demo cn debug &&./build.sh ln882h pwm_demo defconfig && ./build.sh ln882h pwm_demo cn debug

./build.sh ln882h pwmcap_demo defconfig && ./build.sh ln882h pwmcap_demo cn debug

./build.sh bl602 spi_demo defconfig && ./build.sh bl602 spi_demo cn debug &&./build.sh ln882h spi_demo defconfig && ./build.sh ln882h spi_demo cn debug

./build.sh bl602 tcp_client_demo defconfig && ./build.sh bl602 tcp_client_demo cn debug &&./build.sh ln882h tcp_client_demo defconfig && ./build.sh ln882h tcp_client_demo cn debug

./build.sh bl602 tcp_server_demo defconfig && ./build.sh bl602 tcp_server_demo cn debug &&./build.sh ln882h tcp_server_demo defconfig && ./build.sh ln882h tcp_server_demo cn debug

./build.sh bl602 udp_client_demo defconfig && ./build.sh bl602 udp_client_demo cn debug &&./build.sh ln882h udp_client_demo defconfig && ./build.sh ln882h udp_client_demo cn debug

./build.sh bl602 udp_server_demo defconfig && ./build.sh bl602 udp_server_demo cn debug &&./build.sh ln882h udp_server_demo defconfig && ./build.sh ln882h udp_server_demo cn debug

./build.sh bl616 button_demo defconfig && ./build.sh bl616 button_demo cn debug
./build.sh bl618 button_demo defconfig && ./build.sh bl618 button_demo cn debug
./build.sh bl602 button_demo defconfig && ./build.sh bl602 button_demo cn debug &&./build.sh ln882h button_demo defconfig && ./build.sh ln882h button_demo cn debug

./build.sh bl602 mqtt_demo defconfig && ./build.sh bl602 mqtt_demo cn debug && ./build.sh ln882h mqtt_demo defconfig && ./build.sh ln882h mqtt_demo cn debug

./build.sh bl618 nvs_flash_demo defconfig && ./build.sh bl618 nvs_flash_demo cn debug && ./build.sh bl616 nvs_flash_demo defconfig && ./build.sh bl616 nvs_flash_demo cn debug
./build.sh bl602 nvs_flash_demo defconfig && ./build.sh bl602 nvs_flash_demo cn debug && ./build.sh ln882h nvs_flash_demo defconfig && ./build.sh ln882h nvs_flash_demo cn debug

./build.sh bl618 smartconfig_demo defconfig && ./build.sh bl618 smartconfig_demo cn debug
./build.sh bl602 smartconfig_demo defconfig && ./build.sh bl602 smartconfig_demo cn debug && ./build.sh ln882h smartconfig_demo defconfig && ./build.sh ln882h smartconfig_demo cn debug

./build.sh bl602 blufi_demo defconfig && ./build.sh bl602 blufi_demo cn debug && ./build.sh ln882h blufi_demo defconfig && ./build.sh ln882h blufi_demo cn debug && ./build.sh bl618 blufi_demo defconfig && ./build.sh bl618 blufi_demo cn debug

./build.sh bl602 wifi_ap_demo defconfig && ./build.sh bl602 wifi_ap_demo cn debug && ./build.sh ln882h wifi_ap_demo defconfig && ./build.sh ln882h wifi_ap_demo cn debug

./build.sh bl602 wifi_apsta_demo defconfig && ./build.sh bl602 wifi_apsta_demo cn debug && ./build.sh ln882h wifi_apsta_demo defconfig && ./build.sh ln882h wifi_apsta_demo cn debug

./build.sh bl602 wifi_sta_demo defconfig && ./build.sh bl602 wifi_sta_demo cn debug && ./build.sh ln882h wifi_sta_demo defconfig && ./build.sh ln882h wifi_sta_demo cn debug

./build.sh bl602 wifi_demo defconfig && ./build.sh bl602 wifi_demo cn debug && ./build.sh ln882h wifi_demo defconfig && ./build.sh ln882h wifi_demo cn debug

./build.sh bl602 ble_demo defconfig && ./build.sh bl602 ble_demo cn debug && ./build.sh ln882h ble_demo defconfig && ./build.sh ln882h ble_demo cn debug

./build.sh bl602 at_demo defconfig && ./build.sh bl602 at_demo cn debug && ./build.sh ln882h at_demo defconfig && ./build.sh ln882h at_demo cn debug

./build.sh bl602 mqtts_demo defconfig && ./build.sh bl602 mqtts_demo cn debug && ./build.sh ln882h mqtts_demo defconfig && ./build.sh ln882h mqtts_demo cn debug

./build.sh bl602 wifi_ble_demo defconfig && ./build.sh bl602 wifi_ble_demo cn debug && ./build.sh ln882h wifi_ble_demo defconfig && ./build.sh ln882h wifi_ble_demo cn debug

./build.sh bl602 mdns_demo defconfig && ./build.sh bl602 mdns_demo cn debug
./build.sh bl616 mdns_demo defconfig && ./build.sh bl616 mdns_demo cn debug
./build.sh bl618 mdns_demo defconfig && ./build.sh bl618 mdns_demo cn debug
./build.sh ln882h mdns_demo defconfig && ./build.sh ln882h mdns_demo cn debug

./build.sh bl602 nfcconfig defconfig && ./build.sh bl602 nfcconfig cn debug
./build.sh bl616 nfcconfig defconfig && ./build.sh bl616 nfcconfig cn debug
./build.sh bl618 nfcconfig defconfig && ./build.sh bl618 nfcconfig cn debug
./build.sh ln882h nfcconfig defconfig && ./build.sh ln882h nfcconfig cn debug

./build.sh bl618 lvgl_demo defconfig && ./build.sh bl618 lvgl_demo cn debug
./build.sh bl616 lvgl_demo defconfig && ./build.sh bl616 lvgl_demo cn debug

./build.sh bl602 http_server_demo defconfig && ./build.sh bl602 http_server_demo cn debug && ./build.sh ln882h http_server_demo defconfig && ./build.sh ln882h http_server_demo cn debug
./build.sh bl616 http_server_demo defconfig && ./build.sh bl616 http_server_demo cn debug && ./build.sh bl618 http_server_demo defconfig && ./build.sh bl618 http_server_demo cn debug

./build.sh bl618 wifi_ap_demo defconfig && ./build.sh bl618 wifi_ap_demo cn debug && ./build.sh bl616 wifi_ap_demo defconfig && ./build.sh bl616 wifi_ap_demo cn debug

./build.sh bl618 wifi_apsta_demo defconfig && ./build.sh bl618 wifi_apsta_demo cn debug &&./build.sh bl616 wifi_apsta_demo defconfig && ./build.sh bl616 wifi_apsta_demo cn debug

./build.sh bl618 wifi_sta_demo defconfig && ./build.sh bl618 wifi_sta_demo cn debug &&./build.sh bl616 wifi_sta_demo defconfig && ./build.sh bl616 wifi_sta_demo cn debug

./build.sh bl618 wifi_demo defconfig && ./build.sh bl618 wifi_demo cn debug &&./build.sh bl616 wifi_demo defconfig && ./build.sh bl616 wifi_demo cn debug

./build.sh bl618 at_demo defconfig && ./build.sh bl618 at_demo cn debug &&./build.sh bl616 at_demo defconfig && ./build.sh bl616 at_demo cn debug

./build.sh bl618 mqtt_demo defconfig && ./build.sh bl618 mqtt_demo cn debug &&./build.sh bl616 mqtt_demo defconfig && ./build.sh bl616 mqtt_demo cn debug

./build.sh bl618 mqtts_demo defconfig && ./build.sh bl618 mqtts_demo cn debug &&./build.sh bl616 mqtts_demo defconfig && ./build.sh bl616 mqtts_demo cn debug

./build.sh bl618 tcp_client_demo defconfig && ./build.sh bl618 tcp_client_demo cn debug && ./build.sh bl616 tcp_client_demo defconfig && ./build.sh bl616 tcp_client_demo cn debug

./build.sh bl618 tcp_server_demo defconfig && ./build.sh bl618 tcp_server_demo cn debug && ./build.sh bl616 tcp_server_demo defconfig && ./build.sh bl616 tcp_server_demo cn debug

./build.sh bl618 udp_client_demo defconfig && ./build.sh bl618 udp_client_demo cn debug && ./build.sh bl616 udp_client_demo defconfig && ./build.sh bl616 udp_client_demo cn debug

./build.sh bl618 udp_server_demo defconfig && ./build.sh bl618 udp_server_demo cn debug && ./build.sh bl616 udp_server_demo defconfig && ./build.sh bl616 udp_server_demo cn debug

./build.sh bl618 ble_demo defconfig && ./build.sh bl618 ble_demo cn debug

./build.sh bl602 ble_master_demo defconfig && ./build.sh bl602 ble_master_demo cn debug && ./build.sh bl618 ble_master_demo defconfig && ./build.sh bl618 ble_master_demo cn debug
./build.sh ln882h ble_master_demo defconfig && ./build.sh ln882h ble_master_demo cn debug

./build.sh bl618 i2c_demo defconfig && ./build.sh bl618 i2c_demo cn debug
./build.sh bl618 i2s_demo defconfig && ./build.sh bl618 i2s_demo cn debug
./build.sh bl618 airkiss_demo defconfig && ./build.sh bl618 airkiss_demo cn debug

./build.sh bl618 spi_demo defconfig && ./build.sh bl618 spi_demo cn debug
./build.sh bl616 spi_demo defconfig && ./build.sh bl616 spi_demo cn debug

./build.sh bl602 wifi_ap_ble_master defconfig && ./build.sh bl602 wifi_ap_ble_master cn debug
./build.sh bl618 wifi_ap_ble_master defconfig && ./build.sh bl618 wifi_ap_ble_master cn debug
./build.sh bl616 wifi_ap_ble_master defconfig && ./build.sh bl616 wifi_ap_ble_master cn debug
./build.sh ln882h wifi_ap_ble_master defconfig && ./build.sh ln882h wifi_ap_ble_master cn debug

./build.sh bl602 wifi_ap_ble_salve defconfig && ./build.sh bl602 wifi_ap_ble_salve cn debug
./build.sh bl618 wifi_ap_ble_salve defconfig && ./build.sh bl618 wifi_ap_ble_salve cn debug
./build.sh bl616 wifi_ap_ble_salve defconfig && ./build.sh bl616 wifi_ap_ble_salve cn debug
./build.sh ln882h wifi_ap_ble_salve defconfig && ./build.sh ln882h wifi_ap_ble_salve cn debug

./build.sh bl602 wifi_sta_ble_salve defconfig && ./build.sh bl602 wifi_sta_ble_salve cn debug
./build.sh bl618 wifi_sta_ble_salve defconfig && ./build.sh bl618 wifi_sta_ble_salve cn debug
./build.sh bl616 wifi_sta_ble_salve defconfig && ./build.sh bl616 wifi_sta_ble_salve cn debug
./build.sh ln882h wifi_sta_ble_salve defconfig && ./build.sh ln882h wifi_sta_ble_salve cn debug

./build.sh bl602 wifi_sta_ble_master defconfig && ./build.sh bl602 wifi_sta_ble_master cn debug
./build.sh bl618 wifi_sta_ble_master defconfig && ./build.sh bl618 wifi_sta_ble_master cn debug
./build.sh bl616 wifi_sta_ble_master defconfig && ./build.sh bl616 wifi_sta_ble_master cn debug
./build.sh ln882h wifi_sta_ble_master defconfig && ./build.sh ln882h wifi_sta_ble_master cn debug

./build.sh bl602 ir_nec_rx_demo defconfig && ./build.sh bl602 ir_nec_rx_demo cn debug && ./build.sh bl618 ir_nec_rx_demo defconfig && ./build.sh bl618 ir_nec_rx_demo cn debug

./build.sh bl618 DVP_cam_demo defconfig && ./build.sh bl618 DVP_cam_demo cn debug
./build.sh bl618 USB_cam_demo defconfig && ./build.sh bl618 USB_cam_demo cn debug
./build.sh bl616 DVP_cam_demo defconfig && ./build.sh bl616 DVP_cam_demo cn debug
./build.sh bl616 USB_cam_demo defconfig && ./build.sh bl616 USB_cam_demo cn debug

./build.sh bl602 http_speed_demo defconfig && ./build.sh bl602 http_speed_demo cn debug && ./build.sh ln882h http_speed_demo defconfig && ./build.sh ln882h http_speed_demo cn debug
./build.sh bl602 https_speed_demo defconfig && ./build.sh bl602 https_speed_demo cn debug && ./build.sh ln882h https_speed_demo defconfig && ./build.sh ln882h https_speed_demo cn debug

./build.sh bl602 rd01_demo defconfig && ./build.sh bl602 rd01_demo cn debug

tar -zcvf out.tar.gz ./out
