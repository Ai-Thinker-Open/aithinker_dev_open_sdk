/**
 * @brief      AES128 fill and cut interface adaptation.
 *
 * @file       aiio_aes_utils_filling.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       AES128 fill and cut interface adaptation.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2023/02/23      <td>1.0.0       <td>yanch       <td>AES128 filling api define
 * </table>
 *
 */
#include "aiio_utils_filling.h"

int pkcs7_padding(char *data, int dataSize, int dataLen)
{
    if (dataLen <= 0)
    {
        return -1;
    }

    if ((dataLen + 17) >= dataSize)
    {
        return -2;
    }

    uint8_t paddingNum = 16 - (dataLen % 16);

    memset(&data[dataLen], paddingNum, paddingNum);
    data[dataLen + paddingNum] = '\0';

    return dataLen + paddingNum;
}

int pkcs7_cuttingg(char *data, int dataLen)
{
    if (dataLen <= 0)
    {
        return -1;
    }

    uint8_t paddingNum = data[dataLen - 1];

    // check
    for (int i = 0; i < paddingNum; i++)
    {
        if (data[dataLen - paddingNum + i] != paddingNum)
            return -3;
    }

    memset(&data[dataLen - paddingNum], 0, paddingNum);
    data[dataLen - paddingNum] = '\0';

    return dataLen - paddingNum;
}
