/**
 * @file bflb_ringbuffer.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#include <string.h>
#include "bflb_ringbuffer.h"

/*****************************************************************************
* @brief        init ringbuffer
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    pool        memory pool address
* @param[in]    size        memory size in byte,
*                           must be power of 2 !!!
* 
* @retval int               0:Success -1:Error
*****************************************************************************/
int bflb_ringbuffer_init(bflb_ringbuffer_t *rb, void *pool, uint32_t size)
{
    if (NULL == rb) {
        return -1;
    }

    if (NULL == pool) {
        return -1;
    }

    if ((size < 2) || (size & (size - 1))) {
        return -1;
    }

    rb->in = 0;
    rb->out = 0;
    rb->mask = size - 1;
    rb->pool = pool;

    return 0;
}

/*****************************************************************************
* @brief        reset ringbuffer, clean all data, 
*               should be add lock in multithread
* 
* @param[in]    rb          ringbuffer instance
* 
*****************************************************************************/
void bflb_ringbuffer_reset(bflb_ringbuffer_t *rb)
{
    rb->in = 0;
    rb->out = 0;
}

/*****************************************************************************
* @brief        reset ringbuffer, clean all data,
*               should be add lock in multithread,
*               in single write thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* 
*****************************************************************************/
void bflb_ringbuffer_reset_write(bflb_ringbuffer_t *rb)
{
    rb->in = rb->out;
}

/*****************************************************************************
* @brief        reset ringbuffer, clean all data,
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* 
*****************************************************************************/
void bflb_ringbuffer_reset_read(bflb_ringbuffer_t *rb)
{
    rb->out = rb->in;
}

/*****************************************************************************
* @brief        get ringbuffer total size in byte
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval uint32_t          total size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_get_size(bflb_ringbuffer_t *rb)
{
    return rb->mask + 1;
}

/*****************************************************************************
* @brief        get ringbuffer used size in byte
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval uint32_t          used size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_get_used(bflb_ringbuffer_t *rb)
{
    return rb->in - rb->out;
}

/*****************************************************************************
* @brief        get ringbuffer free size in byte
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval uint32_t          free size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_get_free(bflb_ringbuffer_t *rb)
{
    return (rb->mask + 1) - (rb->in - rb->out);
}

/*****************************************************************************
* @brief        check if ringbuffer is full
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval true              full
* @retval false             not full
*****************************************************************************/
bool bflb_ringbuffer_check_full(bflb_ringbuffer_t *rb)
{
    return bflb_ringbuffer_get_used(rb) > rb->mask;
}

/*****************************************************************************
* @brief        check if ringbuffer is empty
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval true              empty
* @retval false             not empty
*****************************************************************************/
bool bflb_ringbuffer_check_empty(bflb_ringbuffer_t *rb)
{
    return rb->in == rb->out;
}

/*****************************************************************************
* @brief        write one byte to ringbuffer,
*               should be add lock in multithread,
*               in single write thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    byte        data
* 
* @retval true              Success
* @retval false             ringbuffer is full
*****************************************************************************/
bool bflb_ringbuffer_write_byte(bflb_ringbuffer_t *rb, uint8_t byte)
{
    if (bflb_ringbuffer_check_full(rb)) {
        return false;
    }

    ((uint8_t *)(rb->pool))[rb->in & rb->mask] = byte;
    rb->in++;
    return true;
}

/*****************************************************************************
* @brief        overwrite one byte to ringbuffer, drop oldest data,
*               should be add lock always
*
* @param[in]    rb          ringbuffer instance
* @param[in]    byte        data
* 
* @retval true              Success
* @retval false             always return true
*****************************************************************************/
bool bflb_ringbuffer_overwrite_byte(bflb_ringbuffer_t *rb, uint8_t byte)
{
    if (bflb_ringbuffer_check_full(rb)) {
        rb->out++;
    }

    ((uint8_t *)(rb->pool))[rb->in & rb->mask] = byte;
    rb->in++;
    return true;
}

/*****************************************************************************
* @brief        peek one byte from ringbuffer,
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    byte        pointer to save data
* 
* @retval true              Success
* @retval false             ringbuffer is empty
*****************************************************************************/
bool bflb_ringbuffer_peek_byte(bflb_ringbuffer_t *rb, uint8_t *byte)
{
    if (bflb_ringbuffer_check_empty(rb)) {
        return false;
    }

    *byte = ((uint8_t *)(rb->pool))[rb->out & rb->mask];
    return true;
}

