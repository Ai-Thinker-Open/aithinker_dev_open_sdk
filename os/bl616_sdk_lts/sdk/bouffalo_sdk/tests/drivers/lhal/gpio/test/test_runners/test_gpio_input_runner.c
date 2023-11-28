#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(gpio_input_low)
{
    RUN_TEST_CASE(gpio_input_low, pu_smten_drv0);
    RUN_TEST_CASE(gpio_input_low, pu_smten_drv1);
    RUN_TEST_CASE(gpio_input_low, pu_smten_drv2);
    RUN_TEST_CASE(gpio_input_low, pu_smten_drv3);

    RUN_TEST_CASE(gpio_input_low, pd_smten_drv0);
    RUN_TEST_CASE(gpio_input_low, pd_smten_drv1);
    RUN_TEST_CASE(gpio_input_low, pd_smten_drv2);
    RUN_TEST_CASE(gpio_input_low, pd_smten_drv3);

    RUN_TEST_CASE(gpio_input_low, pn_smten_drv0);
    RUN_TEST_CASE(gpio_input_low, pn_smten_drv1);
    RUN_TEST_CASE(gpio_input_low, pn_smten_drv2);
    RUN_TEST_CASE(gpio_input_low, pn_smten_drv3);

    RUN_TEST_CASE(gpio_input_low, pu_smtno_drv0);
    RUN_TEST_CASE(gpio_input_low, pu_smtno_drv1);
    RUN_TEST_CASE(gpio_input_low, pu_smtno_drv2);
    RUN_TEST_CASE(gpio_input_low, pu_smtno_drv3);

    RUN_TEST_CASE(gpio_input_low, pd_smtno_drv0);
    RUN_TEST_CASE(gpio_input_low, pd_smtno_drv1);
    RUN_TEST_CASE(gpio_input_low, pd_smtno_drv2);
    RUN_TEST_CASE(gpio_input_low, pd_smtno_drv3);

    RUN_TEST_CASE(gpio_input_low, pn_smtno_drv0);
    RUN_TEST_CASE(gpio_input_low, pn_smtno_drv1);
    RUN_TEST_CASE(gpio_input_low, pn_smtno_drv2);
    RUN_TEST_CASE(gpio_input_low, pn_smtno_drv3);
}

TEST_GROUP_RUNNER(gpio_input_high)
{
    RUN_TEST_CASE(gpio_input_high, pu_smten_drv0);
    RUN_TEST_CASE(gpio_input_high, pu_smten_drv1);
    RUN_TEST_CASE(gpio_input_high, pu_smten_drv2);
    RUN_TEST_CASE(gpio_input_high, pu_smten_drv3);

    RUN_TEST_CASE(gpio_input_high, pd_smten_drv0);
    RUN_TEST_CASE(gpio_input_high, pd_smten_drv1);
    RUN_TEST_CASE(gpio_input_high, pd_smten_drv2);
    RUN_TEST_CASE(gpio_input_high, pd_smten_drv3);

    RUN_TEST_CASE(gpio_input_high, pn_smten_drv0);
    RUN_TEST_CASE(gpio_input_high, pn_smten_drv1);
    RUN_TEST_CASE(gpio_input_high, pn_smten_drv2);
    RUN_TEST_CASE(gpio_input_high, pn_smten_drv3);

    RUN_TEST_CASE(gpio_input_high, pu_smtno_drv0);
    RUN_TEST_CASE(gpio_input_high, pu_smtno_drv1);
    RUN_TEST_CASE(gpio_input_high, pu_smtno_drv2);
    RUN_TEST_CASE(gpio_input_high, pu_smtno_drv3);

    RUN_TEST_CASE(gpio_input_high, pd_smtno_drv0);
    RUN_TEST_CASE(gpio_input_high, pd_smtno_drv1);
    RUN_TEST_CASE(gpio_input_high, pd_smtno_drv2);
    RUN_TEST_CASE(gpio_input_high, pd_smtno_drv3);

    RUN_TEST_CASE(gpio_input_high, pn_smtno_drv0);
    RUN_TEST_CASE(gpio_input_high, pn_smtno_drv1);
    RUN_TEST_CASE(gpio_input_high, pn_smtno_drv2);
    RUN_TEST_CASE(gpio_input_high, pn_smtno_drv3);
}

TEST_GROUP_RUNNER(gpio_input_low_bypull)
{
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smten_drv0);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smten_drv1);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smten_drv2);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smten_drv3);

    RUN_TEST_CASE(gpio_input_low_bypull, pd_smtno_drv0);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smtno_drv1);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smtno_drv2);
    RUN_TEST_CASE(gpio_input_low_bypull, pd_smtno_drv3);
}

TEST_GROUP_RUNNER(gpio_input_high_bypull)
{
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smten_drv0);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smten_drv1);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smten_drv2);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smten_drv3);

    RUN_TEST_CASE(gpio_input_high_bypull, pu_smtno_drv0);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smtno_drv1);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smtno_drv2);
    RUN_TEST_CASE(gpio_input_high_bypull, pu_smtno_drv3);
}