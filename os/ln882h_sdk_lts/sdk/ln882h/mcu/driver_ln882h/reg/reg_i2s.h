#ifndef __REG_I2S_H__
#define __REG_I2S_H__

#include "ln882h.h"

//ier
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                            ier :  1; /* 0: 0,     DW_apb_i2s Enable Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_IER;

//irer
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                           irer :  1; /* 0: 0, I2S Receiver Block Enable Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_IRER;

//iter
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                           iter :  1; /* 0: 0, I2S Transmitter Block Enable Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_ITER;

//rxffr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                          rxffr :  1; /* 0: 0,   Receiver Block FIFO Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_RXFFR;

//txffr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                          txffr :  1; /* 0: 0, Transmitter Block FIFO Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_TXFFR;

//ch0_lrbr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch0_lrbr : 32; /*31: 0,          Left Receive Buffer 0*/
    } bit_field;
} T_I2S_CH0_LRBR;

//ch0_rrbr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch0_rrbr : 32; /*31: 0,         Right Receive Buffer 0*/
    } bit_field;
} T_I2S_CH0_RRBR;

//ch0_rer
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_rer :  1; /* 0: 0,      Receive Enable Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_RER;

//ch0_ter
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_ter :  1; /* 0: 0,     Transmit Enable Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_TER;

//ch0_rcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_rcr :  3; /* 2: 0, Receive Configuration Register 0*/
        uint32_t                     reserved_0 : 29; /*31: 3,                             NA*/
    } bit_field;
} T_I2S_CH0_RCR;

//ch0_tcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_tcr :  3; /* 2: 0, Transmit Configuration Register 0*/
        uint32_t                     reserved_0 : 29; /*31: 3,                             NA*/
    } bit_field;
} T_I2S_CH0_TCR;

//ch0_isr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_isr :  6; /* 5: 0,    Interrupt Status Register 0*/
        uint32_t                     reserved_0 : 26; /*31: 6,                             NA*/
    } bit_field;
} T_I2S_CH0_ISR;

//ch0_imr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_imr :  6; /* 5: 0,      Interrupt Mask Register 0*/
        uint32_t                     reserved_0 : 26; /*31: 6,                             NA*/
    } bit_field;
} T_I2S_CH0_IMR;

//ch0_ror
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_ror :  1; /* 0: 0,     Receive Overrun Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_ROR;

//ch0_tor
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_tor :  1; /* 0: 0,    Transmit Overrun Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_TOR;

//ch0_rfcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch0_rfcr :  4; /* 3: 0, Receive FIFO Configuration Register 0*/
        uint32_t                     reserved_0 : 28; /*31: 4,                             NA*/
    } bit_field;
} T_I2S_CH0_RFCR;

//ch0_tfcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch0_tfcr :  4; /* 3: 0, Transmit FIFO Configuration Register 0*/
        uint32_t                     reserved_0 : 28; /*31: 4,                             NA*/
    } bit_field;
} T_I2S_CH0_TFCR;

//ch0_rff
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_rff :  1; /* 0: 0,           Receive FIFO Flush 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_RFF;

//ch0_tff
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch0_tff :  1; /* 0: 0,          Transmit FIFO Flush 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH0_TFF;

//ch1_lrbr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch1_lrbr : 32; /*31: 0,          Left Receive Buffer 0*/
    } bit_field;
} T_I2S_CH1_LRBR;

//ch1_rrbr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch1_rrbr : 32; /*31: 0,         Right Receive Buffer 0*/
    } bit_field;
} T_I2S_CH1_RRBR;

//ch1_rer
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_rer :  1; /* 0: 0,      Receive Enable Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_RER;

//ch1_ter
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_ter :  1; /* 0: 0,     Transmit Enable Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_TER;

//ch1_rcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_rcr :  3; /* 2: 0, Receive Configuration Register 0*/
        uint32_t                     reserved_0 : 29; /*31: 3,                             NA*/
    } bit_field;
} T_I2S_CH1_RCR;

//ch1_tcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_tcr :  3; /* 2: 0, Transmit Configuration Register 0*/
        uint32_t                     reserved_0 : 29; /*31: 3,                             NA*/
    } bit_field;
} T_I2S_CH1_TCR;

//ch1_isr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_isr :  6; /* 5: 0,    Interrupt Status Register 0*/
        uint32_t                     reserved_0 : 26; /*31: 6,                             NA*/
    } bit_field;
} T_I2S_CH1_ISR;

//ch1_imr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_imr :  6; /* 5: 0,      Interrupt Mask Register 0*/
        uint32_t                     reserved_0 : 26; /*31: 6,                             NA*/
    } bit_field;
} T_I2S_CH1_IMR;

//ch1_ror
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_ror :  1; /* 0: 0,     Receive Overrun Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_ROR;

//ch1_tor
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_tor :  1; /* 0: 0,    Transmit Overrun Register 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_TOR;

//ch1_rfcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch1_rfcr :  4; /* 3: 0, Receive FIFO Configuration Register 0*/
        uint32_t                     reserved_0 : 28; /*31: 4,                             NA*/
    } bit_field;
} T_I2S_CH1_RFCR;

//ch1_tfcr
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       ch1_tfcr :  4; /* 3: 0, Transmit FIFO Configuration Register 0*/
        uint32_t                     reserved_0 : 28; /*31: 4,                             NA*/
    } bit_field;
} T_I2S_CH1_TFCR;

//ch1_rff
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_rff :  1; /* 0: 0,           Receive FIFO Flush 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_RFF;

//ch1_tff
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        ch1_tff :  1; /* 0: 0,          Transmit FIFO Flush 0*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_CH1_TFF;

//rxdma
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                          rxdma : 32; /*31: 0,    Receiver Block DMA Register*/
    } bit_field;
} T_I2S_RXDMA;

