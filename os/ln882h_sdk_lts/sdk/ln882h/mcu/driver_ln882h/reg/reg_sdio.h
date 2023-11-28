#ifndef __REG_SDIO_H__
#define __REG_SDIO_H__
//Auto-gen by fr

#include "ln882h.h"

//sdio_cccr

typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                revision_reg_sd : 12; /*11: 0, bit [7:0] CCCR register 0x0
bit[11:8] CCCR register 0x1*/
        uint32_t                    card_cap_sd :  9; /*20:12, bit [0]: CCCR register 0x7, SCSI
bit [5:1]: CCCR register 0x8 S4MI, SBS, SRW,SMB,SDC
bit[7:6]: CCCR register 0x8  4BLS, LSC
bit [8]: CCCR register 0x12 SMPC*/
        uint32_t                supp_high_speed :  1; /*21:21, high speed mode in CCCR register 0x13*/
        uint32_t                    csa_support :  7; /*28:22, bit 0 is fn1 csa suppport, bit 1 is  fn2 csa_supoort��bit 6is fn7 csa_support in FBR register table*/
        uint32_t                    clr_busy_sd :  1; /*29:29,       will trigger wake up ARM*/
        uint32_t                     reserved_0 :  2; /*31:30,                             NA*/
    } bit_field;
} T_SDIO_SDIO_CCCR;

//sdio_progreg
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      dev_ready :  1; /* 0: 0, indicate CPU is ready for read and write transaction*/
        uint32_t                     card_ready :  1; /* 1: 1, after reset card_ready is set to 1 by HW*/
        uint32_t                  cpu_in_active :  1; /* 2: 2, indicate CPU is in active state*/
        uint32_t                     reserved_0 : 29; /*31: 3,                             NA*/
    } bit_field;
} T_SDIO_SDIO_PROGREG;

//sdio_cis_fn0
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn0_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN0 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN0;

//sdio_cis_fn1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn1_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN1 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN1;

//sdio_cis_fn2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn2_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN2 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN2;

//sdio_cis_fn3
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn3_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN3 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN3;

//sdio_cis_fn4
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn4_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN4 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN4;

//sdio_cis_fn5
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn5_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN5 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN5;

//sdio_cis_fn6
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn6_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN6 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN6;

//sdio_cis_fn7
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   cis_fn7_addr : 32; /*31: 0, this register is programmed by ARM indicating the CIS-FN7 data's are stored in the memory area with starting address as cis_fn0_addr, the AHB master should use this address to perform cis_fn0 transactions*/
    } bit_field;
} T_SDIO_SDIO_CIS_FN7;

//sdio_csa_fn1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn1_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN1;

//sdio_csa_fn2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn2_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN2;

//sdio_csa_fn3
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn3_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN3;

//sdio_csa_fn4
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn4_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN4;

//sdio_csa_fn5
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn5_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN5;

//sdio_csa_fn6
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn6_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN6;

//sdio_csa_fn7
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                   csa_fn7_addr : 32; /*31: 0, this register is programmed by ARM indicating the CSA data's are read/write from/to the memory area with starting address as csa_addr the AHB master should use this address to perform csa transactions*/
    } bit_field;
} T_SDIO_SDIO_CSA_FN7;

//sdio_burst_support
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     burst_size :  3; /* 2: 0,    SDIO AHB master burst size */
        uint32_t                     reserved_1 :  5; /* 7: 3,                             NA*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_BURST_SUPPORT;

//sdio_ocr_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      ocr_value : 24; /*23: 0,               fn1 ocr register*/
        uint32_t                     reserved_0 :  8; /*31:24,                             NA*/
    } bit_field;
} T_SDIO_SDIO_OCR_REG1;

//sdio_fn1_spec
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                    rd_pkt_ctrl :  1; /* 0: 0, CPU read clear; CMD52 can set this register by write SDIO_HOST_READ_PKT_CTRL*/
        uint32_t                     reserved_3 :  7; /* 7: 1,                             NA*/
        uint32_t                    wr_pkt_ctrl :  1; /* 8: 8, CPU read clear; CMD52 can set this register by write SDIO_HOST_WR_PKT_CTRL*/
        uint32_t                     reserved_2 :  7; /*15: 9,                             NA*/
        uint32_t                     retry_ctrl :  1; /*16:16, CPU read clear; CMD52 can set this register by write SDIO_HOST_RETRY_CTRL*/
        uint32_t                     reserved_1 :  7; /*23:17,                             NA*/
        uint32_t                        iir_reg :  1; /*24:24, CPU read clear; CMD52 can set this register by write SDIO_HOST_IIR_REG*/
        uint32_t                     reserved_0 :  7; /*31:25,                             NA*/
    } bit_field;
} T_SDIO_SDIO_FN1_SPEC;

//sdio_int_ena1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                        int_ena : 32; /*31: 0, CPU read only, CMD52 can write to SDIO_HOST_INT_ENA to update this register*/
    } bit_field;
} T_SDIO_SDIO_INT_ENA1;

//sdio_read_fn1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      read_addr : 32; /*31: 0, This register is programmed by ARM indicating the application data are read from the memory area with starting address as read_addr, the AHB master should use this address to perform function1 read transactions*/
    } bit_field;
} T_SDIO_SDIO_READ_FN1;

//sdio_write_fn1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     write_addr : 32; /*31: 0, This register is programmed by ARM indicating the application data are writen to  the memory area with starting address as write_addr, the AHB master should use this address to perform function1 write transactions*/
    } bit_field;
} T_SDIO_SDIO_WRITE_FN1;

//sdio_ahb_trans_cnt1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                       xfer_cnt : 21; /*20: 0, ARM programming the AHB transfer count register. This counter indicates how many bytes of data are available for reading in the ARM memory*/
        uint32_t                     reserved_0 : 11; /*31:21,                             NA*/
    } bit_field;
} T_SDIO_SDIO_AHB_TRANS_CNT1;

//sdio_trans_cnt1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  sdio_xfer_cnt : 21; /*20: 0, Transfer count register to be read by the ARM*/
        uint32_t                     reserved_0 : 11; /*31:21,                             NA*/
    } bit_field;
} T_SDIO_SDIO_TRANS_CNT1;

//sdio_ahb_int_sts1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     status_reg : 21; /*20: 0, write 0 to clear these registers, these intrs will set only when enable is set.
bit 0 : fn1_wr_ovr asserted when all bytes are transferred for write trans
bit 1 : fn1_rd_ovr asserted when all bytes are read from ARM memory
bit 2 : rd_err_fn1 assert when host issues abort for CMD53 read trans
bit 3 : wr_err_fn1 assert when write trans CRC error
bit 4 : wr_abort_fn1 assert when CMD53 write trans abort
bit 5 : rst_fn1 assert when CCCR 0x2 IOE is set from 1 to 0
bit 6 : fn1_en assert when CCCR 0x2 IOE is set from 0 to 1
bit 7 : fn1_status_pcrrt assert when CMD52 write FN1 addr 8'h10 to 1
bit 8 : fn1_status_pcwrt assert when CMD52 write FN1 addr 8'h11 to 1
bit 9 : fn1_rtc_set assert when CMD52 write FN1 addr 8'h12 to 1
bit 10 : fn1_clintrd assert when CMD52 write FN1 addr 8'h13 to 1
bit 11: fn1_int_en_up assert when CMD52 write FN1 addr 8'h14 to 1
bit 19:12 : value 0
bit 20 fn1_m2s_int, m2s1, m2s2, m2s3, m2s4 interrupt*/
        uint32_t                     reserved_0 : 11; /*31:21,                             NA*/
    } bit_field;
} T_SDIO_SDIO_AHB_INT_STS1;

//sdio_ahb_int_sts_en1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     fn1_wr_ovr :  1; /* 0: 0,          interrupt enable bit */
        uint32_t                     fn1_rd_ovr :  1; /* 1: 1,          interrupt enable bit */
        uint32_t                     rd_err_fn1 :  1; /* 2: 2,          interrupt enable bit */
        uint32_t                     wr_err_fn1 :  1; /* 3: 3,          interrupt enable bit */
        uint32_t                   wr_abort_fn1 :  1; /* 4: 4,          interrupt enable bit */
        uint32_t                        rst_fn1 :  1; /* 5: 5,          interrupt enable bit */
        uint32_t                         fn1_en :  1; /* 6: 6,          interrupt enable bit */
        uint32_t               fn1_status_pcrrt :  1; /* 7: 7,          interrupt enable bit */
        uint32_t               fn1_status_pcwrt :  1; /* 8: 8,          interrupt enable bit */
        uint32_t                    fn1_rtc_set :  1; /* 9: 9,          interrupt enable bit */
        uint32_t                    fn1_clintrd :  1; /*10:10,          interrupt enable bit */
        uint32_t                  fn1_int_en_up :  1; /*11:11,          interrupt enable bit */
        uint32_t                     reserved_1 :  8; /*19:12,                             NA*/
        uint32_t                    fn1_m2s_int :  1; /*20:20,          interrupt enable bit */
        uint32_t                     reserved_0 : 11; /*31:21,                             NA*/
    } bit_field;
} T_SDIO_SDIO_AHB_INT_STS_EN1;

