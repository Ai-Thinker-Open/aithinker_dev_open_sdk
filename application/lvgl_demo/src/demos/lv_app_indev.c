/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "chsc6540_i2c.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#if 0
static void mouse_init(void);
static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);
#endif

#if 0
static void keypad_init(void);
static bool keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);
#endif

#if 0
static void encoder_init(void);
static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void encoder_handler(void);
#endif

#if 0
static void button_init(void);
static bool button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *indev_touchpad;
// lv_indev_t *indev_mouse;
// lv_indev_t *indev_keypad;
// lv_indev_t *indev_encoder;
// lv_indev_t *indev_button;

// static int32_t encoder_diff;
// static lv_indev_state_t encoder_state;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_app_indev_init(void)
{
    /* Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    static lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_touchpad = lv_indev_drv_register(&indev_drv);

#if 1
    lv_obj_t *touchpad_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(touchpad_cursor, LV_SYMBOL_EDIT);
    lv_obj_clear_flag(touchpad_cursor, LV_OBJ_FLAG_CLICKABLE);
    lv_indev_set_cursor(indev_touchpad, touchpad_cursor);
#endif

    /*------------------
     * Mouse
     * -----------------*/
#if 0
    /*Initialize your touchpad if you have*/
    mouse_init();

    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t *mouse_cursor = lv_img_create(lv_disp_get_scr_act(NULL), NULL);
    lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);
#endif

    /*------------------
     * Keypad
     * -----------------*/
#if 0
    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);
#endif

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_keypad, group);` */

    /*------------------
     * Encoder
     * -----------------*/
#if 0
    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    indev_encoder = lv_indev_drv_register(&indev_drv);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_encoder, group);` */
#endif

    /*------------------
     * Button
     * -----------------*/
#if 0
    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev_drv.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[2] = {
        { 10, 10 },  /*Button 0 -> x:10; y:10*/
        { 40, 100 }, /*Button 1 -> x:40; y:100*/
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    chsc6540_i2c_init();
}

/* Will be called by the library to read the touchpad */
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    uint8_t point_num = 0;
    touch_coord_t touch_coord;

    chsc6540_i2c_read(&point_num, &touch_coord, 1);

    /*Save the pressed coordinates and the state*/
    if (point_num) {
        last_x = touch_coord.coord_x;
        last_y = touch_coord.coord_y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

/*Return true is the touchpad is pressed*/
// static bool touchpad_is_pressed(void)
// {
//     /*Your code comes here*/
//     return false;
// }

/*Get the x and y coordinates if the touchpad is pressed*/
// static lv_coord_t touchpad_get_xy(lv_coord_t *x, lv_coord_t *y)
// {
//     /*Your code comes here*/
//     lv_disp_t *p_disp_drv_cb;
//     uint8_t res;

//     p_disp_drv_cb = lv_disp_get_default();

//     while (p_disp_drv_cb->driver->draw_buf->flushing)
//         ;

//     uint32_t spi_clock = SPI_DEV(touch_spi)->clk;

//     device_control(touch_spi, DEVICE_CTRL_SPI_CONFIG_CLOCK, (void *)(uintptr_t)360000);
//     res = touch_read(x, y);
//     device_control(touch_spi, DEVICE_CTRL_SPI_CONFIG_CLOCK, (void *)(uintptr_t)spi_clock);
//     return res;
// }

/*------------------
 * Mouse
 * -----------------*/

/* Initialize your mouse */
// static void mouse_init(void)
// {
//     /*Your code comes here*/
// }

/* Will be called by the library to read the mouse */
// static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
// {
//     /*Get the current x and y coordinates*/
//     mouse_get_xy(&data->point.x, &data->point.y);

//     /*Get whether the mouse button is pressed or released*/
//     if(mouse_is_pressed()) {
//         data->state = LV_INDEV_STATE_PR;
//     } else {
//         data->state = LV_INDEV_STATE_REL;
//     }

//     /*Return `false` because we are not buffering and no more data to read*/
//     return false;
// }

/*Return true is the mouse button is pressed*/
// static bool mouse_is_pressed(void)
// {
//     /*Your code comes here*/

//     return false;
// }

/*Get the x and y coordinates if the mouse is pressed*/
// static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
// {
//     /*Your code comes here*/

//     (*x) = 0;
//     (*y) = 0;
// }

/*------------------
 * Keypad
 * -----------------*/

/*------------------
 * Encoder
 * -----------------*/

/* Initialize your keypad */
// static void encoder_init(void)
// {
//     /*Your code comes here*/
// }

/* Will be called by the library to read the encoder */
// static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
// {

//     data->enc_diff = encoder_diff;
//     data->state = encoder_state;

//     /*Return `false` because we are not buffering and no more data to read*/
//     return false;
// }

/*Call this function in an interrupt to process encoder events (turn, press)*/
// static void encoder_handler(void)
// {
//     /*Your code comes here*/

//     encoder_diff += 0;
//     encoder_state = LV_INDEV_STATE_REL;
// }

/*------------------
 * Button
 * -----------------*/

/* Initialize your buttons */
// static void button_init(void)
// {
//     /*Your code comes here*/
// }

/* Will be called by the library to read the button */
// static bool button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
// {

//     static uint8_t last_btn = 0;

//     /*Get the pressed button's ID*/
//     int8_t btn_act = button_get_pressed_id();

//     if(btn_act >= 0) {
//         data->state = LV_INDEV_STATE_PR;
//         last_btn = btn_act;
//     } else {
//         data->state = LV_INDEV_STATE_REL;
//     }

//     /*Save the last pressed button's ID*/
//     data->btn_id = last_btn;

//     /*Return `false` because we are not buffering and no more data to read*/
//     return false;
// }

/*Get ID  (0, 1, 2 ..) of the pressed button*/
// static int8_t button_get_pressed_id(void)
// {
//     uint8_t i;

//     /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
//     for(i = 0; i < 2; i++) {
//         /*Return the pressed button's ID*/
//         if(button_is_pressed(i)) {
//             return i;
//         }
//     }

//     /*No button pressed*/
//     return -1;
// }

/*Test if `id` button is pressed or not*/
// static bool button_is_pressed(uint8_t id)
// {

//     /*Your code comes here*/

//     return false;
// }

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