//rrxdma
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                         rrxdma :  1; /* 0: 0, Reset Receiver Block DMA Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_RRXDMA;

//txdma
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                          txdma : 32; /*31: 0, Transmitter Block DMA Register*/
    } bit_field;
} T_I2S_TXDMA;

//rtxdma
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                         rtxdma :  1; /* 0: 0, Reset Transmitter Block DMA Register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_I2S_RTXDMA;

//i2s_comp_param_2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t               i2s_comp_param_2 : 32; /*31: 0, Component Parameter 2 Register*/
    } bit_field;
} T_I2S_I2S_COMP_PARAM_2;

//i2s_comp_param_1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t               i2s_comp_param_1 : 32; /*31: 0, Component Parameter 1 Register*/
    } bit_field;
} T_I2S_I2S_COMP_PARAM_1;

//i2s_comp_version
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t               i2s_comp_version : 32; /*31: 0,           Component Version ID*/
    } bit_field;
} T_I2S_I2S_COMP_VERSION;

//i2s_comp_type
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  i2s_comp_type : 32; /*31: 0,      DesignWare Component Type*/
    } bit_field;
} T_I2S_I2S_COMP_TYPE;

//Registers Mapping to the same address

typedef struct
{
    volatile                       T_I2S_IER                            ier; /*  0x0,    RW, 0x00000000,                             NA*/
    volatile                      T_I2S_IRER                           irer; /*  0x4,    RW, 0x00000000,                             NA*/
    volatile                      T_I2S_ITER                           iter; /*  0x8,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                  reserved_0[2];
    volatile                     T_I2S_RXFFR                          rxffr; /* 0x14,    RW, 0x00000000,                             NA*/
    volatile                     T_I2S_TXFFR                          txffr; /* 0x18,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                     reserved_1;
    volatile                  T_I2S_CH0_LRBR                       ch0_lrbr; /* 0x20,    RW, 0x00000000,                             NA*/
    volatile                  T_I2S_CH0_RRBR                       ch0_rrbr; /* 0x24,    RW, 0x00000000,                             NA*/
    volatile                   T_I2S_CH0_RER                        ch0_rer; /* 0x28,    RW, 0x00000001,                             NA*/
    volatile                   T_I2S_CH0_TER                        ch0_ter; /* 0x2c,    RW, 0x00000001,                             NA*/
    volatile                   T_I2S_CH0_RCR                        ch0_rcr; /* 0x30,    RW, 0x00000005,                             NA*/
    volatile                   T_I2S_CH0_TCR                        ch0_tcr; /* 0x34,    RW, 0x00000005,                             NA*/
    volatile                   T_I2S_CH0_ISR                        ch0_isr; /* 0x38,    RO, 0x00000010,                             NA*/
    volatile                   T_I2S_CH0_IMR                        ch0_imr; /* 0x3c,    RW, 0x00000033,                             NA*/
    volatile                   T_I2S_CH0_ROR                        ch0_ror; /* 0x40,    RO, 0x00000000,                             NA*/
    volatile                   T_I2S_CH0_TOR                        ch0_tor; /* 0x44,    RO, 0x00000000,                             NA*/
    volatile                  T_I2S_CH0_RFCR                       ch0_rfcr; /* 0x48,    RW, 0x00000003,                             NA*/
    volatile                  T_I2S_CH0_TFCR                       ch0_tfcr; /* 0x4c,    RW, 0x00000003,                             NA*/
    volatile                   T_I2S_CH0_RFF                        ch0_rff; /* 0x50,    RW, 0x00000000,                             NA*/
    volatile                   T_I2S_CH0_TFF                        ch0_tff; /* 0x54,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                  reserved_2[2];
    volatile                  T_I2S_CH1_LRBR                       ch1_lrbr; /* 0x60,    RW, 0x00000000,                             NA*/
    volatile                  T_I2S_CH1_RRBR                       ch1_rrbr; /* 0x64,    RW, 0x00000000,                             NA*/
    volatile                   T_I2S_CH1_RER                        ch1_rer; /* 0x68,    RW, 0x00000001,                             NA*/
    volatile                   T_I2S_CH1_TER                        ch1_ter; /* 0x6c,    RW, 0x00000001,                             NA*/
    volatile                   T_I2S_CH1_RCR                        ch1_rcr; /* 0x70,    RW, 0x00000005,                             NA*/
    volatile                   T_I2S_CH1_TCR                        ch1_tcr; /* 0x74,    RW, 0x00000005,                             NA*/
    volatile                   T_I2S_CH1_ISR                        ch1_isr; /* 0x78,    RO, 0x00000010,                             NA*/
    volatile                   T_I2S_CH1_IMR                        ch1_imr; /* 0x7c,    RW, 0x00000033,                             NA*/
    volatile                   T_I2S_CH1_ROR                        ch1_ror; /* 0x80,    RO, 0x00000000,                             NA*/
    volatile                   T_I2S_CH1_TOR                        ch1_tor; /* 0x84,    RO, 0x00000000,                             NA*/
    volatile                  T_I2S_CH1_RFCR                       ch1_rfcr; /* 0x88,    RW, 0x00000003,                             NA*/
    volatile                  T_I2S_CH1_TFCR                       ch1_tfcr; /* 0x8c,    RW, 0x00000003,                             NA*/
    volatile                   T_I2S_CH1_RFF                        ch1_rff; /* 0x90,    RW, 0x00000000,                             NA*/
    volatile                   T_I2S_CH1_TFF                        ch1_tff; /* 0x94,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                 reserved_3[74];
    volatile                     T_I2S_RXDMA                          rxdma; /*0x1c0,    RO, 0x00000000,                             NA*/
    volatile                    T_I2S_RRXDMA                         rrxdma; /*0x1c4,    RW, 0x00000000,                             NA*/
    volatile                     T_I2S_TXDMA                          txdma; /*0x1c8,    WO, 0x00000000,                             NA*/
    volatile                    T_I2S_RTXDMA                         rtxdma; /*0x1cc,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                  reserved_4[8];
    volatile          T_I2S_I2S_COMP_PARAM_2               i2s_comp_param_2; /*0x1f0,    RO, 0x000004A4,                             NA*/
    volatile          T_I2S_I2S_COMP_PARAM_1               i2s_comp_param_1; /*0x1f4,    RO, 0x026402EA,                             NA*/
    volatile          T_I2S_I2S_COMP_VERSION               i2s_comp_version; /*0x1f8,    RO, 0x3130392A,                             NA*/
    volatile             T_I2S_I2S_COMP_TYPE                  i2s_comp_type; /*0x1fc,    RO, 0x445701A0,                             NA*/
} T_HWP_I2S_T;

