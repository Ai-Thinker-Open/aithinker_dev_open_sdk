#ifndef LN_BLE_TEST_H_
#define LN_BLE_TEST_H_


void app_le_test_tx_start_cmd(uint8_t tx_ch, uint8_t phy, uint8_t tx_len, uint8_t pkt);
void app_le_test_rx_start_cmd(uint8_t rx_ch, uint8_t phy, uint8_t mod_index);
void app_le_test_stop_cmd(void);

#endif
