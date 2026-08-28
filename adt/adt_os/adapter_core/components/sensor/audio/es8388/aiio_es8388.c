/** @brief      ES8388 application interface.
 *
 *  @file       aiio_at24c02.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/08/01      <td>1.0.0       <td>johhn       <td>I2C Init
 *  </table>
 *
 */
#include "aiio_es8388.h"
#include "aiio_os_port.h"
#define ES8388_I2C_PORT AIIO_I2C_0
#define ES8388_I2C_SLAVE_ADDR 0x20

static aiio_es8388_cfg_type_t ES8388Cfg = {
   .work_mode = ES8388_CODEC_MDOE,          /*!< ES8388 work mode */
    .role = ES8388_SLAVE,                    /*!< ES8388 role */
    .mic_input_mode = ES8388_DIFF_ENDED_MIC, /*!< ES8388 mic input mode */
    .mic_pga = ES8388_MIC_PGA_0DB,           /*!< ES8388 mic PGA */
    .i2s_frame = ES8388_LEFT_JUSTIFY_FRAME,  /*!< ES8388 I2S frame */
    .data_width = ES8388_DATA_LEN_16,        /*!< ES8388 I2S dataWitdh */
};


int32_t aiio_es8388_i2c_init(aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    if (aiio_i2c_master_init(ES8388_I2C_PORT, aiio_i2c_cfg) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
int32_t aiio_es8388_write_reg(uint8_t addr, uint8_t data)
{
    if (AIIO_OK != aiio_i2c_master_write(ES8388_I2C_PORT,ES8388_I2C_SLAVE_ADDR,&addr,1,&data,1,1000))
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
int32_t aiio_es8388_read_reg(uint8_t addr, uint8_t *rdata)
{
    if (AIIO_OK != aiio_i2c_master_read(ES8388_I2C_PORT,ES8388_I2C_SLAVE_ADDR, &addr,1,rdata,1,1000))
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

void aiio_es8388_codec_mode(aiio_es8388_cfg_type_t *cfg)
{
    aiio_log_i("aiio_es8388_codec_mode");
    uint8_t tempVal = 0;

    aiio_es8388_write_reg(0x01, 0x58); //power down whole chip analog
    aiio_es8388_write_reg(0x01, 0x50); //power up whole chip analog

    aiio_es8388_write_reg(0x02, 0xf3); //power down ADC/DAC and reset ADC/DAC state machine
    aiio_es8388_write_reg(0x02, 0xf0); //power on   ADC/DAC

    aiio_es8388_write_reg(0x2B, 0x80); //set internal ADC and DAC use sanme LRCK clock

    aiio_es8388_write_reg(0x00, 0x36); //ADC clock is same as DAC . DACMCLK is the chip master clock source

    if (cfg->role == ES8388_MASTER) {
        aiio_es8388_write_reg(0x08, 0x8D);
    } else {
        aiio_es8388_write_reg(0x08, 0x00);
    }

    aiio_es8388_write_reg(0x04, 0x00); //power up dac

    aiio_es8388_write_reg(0x05, 0x00); //stop lowpower mode
    aiio_es8388_write_reg(0x06, 0xc3); //stop lowpower mode

    if (cfg->mic_input_mode == ES8388_SINGLE_ENDED_MIC) {
        /*
         * default set LINPUT1 and RINPUT1 as single ended mic input
         * if user want to use LINPUT2 and RINPUT2 as input
         * please set 0x0a register as 0x50
         */
        aiio_es8388_write_reg(0x0A, 0xf8);
        aiio_es8388_write_reg(0x0B, 0x88); //analog mono mix to left ADC
    } else {
        /*
         * defualt select LIN1 and RIN1 as Mic diff input
         * if user want to use LIN2 and RIN2 as input
         * please set 0x0b register as 0x82
         */
        aiio_es8388_write_reg(0x0A, 0xf8); // Fixed stereo problems
        aiio_es8388_write_reg(0x0B, 0x82); // stereo
    }

    tempVal = cfg->data_width;
    tempVal <<= 2;
    tempVal |= cfg->i2s_frame;

    aiio_es8388_write_reg(0x0C, tempVal); //ADC I2S Format sel as i2s_frame  , data len as data_width

    tempVal = cfg->data_width;
    tempVal <<= 2;
    tempVal |= cfg->i2s_frame;
    tempVal <<= 1;

    aiio_es8388_write_reg(0x17, tempVal); //DAC I2S Format sel as i2s_frame  , data len as data_width

    /* when work in master mode , BCLK is devided form MCLK
     * default divider is 256 , see datasheet reigster 13
     */
    if (cfg->role == ES8388_MASTER) {
        aiio_es8388_write_reg(0x0d, 0x06); //ADCLRCK = MCLK/256
        aiio_es8388_write_reg(0x18, 0x06); //DACLRCK = MCLK/256
    }

    /*set ADC/DAC default volume as 0 db */
    aiio_es8388_write_reg(0x10, 0x00); //LADC volume as 0db
    aiio_es8388_write_reg(0x11, 0x00); //RADC volume as 0db
    aiio_es8388_write_reg(0x1A, 0x00); //LDAC volume as 0db
    aiio_es8388_write_reg(0x1B, 0x00); //RDAC volume as 0db

    tempVal = cfg->mic_pga;
    tempVal <<= 4;
    tempVal |= cfg->mic_pga;

    aiio_es8388_write_reg(0x09, tempVal); //set ADC PGA as mic pga

    /*ADC ALC default Setting */
    aiio_es8388_write_reg(0x12, 0xE2);
    aiio_es8388_write_reg(0x13, 0xC0);
    aiio_es8388_write_reg(0x14, 0x12);
    aiio_es8388_write_reg(0x15, 0x06);
    aiio_es8388_write_reg(0x16, 0xC3);

    /*Mixer setting for LDAC to LOUT and RDAC TO ROUT
     * default close mixer
     */
    aiio_es8388_write_reg(0x27, 0xB8);
    aiio_es8388_write_reg(0x2A, 0xB8);

    aiio_es8388_write_reg(0x02, 0x00); //startup FSM and DLL
    aiio_os_tick_dealy(aiio_os_ms2tick(500));

    /* set LOUT1 ROUT1 LOUT2 ROUT2 volume */
    aiio_es8388_write_reg(0x2e, 0x1E);
    aiio_es8388_write_reg(0x2f, 0x1E);
    aiio_es8388_write_reg(0x30, 0x1E);
    aiio_es8388_write_reg(0x31, 0x1E);

    aiio_es8388_write_reg(0x04,0x3C);//enable LOUT & ROUT
    // aiio_es8388_write_reg(0x04, 0x24);
    // aiio_es8388_write_reg(0x04, 0x18);
    aiio_es8388_write_reg(0x26, 0x01);
    aiio_es8388_write_reg(0x03, 0x09); //power up ADC Enable LIN &RIN.
}

void aiio_es8388_recoding_mode(aiio_es8388_cfg_type_t *cfg)
{
    uint8_t tempVal = 0;
    aiio_es8388_write_reg(0x01, 0x58); //power down whole chip analog
    aiio_es8388_write_reg(0x01, 0x50); //power up whole chip analog

    aiio_es8388_write_reg(0x02, 0xf3); //power down ADC/DAC and reset ADC/DAC state machine
    aiio_es8388_write_reg(0x02, 0xf0); //power on   ADC/DAC
    aiio_es8388_write_reg(0x2B, 0x80); //set internal ADC and DAC use the same LRCK clock
    aiio_es8388_write_reg(0x00, 0x16); //ADC clock is same as DAC. use ADC MCLK as internal MCLK

    if (cfg->role == ES8388_MASTER) {
        aiio_es8388_write_reg(0x08, 0x80);
    } else {
        aiio_es8388_write_reg(0x08, 0x00);
    }

    aiio_es8388_write_reg(0x04, 0xc0);
    aiio_es8388_write_reg(0x05, 0x00);
    aiio_es8388_write_reg(0x06, 0xc3);

    if (cfg->mic_input_mode == ES8388_SINGLE_ENDED_MIC) {
        /*
         * default set LINPUT1 and RINPUT1 as single ended mic input
         * if user want to use LINPUT2 and RINPUT2 as input
         * please set 0x0a register as 0x50
         */
        //aiio_es8388_write_reg(0x0A,0x00);//select lin1 and rin1 as micphone input
        aiio_es8388_write_reg(0x0A, 0xf8);
        aiio_es8388_write_reg(0x0B, 0x88); //analog mono mix to left ADC
        //aiio_es8388_write_reg(0x0B,0x90); //analog mono mix to right ADC
    } else {
        /*
         * defualt select LIN1 and RIN1 as Mic diff input
         * if user want to use LIN2 and RIN2 as input
         * please set 0x0b register as 0x82
         */
        //aiio_es8388_write_reg(0x0A,0xf0);//set micphone input as difference mode
        //aiio_es8388_write_reg(0x0B,0x02);//set LIN1 and RIN1 as micphone different input
        aiio_es8388_write_reg(0x0A, 0xf8); // Fixed stereo problems
        aiio_es8388_write_reg(0x0B, 0x82); // stereo
    }

    tempVal = cfg->data_width;
    tempVal <<= 2;
    tempVal |= cfg->i2s_frame;

    aiio_es8388_write_reg(0x0C, tempVal); //ADC I2S Format sel as i2s_frame  , data len as data_width

    aiio_es8388_write_reg(0x0d, 0x02);    //ADC LRCK = MCLK/256

    aiio_es8388_write_reg(0x10, 0x00);    //ADC VOLUME = 0 DB
    aiio_es8388_write_reg(0x11, 0x00);    //set ADC VOLUME as 0 DB

    tempVal = cfg->mic_pga;
    tempVal <<= 4;
    tempVal |= cfg->mic_pga;

    aiio_es8388_write_reg(0x09, tempVal); //MIC PGA SEL

    aiio_es8388_write_reg(0x12, 0xe2);    //MIC ALC SETTING

    aiio_es8388_write_reg(0x13, 0xc0);
    aiio_es8388_write_reg(0x14, 0x12);
    aiio_es8388_write_reg(0x15, 0x06);
    aiio_es8388_write_reg(0x16, 0xc3);

    aiio_es8388_write_reg(0x02, 0x55); //startup FSM and DLL
    aiio_es8388_write_reg(0x03, 0x09); //power up adc , enable LIN and RIN
}


void aiio_es8388_playback_mode(aiio_es8388_cfg_type_t *cfg)
{
    uint8_t tempVal = 0;

    aiio_es8388_write_reg(0x00, 0x80); //Reset control port register to default
    aiio_es8388_write_reg(0x00, 0x06); //Close reset
    aiio_os_tick_dealy(aiio_os_ms2tick(10));
    aiio_es8388_write_reg(0x02, 0xF3); //Stop STM, DLL and digital block

    if (cfg->role == ES8388_MASTER) {
        aiio_es8388_write_reg(0x08, 0x80);
    } else {
        aiio_es8388_write_reg(0x08, 0x00);
    }

    aiio_es8388_write_reg(0x2B, 0x80); //Set ADC and DAC have the same LRCK
    aiio_es8388_write_reg(0x00, 0x05); //Start up reference
    aiio_es8388_write_reg(0x01, 0x40); //Start up reference
    aiio_os_tick_dealy(aiio_os_ms2tick(30));
    aiio_es8388_write_reg(0x03, 0x00); //Power on ADC and LIN/RIN input
    aiio_es8388_write_reg(0x04, 0x3F); //Power on DAC and LOUT/ROUT input

    /* Set ADC */
    aiio_es8388_write_reg(0x09, 0x77); //MicBoost PGA = +21dB
    aiio_es8388_write_reg(0x0A, 0xF0); //Differential input
    aiio_es8388_write_reg(0x0B, 0x02); //Select LIN1 and RIN1 as differential input pairs

    tempVal = cfg->data_width;
    tempVal <<= 2;
    tempVal |= cfg->i2s_frame;

    aiio_es8388_write_reg(0x0C, tempVal); //ADC I2S Format sel as i2s_frame  , data len as data_width

    aiio_es8388_write_reg(0x0D, 0x04);    //MCLK / LRCK = 512
    aiio_es8388_write_reg(0x10, 0x00);    //LADC volume = 0dB
    aiio_es8388_write_reg(0x11, 0x00);    //RADC volume = 0dB
    aiio_es8388_write_reg(0x12, 0xE2);    //ALC enable, PGA Max.Gain = 23.5dB, Min.Gain = 0dB
    aiio_es8388_write_reg(0x13, 0xC0);    //ALC target = -4.5dB, ALC hold time = 0 ms
    aiio_es8388_write_reg(0x14, 0x12);    //Decay time = 820us, attack time = 416us
    aiio_es8388_write_reg(0x15, 0x06);    //ALC mode
    aiio_es8388_write_reg(0x16, 0xC3);    //Nose gate =-40.5dB, NGG = 0x01(mute ADC)

    tempVal = cfg->data_width;
    tempVal <<= 2;
    tempVal |= cfg->i2s_frame;
    tempVal <<= 1;

    aiio_es8388_write_reg(0x18, 0x04); //MCLK / LRCK = 512
    aiio_es8388_write_reg(0x1A, 0x00); //LDAC volume = 0dB
    aiio_es8388_write_reg(0x1B, 0x00); //RDAC volume = 0dB
    aiio_es8388_write_reg(0x26, 0x00); //Setup mixer
    aiio_es8388_write_reg(0x27, 0xB8); //Setup mixer
    aiio_es8388_write_reg(0x28, 0x38); //Setup mixer
    aiio_es8388_write_reg(0x29, 0x38); //Setup mixer
    aiio_es8388_write_reg(0x2A, 0xB8); //Setup mixer
    aiio_es8388_write_reg(0x2E, 0x1E); //Set Lout/Rout volume:0dB
    aiio_es8388_write_reg(0x2F, 0x1E); //Set Lout/Rout volume:0dB
    aiio_es8388_write_reg(0x30, 0x1E); //Set Lout/Rout volume:0dB
    aiio_es8388_write_reg(0x31, 0x1E); //Set Lout/Rout volume:0dB
    aiio_es8388_write_reg(0x02, 0x00); //Power up DEM and STM
}

int aiio_es8388_set_voice_volume(int volume)
{
    int res;

    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }

    volume /= 3;
    res = aiio_es8388_write_reg(0x2e, volume);
    res |= aiio_es8388_write_reg(0x2f, volume);
    res |= aiio_es8388_write_reg(0x30, volume);
    res |= aiio_es8388_write_reg(0x31, volume);
    return res;
}

void aiio_es8388_reg_dump(void)
{
    int i;
    uint8_t tmp;

    for (i = 0; i < 0X3F; i++) {
        if (aiio_es8388_read_reg(i, &tmp) != 0) {
            printf("iic read err\r\n");
        }

        printf("Reg[%02d]=0x%02x \n", i, tmp);
    }
}


int32_t aiio_es8388_i2s_dma_init(aiio_i2s_pin_cfg_t *aiio_i2s_cfg)
{
    if(aiio_hal_i2s_init(ES8388_I2C_PORT,aiio_i2s_cfg)!= AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}


int32_t aiio_es8388_init(aiio_i2c_pin_cfg_t *aiio_i2c_cfg,aiio_i2s_pin_cfg_t*aiio_i2s_cfg)
{
    aiio_es8388_cfg_type_t *cfg=&ES8388Cfg;   
    if(aiio_es8388_i2c_init(aiio_i2c_cfg)!= AIIO_OK)
    {
        aiio_log_e("aiio_es8388_i2c_init error");
        return AIIO_ERROR;
    }

    switch (cfg->work_mode) {
        case ES8388_CODEC_MDOE:
            aiio_es8388_codec_mode(cfg);
            break;

        case ES8388_RECORDING_MODE:
            aiio_es8388_recoding_mode(cfg);
            break;

        case ES8388_PLAY_BACK_MODE:
            aiio_es8388_playback_mode(cfg);
            break;

        default:
            break;
    }
    aiio_es8388_set_voice_volume(75);
    if(aiio_es8388_i2s_dma_init(aiio_i2s_cfg)!= AIIO_OK)
    {
         aiio_log_e("aiio_es8388_i2s_dma_init error");
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

