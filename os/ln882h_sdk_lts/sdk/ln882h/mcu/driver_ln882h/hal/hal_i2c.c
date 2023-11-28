/**
 * @file     hal_i2c.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


/* Includes ------------------------------------------------------------------*/
#include "hal/hal_i2c.h"
            
void hal_i2c_init(uint32_t i2c_x_base,i2c_init_t_def* i2c_init)
{   
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    hal_assert(IS_I2C_SW_RST(i2c_init->i2c_sw_rst));
    hal_assert(IS_I2C_SMBUS_ALERT_MODEL(i2c_init->i2c_smbus_alert_model));
    hal_assert(IS_I2C_PEC(i2c_init->i2c_pec));
    hal_assert(IS_I2C_POC(i2c_init->i2c_poc));
    hal_assert(IS_I2C_ACK_EN(i2c_init->i2c_ack_en));
    hal_assert(IS_I2C_CLOCK_STRETCH_EN(i2c_init->i2c_clock_stretch_dis));
    hal_assert(IS_I2C_GENERAL_CALL_EN(i2c_init->i2c_general_call_en));
    hal_assert(IS_I2C_PEC_EN(i2c_init->i2c_pec_en));
    hal_assert(IS_I2C_ARP_EN(i2c_init->i2c_arp_en));
    hal_assert(IS_I2C_SMBUS_TYPE(i2c_init->i2c_smbus_type));
    hal_assert(IS_I2C_MODE(i2c_init->i2c_mode));
    hal_assert(IS_I2C_ADD_MODE(i2c_init->i2c_add_mode));
    hal_assert(IS_I2C_MASTER_MODE_SEL(i2c_init->i2c_master_mode_sel));
    hal_assert(IS_I2C_FM_MODE_DUTY_CYCLE(i2c_init->i2c_fm_mode_duty_cycle));
    hal_assert(IS_I2C_CCR(i2c_init->i2c_ccr));
    hal_assert(IS_I2C_TRISE(i2c_init->i2c_trise));

    sysc_cmp_i2c0_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();
   
    /* set the  peripheral clock freq*/
    i2c_freq_setf(i2c_x_base,i2c_init->i2c_peripheral_clock_freq);

    /* set the i2c mode and scl clock speed */
    if (i2c_init->i2c_master_mode_sel == I2C_SM_MODE) {
        i2c_fs_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_master_mode_sel == I2C_FM_MODE) {
        i2c_fs_setf(i2c_x_base,1);
    }
    i2c_duty_setf(i2c_x_base,i2c_init->i2c_fm_mode_duty_cycle);
    i2c_ccr_setf(i2c_x_base,i2c_init->i2c_ccr);
    i2c_trise_setf(i2c_x_base,i2c_init->i2c_trise);

    /* set the i2c CR1 */
    if (i2c_init->i2c_sw_rst == I2C_SW_NO_RST) {
        i2c_swrst_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_sw_rst == I2C_SW_RST) {
        i2c_swrst_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_smbus_alert_model == I2C_SM_ALERT_MODEL_1) {
        i2c_alert_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_smbus_alert_model == I2C_SM_ALERT_MODEL_2) {
        i2c_alert_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_pec == I2C_PEC_CLOSE) {
        i2c_pec_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_pec == I2C_PEC_OPEN) {
        i2c_pec_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_poc == I2C_POC_MODEL_1) {
        i2c_pos_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_poc == I2C_POC_MODEL_2) {
        i2c_pos_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_ack_en == I2C_ACK_DIS) {
        i2c_ack_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_ack_en == I2C_ACK_EN) {
        i2c_ack_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_clock_stretch_dis == I2C_CLOCK_STRETCH_EN) {
        i2c_nostretch_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_clock_stretch_dis == I2C_CLOCK_STRETCH_DIS) {
        i2c_nostretch_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_general_call_en == I2C_GENERAL_CALL_DIS) {
        i2c_engc_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_general_call_en == I2C_GENERAL_CALL_EN) {
        i2c_engc_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_pec_en == I2C_PEC_DIS) {
        i2c_enpec_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_pec_en == I2C_PEC_EN) {
        i2c_enpec_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_arp_en == I2C_ARP_DIS) {
        i2c_enarp_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_arp_en == I2C_ARP_EN) {
        i2c_enarp_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_smbus_type == I2C_SMBUS_DEVICE) {
        i2c_smbtype_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_smbus_type == I2C_SMBUS_HOST) {
        i2c_smbtype_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_mode == I2C_I2C_MODE) {
        i2c_smbus_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_mode == I2C_SMBUS_MODE) {
        i2c_smbus_setf(i2c_x_base,1);
    }

    if (i2c_init->i2c_add_mode == I2C_ADD_7BIT_MODE) {
        i2c_addmode_setf(i2c_x_base,0);
    }
    else if (i2c_init->i2c_add_mode == I2C_ADD_10BIT_MODE) {
        i2c_addmode_setf(i2c_x_base,1);
    }

}