#define hwp_i2s ((T_HWP_I2S_T*)I2S_BASE)


__STATIC_INLINE uint32_t i2s_ier_get(void)
{
    return hwp_i2s->ier.val;
}

__STATIC_INLINE void i2s_ier_set(uint32_t value)
{
    hwp_i2s->ier.val = value;
}

__STATIC_INLINE void i2s_ier_pack(uint8_t ier)
{
    hwp_i2s->ier.val = (((uint32_t)ier << 0));
}

__STATIC_INLINE void i2s_ier_unpack(uint8_t* ier)
{
    T_I2S_IER localVal = hwp_i2s->ier;

    *ier = localVal.bit_field.ier;
}

__STATIC_INLINE uint8_t i2s_ier_getf(void)
{
    return hwp_i2s->ier.bit_field.ier;
}

__STATIC_INLINE void i2s_ier_setf(uint8_t ier)
{
    hwp_i2s->ier.bit_field.ier = ier;
}

__STATIC_INLINE uint32_t i2s_irer_get(void)
{
    return hwp_i2s->irer.val;
}

__STATIC_INLINE void i2s_irer_set(uint32_t value)
{
    hwp_i2s->irer.val = value;
}

__STATIC_INLINE void i2s_irer_pack(uint8_t irer)
{
    hwp_i2s->irer.val = (((uint32_t)irer << 0));
}

__STATIC_INLINE void i2s_irer_unpack(uint8_t* irer)
{
    T_I2S_IRER localVal = hwp_i2s->irer;

    *irer = localVal.bit_field.irer;
}

__STATIC_INLINE uint8_t i2s_irer_getf(void)
{
    return hwp_i2s->irer.bit_field.irer;
}

__STATIC_INLINE void i2s_irer_setf(uint8_t irer)
{
    hwp_i2s->irer.bit_field.irer = irer;
}

__STATIC_INLINE uint32_t i2s_iter_get(void)
{
    return hwp_i2s->iter.val;
}

__STATIC_INLINE void i2s_iter_set(uint32_t value)
{
    hwp_i2s->iter.val = value;
}

__STATIC_INLINE void i2s_iter_pack(uint8_t iter)
{
    hwp_i2s->iter.val = (((uint32_t)iter << 0));
}

__STATIC_INLINE void i2s_iter_unpack(uint8_t* iter)
{
    T_I2S_ITER localVal = hwp_i2s->iter;

    *iter = localVal.bit_field.iter;
}

__STATIC_INLINE uint8_t i2s_iter_getf(void)
{
    return hwp_i2s->iter.bit_field.iter;
}

__STATIC_INLINE void i2s_iter_setf(uint8_t iter)
{
    hwp_i2s->iter.bit_field.iter = iter;
}

__STATIC_INLINE uint32_t i2s_rxffr_get(void)
{
    return hwp_i2s->rxffr.val;
}

__STATIC_INLINE void i2s_rxffr_set(uint32_t value)
{
    hwp_i2s->rxffr.val = value;
}

__STATIC_INLINE void i2s_rxffr_pack(uint8_t rxffr)
{
    hwp_i2s->rxffr.val = (((uint32_t)rxffr << 0));
}

__STATIC_INLINE uint32_t i2s_txffr_get(void)
{
    return hwp_i2s->txffr.val;
}

__STATIC_INLINE void i2s_txffr_set(uint32_t value)
{
    hwp_i2s->txffr.val = value;
}

__STATIC_INLINE void i2s_txffr_pack(uint8_t txffr)
{
    hwp_i2s->txffr.val = (((uint32_t)txffr << 0));
}

__STATIC_INLINE uint32_t i2s_ch0_lrbr_get(void)
{
    return hwp_i2s->ch0_lrbr.val;
}

__STATIC_INLINE void i2s_ch0_lrbr_set(uint32_t value)
{
    hwp_i2s->ch0_lrbr.val = value;
}

__STATIC_INLINE void i2s_ch0_lrbr_pack(uint32_t ch0_lrbr)
{
    hwp_i2s->ch0_lrbr.val = (((uint32_t)ch0_lrbr << 0));
}

__STATIC_INLINE void i2s_ch0_lrbr_unpack(uint32_t* ch0_lrbr)
{
    T_I2S_CH0_LRBR localVal = hwp_i2s->ch0_lrbr;

    *ch0_lrbr = localVal.bit_field.ch0_lrbr;
}

__STATIC_INLINE uint32_t i2s_ch0_lrbr_getf(void)
{
    return hwp_i2s->ch0_lrbr.bit_field.ch0_lrbr;
}

__STATIC_INLINE void i2s_ch0_lrbr_setf(uint32_t ch0_lrbr)
{
    hwp_i2s->ch0_lrbr.bit_field.ch0_lrbr = ch0_lrbr;
}

__STATIC_INLINE uint32_t i2s_ch0_rrbr_get(void)
{
    return hwp_i2s->ch0_rrbr.val;
}

__STATIC_INLINE void i2s_ch0_rrbr_set(uint32_t value)
{
    hwp_i2s->ch0_rrbr.val = value;
}

__STATIC_INLINE void i2s_ch0_rrbr_pack(uint32_t ch0_rrbr)
{
    hwp_i2s->ch0_rrbr.val = (((uint32_t)ch0_rrbr << 0));
}

