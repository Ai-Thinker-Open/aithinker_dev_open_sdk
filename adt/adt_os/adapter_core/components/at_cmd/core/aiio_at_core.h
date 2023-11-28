/** @brief      AT core interface.
 *
 *  @file       aiio_at_core.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_CORE_H__
#define __AIIO_AT_CORE_H__

#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_os_port.h"
#include "aiio_at_io.h"

#define AI_COMBO_SDK_VERSION	"1.0.0"

#define AIIO_AT_TRUE                1
#define AIIO_AT_FALSE               0

#ifdef CONFIG_AIIO_AT_TASK_PRIO
    #define AIIO_AT_TASK_PRIO       CONFIG_AIIO_AT_TASK_PRIO
#else
    #define AIIO_AT_TASK_PRIO       15
#endif
#define AIIO_AT_TASK_STACK_SIZE     4096
#define AIIO_AT_TASK_STACK_DELAY_MS 2

#define AIIO_AT_PASSTHROUGH_IDLE_MS 20

#define AIIO_AT_CMD_HEADER          "AT"
#define AIIO_AT_CMD_MAX_PARAS       52
#define AIIO_AT_CMD_MAX_LEN         256
#define AIIO_AT_CMD_TOKEN_LEN       128
#define AIIO_AT_CMD_LIST_NUM        10

#define AIIO_AT_FW_VER_LEN          32

typedef enum
{
    AT_STATE_IDLE,
    AT_STATE_CMD_PROCESS,
    AT_STATE_TRANSPARENT,
} aiio_at_state_t;

typedef enum
{
    AIIO_AT_RET_FAILURE = -1,
    AIIO_AT_RET_SUCCESS = 0,
    AIIO_AT_RET_CUSTOM = 1,
    AIIO_AT_RET_RECVING = 2,
    AIIO_AT_RET_BUSY = 3,
    AIIO_AT_RET_TRANSPARENT = 4
} aiio_at_ret_t;

typedef enum {
    AIIO_AT_CMD_TYPE_QUERY,
    AIIO_AT_CMD_TYPE_EXE,
    AIIO_AT_CMD_TYPE_SET,
    AIIO_AT_CMD_TYPE_TEST,
    AIIO_AT_CMD_TYPE_UNKNOWN
} aiio_at_cmd_type_t;

typedef int32_t (*aiio_at_cmd_fun_cb_t)(int32_t argc, const char **argv, aiio_at_cmd_type_t type);
typedef void (*aiio_at_idle_callback_t)(void);
typedef void (*aiio_at_exit_callback_t)(void);
typedef void (*aiio_at_port_specific_callback_t)(char data);

typedef struct {
    char fw_version[AIIO_AT_FW_VER_LEN];
} aiio_at_init_param_t;

typedef struct {
    int8_t idle_check;
    uint8_t passthrough_cnt;
    uint32_t last_time_ms;
    aiio_at_idle_callback_t idle_callback;
    aiio_at_exit_callback_t exit_callback;
    aiio_at_port_specific_callback_t specific_callback;
} aiio_at_passthrough_t;

typedef struct
{
    int8_t echo;
    int32_t last_errcode;
    aiio_at_state_t at_state;
    aiio_at_passthrough_t passthrough_cfg;
    aiio_at_port_specific_callback_t specific_callback;
    char fw_version[AIIO_AT_FW_VER_LEN];
} aiio_at_ctrl_t;

typedef struct {
    char *cmd_name;
    aiio_at_cmd_fun_cb_t cb;
    const char *help_info;
} aiio_at_cmd_t;

typedef struct
{
    char cmd_name[AIIO_AT_CMD_MAX_LEN];
    int8_t at_cmd_type;
    int16_t at_cmd_len;
    int16_t at_param_cnt;
    uint32_t param_array[AIIO_AT_CMD_MAX_PARAS];
} aiio_at_cmd_attr;

typedef struct {
    aiio_at_cmd_t *at_cmd_list[AIIO_AT_CMD_LIST_NUM];
    uint16_t at_cmd_num[AIIO_AT_CMD_LIST_NUM];
} aiio_at_cmd_list_t;

typedef int32_t (*aiio_at_enum_cmd_cb_t)(const aiio_at_cmd_t *cmd);

/** @brief Register cmd
 *
 *  @param[in]      cmd_tbl          Cmd list.
 *  @param[in]      cmd_num          Cmd list length.
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          successful.
 *  @retval         AIIO_ERROR       error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_register_cmd(const aiio_at_cmd_t *cmd_tbl, uint16_t cmd_num);

/** @brief Enum at cmd
 *
 *  @param[in]      cb               Output data.
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_enum_cmd(aiio_at_enum_cmd_cb_t cb);

/** @brief Set at cmd errcode
 *
 *  @param[in]      errcode          Error code.
 *  @return
 *  @note
 *  @see
 */
CORE_API void aiio_at_set_errcode(uint32_t errcode);

/** @brief Enter specific recv mode
 *
 *  @param[in]      callback         Callback.
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_port_enter_specific(aiio_at_port_specific_callback_t callback);

/** @brief Exit specific recv mode
 *
 *  @note
 *  @see
 */
CORE_API void aiio_at_port_exit_specific();

/** @brief Exit busy state
 *
 *  @note
 *  @see
 */
CORE_API void aiio_at_exit_busy();

/** @brief Enter passthrough mode
 *
 *  @param[in]      cfg              Passthrough config.
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_enter_passthrough(aiio_at_passthrough_t *cfg);

/** @brief Check passthrough mode
 *
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_check_passthrough(void);

/** @brief Set passthrough idle detect
 *
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_passthrough_idle_set(void);

/** @brief Enable AT echo
 *
 *  @note
 *  @see
 */
CORE_API void aiio_at_enable_echo();

/** @brief Disable AT echo
 *
 *  @note
 *  @see
 */
CORE_API void aiio_at_disable_echo();

/** @brief Get fw version
 *
 *  @note
 *  @see
 */
CORE_API char *aiio_at_get_fw_version(void);

/** @brief Init AT
 *
 *  @param[in]      param            Init config.
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_init(aiio_at_init_param_t *param);

/** @brief AT run
 *
 *  @return                          Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK          Successful.
 *  @retval         AIIO_ERROR       Error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_at_run(void);

#endif
