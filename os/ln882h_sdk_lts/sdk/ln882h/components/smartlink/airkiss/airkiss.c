/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author           Notes
 * 2019-05-16     heyuanjie87      first version
 * 2020-07-10     wuliang
*/

#include "airkiss_port.h"
#include "airkiss.h"
#include <stdint.h>
#include <stddef.h>

#define AK_LOG_D(...)                                    \
        do {                                             \
            if (local_context->cfg->printf) {            \
                local_context->cfg->printf(__VA_ARGS__); \
            }                                            \
        } while (0)

//#define AIRKISS_GET_DATA_LOG_ENABLE
//#define AIRKISS_LOG_RIO_ENABLE
//#define AIRKISS_LOG_DFDUMP_ENABLE


#define AKMEMCPY(dst, src, len)         local_context->cfg->memcpy(dst, src, len)
#define AKMEMSET(ptr, value, len)       local_context->cfg->memset(ptr, value, len)
#define AKMEMCMP(ptr1, ptr2, len)       local_context->cfg->memcmp(ptr1, ptr2, len)

#define AKSTATE_WFG 0
#define AKSTATE_WFM 1
#define AKSTATE_WFP 2
#define AKSTATE_WFD 4
#define AKSTATE_CMP 5

typedef uint16_t akwire_seq_t;

typedef struct
{
    uint16_t val[6];
    uint8_t pos;
    uint8_t scnt; /* 成功计数 */
    uint8_t err;
    uint8_t rcnt; /* 接收计数 */
} akcode_t;

typedef struct
{
    akwire_seq_t ws;
    uint8_t crc;
    uint8_t ind;
}akdatf_seq_t;

typedef struct
{
    uint8_t crc;
    uint8_t ind;
}akdatf_header_t;

typedef struct
{
    uint8_t id[4];
}akaddr_t;

typedef struct
{
    akwire_seq_t ws;
    uint8_t val[4];
    uint8_t pos : 4;
    uint8_t scnt : 4;
    uint8_t err;
    uint8_t wval;
    uint8_t seqcnt;
    akaddr_t sa;
} akcode_guide_t;

typedef struct
{
    char data[16]; /* 保留后4个 */
    uint8_t pos[16];
}akdatf_conflict_t;

typedef struct
{
    union {
        akcode_guide_t code1[3];
        akcode_t code2[1];
    } uc;

    akdatf_conflict_t dcfl; /* 记录有冲突的数据 */
    akwire_seq_t prews;
    akdatf_seq_t preseq;
    akdatf_seq_t curseq;
    akdatf_header_t pendseq[10]; /* 未完成的序列 */
    akaddr_t locked;

    uint8_t seqstep;/* 序列增量 */
    uint8_t reclen;
    uint8_t state;
    uint8_t seq[16]; /* 标记已完成的序列 */

    char data[66];
    uint8_t random;
    uint8_t baselen;
    uint8_t prslen;
    uint8_t ssidcrc;
    uint8_t pwdlen;
    const airkiss_config_t *cfg;
} aklocal_context_t;

typedef struct {
    unsigned    frame_type :16;
    unsigned    duration_id:16;
    uint8_t     i_addr1[6];
    uint8_t     i_addr2[6];
    uint8_t     i_addr3[6];
    unsigned    sequence:16;
} ieee80211_frame_t;

#define AKLOC_CODE1(x, i)               ((x)->uc.code1[i])
#define AKLOC_CODE2(x)                  (&(x)->uc.code2[0])

#define AKLOC_DFSEQ_PREV(local_context) ((local_context)->preseq)
#define AKLOC_DFSEQ_CUR(local_context)  ((local_context)->curseq)

uint8_t airkiss_crc8(uint8_t *message, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t i;

    while (len--)
    {
        crc ^= *message++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0x8c;
            else
                crc >>= 1;
        }
    }

    return crc;
}

static akwire_seq_t akwseq_make(uint8_t seq[2])
{
    akwire_seq_t ws = 0;
    ws = (seq[1] << 4) | (seq[0] >> 4);
    return ws;
}