void hal_i2c_deinit(void)
{
    sysc_cmp_srstn_i2c0_setf(0);
    sysc_cmp_srstn_i2c0_setf(1);

    sysc_cmp_i2c0_gate_en_setf(0);
}

void hal_i2c_set_peripheral_clock_freq(uint32_t i2c_x_base, uint32_t peripheral_clock_freq)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    i2c_freq_setf(i2c_x_base,peripheral_clock_freq);
}


void hal_i2c_en(uint32_t i2c_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        i2c_pe_setf(i2c_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        i2c_pe_setf(i2c_x_base,1);
    }
}

void hal_i2c_dma_en(uint32_t i2c_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        i2c_dmaen_setf(i2c_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        i2c_dmaen_setf(i2c_x_base,1);
    }
}

void hal_i2c_ack_en(uint32_t i2c_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        i2c_ack_setf(i2c_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        i2c_ack_setf(i2c_x_base,1);
    }
}


/***********************************I2C master opetation*************************************/

void hal_i2c_master_reset(uint32_t i2c_x_base)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    i2c_swrst_setf(i2c_x_base, 1);
    i2c_swrst_setf(i2c_x_base, 0);
}

uint8_t hal_i2c_master_start(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    i2c_start_setf(i2c_x_base, 1);
    while (0 == i2c_sb_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

void hal_i2c_master_stop(uint32_t i2c_x_base)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    i2c_stop_setf(i2c_x_base, 1);
}

void hal_i2c_master_send_data(uint32_t i2c_x_base,uint8_t data)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    i2c_dr_set(i2c_x_base, data);
}

uint8_t hal_i2c_master_recv_data(uint32_t i2c_x_base)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    return i2c_dr_get(i2c_x_base);
}


uint8_t hal_i2c_master_wait_addr(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_addr_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}             

uint8_t hal_i2c_master_wait_add10(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_add10_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

/***********************************I2C slave opetation*************************************/

void hal_i2c_slave_set_add_mode(uint32_t i2c_x_base,i2c_add_mode_t add_mode)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_ADD_MODE(add_mode));
    i2c_addmode_setf(i2c_x_base,add_mode);
}

void hal_i2c_slave_set_add1(uint32_t i2c_x_base,uint16_t add)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    if (i2c_addmode_getf(i2c_x_base) == I2C_ADD_7BIT_MODE) {
        hal_assert(IS_I2C_7_bit_ADD(add));
    }
    else if (i2c_addmode_getf(i2c_x_base) == I2C_ADD_10BIT_MODE) {
        hal_assert(IS_I2C_10_bit_ADD(add));
    }
    i2c_add_setf(i2c_x_base,add);
}

void hal_i2c_slave_set_add2(uint32_t i2c_x_base,uint16_t add)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    if (i2c_addmode_getf(i2c_x_base) == I2C_ADD_7BIT_MODE) {
        hal_assert(IS_I2C_7_bit_ADD(add));
    }
    else if (i2c_addmode_getf(i2c_x_base) == I2C_ADD_10BIT_MODE) {
        hal_assert(IS_I2C_10_bit_ADD(add));
    }
    i2c_add2_setf(i2c_x_base,add);
}

