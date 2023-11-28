#ifndef __USR_APP_H__
#define __USR_APP_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


//usr config  these 4 macros
#define SLAVE       1   //SLAVE  set 1, will start an advertising
#define MASTER      0   //MASTER set 1, will start a direct connectable initiating requst

#define SERVICE     1   // provide data device ,for example mouse .keyboard ,sensor
#define CLIENT      0   // get data device,for example PC, mobile phone



void creat_usr_app_task(void);
void app_restart_adv(void);
void app_create_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __USR_APP_H__ */