/*****************************************************************************
* @brief        read one byte from ringbuffer,
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    byte        pointer to save data
* 
* @retval true              Success
* @retval false             ringbuffer is empty
*****************************************************************************/
bool bflb_ringbuffer_read_byte(bflb_ringbuffer_t *rb, uint8_t *byte)
{
    bool ret;
    ret = bflb_ringbuffer_peek_byte(rb, byte);
    rb->out += ret;
    return ret;
}

/*****************************************************************************
* @brief        drop one byte from ringbuffer,
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* 
* @retval true              Success
* @retval false             ringbuffer is empty
*****************************************************************************/
bool bflb_ringbuffer_drop_byte(bflb_ringbuffer_t *rb)
{
    if (bflb_ringbuffer_check_empty(rb)) {
        return false;
    }

    rb->out += 1;
    return true;
}

/*****************************************************************************
* @brief        write data to ringbuffer,
*               should be add lock in multithread,
*               in single write thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    data        data pointer
* @param[in]    size        size in byte
* 
* @retval uint32_t          actual write size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_write(bflb_ringbuffer_t *rb, void *data, uint32_t size)
{
    uint32_t unused;
    uint32_t offset;
    uint32_t remain;

    unused = (rb->mask + 1) - (rb->in - rb->out);

    if (size > unused) {
        size = unused;
    }

    offset = rb->in & rb->mask;

    remain = rb->mask + 1 - offset;
    remain = remain > size ? size : remain;

    memcpy(((uint8_t *)(rb->pool)) + offset, data, remain);
    memcpy(rb->pool, (uint8_t *)data + remain, size - remain);

    rb->in += size;

    return size;
}

/*****************************************************************************
* @brief        write data to ringbuffer,
*               should be add lock always
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    data        data pointer
* @param[in]    size        size in byte
* 
* @retval uint32_t          actual write size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_overwrite(bflb_ringbuffer_t *rb, void *data, uint32_t size)
{
    uint32_t unused;
    uint32_t offset;
    uint32_t remain;

    unused = (rb->mask + 1) - (rb->in - rb->out);

    if (size > unused) {
        if (size > (rb->mask + 1)) {
            size = rb->mask + 1;
        }

        rb->out += size - unused;
    }

    offset = rb->in & rb->mask;

    remain = rb->mask + 1 - offset;
    remain = remain > size ? size : remain;

    memcpy(((uint8_t *)(rb->pool)) + offset, data, remain);
    memcpy(rb->pool, (uint8_t *)data + remain, size - remain);

    rb->in += size;

    return size;
}

/*****************************************************************************
* @brief        peek data from ringbuffer
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    data        data pointer
* @param[in]    size        size in byte
* 
* @retval uint32_t          actual peek size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_peek(bflb_ringbuffer_t *rb, void *data, uint32_t size)
{
    uint32_t used;
    uint32_t offset;
    uint32_t remain;

    used = rb->in - rb->out;
    if (size > used) {
        size = used;
    }

    offset = rb->out & rb->mask;

    remain = rb->mask + 1 - offset;
    remain = remain > size ? size : remain;

    memcpy(data, ((uint8_t *)(rb->pool)) + offset, remain);
    memcpy((uint8_t *)data + remain, rb->pool, size - remain);

    return size;
}

/*****************************************************************************
* @brief        read data from ringbuffer
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    data        data pointer
* @param[in]    size        size in byte
* 
* @retval uint32_t          actual read size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_read(bflb_ringbuffer_t *rb, void *data, uint32_t size)
{
    size = bflb_ringbuffer_peek(rb, data, size);
    rb->out += size;
    return size;
}

/*****************************************************************************
* @brief        drop data from ringbuffer
*               should be add lock in multithread,
*               in single read thread not need lock
* 
* @param[in]    rb          ringbuffer instance
* @param[in]    size        size in byte
* 
* @retval uint32_t          actual drop size in byte
*****************************************************************************/
uint32_t bflb_ringbuffer_drop(bflb_ringbuffer_t *rb, uint32_t size)
{
    uint32_t used;

    used = rb->in - rb->out;
    if (size > used) {
        size = used;
    }

    rb->out += size;
    return size;
}