static uint8_t akinfo_getu8(uint16_t v[2])
{
    uint8_t ret = 0;
    ret = ((v[0] & 0xF) << 4) | (v[1] & 0xF);
    return ret;
}

static uint16_t aklen_udp(aklocal_context_t *local_context, uint16_t len)
{
    return (len - local_context->baselen);
}

static int ak_get_magicfield(aklocal_context_t *local_context, akcode_t *ac)
{
    int ret = 1;

    if (ac->val[0] == 8)
        ac->val[0] = 0;
    local_context->prslen = akinfo_getu8(&ac->val[0]);
    local_context->ssidcrc = akinfo_getu8(&ac->val[2]);

    if (local_context->prslen > (sizeof(local_context->data) - 1))
    {
        ret = 0;
        AK_LOG_D("prslen(%d) large than(%d)\r\n", local_context->prslen, (sizeof(local_context->data) - 1));
    }

    return ret;
}

static int ak_magicfield_input(akcode_t *ac, uint16_t len)
{
    int mc;

    mc = len >> 4;
    if (mc == 0)
    {
        ac->val[0] = len;
        ac->pos = 1;
    }
    else if (mc == ac->pos)
    {
        ac->val[ac->pos] = len;
        ac->pos ++;
    }
    else
    {
        ac->pos = 0;
    }

    return (ac->pos == 4);
}

static int ak_get_prefixfield(aklocal_context_t *local_context, akcode_t *ac)
{
    int ret = 1;
    uint8_t crc;

    local_context->pwdlen = akinfo_getu8(&ac->val[0]);
    crc = akinfo_getu8(&ac->val[2]);
    if (airkiss_crc8(&local_context->pwdlen, 1) != crc)
        ret = 0;

    return ret;
}

static int ak_prefixfield_input(akcode_t *ac, uint16_t len)
{
    int mc;

    mc = len >> 4;
    if (mc == 4)
    {
        ac->val[0] = len;
        ac->pos = 1;
    }
    else if (mc == (ac->pos + 4))
    {
        ac->val[ac->pos] = len;
        ac->pos ++;
    }
    else
    {
        ac->pos = 0;
    }

    return (ac->pos == 4);
}

static int ak_get_datafield(aklocal_context_t *local_context, akcode_t *ac)
{
    uint8_t tmp[6];
    int n;
    int ret = 0;
    int pos;
    int seqi;

    seqi = ac->val[1] & 0x7f;
    if (seqi > (local_context->prslen/4))
    {
        return 0;
    }

    if (local_context->seq[seqi])
        return 0;

    pos = seqi * 4;
    n = local_context->prslen - pos;
    if (n > 4)
        n = 4;

    tmp[0] = ac->val[0] & 0x7F;
    tmp[1] = ac->val[1] & 0x7F;
    tmp[2] = ac->val[2] & 0xFF;
    tmp[3] = ac->val[3] & 0xFF;
    tmp[4] = ac->val[4] & 0xFF;
    tmp[5] = ac->val[5] & 0xFF;

    ret = ((airkiss_crc8(&tmp[1], n + 1) & 0x7F) == tmp[0]);
    if (ret)
    {
        AKMEMCPY(&local_context->data[pos], &tmp[2], n);
        local_context->reclen += n;
        local_context->seq[seqi] = 1;

#ifdef AIRKISS_GET_DATA_LOG_ENABLE
        AK_LOG_D("getdata(%d, %d)\r\n", seqi, n);
#endif
    }

    return ret;
}

static void akaddr_fromframe(akaddr_t *a, uint8_t *f)
{
    ieee80211_frame_t * frame = (ieee80211_frame_t *)f;

    a->id[0] = frame->i_addr2[4];
    a->id[1] = frame->i_addr2[5];
    a->id[2] = frame->i_addr3[4];
    a->id[3] = frame->i_addr3[5];
}

