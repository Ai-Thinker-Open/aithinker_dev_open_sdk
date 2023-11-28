#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "aiio_autoconf.h"

static uint8_t rx_data[100];
static uint8_t tx_data[100];
static uint8_t status[2];
static uint8_t err_cnt = 0;


#define  AIIO_SPI_CS_PORT     CONFIG_SPI_CS_PORT
#define  AIIO_SPI_CS_PIN      CONFIG_SPI_CS_PIN
#define  AIIO_SPI_CLK_PORT    CONFIG_SPI_CLK_PORT
#define  AIIO_SPI_CLK_PIN     CONFIG_SPI_CLK_PIN
#define  AIIO_SPI_MOSI_PORT   CONFIG_SPI_MOSI_PORT
#define  AIIO_SPI_MOSI_PIN    CONFIG_SPI_MOSI_PIN
#define  AIIO_SPI_MISO_PORT   CONFIG_SPI_MISO_PORT
#define  AIIO_SPI_MISO_PIN    CONFIG_SPI_MISO_PIN

// #define  AIIO_SPI_CS_PORT     AIIO_GPIO_A
// #define  AIIO_SPI_CS_PIN      AIIO_GPIO_NUM_10
// #define  AIIO_SPI_CLK_PORT    AIIO_GPIO_A
// #define  AIIO_SPI_CLK_PIN     AIIO_GPIO_NUM_5
// #define  AIIO_SPI_MOSI_PORT   AIIO_GPIO_A
// #define  AIIO_SPI_MOSI_PIN    AIIO_GPIO_NUM_6
// #define  AIIO_SPI_MISO_PORT   AIIO_GPIO_A
// #define  AIIO_SPI_MISO_PIN    AIIO_GPIO_NUM_7

void aiio_main(void *params)
{
    aiio_spi_pin_cfg_t aiio_spi_pin_cfg;

    //ln882h demo
    aiio_spi_pin_cfg.aiio_gpio_cs_port = AIIO_SPI_CS_PORT;
    aiio_spi_pin_cfg.aiio_gpio_cs_pin = AIIO_SPI_CS_PIN;
    aiio_spi_pin_cfg.aiio_gpio_clk_port = AIIO_SPI_CLK_PORT;
    aiio_spi_pin_cfg.aiio_gpio_clk_pin = AIIO_SPI_CLK_PIN;
    aiio_spi_pin_cfg.aiio_gpio_mosi_port = AIIO_SPI_MOSI_PORT;
    aiio_spi_pin_cfg.aiio_gpio_mosi_pin = AIIO_SPI_MOSI_PIN;
    aiio_spi_pin_cfg.aiio_gpio_miso_port = AIIO_SPI_MISO_PORT;
    aiio_spi_pin_cfg.aiio_gpio_miso_pin = AIIO_SPI_MISO_PIN;

    aiio_w25qxx_init(AIIO_SPI_0, &aiio_spi_pin_cfg);

    aiio_w25qxx_read_id(AIIO_SPI_0,rx_data,1000);

    aiio_log_w("25WQ16 ID: %x %x %x",rx_data[1],rx_data[2],rx_data[3]);
    
    aiio_w25qxx_read_status(AIIO_SPI_0,rx_data,1000);

    aiio_w25qxx_enable(AIIO_SPI_0,1000);

    aiio_w25qxx_read_status(AIIO_SPI_0,rx_data,1000);

    aiio_w25qxx_erase(AIIO_SPI_0,0x200,1000);

    while(1)
    {
        aiio_w25qxx_read_status(AIIO_SPI_0,status,1000);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    for(int i = 0;i < 100;i ++)
    {
        tx_data[i] = i;
    }
    aiio_w25qxx_write(AIIO_SPI_0,0x200,tx_data,100,1000);

    while(1)
    {
        aiio_w25qxx_read_status(AIIO_SPI_0,status,1000);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    aiio_w25qxx_read(AIIO_SPI_0,0x200,rx_data,100,1000);
    
    err_cnt = 0;
    for(int i = 0 ; i < 100; i++)
    {
        // aiio_log_w("tx_data:0x%02x  rx_data:0x%02x",tx_data[i],rx_data[i]);
        if(rx_data[i] != tx_data[i])
        {
            err_cnt++;
        }
    }

    if(err_cnt != 0)
    {
        aiio_log_w(" SPI + 25WQ16 test fail! \n");
    }
    else
    {
        aiio_log_w(" SPI + 25WQ16 test success! \n");
    }

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