__STATIC_INLINE void i2s_ch0_rrbr_unpack(uint32_t* ch0_rrbr)
{
    T_I2S_CH0_RRBR localVal = hwp_i2s->ch0_rrbr;

    *ch0_rrbr = localVal.bit_field.ch0_rrbr;
}

__STATIC_INLINE uint32_t i2s_ch0_rrbr_getf(void)
{
    return hwp_i2s->ch0_rrbr.bit_field.ch0_rrbr;
}

__STATIC_INLINE void i2s_ch0_rrbr_setf(uint32_t ch0_rrbr)
{
    hwp_i2s->ch0_rrbr.bit_field.ch0_rrbr = ch0_rrbr;
}

__STATIC_INLINE uint32_t i2s_ch0_rer_get(void)
{
    return hwp_i2s->ch0_rer.val;
}

__STATIC_INLINE void i2s_ch0_rer_set(uint32_t value)
{
    hwp_i2s->ch0_rer.val = value;
}

__STATIC_INLINE void i2s_ch0_rer_pack(uint8_t ch0_rer)
{
    hwp_i2s->ch0_rer.val = (((uint32_t)ch0_rer << 0));
}

__STATIC_INLINE void i2s_ch0_rer_unpack(uint8_t* ch0_rer)
{
    T_I2S_CH0_RER localVal = hwp_i2s->ch0_rer;

    *ch0_rer = localVal.bit_field.ch0_rer;
}

__STATIC_INLINE uint8_t i2s_ch0_rer_getf(void)
{
    return hwp_i2s->ch0_rer.bit_field.ch0_rer;
}

__STATIC_INLINE void i2s_ch0_rer_setf(uint8_t ch0_rer)
{
    hwp_i2s->ch0_rer.bit_field.ch0_rer = ch0_rer;
}

__STATIC_INLINE uint32_t i2s_ch0_ter_get(void)
{
    return hwp_i2s->ch0_ter.val;
}

__STATIC_INLINE void i2s_ch0_ter_set(uint32_t value)
{
    hwp_i2s->ch0_ter.val = value;
}

__STATIC_INLINE void i2s_ch0_ter_pack(uint8_t ch0_ter)
{
    hwp_i2s->ch0_ter.val = (((uint32_t)ch0_ter << 0));
}

__STATIC_INLINE void i2s_ch0_ter_unpack(uint8_t* ch0_ter)
{
    T_I2S_CH0_TER localVal = hwp_i2s->ch0_ter;

    *ch0_ter = localVal.bit_field.ch0_ter;
}

__STATIC_INLINE uint8_t i2s_ch0_ter_getf(void)
{
    return hwp_i2s->ch0_ter.bit_field.ch0_ter;
}

__STATIC_INLINE void i2s_ch0_ter_setf(uint8_t ch0_ter)
{
    hwp_i2s->ch0_ter.bit_field.ch0_ter = ch0_ter;
}

__STATIC_INLINE uint32_t i2s_ch0_rcr_get(void)
{
    return hwp_i2s->ch0_rcr.val;
}

__STATIC_INLINE void i2s_ch0_rcr_set(uint32_t value)
{
    hwp_i2s->ch0_rcr.val = value;
}

__STATIC_INLINE void i2s_ch0_rcr_pack(uint8_t ch0_rcr)
{
    hwp_i2s->ch0_rcr.val = (((uint32_t)ch0_rcr << 0));
}

__STATIC_INLINE void i2s_ch0_rcr_unpack(uint8_t* ch0_rcr)
{
    T_I2S_CH0_RCR localVal = hwp_i2s->ch0_rcr;

    *ch0_rcr = localVal.bit_field.ch0_rcr;
}

__STATIC_INLINE uint8_t i2s_ch0_rcr_getf(void)
{
    return hwp_i2s->ch0_rcr.bit_field.ch0_rcr;
}

__STATIC_INLINE void i2s_ch0_rcr_setf(uint8_t ch0_rcr)
{
    hwp_i2s->ch0_rcr.bit_field.ch0_rcr = ch0_rcr;
}

__STATIC_INLINE uint32_t i2s_ch0_tcr_get(void)
{
    return hwp_i2s->ch0_tcr.val;
}

__STATIC_INLINE void i2s_ch0_tcr_set(uint32_t value)
{
    hwp_i2s->ch0_tcr.val = value;
}

__STATIC_INLINE void i2s_ch0_tcr_pack(uint8_t ch0_tcr)
{
    hwp_i2s->ch0_tcr.val = (((uint32_t)ch0_tcr << 0));
}

__STATIC_INLINE void i2s_ch0_tcr_unpack(uint8_t* ch0_tcr)
{
    T_I2S_CH0_TCR localVal = hwp_i2s->ch0_tcr;

    *ch0_tcr = localVal.bit_field.ch0_tcr;
}

__STATIC_INLINE uint8_t i2s_ch0_tcr_getf(void)
{
    return hwp_i2s->ch0_tcr.bit_field.ch0_tcr;
}

__STATIC_INLINE void i2s_ch0_tcr_setf(uint8_t ch0_tcr)
{
    hwp_i2s->ch0_tcr.bit_field.ch0_tcr = ch0_tcr;
}

__STATIC_INLINE uint32_t i2s_ch0_isr_get(void)
{
    return hwp_i2s->ch0_isr.val;
}

__STATIC_INLINE void i2s_ch0_isr_unpack(uint8_t* ch0_isr)
{
    T_I2S_CH0_ISR localVal = hwp_i2s->ch0_isr;

    *ch0_isr = localVal.bit_field.ch0_isr;
}

__STATIC_INLINE uint8_t i2s_ch0_isr_getf(void)
{
    return hwp_i2s->ch0_isr.bit_field.ch0_isr;
}

__STATIC_INLINE uint32_t i2s_ch0_imr_get(void)
{
    return hwp_i2s->ch0_imr.val;
}

