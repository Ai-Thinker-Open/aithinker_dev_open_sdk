#include "utils/ln_misc.h"

/******************************  Function Declarations  ***********************/
void port_ble_stack_init(void);


void port_ble_stack_init(void)
{
    uint8_t mac[6] ={0};

    ln_generate_random_mac(mac);

    // TODO: 开发阶段使用固定的 BLE MAC 地址
    mac[0] = 0xA0;
    mac[1] = 0xA1;
    mac[2] = 0xA2;
    mac[3] = 0xA3;
    mac[4] = 0xA4;
    mac[5] = 0xA5;

    mac[5] |= 0xC0; // This address is random generated, so assign 0x11 => Static Random Address

    extern void rw_init(uint8_t mac[6]);
    rw_init(mac);

    extern void ble_app_init(void);
    ble_app_init();
}
