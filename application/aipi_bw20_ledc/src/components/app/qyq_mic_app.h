#ifndef __QYQ_MIC_APP_H_
#define __QYQ_MIC_APP_H_
#include "chip_include.h"
#include "app_include.h"
#include "drive_include.h"
#include "interface_include.h"
#include "algorithm_include.h"
#ifdef __QYQ_MIC_APP_C_
#define QYQ_MIC_APP_EXT
#else
#define QYQ_MIC_APP_EXT  extern
#endif

QYQ_MIC_APP_EXT void qyq_mic_app_task(void *para);
#endif