/**
 * @brief Wait for address matched
 */
uint8_t hal_i2c_slave_wait_addr(uint32_t i2c_x_base,uint32_t timeout)                        
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_addr_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

/***********************************I2C universal opetation*************************************/

uint8_t hal_i2c_wait_txe(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_txe_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

uint8_t hal_i2c_wait_rxne(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_rxne_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

uint8_t hal_i2c_wait_btf(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (0 == i2c_btf_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

uint8_t hal_i2c_wait_bus_idle(uint32_t i2c_x_base,uint32_t timeout)
{
    uint32_t count = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));

    while (1 == i2c_busy_getf(i2c_x_base)) 
    {
        count ++;
        if(count > timeout)
            return HAL_RESET;
    }   
    return HAL_SET;
}

/**
 * @brief  Clear SR1 and SR2 after sending address 
 */
void hal_i2c_clear_sr(uint32_t i2c_x_base)                                               
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    i2c_sr1_set(i2c_x_base,0);
    i2c_sr1_get(i2c_x_base);
    i2c_sr2_get(i2c_x_base);
}

/***********************************I2C interrupt opetation*************************************/

void hal_i2c_it_cfg(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_IT_FLAG(i2c_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    switch (i2c_it_flag)
    {
        case I2C_IT_FLAG_SB:
        case I2C_IT_FLAG_ADDR:
        case I2C_IT_FLAG_ADD10:
        case I2C_IT_FLAG_STOPF:
        case I2C_IT_FLAG_BTF:
            if (en == HAL_ENABLE){
                i2c_itevten_setf(i2c_x_base,1);
            }
            else if (en == HAL_DISABLE) {
                i2c_itevten_setf(i2c_x_base,0);
            }
            break;

        case I2C_IT_FLAG_BERR:
        case I2C_IT_FLAG_ARLO:
        case I2C_IT_FLAG_AF:
        case I2C_IT_FLAG_OVR:
        case I2C_IT_FLAG_PECERR:
        case I2C_IT_FLAG_TIMEOUT:
        case I2C_IT_FLAG_SMBALERT:
            if (en == HAL_ENABLE){
                i2c_iterren_setf(i2c_x_base,1);
            }
            else if (en == HAL_DISABLE) {
                i2c_iterren_setf(i2c_x_base,0);
            }
            break;

        case I2C_IT_FLAG_TXE:
        case I2C_IT_FLAG_RXNE:
            if (en == HAL_ENABLE){
                i2c_itevten_setf(i2c_x_base,1);
                i2c_itbufen_setf(i2c_x_base,1);      //TxE = 1 or RxNE = 1 generates Event Interrupt (whatever the state of DMAEN)
            }
            else if (en == HAL_DISABLE) {
                i2c_itevten_setf(i2c_x_base,0);
                i2c_itbufen_setf(i2c_x_base,0);
            }
            break;

        default:
            break;
    }
}

uint8_t hal_i2c_get_it_flag(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag)
{
    uint32_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_IT_FLAG(i2c_it_flag));
    switch (i2c_it_flag)
    {
        case I2C_IT_FLAG_SB:
            it_flag = i2c_sb_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_ADDR:
            it_flag = i2c_addr_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_BTF:
            it_flag = i2c_btf_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_ADD10:
            it_flag = i2c_add10_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_STOPF:
            it_flag = i2c_stopf_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_RXNE:
            it_flag = i2c_rxne_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_TXE:
            it_flag = i2c_txe_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_BERR:
            it_flag = i2c_berr_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_ARLO:
            it_flag = i2c_arlo_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_AF:
            it_flag = i2c_af_getf(i2c_x_base);
            break;
            
        case I2C_IT_FLAG_OVR:
            it_flag = i2c_ovr_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_PECERR:
            it_flag = i2c_pecerr_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_TIMEOUT:
            it_flag = i2c_timeout_getf(i2c_x_base);
            break;

        case I2C_IT_FLAG_SMBALERT:
            it_flag = i2c_smbalert_getf(i2c_x_base);
            break;
        
        default:
            break;
    }
    return it_flag;
}

void hal_i2c_clr_it_flag(uint32_t i2c_x_base,i2c_it_flag_t i2c_it_flag)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_IT_FLAG(i2c_it_flag));
    switch (i2c_it_flag)
    {
        case I2C_IT_FLAG_STOPF:
            i2c_stopf_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_BERR:
            i2c_berr_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_ARLO:
            i2c_arlo_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_AF:
            i2c_af_setf(i2c_x_base,0);
            break;
            
        case I2C_IT_FLAG_OVR:
            i2c_ovr_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_PECERR:
            i2c_pecerr_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_TIMEOUT:
            i2c_timeout_setf(i2c_x_base,0);
            break;

        case I2C_IT_FLAG_SMBALERT:
            i2c_smbalert_setf(i2c_x_base,0);
            break;
        
        default:
            break;
    }
}