static akcode_guide_t *ak_guide_getcode(aklocal_context_t *local_context, uint8_t *f)
{
    akcode_guide_t *ac;

    if (f != NULL)
    {
        akaddr_t sa;
        uint8_t i;
        int found = 0;
        akcode_guide_t *imin;

        akaddr_fromframe(&sa, f);
        imin = &AKLOC_CODE1(local_context, 0);
        ac = imin;
        for (i = 0; i < sizeof(local_context->uc.code1) / sizeof(local_context->uc.code1[0]); i++)
        {
            /* 匹配地址 */
            found = !AKMEMCMP(&sa, &ac->sa, sizeof(ac->sa));
            if (found)
                break;
            /* 记录权值最小的 */
            if (ac->wval < imin->wval)
                imin = ac;
            ac++;
        }

        if (!found)
        {
            /* 淘汰输入最少的 */
            ac = imin;
            ac->pos = 0;
            ac->err = 0;
            ac->scnt = 0;
            ac->wval = 0;
            ac->sa = sa;
        }
    }

    return ac;
}

static int ak_guidefield_input(akcode_guide_t *ac, uint8_t *f, uint16_t len)
{
    akwire_seq_t ws = 0;

    if (f)
        ws = akwseq_make(f + 22);

    if (ac->pos < 4)
    {
        if ((ac->pos != 0) && ((len - ac->val[ac->pos - 1]) != 1))
        {
            ac->pos = 0;
            if (ac->wval > 0)
                ac->wval--;
        }

        if (ac->pos == 0)
        {
            ac->ws = ws;
            ac->seqcnt = 0;
        }
        ac->seqcnt += (ws - ac->ws);

        ac->val[ac->pos] = len;
        ac->pos++;
        ac->wval += ac->pos;
    }

    return (ac->pos == 4);
}

static int ak_waitfor_guidefield(aklocal_context_t *local_context, uint8_t *f, uint16_t len)
{
    int ret = AIRKISS_STATUS_CONTINUE;
    akcode_guide_t *ac;

    ac = ak_guide_getcode(local_context, f);

    if (ak_guidefield_input(ac, f, len))
    {
        ac->pos = 0;
        ac->scnt++;

        /* 至少两次相同的guide code才算获取成功 */
        if ((ac->scnt >= 2) && ac->wval >= 20)
        {
            local_context->state = AKSTATE_WFM;
            local_context->baselen = ac->val[0] - 1;
            local_context->seqstep = ac->seqcnt/6;

            AK_LOG_D("guide baselen(%d) seqstep(%d)\r\n", local_context->baselen, local_context->seqstep);
        }
        else
        {
            AK_LOG_D("ac->scnt(%d) ac->wval(%d)\r\n", ac->scnt, ac->wval);
        }

        if (local_context->state == AKSTATE_WFM)
        {
            local_context->locked = ac->sa;
            AKMEMSET(&local_context->uc, 0, sizeof(local_context->uc));
            ret = AIRKISS_STATUS_CHANNEL_LOCKED;
        }
    }

    return ret;
}

static void akloc_reset(aklocal_context_t *local_context)
{
    const airkiss_config_t *cfg;

    cfg = local_context->cfg;
    AKMEMSET(local_context, 0, sizeof(*local_context));
    local_context->cfg = cfg;
}

static int ak_waitfor_magicfield(aklocal_context_t *local_context, uint16_t len)
{
    int ret = AIRKISS_STATUS_CONTINUE;
    akcode_t *ac = AKLOC_CODE2(local_context);
    int udplen;

    udplen = aklen_udp(local_context, len);

    if (ak_magicfield_input(ac, udplen))
    {
        ac->pos = 0;

        if (ak_get_magicfield(local_context, ac))
        {
            local_context->state = AKSTATE_WFP;
            AK_LOG_D("magic: prslen(%d) ssidcrc(%X)\r\n", local_context->prslen, local_context->ssidcrc);
        }
    }

    if (ac->rcnt++ > 250)
    {
        akloc_reset(local_context);
        AK_LOG_D("reset from magic\r\n");
    }

    return ret;
}

static int ak_waitfor_prefixfield(aklocal_context_t *local_context, uint16_t len)
{
    int ret = AIRKISS_STATUS_CONTINUE;
    akcode_t *ac = AKLOC_CODE2(local_context);
    int udplen;

    udplen = aklen_udp(local_context, len);

    if (ak_prefixfield_input(ac, udplen))
    {
        ac->pos = 0;

        if (ak_get_prefixfield(local_context, ac))
        {
            local_context->state = AKSTATE_WFD;
            AK_LOG_D("prefix: pwdlen(%d)\r\n", local_context->pwdlen);
        }
    }

    return ret;
}