__STATIC_INLINE void i2s_ch0_imr_set(uint32_t value)
{
    hwp_i2s->ch0_imr.val = value;
}

__STATIC_INLINE void i2s_ch0_imr_pack(uint8_t ch0_imr)
{
    hwp_i2s->ch0_imr.val = (((uint32_t)ch0_imr << 0));
}

__STATIC_INLINE void i2s_ch0_imr_unpack(uint8_t* ch0_imr)
{
    T_I2S_CH0_IMR localVal = hwp_i2s->ch0_imr;

    *ch0_imr = localVal.bit_field.ch0_imr;
}

__STATIC_INLINE uint8_t i2s_ch0_imr_getf(void)
{
    return hwp_i2s->ch0_imr.bit_field.ch0_imr;
}

__STATIC_INLINE void i2s_ch0_imr_setf(uint8_t ch0_imr)
{
    hwp_i2s->ch0_imr.bit_field.ch0_imr = ch0_imr;
}

__STATIC_INLINE uint32_t i2s_ch0_ror_get(void)
{
    return hwp_i2s->ch0_ror.val;
}

__STATIC_INLINE void i2s_ch0_ror_unpack(uint8_t* ch0_ror)
{
    T_I2S_CH0_ROR localVal = hwp_i2s->ch0_ror;

    *ch0_ror = localVal.bit_field.ch0_ror;
}

__STATIC_INLINE uint8_t i2s_ch0_ror_getf(void)
{
    return hwp_i2s->ch0_ror.bit_field.ch0_ror;
}

__STATIC_INLINE uint32_t i2s_ch0_tor_get(void)
{
    return hwp_i2s->ch0_tor.val;
}

__STATIC_INLINE void i2s_ch0_tor_unpack(uint8_t* ch0_tor)
{
    T_I2S_CH0_TOR localVal = hwp_i2s->ch0_tor;

    *ch0_tor = localVal.bit_field.ch0_tor;
}

__STATIC_INLINE uint8_t i2s_ch0_tor_getf(void)
{
    return hwp_i2s->ch0_tor.bit_field.ch0_tor;
}

__STATIC_INLINE uint32_t i2s_ch0_rfcr_get(void)
{
    return hwp_i2s->ch0_rfcr.val;
}

__STATIC_INLINE void i2s_ch0_rfcr_set(uint32_t value)
{
    hwp_i2s->ch0_rfcr.val = value;
}

__STATIC_INLINE void i2s_ch0_rfcr_pack(uint8_t ch0_rfcr)
{
    hwp_i2s->ch0_rfcr.val = (((uint32_t)ch0_rfcr << 0));
}

__STATIC_INLINE void i2s_ch0_rfcr_unpack(uint8_t* ch0_rfcr)
{
    T_I2S_CH0_RFCR localVal = hwp_i2s->ch0_rfcr;

    *ch0_rfcr = localVal.bit_field.ch0_rfcr;
}

__STATIC_INLINE uint8_t i2s_ch0_rfcr_getf(void)
{
    return hwp_i2s->ch0_rfcr.bit_field.ch0_rfcr;
}

__STATIC_INLINE void i2s_ch0_rfcr_setf(uint8_t ch0_rfcr)
{
    hwp_i2s->ch0_rfcr.bit_field.ch0_rfcr = ch0_rfcr;
}

__STATIC_INLINE uint32_t i2s_ch0_tfcr_get(void)
{
    return hwp_i2s->ch0_tfcr.val;
}

__STATIC_INLINE void i2s_ch0_tfcr_set(uint32_t value)
{
    hwp_i2s->ch0_tfcr.val = value;
}

__STATIC_INLINE void i2s_ch0_tfcr_pack(uint8_t ch0_tfcr)
{
    hwp_i2s->ch0_tfcr.val = (((uint32_t)ch0_tfcr << 0));
}

__STATIC_INLINE void i2s_ch0_tfcr_unpack(uint8_t* ch0_tfcr)
{
    T_I2S_CH0_TFCR localVal = hwp_i2s->ch0_tfcr;

    *ch0_tfcr = localVal.bit_field.ch0_tfcr;
}

__STATIC_INLINE uint8_t i2s_ch0_tfcr_getf(void)
{
    return hwp_i2s->ch0_tfcr.bit_field.ch0_tfcr;
}

__STATIC_INLINE void i2s_ch0_tfcr_setf(uint8_t ch0_tfcr)
{
    hwp_i2s->ch0_tfcr.bit_field.ch0_tfcr = ch0_tfcr;
}

__STATIC_INLINE uint32_t i2s_ch0_rff_get(void)
{
    return hwp_i2s->ch0_rff.val;
}

__STATIC_INLINE void i2s_ch0_rff_set(uint32_t value)
{
    hwp_i2s->ch0_rff.val = value;
}

__STATIC_INLINE void i2s_ch0_rff_pack(uint8_t ch0_rff)
{
    hwp_i2s->ch0_rff.val = (((uint32_t)ch0_rff << 0));
}

__STATIC_INLINE uint32_t i2s_ch0_tff_get(void)
{
    return hwp_i2s->ch0_tff.val;
}

__STATIC_INLINE void i2s_ch0_tff_set(uint32_t value)
{
    hwp_i2s->ch0_tff.val = value;
}

__STATIC_INLINE void i2s_ch0_tff_pack(uint8_t ch0_tff)
{
    hwp_i2s->ch0_tff.val = (((uint32_t)ch0_tff << 0));
}

__STATIC_INLINE uint32_t i2s_ch1_lrbr_get(void)
{
    return hwp_i2s->ch1_lrbr.val;
}

__STATIC_INLINE void i2s_ch1_lrbr_set(uint32_t value)
{
    hwp_i2s->ch1_lrbr.val = value;
}

