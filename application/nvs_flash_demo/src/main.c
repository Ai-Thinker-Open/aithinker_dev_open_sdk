#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"


void aiio_main(void *params)
{
    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    const char *def_name = "1234567890123456789012345678901234567890123456789012345678901234";
    uint8_t *data_src = NULL;
    uint8_t *data_buf = NULL;
    uint32_t data_len = 0;
    uint32_t tbuf[] = {1, 1024, 2048};
    uint32_t i, j;

    size_t read_len;
    uint32_t res1, res2;

    aiio_nvs_init();

    for (j = 0; j < sizeof(tbuf)/sizeof(tbuf[0]); j++) {
        data_len = tbuf[j];
        read_len = 0;
        data_src = malloc(data_len + 1);
        data_buf = malloc(data_len + 1);
        if ((NULL == data_src) || (NULL == data_buf)) {
            aiio_log_e("kvbin malloc %d byte error\r\n");
            goto kvbin_exit;
        }

        memset(data_src, 0, data_len + 1);  /* for get string */
        memset(data_buf, 0, data_len + 1);  /* for get string */
        for (i = 0; i < data_len; i++) {
            data_src[i] = 'v';
        }

        /* set */
        res1 = aiio_nvs_set_blob(def_name, data_src, data_len);

        /* get */
        res2 = aiio_nvs_get_blob(def_name, data_buf, data_len, &read_len);
        if ((res1 != 0) || (res2 != data_len)) {
            aiio_log_w("kvbin set/get %ld byte error, res1 = %ld, res2 = %d.\r\n", data_len, res1, res2);
            goto kvbin_exit;
        }

        if (memcmp(data_buf, data_src, data_len) != 0) {
            aiio_log_e("kvbin set/get %ld byte , res1 = %ld, res2 = %d. memcmp error\r\n", data_len, res1, res2);
            goto kvbin_exit;
        }

        aiio_log_i("kvbin set %ld byte bin -> read %ld byte -> memcmp success.\r\n", data_len, data_len);
kvbin_exit:
        if (data_src) {
            free(data_src);
        }
        if (data_buf) {
            free(data_buf);
        }
    }

    aiio_nvs_erase_key(def_name);
    aiio_nvs_erase_all();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