#ifdef AIRKISS_LOG_DFDUMP_ENABLE
static void akdata_dump(aklocal_context_t *local_context, uint8_t *f, uint16_t len)
{
    uint8_t seq[2];
    uint16_t dseq;

    seq[0] = f[22];
    seq[1] = f[23];

    dseq = (seq[1] << 4) | (seq[0]>> 4);
    if (len & 0x100) {
        AK_LOG_D("(%d) %X %c\r\n", dseq, len, len & 0xff);
    } else {
        AK_LOG_D("(%d) %X\r\n", dseq, len);
    }
}
#endif

/*
  只判断密码和random是否收完
*/
static int ak_is_pwdrand_complete(aklocal_context_t *local_context)
{
    int ret = 0;
    unsigned i;
    int n = 0;

    for (i = 0; i < (sizeof(local_context->seq) / sizeof(local_context->seq[0])); i++)
    {
        if (local_context->seq[i] == 0)
            break;

        n += 4;
        if (n >= (local_context->pwdlen + 1))
        {
            ret = 1;
            break;
        }
    }

    return ret;
}

static int ak_datainput_onlylength(aklocal_context_t *local_context, akcode_t *ac, uint16_t len)
{
    int n = 6;

    if (len & 0x100)
    {
        if (ac->pos > 1)
        {
            int size;

            ac->val[ac->pos] = len;
            ac->pos ++;

            size = (ac->val[1] & 0x7f) * 4;
            if (size <  local_context->prslen)
            {
                size = local_context->prslen - size;
                if (size < 4) /* 最后一个包不足4 */
                {
                    n = size + 2;
                }
            }
        }
        else
        {
            ac->pos = 0;
        }
    }
    else
    {
        if (ac->pos < 2) {
            ac->val[ac->pos] = len;
            ac->pos ++;
        } else {
            ac->val[0] = len;
            ac->pos = 1;
        }
    }

    return (ac->pos == n);
}

static akdatf_header_t* akseq_getpend(aklocal_context_t *local_context, uint8_t ind)
{
    akdatf_header_t* ret = 0;
    unsigned i;

    for (i = 0; i < sizeof(local_context->pendseq)/sizeof(local_context->pendseq[0]); i ++)
    {
        akdatf_header_t *p = &local_context->pendseq[i];

        if (p->ind == ind)
        {
            ret = p;
            break;
        }
    }

    return ret;
}

static int ak_pendinput_mark(aklocal_context_t *local_context, uint8_t ind)
{
    int ret = 0;
    akdatf_header_t* pd;

    pd = akseq_getpend(local_context, ind);
    if (pd)
    {
        int size, pos, i;
        char d[6] = {0};
        uint8_t crc;

        ind = ind & 0x7f;
        pos = ind * 4;
        size = local_context->prslen - pos;
        if (size > 4)
            size = 4;

        for (i = 0; i < size; i ++)
        {
            if (local_context->data[pos + i] == 0)
                return 0;
        }

        d[0] = ind;
        AKMEMCPY(&d[1], &local_context->data[pos], size);
        crc = airkiss_crc8((uint8_t*)d, size + 1) & 0x7f;
        if (crc == (pd->crc & 0x7f))
        {
            AKMEMSET(pd, 0, sizeof(*pd));
            local_context->seq[ind] = 1;
            local_context->reclen += size;
            ret = 1;

#ifdef AIRKISS_GET_DATA_LOG_ENABLE
            AK_LOG_D("getdata-p(%d, %d)[%s]\r\n", ind, size, &d[1]);
#endif
        }
    }

    return ret;
}