__STATIC_INLINE void i2s_ch1_lrbr_pack(uint32_t ch1_lrbr)
{
    hwp_i2s->ch1_lrbr.val = (((uint32_t)ch1_lrbr << 0));
}

__STATIC_INLINE void i2s_ch1_lrbr_unpack(uint32_t* ch1_lrbr)
{
    T_I2S_CH1_LRBR localVal = hwp_i2s->ch1_lrbr;

    *ch1_lrbr = localVal.bit_field.ch1_lrbr;
}

__STATIC_INLINE uint32_t i2s_ch1_lrbr_getf(void)
{
    return hwp_i2s->ch1_lrbr.bit_field.ch1_lrbr;
}

__STATIC_INLINE void i2s_ch1_lrbr_setf(uint32_t ch1_lrbr)
{
    hwp_i2s->ch1_lrbr.bit_field.ch1_lrbr = ch1_lrbr;
}

__STATIC_INLINE uint32_t i2s_ch1_rrbr_get(void)
{
    return hwp_i2s->ch1_rrbr.val;
}

__STATIC_INLINE void i2s_ch1_rrbr_set(uint32_t value)
{
    hwp_i2s->ch1_rrbr.val = value;
}

__STATIC_INLINE void i2s_ch1_rrbr_pack(uint32_t ch1_rrbr)
{
    hwp_i2s->ch1_rrbr.val = (((uint32_t)ch1_rrbr << 0));
}

__STATIC_INLINE void i2s_ch1_rrbr_unpack(uint32_t* ch1_rrbr)
{
    T_I2S_CH1_RRBR localVal = hwp_i2s->ch1_rrbr;

    *ch1_rrbr = localVal.bit_field.ch1_rrbr;
}

__STATIC_INLINE uint32_t i2s_ch1_rrbr_getf(void)
{
    return hwp_i2s->ch1_rrbr.bit_field.ch1_rrbr;
}

__STATIC_INLINE void i2s_ch1_rrbr_setf(uint32_t ch1_rrbr)
{
    hwp_i2s->ch1_rrbr.bit_field.ch1_rrbr = ch1_rrbr;
}

__STATIC_INLINE uint32_t i2s_ch1_rer_get(void)
{
    return hwp_i2s->ch1_rer.val;
}

__STATIC_INLINE void i2s_ch1_rer_set(uint32_t value)
{
    hwp_i2s->ch1_rer.val = value;
}

__STATIC_INLINE void i2s_ch1_rer_pack(uint8_t ch1_rer)
{
    hwp_i2s->ch1_rer.val = (((uint32_t)ch1_rer << 0));
}

__STATIC_INLINE void i2s_ch1_rer_unpack(uint8_t* ch1_rer)
{
    T_I2S_CH1_RER localVal = hwp_i2s->ch1_rer;

    *ch1_rer = localVal.bit_field.ch1_rer;
}

__STATIC_INLINE uint8_t i2s_ch1_rer_getf(void)
{
    return hwp_i2s->ch1_rer.bit_field.ch1_rer;
}

__STATIC_INLINE void i2s_ch1_rer_setf(uint8_t ch1_rer)
{
    hwp_i2s->ch1_rer.bit_field.ch1_rer = ch1_rer;
}

__STATIC_INLINE uint32_t i2s_ch1_ter_get(void)
{
    return hwp_i2s->ch1_ter.val;
}

__STATIC_INLINE void i2s_ch1_ter_set(uint32_t value)
{
    hwp_i2s->ch1_ter.val = value;
}

__STATIC_INLINE void i2s_ch1_ter_pack(uint8_t ch1_ter)
{
    hwp_i2s->ch1_ter.val = (((uint32_t)ch1_ter << 0));
}

__STATIC_INLINE void i2s_ch1_ter_unpack(uint8_t* ch1_ter)
{
    T_I2S_CH1_TER localVal = hwp_i2s->ch1_ter;

    *ch1_ter = localVal.bit_field.ch1_ter;
}

__STATIC_INLINE uint8_t i2s_ch1_ter_getf(void)
{
    return hwp_i2s->ch1_ter.bit_field.ch1_ter;
}

__STATIC_INLINE void i2s_ch1_ter_setf(uint8_t ch1_ter)
{
    hwp_i2s->ch1_ter.bit_field.ch1_ter = ch1_ter;
}

__STATIC_INLINE uint32_t i2s_ch1_rcr_get(void)
{
    return hwp_i2s->ch1_rcr.val;
}

__STATIC_INLINE void i2s_ch1_rcr_set(uint32_t value)
{
    hwp_i2s->ch1_rcr.val = value;
}

__STATIC_INLINE void i2s_ch1_rcr_pack(uint8_t ch1_rcr)
{
    hwp_i2s->ch1_rcr.val = (((uint32_t)ch1_rcr << 0));
}

__STATIC_INLINE void i2s_ch1_rcr_unpack(uint8_t* ch1_rcr)
{
    T_I2S_CH1_RCR localVal = hwp_i2s->ch1_rcr;

    *ch1_rcr = localVal.bit_field.ch1_rcr;
}

__STATIC_INLINE uint8_t i2s_ch1_rcr_getf(void)
{
    return hwp_i2s->ch1_rcr.bit_field.ch1_rcr;
}

__STATIC_INLINE void i2s_ch1_rcr_setf(uint8_t ch1_rcr)
{
    hwp_i2s->ch1_rcr.bit_field.ch1_rcr = ch1_rcr;
}

__STATIC_INLINE uint32_t i2s_ch1_tcr_get(void)
{
    return hwp_i2s->ch1_tcr.val;
}

__STATIC_INLINE void i2s_ch1_tcr_set(uint32_t value)
{
    hwp_i2s->ch1_tcr.val = value;
}

__STATIC_INLINE void i2s_ch1_tcr_pack(uint8_t ch1_tcr)
{
    hwp_i2s->ch1_tcr.val = (((uint32_t)ch1_tcr << 0));
}