//sdio_fbr_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                 interface_code :  4; /* 3: 0,  FBR 0x100 register bit 3~bit0*/
        uint32_t                       fbr1_reg :  8; /*11: 4,             FBR 0x101 register*/
        uint32_t                            eps :  1; /*12:12, enable power selection in FBR 0x102 register*/
        uint32_t                     reserved_0 : 19; /*31:13,                             NA*/
    } bit_field;
} T_SDIO_SDIO_FBR_REG1;

//sdio_ior_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                         io_rdy :  1; /* 0: 0,          fn1 IO ready register*/
        uint32_t                     reserved_0 : 31; /*31: 1,                             NA*/
    } bit_field;
} T_SDIO_SDIO_IOR_REG1;

//sdio_m2s1_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     m2s1_value :  8; /* 7: 0, CPU read only, CMD52 write to  SDIO_HOST_M2S1_MB_REG1 to update this register,CPU read enable depend on SDIO_M2S_STS_CTRL*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S1_REG1;

//sdio_m2s2_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     m2s2_value :  8; /* 7: 0, CPU read only, CMD52 write to  SDIO_HOST_M2S2_MB_REG1 to update this register, CPU read enable depend on SDIO_M2S_STS_CTRL*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S2_REG1;

//sdio_m2s3_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     m2s3_value :  8; /* 7: 0, CPU read only, CMD52 write to  SDIO_HOST_M2S3_MB_REG1 to update this register, CPU read enable depend on SDIO_M2S_STS_CTRL*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S3_REG1;

//sdio_m2s4_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     m2s4_value :  8; /* 7: 0, CPU read only, CMD52 write to  SDIO_HOST_M2S4_MB_REG1 to update this register, CPU read enable depend on SDIO_M2S_STS_CTRL*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S4_REG1;

//sdio_s2m1_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     s2m1_value :  8; /* 7: 0, CPU read/write, CMD52 can read this register value by SDIO_HOST_S2M1_MB_REG1, CMD52 read enable depend on SDIO_HOST_S2M_MB_ENA_REG1*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M1_REG1;

//sdio_s2m2_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     s2m2_value :  8; /* 7: 0, CPU read/write, CMD52 can read this register value by SDIO_HOST_S2M2_MB_REG1, CMD52 read enable depend on SDIO_HOST_S2M_MB_ENA_REG1*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M2_REG1;

//sdio_s2m3_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     s2m3_value :  8; /* 7: 0, CPU read/write, CMD52 can read this register value by SDIO_HOST_S2M3_MB_REG1, CMD52 read enable depend on SDIO_HOST_S2M_MB_ENA_REG1*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M3_REG1;

//sdio_s2m4_reg1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                     s2m4_value :  8; /* 7: 0, CPU read/write, CMD52 can read this register value by SDIO_HOST_S2M4_MB_REG1, CMD52 read enable depend on SDIO_HOST_S2M_MB_ENA_REG1*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M4_REG1;

//sdio_m2s_sts_ctrl
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      m2s_value :  4; /* 3: 0, bit0:fn1_m2s1_int , CPU read only, write 1 to clear,  when CMD52 write to SDIO_HOST_M2S1_MB_REG1 and both SDIO_AHB_INT_STS_EN1 bit20
and bit 4 in this register is set, this feild will be set.  
bit1:fn1_m2s2_int ,  CPU read only, write 1 to clear,  when CMD52 write to SDIO_HOST_M2S2_MB_REG1 and both SDIO_AHB_INT_STS_EN1 bit20
and bit 5 in this register is set, this feild will be set.  
bit2:fn1_m2s3_int ,  CPU read only, write 1 to clear,  when CMD52 write to SDIO_HOST_M2S3_MB_REG1 and both SDIO_AHB_INT_STS_EN1 bit20
and bit 6 in this register is set, this feild will be set. 
bit3:fn1_m2s4_int  , CPU read only, write 1 to clear,  when CMD52 write to SDIO_HOST_M2S4_MB_REG1 and both SDIO_AHB_INT_STS_EN1 bit20
and bit 7 in this register is set, this feild will be set. */
        uint32_t                     m2s_int_en :  4; /* 7: 4, bit4:m2s1_int_en, to enable CMD52 write to SDIO_M2S1_REG1 bit5:m2s2_int_en, to enable CMD52 write to SDIO_M2S2_REG1 bit6:m2s3_int_en, to enable CMD52 write to SDIO_M2S3_REG1 bit7:m2s4_int_en, to enable CMD52 write to SDIO_M2S4_REG1*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S_STS_CTRL;

//sdio_s2m_sts_ctrl
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                      s2m_value :  8; /* 7: 0, bit0:s2m1_mb_updated, CPU read only, read enable is SDIO_HOST_S2M_MB_ENA_REG1 bit 4.
bit1:s2m2_mb_updated, CPU read only, read enable is SDIO_HOST_S2M_MB_ENA_REG1 bit 5.
bit2:s2m3_mb_updated, CPU read only, read enable is SDIO_HOST_S2M_MB_ENA_REG1 bit 6.
bit3:s2m4_mb_updated, CPU read only, read enable is SDIO_HOST_S2M_MB_ENA_REG1 bit 7.
bit4:s2m1_int_en, the value of SDIO_HOST_S2M_MB_ENA_REG1 bit 4
bit5:s2m2_int_en, the value of SDIO_HOST_S2M_MB_ENA_REG1 bit 5
bit6:s2m3_int_en, the value of SDIO_HOST_S2M_MB_ENA_REG1 bit 6
bit7:s2m4_int_en,the value of SDIO_HOST_S2M_MB_ENA_REG1 bit 7*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M_STS_CTRL;

//sdio_m2s_sc0
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  m2s_sc0_value :  8; /* 7: 0, CPU read CMD52 write scratch register*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S_SC0;

//sdio_m2s_sc1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  m2s_sc1_value :  8; /* 7: 0, CPU read CMD52 write scratch register*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S_SC1;

//sdio_m2s_sc2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  m2s_sc2_value :  8; /* 7: 0, CPU read CMD52 write scratch register*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S_SC2;

//sdio_m2s_sc3
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  m2s_sc3_value :  8; /* 7: 0, CPU read CMD52 write scratch register*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_M2S_SC3;

//sdio_s2m_sc0
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  s2m_sc0_value :  8; /* 7: 0,           CPU write CMD52 read*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M_SC0;

//sdio_s2m_sc1
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  s2m_sc1_value :  8; /* 7: 0,           CPU write CMD52 read*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M_SC1;

//sdio_s2m_sc2
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  s2m_sc2_value :  8; /* 7: 0,           CPU write CMD52 read*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M_SC2;

//sdio_s2m_sc3
typedef union
{
    uint32_t val;
    struct
    {
        uint32_t                  s2m_sc3_value :  8; /* 7: 0,           CPU write CMD52 read*/
        uint32_t                     reserved_0 : 24; /*31: 8,                             NA*/
    } bit_field;
} T_SDIO_SDIO_S2M_SC3;

//Registers Mapping to the same address