static int ak_penddata_getpos(aklocal_context_t *local_context, akdatf_seq_t *ref, akwire_seq_t ws)
{
    int ret = -1;
    uint8_t refind, ind;
    int offs;

    if (ws < ref->ws)
    {//todo
        AK_LOG_D("ws-d overflow(%d, %d)\r\n", ws, ref->ws);
    }
    else
    {
        int maxoffs;
        int fmpos;

        offs = (ws - ref->ws)/local_context->seqstep;
        if ((offs % 6) < 2)
            return -1;
        maxoffs = local_context->prslen + ((local_context->prslen + 3)/4) * 2;
        if (offs > maxoffs) /* 相差太大出错几率增大 */
            return ret;

        refind = ref->ind & 0x7f;
        fmpos = refind * 6 + offs;
        fmpos = fmpos % maxoffs; /* 指向下一轮 */
        ind = fmpos/6;

        ret = ind * 4 + (fmpos % 6) - 2;
    }

    return ret;
}

static int ak_pendcrc_getpos(aklocal_context_t *local_context, akdatf_seq_t *ref, akwire_seq_t ws)
{
    int offs;
    int pos = -1;
    int maxoffs;

    maxoffs = local_context->prslen + ((local_context->prslen + 3)/4) * 2;

    if (ws < ref->ws)
    {//todo
        AK_LOG_D("ws-c overflow(%d, %d)\r\n", ws, ref->ws);
    }
    else
    {
        offs = (ws - ref->ws)/local_context->seqstep;
        if (offs > maxoffs)
            return -1;
        offs = offs + (ref->ind & 0x7f) * 6;
        offs = offs % maxoffs;
        pos = (offs/6) | 0x80;
    }

    return pos;
}

static void ak_dataconflict_add(aklocal_context_t *local_context, uint8_t pos, uint8_t d, int mode)
{
    unsigned i;
    int zi = -1;
    int s, e;

    pos ++;
    if (mode == 0)
    {
        s = 0;
        e = sizeof(local_context->dcfl.pos) - 4;
    }
    else
    {
        s = sizeof(local_context->dcfl.pos) - 4;
        e = sizeof(local_context->dcfl.pos);
    }

    for (i = s; i < e; i ++)
    {
        if ((local_context->dcfl.pos[i] == pos) && (local_context->dcfl.data[i] == d))
            return;
        if (local_context->dcfl.pos[i] == 0)
            zi = i;
    }

    if (zi >= 0)
    {
        local_context->dcfl.data[zi] = d;
        local_context->dcfl.pos[zi] = pos;
    }
}

static int ak_dataconflict_getchar(aklocal_context_t *local_context, uint8_t pos, uint8_t *cpos)
{
    int ch = -1;
    uint8_t i;

    if (*cpos >= sizeof(local_context->dcfl.pos))
        return -1;

    pos ++;
    for (i = *cpos; i < sizeof(local_context->dcfl.pos); i ++)
    {
        if (local_context->dcfl.pos[i] == pos)
        {
            ch = local_context->dcfl.data[i];
            i ++;
            break;
        }
    }

    *cpos = i;

    return ch;
}

static void ak_dataconflict_clear(aklocal_context_t *local_context, int pos)
{
    unsigned i;

    if (pos < 0)
    {
        i = sizeof(local_context->dcfl.pos) - 4;
        for (; i < sizeof(local_context->dcfl.pos); i ++)
        {
            local_context->dcfl.pos[i] = 0;
            local_context->dcfl.data[i] = 0;
        }
    }
    else
    {
        pos ++;
        for (i = 0; i < sizeof(local_context->dcfl.pos) - 4; i ++)
        {
            if (local_context->dcfl.pos[i] == pos)
            {
                local_context->dcfl.pos[i] = 0;
                local_context->dcfl.data[i] = 0;
            }
        }
    }
}

static int _dataconflict_crccheck(aklocal_context_t *local_context, akdatf_header_t* pd, uint8_t dpos, char *d, int size)
{
    int ret = 0;
    uint8_t crc;

    crc = airkiss_crc8((uint8_t*)d, size + 1) & 0x7f;
    if (crc == (pd->crc & 0x7f))
    {
        int pos;

        pos = (pd->ind & 0x7f) * 4;
        AKMEMCPY(&local_context->data[pos], &d[1], size);
        AKMEMSET(pd, 0, sizeof(*pd));
        local_context->seq[(uint8_t)d[0]] = 1;
        local_context->reclen += size;
        ak_dataconflict_clear(local_context, dpos);
        ret = 1;

#ifdef AIRKISS_GET_DATA_LOG_ENABLE
        AK_LOG_D("getdata-c(%d, %d)[%s]\r\n", d[0], size, &d[1]);
#endif
    }

    return ret;
}