__STATIC_INLINE void i2s_ch1_tcr_unpack(uint8_t* ch1_tcr)
{
    T_I2S_CH1_TCR localVal = hwp_i2s->ch1_tcr;

    *ch1_tcr = localVal.bit_field.ch1_tcr;
}

__STATIC_INLINE uint8_t i2s_ch1_tcr_getf(void)
{
    return hwp_i2s->ch1_tcr.bit_field.ch1_tcr;
}

__STATIC_INLINE void i2s_ch1_tcr_setf(uint8_t ch1_tcr)
{
    hwp_i2s->ch1_tcr.bit_field.ch1_tcr = ch1_tcr;
}

__STATIC_INLINE uint32_t i2s_ch1_isr_get(void)
{
    return hwp_i2s->ch1_isr.val;
}

__STATIC_INLINE void i2s_ch1_isr_unpack(uint8_t* ch1_isr)
{
    T_I2S_CH1_ISR localVal = hwp_i2s->ch1_isr;

    *ch1_isr = localVal.bit_field.ch1_isr;
}

__STATIC_INLINE uint8_t i2s_ch1_isr_getf(void)
{
    return hwp_i2s->ch1_isr.bit_field.ch1_isr;
}

__STATIC_INLINE uint32_t i2s_ch1_imr_get(void)
{
    return hwp_i2s->ch1_imr.val;
}

__STATIC_INLINE void i2s_ch1_imr_set(uint32_t value)
{
    hwp_i2s->ch1_imr.val = value;
}

__STATIC_INLINE void i2s_ch1_imr_pack(uint8_t ch1_imr)
{
    hwp_i2s->ch1_imr.val = (((uint32_t)ch1_imr << 0));
}

__STATIC_INLINE void i2s_ch1_imr_unpack(uint8_t* ch1_imr)
{
    T_I2S_CH1_IMR localVal = hwp_i2s->ch1_imr;

    *ch1_imr = localVal.bit_field.ch1_imr;
}

__STATIC_INLINE uint8_t i2s_ch1_imr_getf(void)
{
    return hwp_i2s->ch1_imr.bit_field.ch1_imr;
}

__STATIC_INLINE void i2s_ch1_imr_setf(uint8_t ch1_imr)
{
    hwp_i2s->ch1_imr.bit_field.ch1_imr = ch1_imr;
}

__STATIC_INLINE uint32_t i2s_ch1_ror_get(void)
{
    return hwp_i2s->ch1_ror.val;
}

__STATIC_INLINE void i2s_ch1_ror_unpack(uint8_t* ch1_ror)
{
    T_I2S_CH1_ROR localVal = hwp_i2s->ch1_ror;

    *ch1_ror = localVal.bit_field.ch1_ror;
}

__STATIC_INLINE uint8_t i2s_ch1_ror_getf(void)
{
    return hwp_i2s->ch1_ror.bit_field.ch1_ror;
}

__STATIC_INLINE uint32_t i2s_ch1_tor_get(void)
{
    return hwp_i2s->ch1_tor.val;
}

__STATIC_INLINE void i2s_ch1_tor_unpack(uint8_t* ch1_tor)
{
    T_I2S_CH1_TOR localVal = hwp_i2s->ch1_tor;

    *ch1_tor = localVal.bit_field.ch1_tor;
}

__STATIC_INLINE uint8_t i2s_ch1_tor_getf(void)
{
    return hwp_i2s->ch1_tor.bit_field.ch1_tor;
}

__STATIC_INLINE uint32_t i2s_ch1_rfcr_get(void)
{
    return hwp_i2s->ch1_rfcr.val;
}

__STATIC_INLINE void i2s_ch1_rfcr_set(uint32_t value)
{
    hwp_i2s->ch1_rfcr.val = value;
}

__STATIC_INLINE void i2s_ch1_rfcr_pack(uint8_t ch1_rfcr)
{
    hwp_i2s->ch1_rfcr.val = (((uint32_t)ch1_rfcr << 0));
}

__STATIC_INLINE void i2s_ch1_rfcr_unpack(uint8_t* ch1_rfcr)
{
    T_I2S_CH1_RFCR localVal = hwp_i2s->ch1_rfcr;

    *ch1_rfcr = localVal.bit_field.ch1_rfcr;
}

__STATIC_INLINE uint8_t i2s_ch1_rfcr_getf(void)
{
    return hwp_i2s->ch1_rfcr.bit_field.ch1_rfcr;
}

__STATIC_INLINE void i2s_ch1_rfcr_setf(uint8_t ch1_rfcr)
{
    hwp_i2s->ch1_rfcr.bit_field.ch1_rfcr = ch1_rfcr;
}

__STATIC_INLINE uint32_t i2s_ch1_tfcr_get(void)
{
    return hwp_i2s->ch1_tfcr.val;
}

__STATIC_INLINE void i2s_ch1_tfcr_set(uint32_t value)
{
    hwp_i2s->ch1_tfcr.val = value;
}

__STATIC_INLINE void i2s_ch1_tfcr_pack(uint8_t ch1_tfcr)
{
    hwp_i2s->ch1_tfcr.val = (((uint32_t)ch1_tfcr << 0));
}

__STATIC_INLINE void i2s_ch1_tfcr_unpack(uint8_t* ch1_tfcr)
{
    T_I2S_CH1_TFCR localVal = hwp_i2s->ch1_tfcr;

    *ch1_tfcr = localVal.bit_field.ch1_tfcr;
}

__STATIC_INLINE uint8_t i2s_ch1_tfcr_getf(void)
{
    return hwp_i2s->ch1_tfcr.bit_field.ch1_tfcr;
}

__STATIC_INLINE void i2s_ch1_tfcr_setf(uint8_t ch1_tfcr)
{
    hwp_i2s->ch1_tfcr.bit_field.ch1_tfcr = ch1_tfcr;
}

__STATIC_INLINE uint32_t i2s_ch1_rff_get(void)
{
    return hwp_i2s->ch1_rff.val;
}

