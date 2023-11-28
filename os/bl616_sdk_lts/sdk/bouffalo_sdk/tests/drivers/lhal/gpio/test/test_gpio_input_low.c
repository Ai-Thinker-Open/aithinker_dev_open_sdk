#include "bflb_gpio.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(BL616)
static uint8_t gpio_map[] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
    GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13,
    GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_16, GPIO_PIN_17,
    GPIO_PIN_18, GPIO_PIN_19, GPIO_PIN_20, GPIO_PIN_23,
    GPIO_PIN_24, GPIO_PIN_25, GPIO_PIN_26, GPIO_PIN_27,
    GPIO_PIN_28, GPIO_PIN_29, GPIO_PIN_30, GPIO_PIN_31,
    GPIO_PIN_32, GPIO_PIN_33, GPIO_PIN_34,
#if defined(CONFIG_TEST_ALL_GPIO) && CONFIG_TEST_ALL_GPIO
    GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7,
    GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_21, GPIO_PIN_22
#endif
};
#else
#error "not supported chip"
#endif

static void gpio_input_low_test(uint32_t cfg)
{
    char message[128];
    struct bflb_device_s *gpio;
    gpio = bflb_device_get_by_name("gpio");
    TEST_ASSERT_NOT_NULL(gpio);

    for (uint8_t i = 0; i < sizeof(gpio_map); i++) {
        bflb_gpio_init(gpio, gpio_map[i], cfg);
    }
    bflb_mtimer_delay_ms(100);
    for (uint32_t i = 0; i < 10; i++) {
        for (uint8_t i = 0; i < sizeof(gpio_map); i++) {
            snprintf(message, 128, "Expected GPIO %d LOW But HIGH", gpio_map[i]);
            TEST_ASSERT_FALSE_MESSAGE(bflb_gpio_read(gpio, gpio_map[i]), message);
        }
        bflb_mtimer_delay_ms(10);
    }
    for (uint32_t i = 0; i < 10; i++) {
        uint64_t data;
        data = (uint64_t)bflb_gpio_pin32_63_read(gpio) << 32;
        data |= bflb_gpio_pin0_31_read(gpio);

        for (uint8_t i = 0; i < sizeof(gpio_map); i++) {
            snprintf(message, 128, "Expected PortRead GPIO %d LOW But HIGH", gpio_map[i]);
            TEST_ASSERT_FALSE_MESSAGE(!!(data & (1ULL << gpio_map[i])), message);
        }
        bflb_mtimer_delay_ms(10);
    }
    for (uint8_t i = 0; i < sizeof(gpio_map); i++) {
        bflb_gpio_deinit(gpio, gpio_map[i]);
    }
}

TEST_GROUP(gpio_input_low);

TEST_SETUP(gpio_input_low)
{
}

TEST_TEAR_DOWN(gpio_input_low)
{
}

TEST(gpio_input_low, pu_smten_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
}

TEST(gpio_input_low, pu_smten_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

TEST(gpio_input_low, pu_smten_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
}

TEST(gpio_input_low, pu_smten_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_3);
}

TEST(gpio_input_low, pd_smten_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
}

TEST(gpio_input_low, pd_smten_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
}

TEST(gpio_input_low, pd_smten_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_2);
}

TEST(gpio_input_low, pd_smten_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_3);
}

TEST(gpio_input_low, pn_smten_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_0);
}

TEST(gpio_input_low, pn_smten_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_1);
}

TEST(gpio_input_low, pn_smten_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_2);
}

TEST(gpio_input_low, pn_smten_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_EN | GPIO_DRV_3);
}

TEST(gpio_input_low, pu_smtno_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_DIS | GPIO_DRV_0);
}

TEST(gpio_input_low, pu_smtno_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_DIS | GPIO_DRV_1);
}

TEST(gpio_input_low, pu_smtno_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_DIS | GPIO_DRV_2);
}

TEST(gpio_input_low, pu_smtno_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_DIS | GPIO_DRV_3);
}

TEST(gpio_input_low, pd_smtno_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_0);
}

TEST(gpio_input_low, pd_smtno_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_1);
}

TEST(gpio_input_low, pd_smtno_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_2);
}

TEST(gpio_input_low, pd_smtno_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_3);
}

TEST(gpio_input_low, pn_smtno_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_DIS | GPIO_DRV_0);
}

TEST(gpio_input_low, pn_smtno_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_DIS | GPIO_DRV_1);
}

TEST(gpio_input_low, pn_smtno_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_DIS | GPIO_DRV_2);
}

TEST(gpio_input_low, pn_smtno_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_FLOAT | GPIO_SMT_DIS | GPIO_DRV_3);
}

TEST_GROUP(gpio_input_low_bypull);

TEST_SETUP(gpio_input_low_bypull)
{
}

TEST_TEAR_DOWN(gpio_input_low_bypull)
{
}

TEST(gpio_input_low_bypull, pd_smten_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_0);
}

TEST(gpio_input_low_bypull, pd_smten_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_1);
}

TEST(gpio_input_low_bypull, pd_smten_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_2);
}

TEST(gpio_input_low_bypull, pd_smten_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_EN | GPIO_DRV_3);
}

TEST(gpio_input_low_bypull, pd_smtno_drv0)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_0);
}

TEST(gpio_input_low_bypull, pd_smtno_drv1)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_1);
}

TEST(gpio_input_low_bypull, pd_smtno_drv2)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_2);
}

TEST(gpio_input_low_bypull, pd_smtno_drv3)
{
    gpio_input_low_test(GPIO_INPUT | GPIO_PULLDOWN | GPIO_SMT_DIS | GPIO_DRV_3);
}