static int ak_dataconflict_crccheck(aklocal_context_t *local_context, akdatf_header_t* pd, int size)
{
    char d[6] = {0};
    uint8_t spos;
    uint8_t cflpos0 = 0, cflpos1 = 0, cflpos2 = 0, cflpos3 = 0;
    int i;

    d[0] = pd->ind & 0x7f;
    spos = d[0] * 4;

    /* 把所有冲突的数据都校验一遍 */

    for (i = 0; i < size; i ++)
    {
        ak_dataconflict_add(local_context, spos + i, local_context->data[spos + i], 1);
    }

    while (size > 0)
    {
        int ch;

        ch = ak_dataconflict_getchar(local_context, spos + 0, &cflpos0);
        if (ch < 0)
            break;
        d[1] = ch;
        cflpos1 = 0;

        while (size > 1)
        {
            int ch;

            ch = ak_dataconflict_getchar(local_context, spos + 1, &cflpos1);
            if (ch < 0)
                break;
            d[2] = ch;
            cflpos2 = 0;

            while (size > 2)
            {
                int ch;

                ch = ak_dataconflict_getchar(local_context, spos + 2, &cflpos2);
                if (ch < 0)
                    break;
                d[3] = ch;
                cflpos3 = 0;

                while (size > 3)
                {
                    int ch;

                    ch = ak_dataconflict_getchar(local_context, spos + 3, &cflpos3);
                    if (ch < 0)
                        break;
                    d[4] = ch;

                    if (_dataconflict_crccheck(local_context, pd, spos + 3, d, size))
                    {
                        goto _out;
                    }
                }
            }
        }
    }

_out:
    ak_dataconflict_clear(local_context, -1);

    return 0;
}

static int ak_dataconflict_input(aklocal_context_t *local_context, uint8_t ind, uint8_t pos, uint8_t data)
{
    int ret = 0;
    int i;
    int size;
    int spos;
    akdatf_header_t* pd;

    spos = ind * 4;
    size = local_context->prslen - spos;
    if (size > 4)
        size = 4;

    ak_dataconflict_add(local_context, pos, data, 0);

    /* 检查接收是否足够 */
    for (i = 0; i < size; i ++)
    {
        if (local_context->data[spos + i] == 0)
        {
            return 0;
        }
    }

    /* 查找包头 */
    pd = akseq_getpend(local_context, ind | 0x80);
    if (!pd)
        return 0;

    ret = ak_dataconflict_crccheck(local_context, pd, size);

    return ret;
}

static int ak_databody_input(aklocal_context_t *local_context, akdatf_seq_t *ref, akwire_seq_t ws, uint8_t data)
{
    int pos;
    uint8_t ind;
    uint8_t dif;

    /* 与上一个帧相差太大则不接受 */
    dif = (ws - local_context->prews)/local_context->seqstep;
    if (dif > 5)
        return 0;

    pos = ak_penddata_getpos(local_context, ref, ws);
    if (pos < 0)
    {
        return 0;
    }

    ind = pos / 4;
    if (local_context->seq[ind])
    {
        return 0;
    }

#if AIRKISS_LOG_RIO_ENABLE
    AK_LOG_D("ref(%d %X) input(%d) %c\r\n", ref->ws, ref->ind, pos, data);
#endif

    if (local_context->data[pos] == 0)
    {
        local_context->data[pos] = data;
        ak_pendinput_mark(local_context, ind | 0x80);
    }
    else if (local_context->data[pos] != data)
    {
        /* 出现数据冲突 */
        ak_dataconflict_input(local_context, ind, (uint8_t)pos, data);
    }

    return 1;
}

