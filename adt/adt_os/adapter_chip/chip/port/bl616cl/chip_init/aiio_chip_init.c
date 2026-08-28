#include "aiio_type.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_chip_init.h"

#include "bl616cl_glb.h"
#include "bflb_ef_ctrl.h"

static int btblecontroller_em_config(void)
{
    extern uint8_t __LD_CONFIG_EM_SEL;
    volatile uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;

    if (em_size == 0) {
        GLB_Set_EM_Sel(GLB_WRAM160KB_EM0KB);
    } else if (em_size == 16*1024) {
        GLB_Set_EM_Sel(GLB_WRAM144KB_EM16KB);
    } else if (em_size == 32*1024) {
        GLB_Set_EM_Sel(GLB_WRAM128KB_EM32KB);
    } else {
        /* bl616cl max EM is 32KB, use 32KB as default */
        GLB_Set_EM_Sel(GLB_WRAM128KB_EM32KB);
    }

    return 0;
}

int32_t aiio_chip_init(void)
{
	board_init();
    btblecontroller_em_config();
    if (0 != rfparam_init(0, NULL, 0)) {
        printf("PHY RF init failed!\r\n");
        return 0;
    }
    return AIIO_OK;
}

char *aiio_get_chipname(void)
{
    return "BL616CL";
}