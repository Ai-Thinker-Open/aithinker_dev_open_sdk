/**
 * @file lv_demos.h
 *
 */

#ifndef LV_DEMOS_H
#define LV_DEMOS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_DEMO_WIDGETS
#include "lv_demo_widgets.h"
#endif

#if LV_USE_DEMO_BENCHMARK
#include "lv_demo_benchmark.h"
#endif

#if LV_USE_DEMO_STRESS
#include "lv_demo_stress.h"
#endif

#if LV_USE_DEMO_KEYPAD_AND_ENCODER
#include "lv_demo_keypad_encoder.h"
#endif

#if LV_USE_DEMO_MUSIC
#include "lv_demo_music.h"
#endif

#if LV_USE_DEMO_AIIO_UI
#include "ui.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_H*/