static void akseq_allocpend(aklocal_context_t *local_context, uint8_t crc, uint8_t ind)
{
    akdatf_header_t *found = 0, *idle = 0;
    unsigned i;

    if (local_context->seq[ind & 0x7f])
        return;

    AK_LOG_D("{%X %X}\r\n", crc, ind);
    idle = &local_context->pendseq[0];
    for (i = 0; i < sizeof(local_context->pendseq)/sizeof(local_context->pendseq[0]); i ++)
    {
        akdatf_header_t *p = &local_context->pendseq[i];

        if (p->ind == ind)
        {
            found = p;
            p->crc = crc;
            break;
        }

        if (p->crc == 0)
            idle = p;
    }

    if (found == NULL)
    {
        found = idle;
        found->crc = crc;
        found->ind = ind;
    }
}

static void ak_datahead_input(aklocal_context_t *local_context, akdatf_seq_t *cur, akwire_seq_t ws, uint8_t head)
{
    int seqmax;
    uint8_t dif;

    seqmax = (local_context->prslen/4) | 0x80;

    if (cur->crc != 0)
    {
        dif = (ws - cur->ws)/local_context->seqstep;

        if (head <= seqmax)
        {
            cur->ind = head;
            cur->ws = ws - local_context->seqstep;
            AKLOC_DFSEQ_PREV(local_context) = *cur;

            if (dif < 3)
            {
                /* 暂存包头 */
                akseq_allocpend(local_context, cur->crc, cur->ind);
            }
        }

        if (head > seqmax)
        {
            cur->crc = head;
            cur->ind = 0;
            cur->ws = ws;
        }
    }
    else
    {
        if (head > seqmax) //很大几率是crc
        {
            cur->crc = head;
            cur->ws = ws;
            cur->ind = 0;
        }
        else if (ak_pendcrc_getpos(local_context, cur, ws) == head)
        {
            /* 没收到crc */
            cur->ind = head;
            cur->ws = ws - local_context->seqstep; /* 设置crc的帧序号 */
        }
    }
}

static int ak_datainput_withwireseq(aklocal_context_t *local_context, uint8_t *f, uint16_t len)
{
    akwire_seq_t ws;
    akdatf_seq_t *cur;

    ws = akwseq_make(f + 22);
    cur = &AKLOC_DFSEQ_CUR(local_context);

    if (len & 0x100) /* 输入数据 */
    {
        akdatf_seq_t *ref;

        ref = &AKLOC_DFSEQ_PREV(local_context);

        if ((cur->ind == 0) && (cur->crc != 0))
        {
            int pos;

            /* 如果只收到了crc就根据前一个包推测一个序号 */
            pos = ak_pendcrc_getpos(local_context, ref, ws);
            if (pos > 0) {
                cur->ind = (uint8_t)pos;
                akseq_allocpend(local_context, cur->crc, cur->ind);
            }
        }

        if (cur->ind)
        {
            if (!ak_databody_input(local_context, cur, ws, len)) {
                AKMEMSET(&AKLOC_DFSEQ_CUR(local_context), 0 , sizeof(*cur));
            }

            if (local_context->reclen == local_context->prslen) {
                local_context->state = AKSTATE_CMP;
            }
        }

        AKLOC_DFSEQ_CUR(local_context).crc = 0;/* 标记已收到数据 */
    }
    else
    {
        /* 输入包头 */
        ak_datahead_input(local_context, cur, ws, len);
    }

    local_context->prews = ws;

    return 0;
}

static int ak_waitfor_datafield(aklocal_context_t *local_context, uint8_t *f, uint16_t len)
{
    int ret = AIRKISS_STATUS_CONTINUE;
    akcode_t *ac = AKLOC_CODE2(local_context);
    uint16_t udplen;

    udplen = aklen_udp(local_context, len);
    if (udplen < 0x80) {
        return ret;
    }

#ifdef AIRKISS_LOG_DFDUMP_ENABLE
    if (f) {
        akdata_dump(local_context, f, udplen);
    }
#endif

    if (ak_datainput_onlylength(local_context, ac, udplen))
    {
        ac->pos = 0;

        ak_get_datafield(local_context, ac);

        if (local_context->reclen == local_context->prslen) {
            local_context->state = AKSTATE_CMP;
            goto _out;
        }
    }

    if (f) {
        ak_datainput_withwireseq(local_context, f, udplen);
    }

_out:
    if (local_context->state == AKSTATE_CMP) {
        ret = AIRKISS_STATUS_COMPLETE;
    }

    return ret;
}