typedef struct
{
    volatile                        uint32_t                     reserved_0;
    volatile                T_SDIO_SDIO_CCCR                      sdio_cccr; /*  0x4,    RW, 0x0033F232,                             NA*/
    volatile             T_SDIO_SDIO_PROGREG                   sdio_progreg; /*  0x8,    RW, 0x00000006,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN0                   sdio_cis_fn0; /*  0xc,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN1                   sdio_cis_fn1; /* 0x10,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN2                   sdio_cis_fn2; /* 0x14,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN3                   sdio_cis_fn3; /* 0x18,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN4                   sdio_cis_fn4; /* 0x1c,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN5                   sdio_cis_fn5; /* 0x20,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN6                   sdio_cis_fn6; /* 0x24,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CIS_FN7                   sdio_cis_fn7; /* 0x28,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN1                   sdio_csa_fn1; /* 0x2c,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN2                   sdio_csa_fn2; /* 0x30,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN3                   sdio_csa_fn3; /* 0x34,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN4                   sdio_csa_fn4; /* 0x38,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN5                   sdio_csa_fn5; /* 0x3c,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN6                   sdio_csa_fn6; /* 0x40,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_CSA_FN7                   sdio_csa_fn7; /* 0x44,    RW, 0x00000000,                             NA*/
    volatile       T_SDIO_SDIO_BURST_SUPPORT             sdio_burst_support; /* 0x48,    RW, 0x00000000,                             NA*/
    volatile                        uint32_t                 reserved_1[48];
    volatile            T_SDIO_SDIO_OCR_REG1                  sdio_ocr_reg1; /*0x10c,    RW, 0x00FF8000,                             NA*/
    volatile            T_SDIO_SDIO_FN1_SPEC                  sdio_fn1_spec; /*0x110,    RO, 0x00000000,                             NA*/
    volatile            T_SDIO_SDIO_INT_ENA1                  sdio_int_ena1; /*0x114,    RO, 0x00000000,                             NA*/
    volatile                        uint32_t                  reserved_2[3];
    volatile            T_SDIO_SDIO_READ_FN1                  sdio_read_fn1; /*0x124,    RW, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_WRITE_FN1                 sdio_write_fn1; /*0x128,    RW, 0x00000000,                             NA*/
    volatile      T_SDIO_SDIO_AHB_TRANS_CNT1            sdio_ahb_trans_cnt1; /*0x12c,    RW, 0x00000000,                             NA*/
    volatile          T_SDIO_SDIO_TRANS_CNT1                sdio_trans_cnt1; /*0x130,    RO, 0x00000000,                             NA*/
    volatile        T_SDIO_SDIO_AHB_INT_STS1              sdio_ahb_int_sts1; /*0x134,    RO, 0x00000000,                             NA*/
    volatile     T_SDIO_SDIO_AHB_INT_STS_EN1           sdio_ahb_int_sts_en1; /*0x138,    RW, 0x00000000,                             NA*/
    volatile            T_SDIO_SDIO_FBR_REG1                  sdio_fbr_reg1; /*0x13c,    RW, 0x00001000,                             NA*/
    volatile            T_SDIO_SDIO_IOR_REG1                  sdio_ior_reg1; /*0x140,    RW, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_M2S1_REG1                 sdio_m2s1_reg1; /*0x144,    RO, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_M2S2_REG1                 sdio_m2s2_reg1; /*0x148,    RO, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_M2S3_REG1                 sdio_m2s3_reg1; /*0x14c,    RO, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_M2S4_REG1                 sdio_m2s4_reg1; /*0x150,    RO, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_S2M1_REG1                 sdio_s2m1_reg1; /*0x154,    RW, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_S2M2_REG1                 sdio_s2m2_reg1; /*0x158,    RW, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_S2M3_REG1                 sdio_s2m3_reg1; /*0x15c,    RW, 0x00000000,                             NA*/
    volatile           T_SDIO_SDIO_S2M4_REG1                 sdio_s2m4_reg1; /*0x160,    RW, 0x00000000,                             NA*/
    volatile        T_SDIO_SDIO_M2S_STS_CTRL              sdio_m2s_sts_ctrl; /*0x164,    RW, 0x00000000,                             NA*/
    volatile        T_SDIO_SDIO_S2M_STS_CTRL              sdio_s2m_sts_ctrl; /*0x168,    RO, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_M2S_SC0                   sdio_m2s_sc0; /*0x16c,    RO, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_M2S_SC1                   sdio_m2s_sc1; /*0x170,    RO, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_M2S_SC2                   sdio_m2s_sc2; /*0x174,    RO, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_M2S_SC3                   sdio_m2s_sc3; /*0x178,    RO, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_S2M_SC0                   sdio_s2m_sc0; /*0x17c,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_S2M_SC1                   sdio_s2m_sc1; /*0x180,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_S2M_SC2                   sdio_s2m_sc2; /*0x184,    RW, 0x00000000,                             NA*/
    volatile             T_SDIO_SDIO_S2M_SC3                   sdio_s2m_sc3; /*0x188,    RW, 0x00000000,                             NA*/
} T_HWP_SDIO_T;

#define hwp_sdio ((T_HWP_SDIO_T*)SDIO_BASE)

__STATIC_INLINE void sdio_iir_reg_setf(uint8_t value)
{
    hwp_sdio->sdio_fn1_spec.bit_field.iir_reg = value;
}
__STATIC_INLINE uint32_t sdio_sdio_cccr_get(void)
{
    return hwp_sdio->sdio_cccr.val;
}

__STATIC_INLINE void sdio_sdio_cccr_set(uint32_t value)
{
    hwp_sdio->sdio_cccr.val = value;
}

__STATIC_INLINE void sdio_sdio_cccr_pack(uint8_t clr_busy_sd, uint8_t csa_support, uint8_t supp_high_speed, uint16_t card_cap_sd, uint16_t revision_reg_sd)
{
    hwp_sdio->sdio_cccr.val = (((uint32_t)clr_busy_sd << 29) | ((uint32_t)csa_support << 22) | ((uint32_t)supp_high_speed << 21) | ((uint32_t)card_cap_sd << 12) | ((uint32_t)revision_reg_sd << 0));
}

__STATIC_INLINE void sdio_sdio_cccr_unpack(uint8_t* clr_busy_sd, uint8_t* csa_support, uint8_t* supp_high_speed, uint16_t* card_cap_sd, uint16_t* revision_reg_sd)
{
    T_SDIO_SDIO_CCCR localVal = hwp_sdio->sdio_cccr;

    *clr_busy_sd = localVal.bit_field.clr_busy_sd;
    *csa_support = localVal.bit_field.csa_support;
    *supp_high_speed = localVal.bit_field.supp_high_speed;
    *card_cap_sd = localVal.bit_field.card_cap_sd;
    *revision_reg_sd = localVal.bit_field.revision_reg_sd;
}

__STATIC_INLINE uint8_t sdio_clr_busy_sd_getf(void)
{
    return hwp_sdio->sdio_cccr.bit_field.clr_busy_sd;
}

__STATIC_INLINE void sdio_clr_busy_sd_setf(uint8_t clr_busy_sd)
{
    hwp_sdio->sdio_cccr.bit_field.clr_busy_sd = clr_busy_sd;
}

__STATIC_INLINE uint8_t sdio_csa_support_getf(void)
{
    return hwp_sdio->sdio_cccr.bit_field.csa_support;
}

__STATIC_INLINE void sdio_csa_support_setf(uint8_t csa_support)
{
    hwp_sdio->sdio_cccr.bit_field.csa_support = csa_support;
}

__STATIC_INLINE uint8_t sdio_supp_high_speed_getf(void)
{
    return hwp_sdio->sdio_cccr.bit_field.supp_high_speed;
}

__STATIC_INLINE void sdio_supp_high_speed_setf(uint8_t supp_high_speed)
{
    hwp_sdio->sdio_cccr.bit_field.supp_high_speed = supp_high_speed;
}

__STATIC_INLINE uint16_t sdio_card_cap_sd_getf(void)
{
    return hwp_sdio->sdio_cccr.bit_field.card_cap_sd;
}

__STATIC_INLINE void sdio_card_cap_sd_setf(uint16_t card_cap_sd)
{
    hwp_sdio->sdio_cccr.bit_field.card_cap_sd = card_cap_sd;
}

__STATIC_INLINE uint16_t sdio_revision_reg_sd_getf(void)
{
    return hwp_sdio->sdio_cccr.bit_field.revision_reg_sd;
}

__STATIC_INLINE void sdio_revision_reg_sd_setf(uint16_t revision_reg_sd)
{
    hwp_sdio->sdio_cccr.bit_field.revision_reg_sd = revision_reg_sd;
}

__STATIC_INLINE uint32_t sdio_sdio_progreg_get(void)
{
    return hwp_sdio->sdio_progreg.val;
}

__STATIC_INLINE void sdio_sdio_progreg_set(uint32_t value)
{
    hwp_sdio->sdio_progreg.val = value;
}

__STATIC_INLINE void sdio_sdio_progreg_pack(uint8_t cpu_in_active, uint8_t dev_ready)
{
    hwp_sdio->sdio_progreg.val = (((uint32_t)cpu_in_active << 2) | ((uint32_t)dev_ready << 0));
}

__STATIC_INLINE void sdio_sdio_progreg_unpack(uint8_t* cpu_in_active, uint8_t* card_ready, uint8_t* dev_ready)
{
    T_SDIO_SDIO_PROGREG localVal = hwp_sdio->sdio_progreg;

    *cpu_in_active = localVal.bit_field.cpu_in_active;
    *card_ready = localVal.bit_field.card_ready;
    *dev_ready = localVal.bit_field.dev_ready;
}

__STATIC_INLINE uint8_t sdio_cpu_in_active_getf(void)
{
    return hwp_sdio->sdio_progreg.bit_field.cpu_in_active;
}

__STATIC_INLINE void sdio_cpu_in_active_setf(uint8_t cpu_in_active)
{
    hwp_sdio->sdio_progreg.bit_field.cpu_in_active = cpu_in_active;
}

