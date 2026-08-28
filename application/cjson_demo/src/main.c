#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "cJSON.h"

void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_assert(1);
    // 创建一个JSON对象
    cJSON *root = cJSON_CreateObject();

    if (root == NULL)
    {
        aiio_log_a("创建JSON对象失败！\n");
        return;
    }

    // 添加一个浮点值到JSON对象中
    double myFloatValue = 123.456;
    cJSON_AddNumberToObject(root, "myFloatKey", myFloatValue);

    // 打印JSON对象
    char *jsonString = cJSON_Print(root);
    if (jsonString == NULL)
    {
        aiio_log_a("JSON对象转换为字符串失败！\n");
        cJSON_Delete(root);
        return;
    }

    aiio_log_a("%s\n", jsonString);

    // 释放内存
    free(jsonString);
    cJSON_Delete(root);
    aiio_assert(0);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