__STATIC_INLINE void i2s_ch1_rff_set(uint32_t value)
{
    hwp_i2s->ch1_rff.val = value;
}

__STATIC_INLINE void i2s_ch1_rff_pack(uint8_t ch1_rff)
{
    hwp_i2s->ch1_rff.val = (((uint32_t)ch1_rff << 0));
}

__STATIC_INLINE uint32_t i2s_ch1_tff_get(void)
{
    return hwp_i2s->ch1_tff.val;
}

__STATIC_INLINE void i2s_ch1_tff_set(uint32_t value)
{
    hwp_i2s->ch1_tff.val = value;
}

__STATIC_INLINE void i2s_ch1_tff_pack(uint8_t ch1_tff)
{
    hwp_i2s->ch1_tff.val = (((uint32_t)ch1_tff << 0));
}

__STATIC_INLINE uint32_t i2s_rxdma_get(void)
{
    return hwp_i2s->rxdma.val;
}

__STATIC_INLINE void i2s_rxdma_unpack(uint32_t* rxdma)
{
    T_I2S_RXDMA localVal = hwp_i2s->rxdma;

    *rxdma = localVal.bit_field.rxdma;
}

__STATIC_INLINE uint32_t i2s_rxdma_getf(void)
{
    return hwp_i2s->rxdma.bit_field.rxdma;
}

__STATIC_INLINE uint32_t i2s_rrxdma_get(void)
{
    return hwp_i2s->rrxdma.val;
}

__STATIC_INLINE void i2s_rrxdma_set(uint32_t value)
{
    hwp_i2s->rrxdma.val = value;
}

__STATIC_INLINE void i2s_rrxdma_pack(uint8_t rrxdma)
{
    hwp_i2s->rrxdma.val = (((uint32_t)rrxdma << 0));
}

__STATIC_INLINE void i2s_rrxdma_unpack(uint8_t* rrxdma)
{
    T_I2S_RRXDMA localVal = hwp_i2s->rrxdma;

    *rrxdma = localVal.bit_field.rrxdma;
}

__STATIC_INLINE uint8_t i2s_rrxdma_getf(void)
{
    return hwp_i2s->rrxdma.bit_field.rrxdma;
}

__STATIC_INLINE void i2s_rrxdma_setf(uint8_t rrxdma)
{
    hwp_i2s->rrxdma.bit_field.rrxdma = rrxdma;
}

__STATIC_INLINE void i2s_txdma_set(uint32_t value)
{
    hwp_i2s->txdma.val = value;
}

__STATIC_INLINE void i2s_txdma_pack(uint32_t txdma)
{
    hwp_i2s->txdma.val = (((uint32_t)txdma << 0));
}

__STATIC_INLINE uint32_t i2s_rtxdma_get(void)
{
    return hwp_i2s->rtxdma.val;
}

__STATIC_INLINE void i2s_rtxdma_set(uint32_t value)
{
    hwp_i2s->rtxdma.val = value;
}

__STATIC_INLINE void i2s_rtxdma_pack(uint8_t rtxdma)
{
    hwp_i2s->rtxdma.val = (((uint32_t)rtxdma << 0));
}

__STATIC_INLINE void i2s_rtxdma_unpack(uint8_t* rtxdma)
{
    T_I2S_RTXDMA localVal = hwp_i2s->rtxdma;

    *rtxdma = localVal.bit_field.rtxdma;
}

__STATIC_INLINE uint8_t i2s_rtxdma_getf(void)
{
    return hwp_i2s->rtxdma.bit_field.rtxdma;
}

__STATIC_INLINE void i2s_rtxdma_setf(uint8_t rtxdma)
{
    hwp_i2s->rtxdma.bit_field.rtxdma = rtxdma;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_param_2_get(void)
{
    return hwp_i2s->i2s_comp_param_2.val;
}

__STATIC_INLINE void i2s_i2s_comp_param_2_unpack(uint32_t* i2s_comp_param_2)
{
    T_I2S_I2S_COMP_PARAM_2 localVal = hwp_i2s->i2s_comp_param_2;

    *i2s_comp_param_2 = localVal.bit_field.i2s_comp_param_2;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_param_2_getf(void)
{
    return hwp_i2s->i2s_comp_param_2.bit_field.i2s_comp_param_2;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_param_1_get(void)
{
    return hwp_i2s->i2s_comp_param_1.val;
}

__STATIC_INLINE void i2s_i2s_comp_param_1_unpack(uint32_t* i2s_comp_param_1)
{
    T_I2S_I2S_COMP_PARAM_1 localVal = hwp_i2s->i2s_comp_param_1;

    *i2s_comp_param_1 = localVal.bit_field.i2s_comp_param_1;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_param_1_getf(void)
{
    return hwp_i2s->i2s_comp_param_1.bit_field.i2s_comp_param_1;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_version_get(void)
{
    return hwp_i2s->i2s_comp_version.val;
}

__STATIC_INLINE void i2s_i2s_comp_version_unpack(uint32_t* i2s_comp_version)
{
    T_I2S_I2S_COMP_VERSION localVal = hwp_i2s->i2s_comp_version;

    *i2s_comp_version = localVal.bit_field.i2s_comp_version;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_version_getf(void)
{
    return hwp_i2s->i2s_comp_version.bit_field.i2s_comp_version;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_type_get(void)
{
    return hwp_i2s->i2s_comp_type.val;
}

__STATIC_INLINE void i2s_i2s_comp_type_unpack(uint32_t* i2s_comp_type)
{
    T_I2S_I2S_COMP_TYPE localVal = hwp_i2s->i2s_comp_type;

    *i2s_comp_type = localVal.bit_field.i2s_comp_type;
}

__STATIC_INLINE uint32_t i2s_i2s_comp_type_getf(void)
{
    return hwp_i2s->i2s_comp_type.bit_field.i2s_comp_type;
}
#endif