uint8_t hal_i2c_get_status_flag(uint32_t i2c_x_base,i2c_status_flag_t i2c_status_flag)
{
    uint32_t status_flag = 0;
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_STATUS_FLAG(i2c_status_flag));
    switch (i2c_status_flag)
    {
        case I2C_STATUS_FLAG_SB:
            status_flag = i2c_sb_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_ADDR:
            status_flag = i2c_addr_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_BTF:
            status_flag = i2c_btf_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_ADD10:
            status_flag = i2c_add10_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_STOPF:
            status_flag = i2c_stopf_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_RXNE:
            status_flag = i2c_rxne_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_TXE:
            status_flag = i2c_txe_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_BERR:
            status_flag = i2c_berr_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_ARLO:
            status_flag = i2c_arlo_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_AF:
            status_flag = i2c_af_getf(i2c_x_base);
            break;
            
        case I2C_STATUS_FLAG_OVR:
            status_flag = i2c_ovr_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_PECERR:
            status_flag = i2c_pecerr_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_TIMEOUT:
            status_flag = i2c_timeout_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_SMBALERT:
            status_flag = i2c_smbalert_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_MSL:
            status_flag = i2c_msl_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_BUSY:
            status_flag = i2c_busy_getf(i2c_x_base);
            break;    

        case I2C_STATUS_FLAG_TRA:
            status_flag = i2c_tra_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_GENCALL:
            status_flag = i2c_gencall_getf(i2c_x_base);
            break;
            
        case I2C_STATUS_FLAG_SMBDEFAULT:
            status_flag = i2c_smbdefault_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_SMBHOST:
            status_flag = i2c_smbhost_getf(i2c_x_base);
            break;
            
        case I2C_STATUS_FLAG_DUALF:
            status_flag = i2c_dualf_getf(i2c_x_base);
            break;

        case I2C_STATUS_FLAG_PEC:
            status_flag = i2c_pec1_getf(i2c_x_base);
            break;  
        
        default:
            break;
    }
    return status_flag;
}

void hal_i2c_clr_status_flag(uint32_t i2c_x_base,i2c_status_flag_t i2c_status_flag)
{
    /* check the parameters */
    hal_assert(IS_I2C_ALL_PERIPH(i2c_x_base));
    hal_assert(IS_I2C_STATUS_FLAG(i2c_status_flag));
    switch (i2c_status_flag)
    {
        case I2C_STATUS_FLAG_STOPF:
            i2c_stopf_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_BERR:
            i2c_berr_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_ARLO:
            i2c_arlo_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_AF:
            i2c_af_setf(i2c_x_base,0);
            break;
            
        case I2C_STATUS_FLAG_OVR:
            i2c_ovr_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_PECERR:
            i2c_pecerr_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_TIMEOUT:
            i2c_timeout_setf(i2c_x_base,0);
            break;

        case I2C_STATUS_FLAG_SMBALERT:
            i2c_smbalert_setf(i2c_x_base,0);
            break;
        
        default:
            break;
    }
}

/************************END OF FILE*******************************/