static int ak_sa_filter(aklocal_context_t *local_context, uint8_t *f)
{
    int ret = 0;
    akaddr_t sa;

    if (local_context->state != AKSTATE_WFG) {
        akaddr_fromframe(&sa, f);
        ret = AKMEMCMP(&local_context->locked, &sa, sizeof(sa));
    }

    return ret;
}

int airkiss_recv(airkiss_context_t *context, const void *frame, uint16_t length)
{
    int ret = AIRKISS_STATUS_CONTINUE;
    aklocal_context_t *local_context = (aklocal_context_t *)context;
    uint8_t *f = (uint8_t *)frame;

    if (frame != NULL)
    {
        ieee80211_frame_t * ieee_f = (ieee80211_frame_t *)frame;
        #if 0
         if ((ieee_f->i_addr1[0] != 0xff) || (ieee_f->i_addr1[1] != 0xff) || (ieee_f->i_addr1[2] != 0xff) || \
             (ieee_f->i_addr1[3] != 0xff) || (ieee_f->i_addr1[4] != 0xff) || (ieee_f->i_addr1[5] != 0xff)) {
             return ret;
         }
         if ((ieee_f->i_addr3[0] != 0xff) || (ieee_f->i_addr3[1] != 0xff) || (ieee_f->i_addr3[2] != 0xff) || \
             (ieee_f->i_addr3[3] != 0xff) || (ieee_f->i_addr3[4] != 0xff) || (ieee_f->i_addr3[5] != 0xff)) {
             return ret;
         }
        #endif

        if (ak_sa_filter(local_context, f))
            return ret;
    }

    switch (local_context->state)
    {
        case AKSTATE_WFG:
            ret = ak_waitfor_guidefield(local_context, f, length);
            break;
        case AKSTATE_WFM:
            ret = ak_waitfor_magicfield(local_context, length);
            break;
        case AKSTATE_WFP:
            ret = ak_waitfor_prefixfield(local_context, length);
            break;
        case AKSTATE_WFD:
            ret = ak_waitfor_datafield(local_context, f, length);
            break;
        case AKSTATE_CMP:
            ret = AIRKISS_STATUS_COMPLETE;
            break;
    }

    return ret;
}

int airkiss_init(airkiss_context_t *context, const airkiss_config_t *config)
{
    aklocal_context_t *local_context = (aklocal_context_t *)context;


    if (!context || !config || !config->memcpy || !config->memset || !config->memcmp) {
        return -1;
    }

    local_context->cfg = config;

    if (sizeof(aklocal_context_t) > sizeof(airkiss_context_t)) {
        local_context->cfg->printf("sizeof(aklocal_context_t) > sizeof(airkiss_context_t)!!!\r\n");
        return -1;
    }

    akloc_reset(local_context);

    ak_port_init();

    return 0;
}

int airkiss_get_result(airkiss_context_t *context, airkiss_result_t *res)
{
    aklocal_context_t *local_context = (aklocal_context_t *)context;

    if (local_context->state != AKSTATE_CMP)
        return -1;

    res->pwd = (char *)&local_context->data[0];
    res->pwd_length = local_context->pwdlen;

    if (local_context->data[local_context->pwdlen] == 0)
    {
        res->random = local_context->random;
    }
    else
    {
        res->random = local_context->data[local_context->pwdlen];
        local_context->random = local_context->data[local_context->pwdlen];
        local_context->data[local_context->pwdlen] = 0;
    }

    res->ssid = (char *)&local_context->data[local_context->pwdlen + 1];
    res->ssid_length = local_context->prslen - local_context->pwdlen - 1;

    local_context->data[local_context->prslen] = 0;

    return 0;
}

int airkiss_change_channel(airkiss_context_t *context)
{
    aklocal_context_t *local_context = (aklocal_context_t *)context;

    akloc_reset(local_context);

    return 0;
}