__STATIC_INLINE uint8_t sdio_card_ready_getf(void)
{
    return hwp_sdio->sdio_progreg.bit_field.card_ready;
}

__STATIC_INLINE uint8_t sdio_dev_ready_getf(void)
{
    return hwp_sdio->sdio_progreg.bit_field.dev_ready;
}

__STATIC_INLINE void sdio_dev_ready_setf(uint8_t dev_ready)
{
    hwp_sdio->sdio_progreg.bit_field.dev_ready = dev_ready;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn0_get(void)
{
    return hwp_sdio->sdio_cis_fn0.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn0_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn0.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn0_pack(uint32_t cis_fn0_addr)
{
    hwp_sdio->sdio_cis_fn0.val = (((uint32_t)cis_fn0_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn0_unpack(uint32_t* cis_fn0_addr)
{
    T_SDIO_SDIO_CIS_FN0 localVal = hwp_sdio->sdio_cis_fn0;

    *cis_fn0_addr = localVal.bit_field.cis_fn0_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn0_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn0.bit_field.cis_fn0_addr;
}

__STATIC_INLINE void sdio_cis_fn0_addr_setf(uint32_t cis_fn0_addr)
{
    hwp_sdio->sdio_cis_fn0.bit_field.cis_fn0_addr = cis_fn0_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn1_get(void)
{
    return hwp_sdio->sdio_cis_fn1.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn1_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn1.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn1_pack(uint32_t cis_fn1_addr)
{
    hwp_sdio->sdio_cis_fn1.val = (((uint32_t)cis_fn1_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn1_unpack(uint32_t* cis_fn1_addr)
{
    T_SDIO_SDIO_CIS_FN1 localVal = hwp_sdio->sdio_cis_fn1;

    *cis_fn1_addr = localVal.bit_field.cis_fn1_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn1_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn1.bit_field.cis_fn1_addr;
}

__STATIC_INLINE void sdio_cis_fn1_addr_setf(uint32_t cis_fn1_addr)
{
    hwp_sdio->sdio_cis_fn1.bit_field.cis_fn1_addr = cis_fn1_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn2_get(void)
{
    return hwp_sdio->sdio_cis_fn2.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn2_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn2.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn2_pack(uint32_t cis_fn2_addr)
{
    hwp_sdio->sdio_cis_fn2.val = (((uint32_t)cis_fn2_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn2_unpack(uint32_t* cis_fn2_addr)
{
    T_SDIO_SDIO_CIS_FN2 localVal = hwp_sdio->sdio_cis_fn2;

    *cis_fn2_addr = localVal.bit_field.cis_fn2_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn2_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn2.bit_field.cis_fn2_addr;
}

__STATIC_INLINE void sdio_cis_fn2_addr_setf(uint32_t cis_fn2_addr)
{
    hwp_sdio->sdio_cis_fn2.bit_field.cis_fn2_addr = cis_fn2_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn3_get(void)
{
    return hwp_sdio->sdio_cis_fn3.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn3_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn3.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn3_pack(uint32_t cis_fn3_addr)
{
    hwp_sdio->sdio_cis_fn3.val = (((uint32_t)cis_fn3_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn3_unpack(uint32_t* cis_fn3_addr)
{
    T_SDIO_SDIO_CIS_FN3 localVal = hwp_sdio->sdio_cis_fn3;

    *cis_fn3_addr = localVal.bit_field.cis_fn3_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn3_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn3.bit_field.cis_fn3_addr;
}

__STATIC_INLINE void sdio_cis_fn3_addr_setf(uint32_t cis_fn3_addr)
{
    hwp_sdio->sdio_cis_fn3.bit_field.cis_fn3_addr = cis_fn3_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn4_get(void)
{
    return hwp_sdio->sdio_cis_fn4.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn4_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn4.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn4_pack(uint32_t cis_fn4_addr)
{
    hwp_sdio->sdio_cis_fn4.val = (((uint32_t)cis_fn4_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn4_unpack(uint32_t* cis_fn4_addr)
{
    T_SDIO_SDIO_CIS_FN4 localVal = hwp_sdio->sdio_cis_fn4;

    *cis_fn4_addr = localVal.bit_field.cis_fn4_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn4_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn4.bit_field.cis_fn4_addr;
}

__STATIC_INLINE void sdio_cis_fn4_addr_setf(uint32_t cis_fn4_addr)
{
    hwp_sdio->sdio_cis_fn4.bit_field.cis_fn4_addr = cis_fn4_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn5_get(void)
{
    return hwp_sdio->sdio_cis_fn5.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn5_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn5.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn5_pack(uint32_t cis_fn5_addr)
{
    hwp_sdio->sdio_cis_fn5.val = (((uint32_t)cis_fn5_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn5_unpack(uint32_t* cis_fn5_addr)
{
    T_SDIO_SDIO_CIS_FN5 localVal = hwp_sdio->sdio_cis_fn5;

    *cis_fn5_addr = localVal.bit_field.cis_fn5_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn5_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn5.bit_field.cis_fn5_addr;
}

__STATIC_INLINE void sdio_cis_fn5_addr_setf(uint32_t cis_fn5_addr)
{
    hwp_sdio->sdio_cis_fn5.bit_field.cis_fn5_addr = cis_fn5_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn6_get(void)
{
    return hwp_sdio->sdio_cis_fn6.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn6_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn6.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn6_pack(uint32_t cis_fn6_addr)
{
    hwp_sdio->sdio_cis_fn6.val = (((uint32_t)cis_fn6_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn6_unpack(uint32_t* cis_fn6_addr)
{
    T_SDIO_SDIO_CIS_FN6 localVal = hwp_sdio->sdio_cis_fn6;

    *cis_fn6_addr = localVal.bit_field.cis_fn6_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn6_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn6.bit_field.cis_fn6_addr;
}

__STATIC_INLINE void sdio_cis_fn6_addr_setf(uint32_t cis_fn6_addr)
{
    hwp_sdio->sdio_cis_fn6.bit_field.cis_fn6_addr = cis_fn6_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_cis_fn7_get(void)
{
    return hwp_sdio->sdio_cis_fn7.val;
}

__STATIC_INLINE void sdio_sdio_cis_fn7_set(uint32_t value)
{
    hwp_sdio->sdio_cis_fn7.val = value;
}

__STATIC_INLINE void sdio_sdio_cis_fn7_pack(uint32_t cis_fn7_addr)
{
    hwp_sdio->sdio_cis_fn7.val = (((uint32_t)cis_fn7_addr << 0));
}

__STATIC_INLINE void sdio_sdio_cis_fn7_unpack(uint32_t* cis_fn7_addr)
{
    T_SDIO_SDIO_CIS_FN7 localVal = hwp_sdio->sdio_cis_fn7;

    *cis_fn7_addr = localVal.bit_field.cis_fn7_addr;
}

__STATIC_INLINE uint32_t sdio_cis_fn7_addr_getf(void)
{
    return hwp_sdio->sdio_cis_fn7.bit_field.cis_fn7_addr;
}

__STATIC_INLINE void sdio_cis_fn7_addr_setf(uint32_t cis_fn7_addr)
{
    hwp_sdio->sdio_cis_fn7.bit_field.cis_fn7_addr = cis_fn7_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn1_get(void)
{
    return hwp_sdio->sdio_csa_fn1.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn1_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn1.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn1_pack(uint32_t csa_fn1_addr)
{
    hwp_sdio->sdio_csa_fn1.val = (((uint32_t)csa_fn1_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn1_unpack(uint32_t* csa_fn1_addr)
{
    T_SDIO_SDIO_CSA_FN1 localVal = hwp_sdio->sdio_csa_fn1;

    *csa_fn1_addr = localVal.bit_field.csa_fn1_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn1_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn1.bit_field.csa_fn1_addr;
}

__STATIC_INLINE void sdio_csa_fn1_addr_setf(uint32_t csa_fn1_addr)
{
    hwp_sdio->sdio_csa_fn1.bit_field.csa_fn1_addr = csa_fn1_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn2_get(void)
{
    return hwp_sdio->sdio_csa_fn2.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn2_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn2.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn2_pack(uint32_t csa_fn2_addr)
{
    hwp_sdio->sdio_csa_fn2.val = (((uint32_t)csa_fn2_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn2_unpack(uint32_t* csa_fn2_addr)
{
    T_SDIO_SDIO_CSA_FN2 localVal = hwp_sdio->sdio_csa_fn2;

    *csa_fn2_addr = localVal.bit_field.csa_fn2_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn2_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn2.bit_field.csa_fn2_addr;
}

__STATIC_INLINE void sdio_csa_fn2_addr_setf(uint32_t csa_fn2_addr)
{
    hwp_sdio->sdio_csa_fn2.bit_field.csa_fn2_addr = csa_fn2_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn3_get(void)
{
    return hwp_sdio->sdio_csa_fn3.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn3_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn3.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn3_pack(uint32_t csa_fn3_addr)
{
    hwp_sdio->sdio_csa_fn3.val = (((uint32_t)csa_fn3_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn3_unpack(uint32_t* csa_fn3_addr)
{
    T_SDIO_SDIO_CSA_FN3 localVal = hwp_sdio->sdio_csa_fn3;

    *csa_fn3_addr = localVal.bit_field.csa_fn3_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn3_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn3.bit_field.csa_fn3_addr;
}

__STATIC_INLINE void sdio_csa_fn3_addr_setf(uint32_t csa_fn3_addr)
{
    hwp_sdio->sdio_csa_fn3.bit_field.csa_fn3_addr = csa_fn3_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn4_get(void)
{
    return hwp_sdio->sdio_csa_fn4.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn4_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn4.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn4_pack(uint32_t csa_fn4_addr)
{
    hwp_sdio->sdio_csa_fn4.val = (((uint32_t)csa_fn4_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn4_unpack(uint32_t* csa_fn4_addr)
{
    T_SDIO_SDIO_CSA_FN4 localVal = hwp_sdio->sdio_csa_fn4;

    *csa_fn4_addr = localVal.bit_field.csa_fn4_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn4_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn4.bit_field.csa_fn4_addr;
}

__STATIC_INLINE void sdio_csa_fn4_addr_setf(uint32_t csa_fn4_addr)
{
    hwp_sdio->sdio_csa_fn4.bit_field.csa_fn4_addr = csa_fn4_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn5_get(void)
{
    return hwp_sdio->sdio_csa_fn5.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn5_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn5.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn5_pack(uint32_t csa_fn5_addr)
{
    hwp_sdio->sdio_csa_fn5.val = (((uint32_t)csa_fn5_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn5_unpack(uint32_t* csa_fn5_addr)
{
    T_SDIO_SDIO_CSA_FN5 localVal = hwp_sdio->sdio_csa_fn5;

    *csa_fn5_addr = localVal.bit_field.csa_fn5_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn5_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn5.bit_field.csa_fn5_addr;
}

__STATIC_INLINE void sdio_csa_fn5_addr_setf(uint32_t csa_fn5_addr)
{
    hwp_sdio->sdio_csa_fn5.bit_field.csa_fn5_addr = csa_fn5_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn6_get(void)
{
    return hwp_sdio->sdio_csa_fn6.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn6_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn6.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn6_pack(uint32_t csa_fn6_addr)
{
    hwp_sdio->sdio_csa_fn6.val = (((uint32_t)csa_fn6_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn6_unpack(uint32_t* csa_fn6_addr)
{
    T_SDIO_SDIO_CSA_FN6 localVal = hwp_sdio->sdio_csa_fn6;

    *csa_fn6_addr = localVal.bit_field.csa_fn6_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn6_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn6.bit_field.csa_fn6_addr;
}

__STATIC_INLINE void sdio_csa_fn6_addr_setf(uint32_t csa_fn6_addr)
{
    hwp_sdio->sdio_csa_fn6.bit_field.csa_fn6_addr = csa_fn6_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_csa_fn7_get(void)
{
    return hwp_sdio->sdio_csa_fn7.val;
}

__STATIC_INLINE void sdio_sdio_csa_fn7_set(uint32_t value)
{
    hwp_sdio->sdio_csa_fn7.val = value;
}

__STATIC_INLINE void sdio_sdio_csa_fn7_pack(uint32_t csa_fn7_addr)
{
    hwp_sdio->sdio_csa_fn7.val = (((uint32_t)csa_fn7_addr << 0));
}

__STATIC_INLINE void sdio_sdio_csa_fn7_unpack(uint32_t* csa_fn7_addr)
{
    T_SDIO_SDIO_CSA_FN7 localVal = hwp_sdio->sdio_csa_fn7;

    *csa_fn7_addr = localVal.bit_field.csa_fn7_addr;
}

__STATIC_INLINE uint32_t sdio_csa_fn7_addr_getf(void)
{
    return hwp_sdio->sdio_csa_fn7.bit_field.csa_fn7_addr;
}

__STATIC_INLINE void sdio_csa_fn7_addr_setf(uint32_t csa_fn7_addr)
{
    hwp_sdio->sdio_csa_fn7.bit_field.csa_fn7_addr = csa_fn7_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_burst_support_get(void)
{
    return hwp_sdio->sdio_burst_support.val;
}

__STATIC_INLINE void sdio_sdio_burst_support_set(uint32_t value)
{
    hwp_sdio->sdio_burst_support.val = value;
}

__STATIC_INLINE void sdio_sdio_burst_support_pack(uint8_t burst_size)
{
    hwp_sdio->sdio_burst_support.val = (((uint32_t)burst_size << 0));
}

__STATIC_INLINE void sdio_sdio_burst_support_unpack(uint8_t* burst_size)
{
    T_SDIO_SDIO_BURST_SUPPORT localVal = hwp_sdio->sdio_burst_support;

    *burst_size = localVal.bit_field.burst_size;
}

__STATIC_INLINE uint8_t sdio_burst_size_getf(void)
{
    return hwp_sdio->sdio_burst_support.bit_field.burst_size;
}

__STATIC_INLINE void sdio_burst_size_setf(uint8_t burst_size)
{
    hwp_sdio->sdio_burst_support.bit_field.burst_size = burst_size;
}

__STATIC_INLINE uint32_t sdio_sdio_ocr_reg1_get(void)
{
    return hwp_sdio->sdio_ocr_reg1.val;
}

__STATIC_INLINE void sdio_sdio_ocr_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_ocr_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_ocr_reg1_pack(uint32_t ocr_value)
{
    hwp_sdio->sdio_ocr_reg1.val = (((uint32_t)ocr_value << 0));
}

__STATIC_INLINE void sdio_sdio_ocr_reg1_unpack(uint32_t* ocr_value)
{
    T_SDIO_SDIO_OCR_REG1 localVal = hwp_sdio->sdio_ocr_reg1;

    *ocr_value = localVal.bit_field.ocr_value;
}

__STATIC_INLINE uint32_t sdio_ocr_value_getf(void)
{
    return hwp_sdio->sdio_ocr_reg1.bit_field.ocr_value;
}

__STATIC_INLINE void sdio_ocr_value_setf(uint32_t ocr_value)
{
    hwp_sdio->sdio_ocr_reg1.bit_field.ocr_value = ocr_value;
}

__STATIC_INLINE uint32_t sdio_sdio_fn1_spec_get(void)
{
    return hwp_sdio->sdio_fn1_spec.val;
}

__STATIC_INLINE void sdio_sdio_fn1_spec_pack(uint8_t iir_reg)
{
    hwp_sdio->sdio_fn1_spec.val = (((uint32_t)iir_reg << 24));
}

__STATIC_INLINE void sdio_sdio_fn1_spec_unpack(uint8_t* iir_reg, uint8_t* retry_ctrl, uint8_t* wr_pkt_ctrl, uint8_t* rd_pkt_ctrl)
{
    T_SDIO_SDIO_FN1_SPEC localVal = hwp_sdio->sdio_fn1_spec;

    *iir_reg = localVal.bit_field.iir_reg;
    *retry_ctrl = localVal.bit_field.retry_ctrl;
    *wr_pkt_ctrl = localVal.bit_field.wr_pkt_ctrl;
    *rd_pkt_ctrl = localVal.bit_field.rd_pkt_ctrl;
}

__STATIC_INLINE uint8_t sdio_iir_reg_getf(void)
{
    return hwp_sdio->sdio_fn1_spec.bit_field.iir_reg;
}

__STATIC_INLINE uint8_t sdio_retry_ctrl_getf(void)
{
    return hwp_sdio->sdio_fn1_spec.bit_field.retry_ctrl;
}

__STATIC_INLINE uint8_t sdio_wr_pkt_ctrl_getf(void)
{
    return hwp_sdio->sdio_fn1_spec.bit_field.wr_pkt_ctrl;
}

__STATIC_INLINE uint8_t sdio_rd_pkt_ctrl_getf(void)
{
    return hwp_sdio->sdio_fn1_spec.bit_field.rd_pkt_ctrl;
}



__STATIC_INLINE uint32_t sdio_sdio_int_ena1_get(void)
{
    return hwp_sdio->sdio_int_ena1.val;
}

__STATIC_INLINE void sdio_sdio_int_ena1_unpack(uint32_t* int_ena)
{
    T_SDIO_SDIO_INT_ENA1 localVal = hwp_sdio->sdio_int_ena1;

    *int_ena = localVal.bit_field.int_ena;
}

__STATIC_INLINE uint32_t sdio_int_ena_getf(void)
{
    return hwp_sdio->sdio_int_ena1.bit_field.int_ena;
}

__STATIC_INLINE uint32_t sdio_sdio_read_fn1_get(void)
{
    return hwp_sdio->sdio_read_fn1.val;
}

__STATIC_INLINE void sdio_sdio_read_fn1_set(uint32_t value)
{
    hwp_sdio->sdio_read_fn1.val = value;
}

__STATIC_INLINE void sdio_sdio_read_fn1_pack(uint32_t read_addr)
{
    hwp_sdio->sdio_read_fn1.val = (((uint32_t)read_addr << 0));
}

__STATIC_INLINE void sdio_sdio_read_fn1_unpack(uint32_t* read_addr)
{
    T_SDIO_SDIO_READ_FN1 localVal = hwp_sdio->sdio_read_fn1;

    *read_addr = localVal.bit_field.read_addr;
}

__STATIC_INLINE uint32_t sdio_read_addr_getf(void)
{
    return hwp_sdio->sdio_read_fn1.bit_field.read_addr;
}

__STATIC_INLINE void sdio_read_addr_setf(uint32_t read_addr)
{
    hwp_sdio->sdio_read_fn1.bit_field.read_addr = read_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_write_fn1_get(void)
{
    return hwp_sdio->sdio_write_fn1.val;
}

__STATIC_INLINE void sdio_sdio_write_fn1_set(uint32_t value)
{
    hwp_sdio->sdio_write_fn1.val = value;
}

__STATIC_INLINE void sdio_sdio_write_fn1_pack(uint32_t write_addr)
{
    hwp_sdio->sdio_write_fn1.val = (((uint32_t)write_addr << 0));
}

__STATIC_INLINE void sdio_sdio_write_fn1_unpack(uint32_t* write_addr)
{
    T_SDIO_SDIO_WRITE_FN1 localVal = hwp_sdio->sdio_write_fn1;

    *write_addr = localVal.bit_field.write_addr;
}

__STATIC_INLINE uint32_t sdio_write_addr_getf(void)
{
    return hwp_sdio->sdio_write_fn1.bit_field.write_addr;
}

__STATIC_INLINE void sdio_write_addr_setf(uint32_t write_addr)
{
    hwp_sdio->sdio_write_fn1.bit_field.write_addr = write_addr;
}

__STATIC_INLINE uint32_t sdio_sdio_ahb_trans_cnt1_get(void)
{
    return hwp_sdio->sdio_ahb_trans_cnt1.val;
}

__STATIC_INLINE void sdio_sdio_ahb_trans_cnt1_set(uint32_t value)
{
    hwp_sdio->sdio_ahb_trans_cnt1.val = value;
}

__STATIC_INLINE void sdio_sdio_ahb_trans_cnt1_pack(uint32_t xfer_cnt)
{
    hwp_sdio->sdio_ahb_trans_cnt1.val = (((uint32_t)xfer_cnt << 0));
}

__STATIC_INLINE void sdio_sdio_ahb_trans_cnt1_unpack(uint32_t* xfer_cnt)
{
    T_SDIO_SDIO_AHB_TRANS_CNT1 localVal = hwp_sdio->sdio_ahb_trans_cnt1;

    *xfer_cnt = localVal.bit_field.xfer_cnt;
}

__STATIC_INLINE uint32_t sdio_xfer_cnt_getf(void)
{
    return hwp_sdio->sdio_ahb_trans_cnt1.bit_field.xfer_cnt;
}

__STATIC_INLINE void sdio_xfer_cnt_setf(uint32_t xfer_cnt)
{
    hwp_sdio->sdio_ahb_trans_cnt1.bit_field.xfer_cnt = xfer_cnt;
}

__STATIC_INLINE uint32_t sdio_sdio_trans_cnt1_get(void)
{
    return hwp_sdio->sdio_trans_cnt1.val;
}

__STATIC_INLINE void sdio_sdio_trans_cnt1_unpack(uint32_t* sdio_xfer_cnt)
{
    T_SDIO_SDIO_TRANS_CNT1 localVal = hwp_sdio->sdio_trans_cnt1;

    *sdio_xfer_cnt = localVal.bit_field.sdio_xfer_cnt;
}

__STATIC_INLINE uint32_t sdio_sdio_xfer_cnt_getf(void)
{
    return hwp_sdio->sdio_trans_cnt1.bit_field.sdio_xfer_cnt;
}

__STATIC_INLINE uint32_t sdio_sdio_ahb_int_sts1_get(void)
{
    return hwp_sdio->sdio_ahb_int_sts1.val;
}
__STATIC_INLINE void sdio_sdio_ahb_int_sts1_set(uint32_t value)
{
    hwp_sdio->sdio_ahb_int_sts1.val = value;
}


__STATIC_INLINE void sdio_sdio_ahb_int_sts1_unpack(uint32_t* status_reg)
{
    T_SDIO_SDIO_AHB_INT_STS1 localVal = hwp_sdio->sdio_ahb_int_sts1;

    *status_reg = localVal.bit_field.status_reg;
}

__STATIC_INLINE uint32_t sdio_status_reg_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts1.bit_field.status_reg;
}

__STATIC_INLINE uint32_t sdio_sdio_ahb_int_sts_en1_get(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.val;
}

__STATIC_INLINE void sdio_sdio_ahb_int_sts_en1_set(uint32_t value)
{
    hwp_sdio->sdio_ahb_int_sts_en1.val = value;
}

__STATIC_INLINE void sdio_sdio_ahb_int_sts_en1_pack(uint8_t fn1_m2s_int, uint8_t fn1_int_en_up, uint8_t fn1_clintrd, uint8_t fn1_rtc_set, uint8_t fn1_status_pcwrt, uint8_t fn1_status_pcrrt, uint8_t fn1_en, uint8_t rst_fn1, uint8_t wr_abort_fn1, uint8_t wr_err_fn1, uint8_t rd_err_fn1, uint8_t fn1_rd_ovr, uint8_t fn1_wr_ovr)
{
    hwp_sdio->sdio_ahb_int_sts_en1.val = (((uint32_t)fn1_m2s_int << 20) | ((uint32_t)fn1_int_en_up << 11) | ((uint32_t)fn1_clintrd << 10) | ((uint32_t)fn1_rtc_set << 9) | ((uint32_t)fn1_status_pcwrt << 8) | ((uint32_t)fn1_status_pcrrt << 7) | ((uint32_t)fn1_en << 6) | ((uint32_t)rst_fn1 << 5) | ((uint32_t)wr_abort_fn1 << 4) | ((uint32_t)wr_err_fn1 << 3) | ((uint32_t)rd_err_fn1 << 2) | ((uint32_t)fn1_rd_ovr << 1) | ((uint32_t)fn1_wr_ovr << 0));
}

__STATIC_INLINE void sdio_sdio_ahb_int_sts_en1_unpack(uint8_t* fn1_m2s_int, uint8_t* fn1_int_en_up, uint8_t* fn1_clintrd, uint8_t* fn1_rtc_set, uint8_t* fn1_status_pcwrt, uint8_t* fn1_status_pcrrt, uint8_t* fn1_en, uint8_t* rst_fn1, uint8_t* wr_abort_fn1, uint8_t* wr_err_fn1, uint8_t* rd_err_fn1, uint8_t* fn1_rd_ovr, uint8_t* fn1_wr_ovr)
{
    T_SDIO_SDIO_AHB_INT_STS_EN1 localVal = hwp_sdio->sdio_ahb_int_sts_en1;

    *fn1_m2s_int = localVal.bit_field.fn1_m2s_int;
    *fn1_int_en_up = localVal.bit_field.fn1_int_en_up;
    *fn1_clintrd = localVal.bit_field.fn1_clintrd;
    *fn1_rtc_set = localVal.bit_field.fn1_rtc_set;
    *fn1_status_pcwrt = localVal.bit_field.fn1_status_pcwrt;
    *fn1_status_pcrrt = localVal.bit_field.fn1_status_pcrrt;
    *fn1_en = localVal.bit_field.fn1_en;
    *rst_fn1 = localVal.bit_field.rst_fn1;
    *wr_abort_fn1 = localVal.bit_field.wr_abort_fn1;
    *wr_err_fn1 = localVal.bit_field.wr_err_fn1;
    *rd_err_fn1 = localVal.bit_field.rd_err_fn1;
    *fn1_rd_ovr = localVal.bit_field.fn1_rd_ovr;
    *fn1_wr_ovr = localVal.bit_field.fn1_wr_ovr;
}

__STATIC_INLINE uint8_t sdio_fn1_m2s_int_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_m2s_int;
}

__STATIC_INLINE uint8_t sdio_fn1_int_en_up_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_int_en_up;
}

__STATIC_INLINE uint8_t sdio_fn1_clintrd_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_clintrd;
}

__STATIC_INLINE uint8_t sdio_fn1_rtc_set_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_rtc_set;
}

__STATIC_INLINE uint8_t sdio_fn1_status_pcwrt_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_status_pcwrt;
}

__STATIC_INLINE uint8_t sdio_fn1_status_pcrrt_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_status_pcrrt;
}

__STATIC_INLINE uint8_t sdio_fn1_en_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_en;
}

__STATIC_INLINE uint8_t sdio_rst_fn1_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.rst_fn1;
}

__STATIC_INLINE uint8_t sdio_wr_abort_fn1_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.wr_abort_fn1;
}

__STATIC_INLINE uint8_t sdio_wr_err_fn1_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.wr_err_fn1;
}

__STATIC_INLINE uint8_t sdio_rd_err_fn1_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.rd_err_fn1;
}

__STATIC_INLINE uint8_t sdio_fn1_rd_ovr_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_rd_ovr;
}

__STATIC_INLINE uint8_t sdio_fn1_wr_ovr_getf(void)
{
    return hwp_sdio->sdio_ahb_int_sts_en1.bit_field.fn1_wr_ovr;
}

__STATIC_INLINE uint32_t sdio_sdio_fbr_reg1_get(void)
{
    return hwp_sdio->sdio_fbr_reg1.val;
}

__STATIC_INLINE void sdio_sdio_fbr_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_fbr_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_fbr_reg1_pack(uint8_t eps, uint8_t fbr1_reg, uint8_t interface_code)
{
    hwp_sdio->sdio_fbr_reg1.val = (((uint32_t)eps << 12) | ((uint32_t)fbr1_reg << 4) | ((uint32_t)interface_code << 0));
}

__STATIC_INLINE void sdio_sdio_fbr_reg1_unpack(uint8_t* eps, uint8_t* fbr1_reg, uint8_t* interface_code)
{
    T_SDIO_SDIO_FBR_REG1 localVal = hwp_sdio->sdio_fbr_reg1;

    *eps = localVal.bit_field.eps;
    *fbr1_reg = localVal.bit_field.fbr1_reg;
    *interface_code = localVal.bit_field.interface_code;
}

__STATIC_INLINE uint8_t sdio_eps_getf(void)
{
    return hwp_sdio->sdio_fbr_reg1.bit_field.eps;
}

__STATIC_INLINE void sdio_eps_setf(uint8_t eps)
{
    hwp_sdio->sdio_fbr_reg1.bit_field.eps = eps;
}

__STATIC_INLINE uint8_t sdio_fbr1_reg_getf(void)
{
    return hwp_sdio->sdio_fbr_reg1.bit_field.fbr1_reg;
}

__STATIC_INLINE void sdio_fbr1_reg_setf(uint8_t fbr1_reg)
{
    hwp_sdio->sdio_fbr_reg1.bit_field.fbr1_reg = fbr1_reg;
}

__STATIC_INLINE uint8_t sdio_interface_code_getf(void)
{
    return hwp_sdio->sdio_fbr_reg1.bit_field.interface_code;
}

__STATIC_INLINE void sdio_interface_code_setf(uint8_t interface_code)
{
    hwp_sdio->sdio_fbr_reg1.bit_field.interface_code = interface_code;
}

__STATIC_INLINE uint32_t sdio_sdio_ior_reg1_get(void)
{
    return hwp_sdio->sdio_ior_reg1.val;
}

__STATIC_INLINE void sdio_sdio_ior_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_ior_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_ior_reg1_pack(uint8_t io_rdy)
{
    hwp_sdio->sdio_ior_reg1.val = (((uint32_t)io_rdy << 0));
}

__STATIC_INLINE void sdio_sdio_ior_reg1_unpack(uint8_t* io_rdy)
{
    T_SDIO_SDIO_IOR_REG1 localVal = hwp_sdio->sdio_ior_reg1;

    *io_rdy = localVal.bit_field.io_rdy;
}

__STATIC_INLINE uint8_t sdio_io_rdy_getf(void)
{
    return hwp_sdio->sdio_ior_reg1.bit_field.io_rdy;
}

__STATIC_INLINE void sdio_io_rdy_setf(uint8_t io_rdy)
{
    hwp_sdio->sdio_ior_reg1.bit_field.io_rdy = io_rdy;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s1_reg1_get(void)
{
    return hwp_sdio->sdio_m2s1_reg1.val;
}

__STATIC_INLINE void sdio_sdio_m2s1_reg1_unpack(uint8_t* m2s1_value)
{
    T_SDIO_SDIO_M2S1_REG1 localVal = hwp_sdio->sdio_m2s1_reg1;

    *m2s1_value = localVal.bit_field.m2s1_value;
}

__STATIC_INLINE uint8_t sdio_m2s1_value_getf(void)
{
    return hwp_sdio->sdio_m2s1_reg1.bit_field.m2s1_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s2_reg1_get(void)
{
    return hwp_sdio->sdio_m2s2_reg1.val;
}

__STATIC_INLINE void sdio_sdio_m2s2_reg1_unpack(uint8_t* m2s2_value)
{
    T_SDIO_SDIO_M2S2_REG1 localVal = hwp_sdio->sdio_m2s2_reg1;

    *m2s2_value = localVal.bit_field.m2s2_value;
}

__STATIC_INLINE uint8_t sdio_m2s2_value_getf(void)
{
    return hwp_sdio->sdio_m2s2_reg1.bit_field.m2s2_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s3_reg1_get(void)
{
    return hwp_sdio->sdio_m2s3_reg1.val;
}

__STATIC_INLINE void sdio_sdio_m2s3_reg1_unpack(uint8_t* m2s3_value)
{
    T_SDIO_SDIO_M2S3_REG1 localVal = hwp_sdio->sdio_m2s3_reg1;

    *m2s3_value = localVal.bit_field.m2s3_value;
}

__STATIC_INLINE uint8_t sdio_m2s3_value_getf(void)
{
    return hwp_sdio->sdio_m2s3_reg1.bit_field.m2s3_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s4_reg1_get(void)
{
    return hwp_sdio->sdio_m2s4_reg1.val;
}

__STATIC_INLINE void sdio_sdio_m2s4_reg1_unpack(uint8_t* m2s4_value)
{
    T_SDIO_SDIO_M2S4_REG1 localVal = hwp_sdio->sdio_m2s4_reg1;

    *m2s4_value = localVal.bit_field.m2s4_value;
}

__STATIC_INLINE uint8_t sdio_m2s4_value_getf(void)
{
    return hwp_sdio->sdio_m2s4_reg1.bit_field.m2s4_value;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m1_reg1_get(void)
{
    return hwp_sdio->sdio_s2m1_reg1.val;
}

__STATIC_INLINE void sdio_sdio_s2m1_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_s2m1_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m1_reg1_pack(uint8_t s2m1_value)
{
    hwp_sdio->sdio_s2m1_reg1.val = (((uint32_t)s2m1_value << 0));
}

__STATIC_INLINE void sdio_sdio_s2m1_reg1_unpack(uint8_t* s2m1_value)
{
    T_SDIO_SDIO_S2M1_REG1 localVal = hwp_sdio->sdio_s2m1_reg1;

    *s2m1_value = localVal.bit_field.s2m1_value;
}

__STATIC_INLINE uint8_t sdio_s2m1_value_getf(void)
{
    return hwp_sdio->sdio_s2m1_reg1.bit_field.s2m1_value;
}

__STATIC_INLINE void sdio_s2m1_value_setf(uint8_t s2m1_value)
{
    hwp_sdio->sdio_s2m1_reg1.bit_field.s2m1_value = s2m1_value;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m2_reg1_get(void)
{
    return hwp_sdio->sdio_s2m2_reg1.val;
}

__STATIC_INLINE void sdio_sdio_s2m2_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_s2m2_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m2_reg1_pack(uint8_t s2m2_value)
{
    hwp_sdio->sdio_s2m2_reg1.val = (((uint32_t)s2m2_value << 0));
}

__STATIC_INLINE void sdio_sdio_s2m2_reg1_unpack(uint8_t* s2m2_value)
{
    T_SDIO_SDIO_S2M2_REG1 localVal = hwp_sdio->sdio_s2m2_reg1;

    *s2m2_value = localVal.bit_field.s2m2_value;
}

__STATIC_INLINE uint8_t sdio_s2m2_value_getf(void)
{
    return hwp_sdio->sdio_s2m2_reg1.bit_field.s2m2_value;
}

__STATIC_INLINE void sdio_s2m2_value_setf(uint8_t s2m2_value)
{
    hwp_sdio->sdio_s2m2_reg1.bit_field.s2m2_value = s2m2_value;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m3_reg1_get(void)
{
    return hwp_sdio->sdio_s2m3_reg1.val;
}

__STATIC_INLINE void sdio_sdio_s2m3_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_s2m3_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m3_reg1_pack(uint8_t s2m3_value)
{
    hwp_sdio->sdio_s2m3_reg1.val = (((uint32_t)s2m3_value << 0));
}

__STATIC_INLINE void sdio_sdio_s2m3_reg1_unpack(uint8_t* s2m3_value)
{
    T_SDIO_SDIO_S2M3_REG1 localVal = hwp_sdio->sdio_s2m3_reg1;

    *s2m3_value = localVal.bit_field.s2m3_value;
}

__STATIC_INLINE uint8_t sdio_s2m3_value_getf(void)
{
    return hwp_sdio->sdio_s2m3_reg1.bit_field.s2m3_value;
}

__STATIC_INLINE void sdio_s2m3_value_setf(uint8_t s2m3_value)
{
    hwp_sdio->sdio_s2m3_reg1.bit_field.s2m3_value = s2m3_value;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m4_reg1_get(void)
{
    return hwp_sdio->sdio_s2m4_reg1.val;
}

__STATIC_INLINE void sdio_sdio_s2m4_reg1_set(uint32_t value)
{
    hwp_sdio->sdio_s2m4_reg1.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m4_reg1_pack(uint8_t s2m4_value)
{
    hwp_sdio->sdio_s2m4_reg1.val = (((uint32_t)s2m4_value << 0));
}

__STATIC_INLINE void sdio_sdio_s2m4_reg1_unpack(uint8_t* s2m4_value)
{
    T_SDIO_SDIO_S2M4_REG1 localVal = hwp_sdio->sdio_s2m4_reg1;

    *s2m4_value = localVal.bit_field.s2m4_value;
}

__STATIC_INLINE uint8_t sdio_s2m4_value_getf(void)
{
    return hwp_sdio->sdio_s2m4_reg1.bit_field.s2m4_value;
}

__STATIC_INLINE void sdio_s2m4_value_setf(uint8_t s2m4_value)
{
    hwp_sdio->sdio_s2m4_reg1.bit_field.s2m4_value = s2m4_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s_sts_ctrl_get(void)
{
    return hwp_sdio->sdio_m2s_sts_ctrl.val;
}

__STATIC_INLINE void sdio_sdio_m2s_sts_ctrl_set(uint32_t value)
{
    hwp_sdio->sdio_m2s_sts_ctrl.val = value;
}

__STATIC_INLINE void sdio_sdio_m2s_sts_ctrl_pack(uint8_t m2s_int_en, uint8_t m2s_value)
{
    hwp_sdio->sdio_m2s_sts_ctrl.val = (((uint32_t)m2s_int_en << 4) | ((uint32_t)m2s_value << 0));
}

__STATIC_INLINE void sdio_sdio_m2s_sts_ctrl_unpack(uint8_t* m2s_int_en)
{
    T_SDIO_SDIO_M2S_STS_CTRL localVal = hwp_sdio->sdio_m2s_sts_ctrl;

    *m2s_int_en = localVal.bit_field.m2s_int_en;
}

__STATIC_INLINE uint8_t sdio_m2s_int_en_getf(void)
{
    return hwp_sdio->sdio_m2s_sts_ctrl.bit_field.m2s_int_en;
}

__STATIC_INLINE void sdio_m2s_int_en_setf(uint8_t m2s_int_en)
{
    hwp_sdio->sdio_m2s_sts_ctrl.bit_field.m2s_int_en = m2s_int_en;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m_sts_ctrl_get(void)
{
    return hwp_sdio->sdio_s2m_sts_ctrl.val;
}

__STATIC_INLINE void sdio_sdio_s2m_sts_ctrl_unpack(uint8_t* s2m_value)
{
    T_SDIO_SDIO_S2M_STS_CTRL localVal = hwp_sdio->sdio_s2m_sts_ctrl;

    *s2m_value = localVal.bit_field.s2m_value;
}

__STATIC_INLINE uint8_t sdio_s2m_value_getf(void)
{
    return hwp_sdio->sdio_s2m_sts_ctrl.bit_field.s2m_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s_sc0_get(void)
{
    return hwp_sdio->sdio_m2s_sc0.val;
}

__STATIC_INLINE void sdio_sdio_m2s_sc0_unpack(uint8_t* m2s_sc0_value)
{
    T_SDIO_SDIO_M2S_SC0 localVal = hwp_sdio->sdio_m2s_sc0;

    *m2s_sc0_value = localVal.bit_field.m2s_sc0_value;
}

__STATIC_INLINE uint8_t sdio_m2s_sc0_value_getf(void)
{
    return hwp_sdio->sdio_m2s_sc0.bit_field.m2s_sc0_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s_sc1_get(void)
{
    return hwp_sdio->sdio_m2s_sc1.val;
}

__STATIC_INLINE void sdio_sdio_m2s_sc1_unpack(uint8_t* m2s_sc1_value)
{
    T_SDIO_SDIO_M2S_SC1 localVal = hwp_sdio->sdio_m2s_sc1;

    *m2s_sc1_value = localVal.bit_field.m2s_sc1_value;
}

__STATIC_INLINE uint8_t sdio_m2s_sc1_value_getf(void)
{
    return hwp_sdio->sdio_m2s_sc1.bit_field.m2s_sc1_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s_sc2_get(void)
{
    return hwp_sdio->sdio_m2s_sc2.val;
}

__STATIC_INLINE void sdio_sdio_m2s_sc2_unpack(uint8_t* m2s_sc2_value)
{
    T_SDIO_SDIO_M2S_SC2 localVal = hwp_sdio->sdio_m2s_sc2;

    *m2s_sc2_value = localVal.bit_field.m2s_sc2_value;
}

__STATIC_INLINE uint8_t sdio_m2s_sc2_value_getf(void)
{
    return hwp_sdio->sdio_m2s_sc2.bit_field.m2s_sc2_value;
}

__STATIC_INLINE uint32_t sdio_sdio_m2s_sc3_get(void)
{
    return hwp_sdio->sdio_m2s_sc3.val;
}

__STATIC_INLINE void sdio_sdio_m2s_sc3_unpack(uint8_t* m2s_sc3_value)
{
    T_SDIO_SDIO_M2S_SC3 localVal = hwp_sdio->sdio_m2s_sc3;

    *m2s_sc3_value = localVal.bit_field.m2s_sc3_value;
}

__STATIC_INLINE uint8_t sdio_m2s_sc3_value_getf(void)
{
    return hwp_sdio->sdio_m2s_sc3.bit_field.m2s_sc3_value;
}

__STATIC_INLINE uint32_t sdio_sdio_s2m_sc0_get(void)
{
    return hwp_sdio->sdio_s2m_sc0.val;
}

__STATIC_INLINE void sdio_sdio_s2m_sc0_set(uint32_t value)
{
    hwp_sdio->sdio_s2m_sc0.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m_sc0_pack(uint8_t s2m_sc0_value)
{
    hwp_sdio->sdio_s2m_sc0.val = (((uint32_t)s2m_sc0_value << 0));
}

__STATIC_INLINE uint32_t sdio_sdio_s2m_sc1_get(void)
{
    return hwp_sdio->sdio_s2m_sc1.val;
}

__STATIC_INLINE void sdio_sdio_s2m_sc1_set(uint32_t value)
{
    hwp_sdio->sdio_s2m_sc1.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m_sc1_pack(uint8_t s2m_sc1_value)
{
    hwp_sdio->sdio_s2m_sc1.val = (((uint32_t)s2m_sc1_value << 0));
}

__STATIC_INLINE uint32_t sdio_sdio_s2m_sc2_get(void)
{
    return hwp_sdio->sdio_s2m_sc2.val;
}

__STATIC_INLINE void sdio_sdio_s2m_sc2_set(uint32_t value)
{
    hwp_sdio->sdio_s2m_sc2.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m_sc2_pack(uint8_t s2m_sc2_value)
{
    hwp_sdio->sdio_s2m_sc2.val = (((uint32_t)s2m_sc2_value << 0));
}

__STATIC_INLINE uint32_t sdio_sdio_s2m_sc3_get(void)
{
    return hwp_sdio->sdio_s2m_sc3.val;
}

__STATIC_INLINE void sdio_sdio_s2m_sc3_set(uint32_t value)
{
    hwp_sdio->sdio_s2m_sc3.val = value;
}

__STATIC_INLINE void sdio_sdio_s2m_sc3_pack(uint8_t s2m_sc3_value)
{
    hwp_sdio->sdio_s2m_sc3.val = (((uint32_t)s2m_sc3_value << 0));
}
#endif


