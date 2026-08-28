#ifndef _AXK_RADAR_AT_CMD_H_
#define _AXK_RADAR_AT_CMD_H_

#include <stdint.h>

#include "aiio_type.h"
// #include "aiio_chip_spec.h"

typedef enum
{
    RADAR_NO_ONE = 0,
    RADAR_MOVING = 1,
    RADAR_MOTIONLESS = 2,
    RADAR_MOVING_AND_MOTIONLESS = 3,
} aiio_at_radar_body_status_t;

typedef struct
{
    aiio_at_radar_body_status_t status;
    uint16_t distance;
} aiio_at_radar_body_t;

void radar_cmd_ack_cb(uint8_t *data, uint16_t len);
void radar_data_output_cb(uint8_t *data, uint16_t len);
int32_t aiio_radar_at_run(void);
int32_t aiio_at_radar_cmd_regist(void);
void radar_load_nv_param(void);

#endif //_AXK_RADAR_AT_CMD_RECV_H_
