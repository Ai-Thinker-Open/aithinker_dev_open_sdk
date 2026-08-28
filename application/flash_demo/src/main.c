
#include "aiio_flash.h"
#include "aiio_adapter_include.h"

// /**********LN882h*********/
// #define DEMO_FLASH_TEST_ADDR 0x100000
// #define DEMO_FLASH_TEST_LEN_ERASE  0x1000
// #define DEMO_FLASH_TEST_LEN   0x1000

// /**********Bl602*********/
// #define DEMO_FLASH_TEST_ADDR 0x2C2000
// #define DEMO_FLASH_TEST_LEN_ERASE 0x1000
// #define DEMO_FLASH_TEST_LEN 0x1000

/**********Bl602*********/
// #define DEMO_FLASH_TEST_ADDR 0x7C000
#define DEMO_FLASH_TEST_ADDR                 (CONFIG_FLASH_RW_START_ADDR)
#define DEMO_FLASH_TEST_LEN_ERASE 0x1000
#define DEMO_FLASH_TEST_LEN 0x1000

// static uint8_t write_data[DEMO_FLASH_TEST_LEN];
// static uint8_t read_data[DEMO_FLASH_TEST_LEN];
static uint32_t err_cnt = 0;
// static uint32_t flash_id = 0;
// static uint32_t device_id = 0;
// static uint32_t manufacturer_id = 0;

static void flash_test(void)
{
    int i;
    uint8_t *p_wbuf, *p_rbuf;

    p_wbuf = malloc(DEMO_FLASH_TEST_LEN);
    if (p_wbuf == NULL)
    {
        aiio_log_e("no memory!\r\n");
        return;
    }

    p_rbuf = malloc(DEMO_FLASH_TEST_LEN);
    if (p_rbuf == NULL)
    {
        aiio_log_e("no memory!\r\n");
        return;
    }

    for (i = 0; i < DEMO_FLASH_TEST_LEN; i++)
    {
        p_wbuf[i] = i; //& 0xff;
    }

    aiio_flash_init();
    aiio_flash_erase(DEMO_FLASH_TEST_ADDR, DEMO_FLASH_TEST_LEN);
    aiio_flash_write(DEMO_FLASH_TEST_ADDR, p_wbuf, DEMO_FLASH_TEST_LEN);

    for (int w = 0; w < 20; w++)
    {
        aiio_log_w("write_date read_test[%02d] : 0x%02x", w, p_wbuf[w]);
    }
    aiio_log_w("aiio_flash_write_erase successfully!");

    aiio_flash_read(DEMO_FLASH_TEST_ADDR, p_rbuf, DEMO_FLASH_TEST_LEN);
    aiio_log_w("aiio_flash_read successfully!");
    for (int r = 0; r < 20; r++)
    {
        aiio_log_w("read_date read_test[%02d] : 0x%02x", r, p_rbuf[r]);
    }

    for (int date = 0; date < DEMO_FLASH_TEST_LEN; date++)
    {
        if (p_rbuf[date] != p_wbuf[date])
        {
            err_cnt++;
        }
    }
    if (err_cnt != 0)
    {
        aiio_log_e("Flash test fail!");
    }
    else
    {
        aiio_log_a("Flash test successfully!");
    }

    // Erase flash
    if (aiio_flash_erase(DEMO_FLASH_TEST_ADDR, DEMO_FLASH_TEST_LEN) != 0)
    {
        aiio_log_e("Flash Erase Fail!");
    }
    uint8_t *read_test;
    read_test = malloc(DEMO_FLASH_TEST_LEN);
    if (read_test == NULL)
    {
        aiio_log_e("no memory!\r\n");
        return;
    }

    // memset(read_test,0,sizeof(read_test));
    aiio_flash_read(DEMO_FLASH_TEST_ADDR, read_test, DEMO_FLASH_TEST_LEN);
    // aiio_log_a("0x%02X  0x%02X 0x%02X  0x%02X  0x%02X",read_test[0],read_test[1],read_test[2],read_test[3],read_test[4]);
    for (int d = 0; d < 10; d++)
    {
        aiio_log_a("read_date after erase read_test[%x] : 0x%02X", d, read_test[d]);
    }

    aiio_os_free(p_wbuf);
    aiio_os_free(p_rbuf);
    aiio_os_free(read_test);
}

void aiio_main(void *params)
{
    params = params;

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    flash_test();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
